#ifndef MPROCESS_H
#define MPROCESS_H

#include <vector>
#include <string>
#include <map>
#include "Message.h"

using namespace std;

/**
 * MProcesses are the MASS threads executing on various machines.  They are 
 * responsible for maintaining some number of the total Places being used by
 * the entire MASS program, as well as the associated Agents.  Each MProcess
 * is referred to by its rank.
 * @param hostName
 * @param myPid
 * @param nProc
 * @param nThr
 * @param port
 */
class MProcess {
 public:
  MProcess( char *hostName, int myPid, int nProc, int nThr, int port );
  void start( );
 private:
  string *hostName; // my local host name 
  int myPid;        // my pid or rank
  int nProc;        // # processes
  int nThr;         // # threads
  int port;         // TCP port
  int sd;           // bare TCP socket to the master
  vector<string> hosts;  // all hosts participated in computation

  void sendAck( );
  void sendAck( int localPopulation );
  void sendReturnValues( void *argument, int nPlaces, int return_size );
  void sendReturnValues( void *argument, int nAgents, int return_size, 
			 int localPopulation );
  void sendMessage( Message *msg );
  Message *receiveMessage( );
};

#endif
