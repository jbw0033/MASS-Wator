#ifndef AGENTS_BASE_H
#define AGENTS_BASE_H

#include <map>
#include <string>
#include <stdlib.h>
#include "Agent.h"

// AgentId = 0..4294967295 i.e. 42.9 milion
#define MAX_AGENTS_PER_NODE 100000000 // 100 million

using namespace std;

/**
 * 
 */
class Agents_base {
  friend class MProcess;
 public:
  Agents_base( int handle, string className, void *argument,
	       int argument_size, int placesHandle, int initPopulation );
  ~Agents_base( );
  
  void callAll( int functionId, void *argument, int tid );
  void callAll( int functionId, void *argument, int arg_size,
		int ret_size, int tid );
  void manageAll( int tid );
  int nLocalAgents( ) { return localPopulation; };

 void getGlobalAgentArrayIndex( vector<int> src_index,
				    int dst_size[], int dst_dimension,
				    int dest_index[] );


 protected:
  const int handle;
  const string className;
  const int placesHandle;

  int initPopulation;
  int localPopulation;
  unsigned int currentAgentId; 

  static void *processAgentMigrationRequest( void *param );

  static void *sendMessageByChild( void *param );
  struct MigrationSendMessage {
    int rank;
    void *message;
  };
};

#endif
