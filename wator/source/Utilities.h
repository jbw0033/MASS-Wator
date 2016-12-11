#ifndef UTILITIES_H
#define UTILITIES_H

#include "libssh2_config.h"
#include <libssh2.h>
#include <iostream>         // cerr

#include "Socket.h"         // Socket
#include "Ssh2Connection.h"

using namespace std;

/**
 * Utilities for the MASS library.
 * Includes use of sockets and SSH2
 * Mostly focuses on providing SSH2 connections between MProcesses.
 */
class Utilities {
 public:
  Utilities( ) : keyfile1( "~/.ssh/id_rsa.pub" ),  keyfile2( "~/.ssh/id_rsa" )
    { };
  Ssh2Connection *establishConnection( const char host[], 
				       const int port,
				       const char username[], 
				       const char password[] );
  bool launchRemoteProcess( const Ssh2Connection *ssh2connection, 
			    const char cmd[] );
  void shutdown( const Ssh2Connection *ssh2connection, const char msg[] );

 private:
  const char *keyfile1;
  const char *keyfile2;

  int waitsocket( int socket_fd, LIBSSH2_SESSION *session );
  void shutdown( Socket *socket, LIBSSH2_SESSION *session,
		 LIBSSH2_CHANNEL *channel, const char msg[] );
};

#endif
