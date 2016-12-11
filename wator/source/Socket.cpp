#include "Socket.h"
#include "MASS_base.h" // MASS_base::log()
#include <sstream>     // ostringstream

/**
 * Open a new socket on a given port.
 * @param port
 */
Socket::Socket( const int port )
  : port( port ), clientFd( NULL_FD ), serverFd( NULL_FD ) {
}

/**
 * Close the socket.
 */
Socket::~Socket( ) {
  ostringstream convert;
  convert << "~Socket called: clientFd = " << clientFd
	  << " serverFd = " << serverFd;
  MASS_base::log( convert.str( ) );

  if ( clientFd != NULL_FD )
    close( clientFd );
  if ( serverFd != NULL_FD )
    close( serverFd );
}

/**
 * Get the file descriptor
 * @return clientFd or serverFd or NULL_FD
 */
int Socket::getDescriptor( ) {
  //ostringstream convert;
  //convert << "getDescriptor: clientFd = " << clientFd 
  //	  << " serverFd = " << serverFd;
  //MASS_base::log( convert.str( ) );
  if ( clientFd != NULL_FD )
    return clientFd;
  if ( serverFd != NULL_FD )
    return serverFd;
  return NULL_FD;
}

/**
 * Get the client socket
 * @param ipName
 * @return clientFd
 */
int Socket::getClientSocket( const char ipName[] ) {
  // Get the host entry corresponding to ipName
  struct hostent* host = gethostbyname( ipName );
  if( host == NULL ) {
    perror ( "Cannot find hostname." );
    return NULL_FD;
  }
      
  // Fill in the structure "sendSockAddr" with the address of the server.
  sockaddr_in sendSockAddr;
  bzero( (char*)&sendSockAddr, sizeof( sendSockAddr ) );
  sendSockAddr.sin_family      = AF_INET; // Address Family Internet
  sendSockAddr.sin_addr.s_addr =
    inet_addr( inet_ntoa( *(struct in_addr*)*host->h_addr_list ) );
  sendSockAddr.sin_port        = htons( port );

  // Open a TCP socket (an Internet strem socket).
  if( ( clientFd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
    perror( "Cannot open a client TCP socket." );
    return NULL_FD;
  }

  // Set the nodely option
  const int on = 1;
  if ( setsockopt( clientFd, IPPROTO_TCP, TCP_NODELAY,
		   ( char * )&on, sizeof( int ) ) < 0 ) {
    perror( "setsockopt TCP_NODELAY failed" );
    return NULL_FD;
  }
    
  // Connect to the server.
  while ( connect( clientFd, (sockaddr*)&sendSockAddr, 
		   sizeof( sendSockAddr ) ) < 0 );

  // Connected
  /*
  ostringstream convert;
  convert << "getClientSocket: clientFd = " << clientFd;
  MASS_base::log( convert.str( ) );
  */

  return clientFd;
}

/**
 * Return a server socket
 * @return newFd
 */
int Socket::getServerSocket( ) {
  if ( serverFd == NULL_FD ) { // Server not ready
    sockaddr_in acceptSockAddr;

    // Open a TCP socket (an internet stream socket).
    if( ( serverFd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
      perror( "Cannot open a server TCP socket." );
      return NULL_FD;
    }

    // Set the resuseaddr option
    const int on = 1;
    if ( setsockopt( serverFd, SOL_SOCKET, SO_REUSEADDR,
                     ( char * )&on, sizeof( on ) ) < 0 ) {
      perror( "setsockopt SO_REUSEADDR failed" );
      return NULL_FD;
    }

    if ( setsockopt( serverFd, IPPROTO_TCP, TCP_NODELAY,
		     ( char * )&on, sizeof( on ) ) < 0 ) {
      perror( "setsockopt TCP_NODELAY failed" );
      return NULL_FD;
    }
    
    // Bind our local address so that the client can send to us
    bzero( (char*)&acceptSockAddr, sizeof( acceptSockAddr ) );
    acceptSockAddr.sin_family      = AF_INET; // Address Family Internet
    acceptSockAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    acceptSockAddr.sin_port        = htons( port );
    
    if( bind( serverFd, (sockaddr*)&acceptSockAddr,
	      sizeof( acceptSockAddr ) ) < 0 ) {
      perror( "Cannot bind the local address to the server socket." );
      return NULL_FD;
    }
    
    listen( serverFd, 5 );
  }
  
  // Read to accept new requests
  int newFd = NULL_FD;
  sockaddr_in newSockAddr;
  socklen_t newSockAddrSize = sizeof( newSockAddr );
  
  if( ( newFd =
	accept( serverFd, (sockaddr*)&newSockAddr, &newSockAddrSize ) ) < 0 ) {
    perror( "Cannot accept from another host." );
    return NULL_FD;
  }

  /*
  ostringstream convert;
  convert << "getServerSocket: serverFd = " << newFd;
  MASS_base::log( convert.str( ) );
  */

  return newFd;
}
