#ifndef AGENTMIGRATIONREQUEST_H
#define AGENTMIGRATIONREQUEST_H

#include "Agent.h"

/**
 * 
 * @param destIndex
 * @param agent
 */
class AgentMigrationRequest {
 friend class Agents_base;
 friend class Message;
 public:
  AgentMigrationRequest( int destIndex, Agent *agent ) :
    destGlobalLinearIndex( destIndex ), agent( agent ) {
  };

  ~AgentMigrationRequest( ) {
    /* DON'T DELETE AN AGENT!!
    if ( agent->migratableDataSize > 0 && agent->migratableData != NULL )
      free( agent->migratableData );
    delete agent;
    */
  }

  int destGlobalLinearIndex;
  Agent *agent;
};

#endif
