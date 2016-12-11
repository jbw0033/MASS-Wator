#ifndef SSH2CONNECTION_H
#define SSH2CONNECTION_H

#include <libssh2.h>
#include "Socket.h"

class Ssh2Connection {
 public:
  Ssh2Connection( Socket *socket, 
		  LIBSSH2_SESSION *session, LIBSSH2_CHANNEL *channel ) :
    socket( socket ), session( session ), channel( channel ) { };
  int read( char* buf, int size );
  int write( char *buf, int size );

  Socket *socket;
  LIBSSH2_SESSION *session;
  LIBSSH2_CHANNEL *channel;
};

#endif
