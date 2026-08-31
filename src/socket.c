/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/net.h"
#include "ofc/socket.h"
#include "ofc/libc.h"
#include "ofc/impl/socketimpl.h"
#include "ofc/process.h"

#include "ofc/heap.h"

#define OFC_SCOPE_ALGORITHM

/*
* Forward Delcarations
*/
typedef struct {
    OFC_HANDLE impl;
    OFC_BOOL connected;
    OFC_SOCKET_TYPE type;
    OFC_IPADDR ip;        /* For input stream sockets */
    OFC_UINT16 port;        /* ... */
    OFC_SIZET write_count;
    OFC_INT write_offset;
    OFC_HANDLE send_queue;
} OFC_SOCKET;

static OFC_SOCKET *ofc_socket_alloc(OFC_VOID);

static OFC_VOID ofc_socket_free(OFC_SOCKET *sock);

/*
 * SOCKET_create - Create a socket
 *
 * Accepts:
 *    Nothing
 *
 * Returns:
 *    socket
 *
 * This is an internal routine to create a socket structure
 */
static OFC_SOCKET *
ofc_socket_alloc(OFC_VOID) {
    OFC_SOCKET *sock;

    sock = ofc_malloc(sizeof(OFC_SOCKET));
    sock->impl = OFC_HANDLE_NULL;
    sock->type = SOCKET_TYPE_NONE;

    return (sock);
}

static OFC_VOID
ofc_socket_free(OFC_SOCKET *sock) {
    ofc_free(sock);
}

/*
 * SOCKET_destroy - destroy a socket
 *
 * Accpets:
 *    The socket to destroy
 *
 * Returns:
 *    nothing
 */
OFC_CORE_LIB OFC_VOID
ofc_socket_destroy(OFC_HANDLE hSock) {
    OFC_SOCKET *sock;

    sock = ofc_handle_lock(hSock);
    if (sock != OFC_NULL) {
        ofc_socket_impl_close(sock->impl);

        ofc_socket_impl_destroy(sock->impl);
        ofc_socket_free(sock);
        ofc_handle_destroy(hSock);
        ofc_handle_unlock(hSock);
    }
}

static OFC_INT ofc_ipv6_get_scope(const OFC_IPADDR *ip) {
    OFC_INT scope;

    if (OFC_IN6_IS_ADDR_LOOPBACK (ip->u.ipv6._s6_addr))
        scope = 0x01;
    else if ((OFC_IN6_IS_ADDR_LINKLOCAL (ip->u.ipv6._s6_addr)) ||
             (OFC_IN6_IS_ADDR_MC_LINKLOCAL (ip->u.ipv6._s6_addr)))
        scope = 0x02;
    else if ((OFC_IN6_IS_ADDR_SITELOCAL (ip->u.ipv6._s6_addr)) ||
             (OFC_IN6_IS_ADDR_MC_SITELOCAL (ip->u.ipv6._s6_addr)))
        scope = 0x05;
    else
        scope = 0x0e;

    return (scope);
}

/**
 * Pick a source address for outgoing traffic to [dest].
 *
 * Primary path (kernel-delegated): open a scratch UDP socket, connect it
 * to [dest], and read the source address the kernel picked via
 * get_addresses. UDP connect() does not transmit a packet — it just
 * triggers source-address selection against the current routing table
 * and interface state. This is the standard pattern on hosts with a
 * full IP stack (Linux, Windows, macOS, Android): the kernel picks the
 * best interface for the destination and RFC 6724-selects a source on
 * that interface. Because the kernel has real-time visibility into
 * which interfaces are up and which routes exist, this correctly
 * handles multi-homed hosts even when one interface goes down at
 * runtime (which is what the earlier scope-only algorithm did NOT
 * handle — reported by HP with a two-interface host getting
 * ENETUNREACH after the primary interface went down even though the
 * secondary retained a route to the target).
 *
 * Fallback path (scope-based): if the scratch UDP dance fails for any
 * reason — the stack doesn't support connected UDP, doesn't perform
 * source selection at connect() time, or the destination is
 * genuinely unreachable — fall through to the legacy scope-based
 * algorithm. This is a simplified variant of RFC 6724 that applies
 * scope rules without consulting a routing table; it was OpenFiles'
 * original algorithm and remains the correct pattern on embedded
 * stacks that lack a general routing table.
 */
OFC_CORE_LIB OFC_VOID
ofc_socket_source_address(const OFC_IPADDR *dest,
                          OFC_IPADDR *source) {
    OFC_HANDLE scratch;
    OFC_SOCKADDR local_sa;
    OFC_SOCKADDR remote_sa;
    OFC_BOOL kernel_picked;
    OFC_INT count;
    OFC_INT i;
    OFC_IPADDR ifip;
    OFC_IPADDR ifmask;

    /*
     * Initialize source to the family-appropriate any-address so a
     * fallback that doesn't populate source leaves it in a safe state.
     */
    source->ip_version = dest->ip_version;
    if (dest->ip_version == OFC_FAMILY_IP)
        source->u.ipv4.addr = OFC_INADDR_ANY;
    else
        source->u.ipv6 = ofc_in6addr_any;

    /*
     * Primary path: ask the stack via a scratch UDP socket. Arbitrary
     * port; UDP connect doesn't need it to be reachable — we're only
     * triggering source selection, not exchanging traffic.
     */
    kernel_picked = OFC_FALSE;
    scratch = ofc_socket_impl_create(dest->ip_version, SOCKET_TYPE_DGRAM);
    if (scratch != OFC_HANDLE_NULL) {
        if (ofc_socket_impl_connect(scratch, dest, 53)) {
            if (ofc_socket_impl_get_addresses(scratch,
                                              &local_sa, &remote_sa)) {
                *source = local_sa.sin_addr;
                kernel_picked = OFC_TRUE;
            }
        }
        ofc_socket_impl_destroy(scratch);
    }
    if (kernel_picked)
        return;

    /*
     * Fallback: legacy scope-based algorithm. Kept intact from the
     * pre-scratch-UDP implementation.
     */
    count = ofc_net_interface_count();
    if (dest->ip_version == OFC_FAMILY_IP) {
        /* try to match by subnet */
        for (i = 0; i < count && source->u.ipv4.addr == OFC_INADDR_ANY; i++) {
            ofc_net_interface_addr(i, &ifip, OFC_NULL, &ifmask);
            if (ifip.ip_version == OFC_FAMILY_IP) {
                if ((ifip.u.ipv4.addr & ifmask.u.ipv4.addr) ==
                    (dest->u.ipv4.addr & ifmask.u.ipv4.addr))
                    *source = ifip;
            }
        }
    } else {
#if defined(OFC_SOCKET_SCOPE_ALGORITHM)
        for (i = 0 ; i < count ; i++)
      {
        ofc_net_interface_addr (i, &ifip, OFC_NULL, OFC_NULL) ;
        /*
         * Rule 1, prefer same address
         */
        if (OFC_IN6_ARE_ADDR_EQUAL (ifip.u.ipv6.s6_addr,
                         dest->u.ipv6.s6_addr))
          *source = ifip ;
        /*
         * Rule 2, Prefer scope
         */
        else if (ofc_ipv6_get_scope (&ifip) <= ofc_ipv6_get_scope (source))
          {
            if (ofc_ipv6_get_scope (&ifip) >=  ofc_ipv6_get_scope (dest))
          *source = ifip ;
          }
        else
          {
            if (ofc_ipv6_get_scope (source) <= ofc_ipv6_get_scope (dest))
          *source = ifip ;
          }
      }
#else
        OFC_INT dscope;
        OFC_INT ifscope;
        OFC_INT sscope;

        dscope = ofc_ipv6_get_scope(dest);
        sscope = ofc_ipv6_get_scope(source);
        for (i = 0; i < count; i++) {
            ofc_net_interface_addr(i, &ifip, OFC_NULL, OFC_NULL);
            if (ifip.ip_version == OFC_FAMILY_IPV6) {
                ifscope = ofc_ipv6_get_scope(&ifip);
                if (dscope <= ifscope && ifscope <= sscope) {
                    *source = ifip;
                    sscope = ifscope;
                }
            }
        }
#endif
    }
}

/*
 * SOCKET_connect - Connect to a remote socket
 *
 * Accepts:
 *    ip to connect to
 *    port to connect to
 *
 * Returns:
 *    socket structure for connection
 */
OFC_CORE_LIB OFC_HANDLE
ofc_socket_connect(const OFC_IPADDR *ip, OFC_UINT16 port) {
    OFC_HANDLE hSocket;
    OFC_SOCKET *pSock;
    OFC_IPADDR myinaddr;
    OFC_IPADDR dip;

    /*
     * Create a socket
     */
    hSocket = OFC_HANDLE_NULL;
    pSock = ofc_socket_alloc();
    pSock->type = SOCKET_TYPE_STREAM;

    dip = *ip;
    /*
     * Yes, open a stream
     */
    pSock->impl = ofc_socket_impl_create(dip.ip_version, SOCKET_TYPE_STREAM);
    if (pSock->impl != OFC_HANDLE_NULL) {
        ofc_socket_impl_reuse_addr(pSock->impl, OFC_TRUE);

        ofc_socket_source_address(&dip, &myinaddr);
        if (ofc_socket_impl_bind(pSock->impl, &myinaddr, 0)) {
            ofc_socket_impl_no_block(pSock->impl, OFC_TRUE);
            if (dip.ip_version == OFC_FAMILY_IPV6)
                dip.u.ipv6.scope = myinaddr.u.ipv6.scope;

            if (ofc_socket_impl_connect(pSock->impl, &dip, port)) {
                hSocket = ofc_handle_create(OFC_HANDLE_SOCKET, pSock);
            }
        }

        if (hSocket == OFC_HANDLE_NULL)
            ofc_socket_impl_destroy(pSock->impl);
    }

    if (hSocket == OFC_HANDLE_NULL)
        ofc_socket_free(pSock);

    return (hSocket);
}

OFC_CORE_LIB OFC_BOOL
ofc_socket_connected(OFC_HANDLE hSocket) {
    OFC_SOCKET *pSocket;
    OFC_BOOL connected;

    connected = OFC_FALSE;
    pSocket = ofc_handle_lock(hSocket);
    if (pSocket != OFC_NULL) {
        connected = ofc_socket_impl_connected(pSocket->impl);
        ofc_handle_unlock(hSocket);
    }
    return (connected);
}

/*
 * SOCK_listen - Open up a socket to listen on
 *
 * Accepts:
 *    interface to accept listens on
 *    port to accept listens on
 *
 * Returns:
 *    listening socket
 */
OFC_CORE_LIB OFC_HANDLE
ofc_socket_listen(const OFC_IPADDR *ip, OFC_UINT16 port) {
    OFC_HANDLE hSocket;
    OFC_SOCKET *pSock;

    /*
     * Create a socket
     */
    hSocket = OFC_HANDLE_NULL;
    pSock = ofc_socket_alloc();
    pSock->type = SOCKET_TYPE_STREAM;

    pSock->impl = ofc_socket_impl_create(ip->ip_version, SOCKET_TYPE_STREAM);
    if (pSock->impl != OFC_HANDLE_NULL) {
        ofc_socket_impl_reuse_addr(pSock->impl, OFC_TRUE);
        if (ofc_socket_impl_bind(pSock->impl, ip, port)) {
            ofc_socket_impl_no_block(pSock->impl, OFC_TRUE);
            if (ofc_socket_impl_listen(pSock->impl, 5)) {
                hSocket = ofc_handle_create(OFC_HANDLE_SOCKET, pSock);
            }
        }

        if (hSocket == OFC_HANDLE_NULL)
            ofc_socket_impl_destroy(pSock->impl);
    }

    if (hSocket == OFC_HANDLE_NULL)
        ofc_socket_free(pSock);

    return (hSocket);
}


/*
* SOCK_datagram - Open up a udp socket to listen on
*
* Accepts:
*    interface to accept datagrams on
*    port to accept datagrams on
*
* Returns:
*    datagram socket
*/
OFC_CORE_LIB OFC_HANDLE
ofc_socket_datagram(const OFC_IPADDR *ip, OFC_UINT16 port) {
    OFC_SOCKET *sock;
    OFC_HANDLE hSocket;
    OFC_BOOL status;

    /*
     * Create the socket
     */
    sock = ofc_socket_alloc();
    sock->type = SOCKET_TYPE_DGRAM;
    /*
     * Yes, open up a datagram socket
     */
    hSocket = OFC_HANDLE_NULL;
    sock->impl = ofc_socket_impl_create(ip->ip_version, SOCKET_TYPE_DGRAM);
    /*
     * Is it open?
     */
    if (sock->impl != OFC_HANDLE_NULL) {
        /*
         * Yes, bind to the ip and port we want to accept datagrams on
         */
        ofc_socket_impl_reuse_addr(sock->impl, OFC_TRUE);
        status = ofc_socket_impl_bind(sock->impl, ip, port);
        if (status == OFC_TRUE) {
            ofc_socket_impl_no_block(sock->impl, OFC_TRUE);
            hSocket = ofc_handle_create(OFC_HANDLE_SOCKET, sock);
        } else
            ofc_socket_impl_destroy(sock->impl);
    }

    if (hSocket == OFC_HANDLE_NULL)
        ofc_socket_free(sock);

    return (hSocket);
}


/*
* SOCK_datagram - Open up a udp socket to listen on
*
* Accepts:
*    interface to accept datagrams on
*    port to accept datagrams on
*
* Returns:
*    datagram socket
*/
OFC_CORE_LIB OFC_HANDLE
ofc_socket_raw(const OFC_IPADDR *ip, OFC_SOCKET_TYPE socktype) {
    OFC_SOCKET *sock;
    OFC_HANDLE hSocket;

    /*
     * Create the socket
     */
    hSocket = OFC_HANDLE_NULL;

    sock = ofc_socket_alloc();
    sock->type = socktype;
    /*
     * Yes, open up a datagram socket
     */
    hSocket = OFC_HANDLE_NULL;
    sock->impl = ofc_socket_impl_create(ip->ip_version, socktype);
    /*
     * Is it open?
     */
    if (sock->impl != OFC_HANDLE_NULL) {
        ofc_socket_impl_no_block(sock->impl, OFC_TRUE);
        hSocket = ofc_handle_create(OFC_HANDLE_SOCKET, sock);
    } else
        ofc_socket_free(sock);

    return (hSocket);
}

/*
 * SOCKET_accept - Accept an incoming connection on a listening socket
 *
 * Accepts:
 *    master socket to accept listen from
 *
 * Returns:
 *    socket structure
 */
OFC_CORE_LIB OFC_HANDLE
ofc_socket_accept(OFC_HANDLE hMasterSocket) {
    OFC_SOCKET *socket;
    OFC_SOCKET *master_socket;
    OFC_HANDLE hSocket;

    hSocket = OFC_HANDLE_NULL;
    master_socket = ofc_handle_lock(hMasterSocket);
    if (master_socket != OFC_NULL) {
        /*
         * Create a new socket
         */
        socket = ofc_socket_alloc();
        socket->type = SOCKET_TYPE_STREAM;

        socket->impl = ofc_socket_impl_accept(master_socket->impl,
                                              &socket->ip, &socket->port);

        if (socket->impl == OFC_HANDLE_NULL) {
            ofc_socket_free(socket);
        } else {
          OFC_BOOL ret;

          ret = ofc_socket_impl_no_block(socket->impl, OFC_TRUE);
          ofc_assert (ret == OFC_TRUE, "Could not set socket to non blocking");
          hSocket = ofc_handle_create(OFC_HANDLE_SOCKET, socket);
          if (hSocket == OFC_HANDLE_NULL)
            ofc_socket_free(socket);
        }
        ofc_handle_unlock(hMasterSocket);
    }

    return (hSocket);
}


/*
 * SOCKET_write - Write data to a stream socket
 *
 * Accepts:
 *    socket to write to
 *    buffer to write
 *    where in buffer to start writing from
 *    Number of bytes to write from buffer
 *
 * Returns:
 *   True if we wrote something, false otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_socket_write(OFC_HANDLE hSocket, OFC_MESSAGE *msg)
{
  OFC_SIZET nbytes;
  OFC_SOCKET *socket;
  OFC_BOOL progress;
  OFC_SIZET count;
  OFC_SIZET len;
  OFC_VOID *ptr;

  socket = ofc_handle_lock(hSocket);
  nbytes = 0;

  progress = OFC_FALSE;
  if (socket != OFC_NULL)
    {
      /*
       * Do we have bytes to send?
       */
      len = 0;
      if (msg->count > 0)
        {
          OFC_IOVEC *iovec;
          OFC_INT veclen;
          /*
           * Yes, so try to send it
           */
          ofc_iovec_get(msg->map, msg->offset, msg->offset + msg->count,
                        &iovec, &veclen);
          ptr = iovec[0].iov_base;
          count = OFC_MIN(msg->count, iovec[0].iov_len);
          ofc_free(iovec);

          if (socket->type == SOCKET_TYPE_STREAM)
            {
              len = ofc_socket_impl_send(socket->impl, ptr, count);
            }
          else if (socket->type == SOCKET_TYPE_DGRAM ||
                   socket->type == SOCKET_TYPE_ICMP)
            {
              len = ofc_socket_impl_sendto(socket->impl,
                                           ptr, count,
                                           &msg->ip,
                                           msg->port);
            }

          if ((len < 0) ||
              ((socket->type == SOCKET_TYPE_DGRAM) && (len == 0)))
            {
              OFC_VOID *handle;
              handle = ofc_handle_lock(socket->impl);
              ofc_log(OFC_LOG_WARN,
		      "Socket Error on write, type %d, Impl 0x%08x, handle 0x%08x, count %d\n",
		      socket->type, socket->impl, handle, msg->count);
              /*
               * Force message to retire
               */
              msg->count = 0;
              ofc_handle_unlock(socket->impl);
            }
          else
            {
              progress = OFC_TRUE;
              msg->count -= len;
              if (msg->count < 0)
                ofc_process_crash("here\n");
              msg->offset += len;
              nbytes += len;
            }
        }

        ofc_handle_unlock(hSocket);
    }
    return (progress);
}

/*
 * SOCKET_read - Read data from a socket
 *
 * Accepts:
 *    socket to read from
 *    buffer to read into
 *    offset within buffer to read to
 *    number of bytes to read
 *
 */
OFC_CORE_LIB OFC_BOOL
ofc_socket_read(OFC_HANDLE hSocket, OFC_MESSAGE *msg) {
    OFC_SOCKET *socket;
    OFC_SIZET len;
    OFC_BOOL progress;

    socket = ofc_handle_lock(hSocket);
    progress = OFC_FALSE;
    if (socket != OFC_NULL) {
        len = 0;
        if (msg->count > 0)
          {
            /*
             * Yes, so try to receive it.
             */
            OFC_UCHAR *ptr;
            ptr = ofc_iovec_lookup(msg->map, msg->offset, msg->count);
            if (socket->type == SOCKET_TYPE_STREAM)
              {
                len = ofc_socket_impl_recv(socket->impl, ptr,
                                           msg->count);
                msg->ip = socket->ip;
                msg->port = socket->port;
            } else if (socket->type == SOCKET_TYPE_DGRAM ||
                       socket->type == SOCKET_TYPE_ICMP) {
                len = ofc_socket_impl_recv_from(socket->impl,
                                                ptr,
                                                msg->count,
                                                &msg->ip, &msg->port);
            }
        }

        if (len > 0) {
            msg->offset += len;
            msg->count -= len;
            if (msg->count < 0)
              ofc_process_crash("here\n");

            progress = OFC_TRUE;
        }

        ofc_handle_unlock(hSocket);
    }
    return (progress);
}

OFC_CORE_LIB OFC_HANDLE
ofc_socket_get_impl(OFC_HANDLE hSocket) {
    OFC_SOCKET *sock;
    OFC_HANDLE impl;

    impl = OFC_HANDLE_NULL;
    sock = ofc_handle_lock(hSocket);
    if (sock != OFC_NULL) {
        impl = sock->impl;
        ofc_handle_unlock(hSocket);
    }
    return (impl);
}

OFC_CORE_LIB OFC_SOCKET_EVENT_TYPE
ofc_socket_test(OFC_HANDLE hSocket) {
    OFC_SOCKET_EVENT_TYPE ret;
    OFC_HANDLE impl;
    OFC_SOCKET *sock;

    ret = OFC_FALSE;
    impl = OFC_HANDLE_NULL;
    sock = ofc_handle_lock(hSocket);
    if (sock != OFC_NULL) {
        ret = ofc_socket_impl_test(sock->impl);
        ofc_handle_unlock(hSocket);
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_socket_enable(OFC_HANDLE hSocket, OFC_SOCKET_EVENT_TYPE type) {
    OFC_BOOL ret;
    OFC_HANDLE impl;
    OFC_SOCKET *sock;

    ret = OFC_FALSE;
    impl = OFC_HANDLE_NULL;
    sock = ofc_handle_lock(hSocket);
    if (sock != OFC_NULL) {
        ret = ofc_socket_impl_enable(sock->impl, type);
        ofc_handle_unlock(hSocket);
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_socket_set_send_size(OFC_HANDLE hSocket, OFC_INT size) {
    OFC_SOCKET *sock;

    sock = ofc_handle_lock(hSocket);
    if (sock != OFC_NULL) {
        ofc_socket_impl_set_send_size(sock->impl, size);
        ofc_handle_unlock(hSocket);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_socket_set_recv_size(OFC_HANDLE hSocket, OFC_INT size) {
    OFC_SOCKET *sock;

    sock = ofc_handle_lock(hSocket);
    if (sock != OFC_NULL) {
        ofc_socket_impl_set_recv_size(sock->impl, size);
        ofc_handle_unlock(hSocket);
    }
}

OFC_CORE_LIB OFC_BOOL
ofc_socket_get_addresses(OFC_HANDLE hSock, OFC_SOCKADDR *local,
                         OFC_SOCKADDR *remote) {
    OFC_SOCKET *sock;
    OFC_BOOL ret;

    ret = OFC_FALSE;
    sock = ofc_handle_lock(hSock);
    if (sock != OFC_NULL) {
        ret = ofc_socket_impl_get_addresses(sock->impl, local, remote);
        ofc_handle_unlock(hSock);
    }
    return (ret);
}
