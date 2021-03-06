/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_SOCKET_IMPL_H__)
#define __OFC_SOCKET_IMPL_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/socket.h"
#include "ofc/net.h"

/**
 * \defgroup socket_impl Platform Dependent Socket Handling
 * \ingroup port
 *
 * This layer is used to interface with the Open Files platform independent
 * socket handling.  This layer should probably only be ported to a target
 * platform if the target has a BSD like socket facility.  If it doesn't,
 * then it is probably more effective to port the upper socket layer to
 * the target platform.
 *
 * These calls provide a similar framework to a BSD socket like layer.  
 * Unfortunately, not all BSD-like socket layers are created equal.  In fact,
 * every implementation seems to have some differences.  This layer in 
 * Open Files is designed to abstract those differences.
 */

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif

/**
 * Create a socket
 *
 * Similar to a simple socket() call
 *
 * \param family
 * The family (IP, NETBIOS) for the socket
 *
 * \param socktype
 * The type (stream, datagram) for the socket
 *
 * \returns
 * a Handle to the socket
 */
OFC_HANDLE
ofc_socket_impl_create(OFC_FAMILY_TYPE family,
                       OFC_SOCKET_TYPE socktype);

/**
 * Bind a socket to an address and port
 *
 * Similar to the bind() call.
 *
 * \param hSocket
 * Handle to the implementation socket
 *
 * \param ip
 * ip address to bind to.  The target sockets layer usually accepts a
 * more general struct sockaddr.  We don't need that generality.  This
 * should be specified in normal platform endianess.
 *
 * \param port
 * The port to bind to.  This should be specified in normal platform
 * endianess.
 *
 * \returns
 * OFC_TRUE if the bind was successful, OFC_FALSE otherwise.
 */
OFC_BOOL
ofc_socket_impl_bind(OFC_HANDLE hSocket, const OFC_IPADDR *ip,
                     OFC_UINT16 port);

/**
 * Close a socket
 *
 * This is similar to a close() call.  It performs an implicit shutdown().
 *
 * \param hSocket
 * Socket to close
 *
 * \returns
 * OFC_TRUE if socket is closed, OFC_FALSE otherwise.
 */
OFC_BOOL
ofc_socket_impl_close(OFC_HANDLE hSocket);

/**
 * Connect to a remote
 *
 * Similar to the connect() call.
 *
 * \param hSocket
 * Socket to connect
 *
 * \param ip
 * Remote ip to connect to.  Should be specified in platform endianess.
 *
 * \param port
 * remote port to connect to.  Should be specified in platform endianess.
 *
 * \returns
 * OFC_TRUE if connect initiated.  OFC_FALSE otherwise
 */
OFC_BOOL
ofc_socket_impl_connect(OFC_HANDLE hSocket,
                        const OFC_IPADDR *ip, OFC_UINT16 port);

/**
 * Listen for an incoming connection
 *
 * Similar to the listen() call.
 *
 * \param hSocket
 * Socket to listen on
 *
 * \param backlog
 * Number of simulatenous connections to allow
 *
 * \returns
 * OFC_TRUE if socket is listening, OFC_FALSE otherwise.
 */
OFC_BOOL
ofc_socket_impl_listen(OFC_HANDLE hSocket, OFC_INT backlog);

/**
 * Accept an incoming connection
 *
 * Similar to an accept() call.
 *
 * \param hSocket
 * Listening socket to accept connection for
 *
 * \param ip
 * Receives the IP address of the connection client.  Specified in platform
 * endianess.
 *
 * \param port
 * Receives the port of the connecting client.  Specified in platform
 * endianness
 *
 * \returns
 * Handle to socket implementation
 */
OFC_HANDLE
ofc_socket_impl_accept(OFC_HANDLE hSocket,
                       OFC_IPADDR *ip, OFC_UINT16 *port);

/**
 * Resuse ip address and port
 *
 * similar to a setsockopt() with SO_REUSEADDR.
 *
 * \param hSocket
 * The socket to allow/disallow reuse on
 *
 * \param onoff
 * Whether to allow or disallow reuse
 *
 * \returns
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_BOOL
ofc_socket_impl_reuse_addr(OFC_HANDLE hSocket, OFC_BOOL onoff);

/**
 * Set a socket as non blocking
 *
 * Similar to an ioctl() with FIONBIO
 *
 * \param hSocket
 * Socket to set nonblocking or blocking
 *
 * \param onoff
 * Whether to set to nonblocking (OFC_TRUE) or blocking (OFC_FALSE)
 *
 * \returns
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_BOOL
ofc_socket_impl_no_block(OFC_HANDLE hSocket, OFC_BOOL onoff);

/**
 * Test if a socket is connected.
 *
 * This is platform specific and can be implemented using the most
 * appropriate method for that platform.  A reliable method on many platforms
 * is a call to getpeername().  If this is successful, then the socket is
 * connected.
 *
 * \param hSocket
 * The socket to test if it's connected.
 *
 * \returns
 * OFC_TRUE if connected, OFC_FALSE otherwise.
 */
OFC_BOOL
ofc_socket_impl_connected(OFC_HANDLE hSocket);

/**
 * Send data on a socket
 *
 * Similar to a send() or write() call
 *
 * \param hSocket
 * Socket to send data on
 *
 * \param buf
 * buffer containing the data
 *
 * \param len
 * Number of bytes to send.
 *
 * \returns
 * Number of bytes sent, or -1 if error.
 */
OFC_SIZET
ofc_socket_impl_send(OFC_HANDLE hSocket, const OFC_VOID *buf,
                     OFC_SIZET len);

/**
 * Send data on a datagram socket
 *
 * Similar to sendto()
 *
 * \param hSocket
 * Socket to send data on
 *
 * \param buf
 * Buffer containing data
 *
 * \param len
 * Number of bytes to send
 *
 * \param ip
 * ip address to send data to
 *
 * \param port
 * port to send data to
 *
 * \returns
 * Number of bytes sent or -1 if error
 */
OFC_SIZET
ofc_socket_impl_sendto(OFC_HANDLE hSocket, const OFC_VOID *buf,
                       OFC_SIZET len,
                       const OFC_IPADDR *ip,
                       OFC_UINT16 port);

/**
 * Receive data from a socket
 *
 * Similar to recv() or read()
 *
 * \param hSocket
 * Socket to read data from
 *
 * \param buf
 * buffer to read data into
 *
 * \param len
 * maximum number of bytes to read
 *
 * \returns
 * Number of bytes read, or -1 if error.
 */
OFC_SIZET
ofc_socket_impl_recv(OFC_HANDLE hSocket,
                     OFC_VOID *buf,
                     OFC_SIZET len);

/**
 * Receive data from a datagram socket
 *
 * Similar to recvfrom()
 *
 * \param hSocket
 * Socket to receive data from
 *
 * \param buf
 * buffer to receive data into
 *
 * \param len
 * maximum number of bytes to read
 *
 * \param ip
 * pointer to location to store ip that the data was received from
 *
 * \param port
 * pointer to location to store port that the data was received from
 *
 * \returns
 * Number of bytes received, or -1 if error.
 */
OFC_SIZET
ofc_socket_impl_recv_from(OFC_HANDLE hSocket,
                          OFC_VOID *buf,
                          OFC_SIZET len,
                          OFC_IPADDR *ip,
                          OFC_UINT16 *port);

/**
 * Destroy a socket
 *
 * Free up all resources associated with a socket.
 *
 * \param hSocket
 * Socket to destroy
 */
OFC_VOID
ofc_socket_impl_destroy(OFC_HANDLE hSocket);

/**
 * Test for an event on the socket
 *
 * This is implementation specific.  On some platforms it will test
 * the appropriate FD_SET, on others, it will issue calls to the kernel.
 *
 * \param hSocket
 * Socket to test event for
 *
 * \returns
 * Events that are set
 */
OFC_SOCKET_EVENT_TYPE
ofc_socket_impl_test(OFC_HANDLE hSocket);

/**
 * Enable an event on the socket
 *
 * This is implementation specific.  On some platforms it will test
 * the appropriate FD_SET, on others, it will issue calls to the kernel.
 *
 * \param hSocket
 * Socket to enable
 *
 * \param type
 * event mask to set
 *
 * \returns
 * OFC_TRUE if event is set, otherwise OFC_FALSE
 */
OFC_BOOL
ofc_socket_impl_enable(OFC_HANDLE hSocket,
                       OFC_SOCKET_EVENT_TYPE type);

/**
 * Set the sockets receive buffer size
 *
 * Similar to a setsockopt() with SO_RCVBUF
 *
 * \param hSocket
 * Socket to set the receive buffer size
 *
 * \param size
 * size to set the buffer to
 */
OFC_VOID
ofc_socket_impl_set_recv_size(OFC_HANDLE hSocket, OFC_INT size);

/**
 * Set the sockets send buffer size
 *
 * Similar to a setsockopt() with SO_SNDBUF
 *
 * \param hSocket
 * Socket to set the send buffer size
 *
 * \param size
 * size to set the buffer to
 */
OFC_VOID
ofc_socket_impl_set_send_size(OFC_HANDLE hSocket, OFC_INT size);

/**
 * Get the Implementations Event Handle
 *
 * This is a noop for most socket handlers but currently is used by ThreadX
 *
 * \param hSocket
 * The socket to get the event handle for
 *
 * \returns
 * The event handle associated with the socket
 */
OFC_HANDLE
ofc_socket_impl_get_event_handle(OFC_HANDLE hSocket);

/**
 * Get the ip and ports of a tcp connection
 *
 * \param hSock
 * The socket of the connection
 *
 * \param local
 * Pointer to the returned sockaddress structure for the local side
 *
 * \param remote
 * Poitner to the returned socketaddress structure for the remote side
 *
 * \returns
 * OFC_TRUE if success, OFC_FALSE otherwise
 */
OFC_BOOL
ofc_socket_impl_get_addresses(OFC_HANDLE hSock,
                              OFC_SOCKADDR *local,
                              OFC_SOCKADDR *remote);

OFC_INT ofc_socket_impl_get_fd(OFC_HANDLE hSocket);

OFC_UINT16 ofc_socket_impl_get_event(OFC_HANDLE hSocket);

OFC_VOID ofc_socket_impl_set_event(OFC_HANDLE hSocket,
                                   OFC_UINT16 revents);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

