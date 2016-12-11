#ifndef MASS_BASE_H
#define MASS_BASE_H

#include <iostream>  // cerr cout endl
#include <fstream>   // ofstream
#include <string>    // string
#include <map>
#include "Mthread.h" // pthread_t
#include "Places_base.h" 
#include "Agents_base.h"
#include "Message.h"
#include "DllClass.h"
#include "ExchangeHelper.h"
#include "RemoteExchangeRequest.h"
#define CUR_SIZE 256
#define MASS_LOGS "MASS_logs"

using namespace std;

/**
 * 
 */
class MASS_base {
  friend class MProcess;
  friend class Places_base;
  friend class Agents_base;
  friend class Message;
  friend class Place;
 public:
  static void initMASS_base( const char *name, int myPid, int nProc, int port );
  static bool initializeThreads( int nThr );
  static void log( string msg );
  static int getCores( );
  static int getMyPid( ) { return myPid; };

  static Places_base *getCurrentPlaces( ) { return currentPlaces; };
  static Places_base *getDestinationPlaces( ) { return destinationPlaces; };
  static Agents_base *getCurrentAgents( ) { return currentAgents; };

  static int getCurrentFunctionId( ) { return currentFunctionId; };
  static void *getCurrentArgument( ) { return currentArgument; };
  static int getCurrentArgSize( ) { return currentArgSize; };
  static int getCurrentRetSize( ) { return currentRetSize; };
  static Message::ACTION_TYPE getCurrentMsgType( ) { return currentMsgType; };
  static vector<int*> *getCurrentDestinations( ) {return currentDestinations;};
  static vector<pthread_t> threads;       // including main and child threads
  static void setHosts( vector<string> host_args );
  static void showHosts( );

 protected:
  static int MASS_PORT;
  static bool INITIALIZED;
  static char CUR_DIR[CUR_SIZE];
  static string hostName;                 // my local host name
  static int myPid;                       // my pid or rank
  static int systemSize;                  // # processes
  static ofstream logger;                 // logger
  static vector<string> hosts;             // all host names

  static map<int, Places_base*> placesMap; // a collection of Places
  static map<int, Agents_base*> agentsMap; // a collection of Agents
  static map<int, DllClass*> dllMap;       // a collection of DllClasses
  static vector<vector<RemoteExchangeRequest*>* > remoteRequests;
  static vector<vector<AgentMigrationRequest*>* > migrationRequests;
  static int requestCounter;
  static Places_base* currentPlaces;
  static Places_base* destinationPlaces;
  static int currentFunctionId;
  static void *currentArgument;
  static int currentArgSize;
  static int currentRetSize;
  static char *currentReturns;
  static vector<int*> *currentDestinations;
  static Message::ACTION_TYPE currentMsgType;
  static ExchangeHelper exchange;
  static Agents_base* currentAgents;

 private:
  static pthread_mutex_t log_lock;
  static pthread_mutex_t request_lock;
};

#endif
