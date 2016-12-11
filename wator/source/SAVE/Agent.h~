#ifndef AGENT_H
#define AGENT_H

#include "MObject.h"
#include <vector>
#include <iostream>
#include <stdlib.h>
#include "Place.h"
using namespace std;

/**
 * Agent class
 * @param argument
 */
class Agent : MObject {
  friend class Agents_base;
  friend class Message;
  friend class AgentMigrationRequest;
 public:
   
  /**
   * Is the default constructor. A contiguous space of arguments is passed to
   * the constructor
   * @param argument
   */
  Agent( void *argument ) : alive( true ), newChildren( 0 ), 
    migratableData( NULL ), migratableDataSize( 0 ) { };
    
    
  /**
   * Is called from Agents.callAll. It invokes the function specified with
   * functionId as passing arguments to this function. A user-derived Agent
   * class must implement this method.
   * @param functionId
   * @param argument
   * @return 
   */  
  virtual void *callMethod( int functionId, void *argument ) = 0;
  
  /**
   * Destructor
   */
  ~Agent( ) { if ( migratableData != NULL ) free( migratableData ); };
  
 /**
  * Returns the number of agents to initially instantiate on a place indexed
  * with coordinates[]. The maxAgents parameter indicates the number of
  * agents to create over the entire application. The argument size[] defines
  * the size of the "Place" matrix to which a given "Agent" class belongs. The
  * system-provided (thus default) map( ) method distributes agents over
  * places uniformly as in:
  *        maxAgents / size.length
  * The map( ) method may be overloaded by an application-specific method.
  * A user-provided map( ) method may ignore maxAgents when creating
  * agents.
  * @param initPopulation
  * @param size
  * @param index
  * @return 
  */
  int map( int initPopulation, vector<int> size, vector<int> index, 
	   Place* curPlace) {

    // compute the total # places
    int placeTotal = 1;
    for ( int x = 0; x < int( size.size( ) ); x++ ) 
      placeTotal *= size[x];

    // compute the global linear index
    int linearIndex = 0;
    for ( int i = 0; i < int( index.size( ) ); i++ ) {
      if ( index[i] >= 0 && size[i] > 0 && index[i] < size[i] ) {
	linearIndex = linearIndex * size[i];
	linearIndex += index[i];
      }
    }

    // compute #agents per place a.k.a. colonists
    int colonists = initPopulation / placeTotal;
    int remainders = initPopulation % placeTotal;
    if ( linearIndex < remainders ) colonists++; // add a remainder
    
    return colonists;
  };
  
 /**
  * Terminates the calling agent upon a next call to Agents.manageAll( ).
  * More specifically, kill( ) sets the "alive" variable false.
  */
  void kill( ) { alive = false; };

 /**
  * 
  * @return id
  */
  int getDebugData() { 
    int id = agentId;
    return id;
  }

 /**
  * modify the debug data of the agent
  */
  void setDebugData(int argument) {
    // currently empty
  }

 protected:
  bool migrate( int index, ... ) { return true; };
  bool migrate( vector<int> index );
  void spawn( int nAgents, vector<void*> arguments, int arg_size );

  //void spawn( int nAgents, void *arguments , int arg_size );

  /**
   * Maintains this handle of the agents class to which this agent belongs. 
   */
  int agentsHandle;
  
  /**
   * Maintains this handle of the agents class with which this agent is
   * associated.
   */
  int placesHandle;
  
  /** 
   * Is this agent’s identifier. It is calculated as: 
   * the sequence number * the size of this agent’s belonging 
   * matrix + the index of the current place when all
   * places are flattened to a single dimensional array.
   */
  int agentId;
  
  /**
   * Is the identifier of this agent’s parent.
   */
  int parentId;
  
  /**
   * Points to the current place where this agent resides.
   */
  Place *place;
  
  /**
   * Is an array that maintains the coordinates of where this agent resides.
   * Intuitively, index[0], index[1], and index[2] correspond to coordinates of
   * x, y, and z, or those of i, j, and k.
   */
  vector<int> index;
  
  /** 
   * Is true while this agent is active. Once it is set false, this agent is 
   * killed upon a next call to Agents.manageAll( ).
   */
  bool alive;
  
  /**
   * Is the number of new children created by this agent upon a next call to
   * Agents.manageAll( ).
   */
  int newChildren;
  
  /** 
   * Is an array of arguments, each passed to a different new child.
   */
  vector<void*> arguments;
  
  /**
   * Is a pointer to a user-allocated space that will be carried
   * with the agent when it migrates to a different space.
   * To be cast to a user-defined data type.
   */
  void *migratableData;
  
  /** 
   * Indicates the size of the migratebleData space.
   */
  int migratableDataSize;
};

#endif
