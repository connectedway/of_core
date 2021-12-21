/* Copyright (c) 2009 Blue Peach Solutions, Inc. 
 * All rights reserved.
 *
 * This software is protected by copyright and intellectual 
 * property laws as well as international treaties.  It is to be 
 * used and copied only by authorized licensees under the 
 * conditions described in their licenses.  
 *
 * Title to and ownership of the software shall at all times
 * remain with Blue Peach Solutions.
 */
#if !defined(__BLUE_SOCKET_H__)
#define __BLUE_SOCKET_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/net.h"
#include "ofc/message.h"

/** 
 * \defgroup BlueSocket Network Socket Facility
 * \ingroup BlueInternal
 *
 * Provides network socket level abstraction and helper routines
 *
 * There are two levels to the network abstraction.  A 'socket' layer and
 * an 'implementation' layer.  The socket layer is platform independent and
 * is used to perform common management of network sockets.  The implementation
 * layer is platform dependent and interfaces with the platforms socket calls.
 *
 * If a platform has a socket model, then it is recommended that an 
 * implementation layer be created to interface with that socket layer.  If
 * a platform does not use a socket model for communication, it is recommended
 * that the Blue Real 'socket' layer be replaced with a network layer that
 * interfaces directly to the platform network layer.
 */

/** \{ */

/**
* Socket Types Supported by Blue Real.
*/
typedef enum
  {
    /**
     * Unspecified Socket Type (Do not use)
     */
    SOCKET_TYPE_NONE,
    /**
     * Socket is a Stream Type.  Usually indicates TCP.  Bytes are transferred
     * as a stream of bytes.  Any packet delimination is the responsibility
     * of higher layer protocols and typicaly is based on the byte position
     * of the stream.
     */
    SOCKET_TYPE_STREAM,
    /**
     * Socket is a Datgram Type.  Messages are grouped into packets without
     * any guarantee of delivery.  
     */
    SOCKET_TYPE_DGRAM,
    /**
     * Socket is a NetBIOS Session Type
     */
    SOCKET_TYPE_NETBIOS_SESSION,
    /**
     * Socket is an ICMP socket
     */
    SOCKET_TYPE_ICMP,
    /**
     * Number of Socket Types Supported
     */
    SOCKET_TYPE_NUM
  } BLUE_SOCKET_TYPE ;

/**
 * Socket Event Types
 *
 * All open sockets are through an associated event.  When an event fires,
 * it can be for any number of reasons.  Applications may poll the socket's
 * event to determine the specific event type.
 *
 * Not all these event types will be genererated in the current implementation
 */
typedef enum
  {
    /**
     * The socket has been closed
     */
    BLUE_SOCKET_EVENT_CLOSE = 0x01,
    /**
     * An incoming connection request has been received on a socket
     * that has been listening for connections
     */
    BLUE_SOCKET_EVENT_ACCEPT = 0x02,
    /**
     * The underlying IP address has changed for the socket
     */
    BLUE_SOCKET_EVENT_ADDRESSCHANGE = 0x04,
    /**
     * A Quality of Service Event has occurred.  This may be reported
     * by the lower level platform specific network code.
     */
    BLUE_SOCKET_EVENT_QOS = 0x08,
    /**
     * Out of Band data has been received.  
     */
    BLUE_SOCKET_EVENT_QOB = 0x10,
    /**
     * Data has been received and is available
     */
    BLUE_SOCKET_EVENT_READ = 0x20,
    /**
     * Buffer space is available to write data for transmission
     */
    BLUE_SOCKET_EVENT_WRITE = 0x40,
  } BLUE_SOCKET_EVENT_TYPE ;

#if defined(__cplusplus)
extern "C"
{
#endif

  BLUE_CORE_LIB BLUE_VOID 
  BlueSocketSourceAddress (const BLUE_IPADDR *dest, 
			   BLUE_IPADDR *source) ;
  /**
   * Create a Stream socket and connect to a remote target
   *
   * This call will create a TCP connection to the specified port on the
   * remote IP.  All sockets are set as non-blocking.  All parameters should
   * be specified using the platform endianess.
   *
   * \param ip
   * Pointer to the remote IP to connect to.
   *
   * \param port
   * The port on the remote to connect to.
   *
   * \returns
   * Handle to the created socket or BLUE_HANDLE_NULL if creation failed.
   * This call is non-blocking so on return, the connection may not be
   * complete.  When the socket is connected, an event will be set for the 
   * handle and a call to BlueSocketConnected will return success.
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueSocketConnect (const BLUE_IPADDR *ip, BLUE_UINT16 port) ;
  /**
   * Create a TCP socket and listen for incoming connections
   * 
   * This call will listen for incoming connections on the interface
   * specified by the ip address and on the port specified by the port.
   * All arguments are specified in the platform endianess.
   *
   * \param ip
   * The ip for the interface to listen for connections on.  Can be
   * BLUE_INADDR_ANY to listen for connections on any interface.
   *
   * \param port
   * The port to listen for incoming connections on.
   *
   * \returns
   * A handle to the created socket.  If BLUE_HANDLE_NULL, socket creation
   * failed.  The call is non-blocking so it will return immediately.
   * When an incoming connection arrives, a socket event will be generated.
   * A subsequent call to BlueSocketAccept can then be issued.
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueSocketListen (const BLUE_IPADDR *ip, BLUE_UINT16 port) ;
  /**
   * Create a datagram socket
   *
   * This call will create a UDP datagram socket and assign it to the interface
   * of the specified ip address with the specified port.
   *
   * \param ip
   * IP of the interface that the datagram socket should be assigned to. The
   * datagram socket will receive incoming datagrams from this interface.  If
   * the ip is BLUE_INADDR_ANY, then this socket will accept datagrams from
   * any interface.
   *
   * \param port
   * Port to accept incoming datagrams from
   *
   * \returns
   * A handle to the created datagram socket.  If BLUE_HANDLE_NULL is 
   * returned, then the datagram creation failed.  The socket is non-blocking
   * so this will return immediately.  Events will be generated when 
   * traffic arrives or buffer space is available.
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueSocketDatagram (const BLUE_IPADDR *ip, BLUE_UINT16 port) ;
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueSocketRaw (const BLUE_IPADDR *ip, BLUE_SOCKET_TYPE socktype) ;
  /**
   * Accept an incoming TCP connection
   *
   * This call will accept an incoming connection that has been signalled
   * for a 'listening' socket.
   *
   * \param hMasterSocket
   * The handle to the listening socket
   *
   * \returns
   * A handle to the newly accepted connection
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueSocketAccept (BLUE_HANDLE hMasterSocket) ;
  /**
   * Destroy a previously created socket.
   * 
   * The socket can be a datagram or stream socket created through any of
   * the socket creation calls (BlueSocketConnect, BlueSocketListen, 
   * BlueSocketDatagram, BlueSocketAccept).
   *
   * \param hSock
   * The handle to the socket to destroy
   *
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSocketDestroy (BLUE_HANDLE hSock) ;
  /**
   * Write data on a socket
   *
   * This call will write data on a stream or datagram socket.
   *
   * \param hSocket
   * Socket to write the data on.
   *
   * \param msg
   * The message to write.  See BLUE_MESSAGE for a description of the format
   * of this parameter.
   *
   * \returns
   * True if we made progress on a write, false otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueSocketWrite (BLUE_HANDLE hSocket, BLUE_MESSAGE *msg) ;
  /**
   * Read Data from a socket
   * 
   * Read data from a datagram or stream socket.  Any received data will
   * be reflected in the specified message.  See BLUE_MESSAGE for more
   * detail.
   *
   * \param hSocket
   * Socket to read data from
   *
   * \param msg
   * The message to receive data into.
   *
   * \returns
   * True if we made progress on a read, false otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueSocketRead (BLUE_HANDLE hSocket, BLUE_MESSAGE *msg) ;
  /**
   * Test if a socket is connected or not
   *
   * \param hSocket
   * Socket to test
   *
   * \returns
   * BLUE_TRUE if the socket is connected, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueSocketConnected (BLUE_HANDLE hSocket) ;
  /**
   * Test for a particular event on the socket
   *
   * \param hSocket
   * Socket to test
   *
   * \returns
   * bit mask of events set
   */
  BLUE_CORE_LIB BLUE_SOCKET_EVENT_TYPE 
  BlueSocketTest (BLUE_HANDLE hSocket) ;
  /**
   * Enable an event on the socket
   *
   * \param hSocket
   * Socket to enable event (mask) on
   *
   * \param type
   * Event Mask (events ORd together)
   *
   * \returns
   * BLUE_TRUE if successful
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueSocketEnable (BLUE_HANDLE hSocket, BLUE_SOCKET_EVENT_TYPE type)  ;
  /**
   * Set the send buffer size of the lower socket layers
   * 
   * This call normally does not need to be used, but may improve
   * performance.
   *
   * \param hSocket
   * Socket to set the send size for
   * 
   * \param size
   * size to set the send buffer to.
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSocketSetSendSize (BLUE_HANDLE hSocket, BLUE_INT size) ;
  /**
   * Set the receive buffer size of the lower socket layers
   * 
   * This call normally does not need to be used, but may improve
   * performance.
   *
   * \param hSocket
   * Socket to set the receive size for
   * 
   * \param size
   * size to set the receive buffer to.
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSocketSetRecvSize (BLUE_HANDLE hSocket, BLUE_INT size) ;
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
   * BLUE_TRUE if success, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueSocketGetAddresses (BLUE_HANDLE hSock, BLUE_SOCKADDR *local, 
			  BLUE_SOCKADDR *remote) ;

  /**
   * Get a handle to the socket implementation.
   * 
   * This call is only used by the platform handling within Blue Real and
   * should be considered private.  It is needed to determine the platform
   * specific event to wait on in platform select or WaitForObject calls.
   * Any port of this layer will possibly create a different interface
   *
   * \param hSocket
   * Socket to get the implementation details for
   *
   * \returns
   * a handle to the implementation specific layer.  A different interface
   * may return the event to wait for or some other information
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueSocketGetImpl (BLUE_HANDLE hSocket) ;
#if defined(__cplusplus)
}
#endif
#endif

/** \} */
