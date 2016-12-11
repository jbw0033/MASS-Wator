#ifndef SOCKET_H
#define SOCKET_H

#include <iostream>      // cerr

using namespace std;

extern "C"
{
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <string.h>       // bzero
#include <netinet/tcp.h>  // TCP_NODELAY
#include <stdlib.h>
#include <stdio.h>
}

#define NULL_FD -1

class Socket {
 public:
  Socket( const int );
  ~Socket( );
  int getClientSocket( const char[] );
  int getClientSocket( const char[], const int sndbufsize, const 
		       bool nodelay );
  int getServerSocket( );
  int getServerSocket( const int rcvbufsize, const bool nodelay );
  int getDescriptor( );
 private:
  int port;
  int clientFd;
  int serverFd;
};  

#endif  
