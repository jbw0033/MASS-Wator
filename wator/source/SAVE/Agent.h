/*
 MASS C++ Software License
 © 2014-2015 University of Washington
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 The following acknowledgment shall be used where appropriate in publications, presentations, etc.:
 © 2014-2015 University of Washington. MASS was developed by Computing and Software Systems at University of Washington Bothell.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#ifndef AGENT_H
#define AGENT_H

#include "MObject.h"
#include <vector>
#include <iostream>
#include <stdlib.h>
#include "Place.h"
using namespace std;

class Agent : MObject {
  friend class Agents_base;
  friend class Message;
  friend class AgentMigrationRequest;
 public:
  Agent( void *argument ) : alive( true ), newChildren( 0 ), 
    migratableData( NULL ), migratableDataSize( 0 ) { };
  virtual void *callMethod( int functionId, void *argument ) = 0;
  ~Agent( ) { if ( migratableData != NULL ) free( migratableData ); };
  int map( int initPopulation, vector<int> size, vector<int> index ) {
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
  void kill( ) { alive = false; };

 protected:
  bool migrate( int index, ... ) { return true; };
  bool migrate( vector<int> index );
  void spawn( int nAgents, vector<void*> arguments, int arg_size );

  //void spawn( int nAgents, void *arguments , int arg_size );

  int agentsHandle;
  int placesHandle;
  int agentId;
  int parentId;
  Place *place;
  vector<int> index;
  bool alive;
  int newChildren;
  vector<void*> arguments;
  void *migratableData;    // to be cast to a user-defined data type 
  int migratableDataSize;
};

#endif
