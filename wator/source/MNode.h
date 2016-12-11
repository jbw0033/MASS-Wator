#ifndef MNODE_H
#define MNODE_H

#include <string.h>
#include "Ssh2Connection.h"
#include "Message.h"
#include "Socket.h"

class MNode {
 public:
  MNode( string hostName, int pid, Ssh2Connection *ssh2connection, int sd ) : 
    hostName( hostName ),  pid( pid ), connection( ssh2connection ), 
    sd( sd ) { };
  void closeMainConnection( ); 
  void sendMessage( Message *m );  
  Message *receiveMessage( );

  string getHostName( ) { return hostName; };
  int getPid( ) { return pid; };
 private:
  const string hostName;
  const int pid;
  Ssh2Connection *connection;
  const int sd; // a bare TCP socket to each slave
};

#endif
