#include "Agents_base.h"
#include "MASS_base.h"
#include "DllClass.h"
#include <sstream> // ostringstream
#include <vector>

//Used to enable or disable output for Agents
const bool printOutput = false;
//const bool printOutput = true;

/**
 * 
 * @param handle
 * @param className
 * @param argument
 * @param argument_size
 * @param placesHandle
 * @param initPopulation
 */
Agents_base::Agents_base( int handle, string className, void *argument,
			  int argument_size, int placesHandle, 
			  int initPopulation ) 
  : handle( handle ), className( className ), 
    placesHandle( placesHandle ), initPopulation( initPopulation ) {

  // For debugging
  ostringstream convert;
  if(printOutput == true){
      convert << "handle = " << handle
	      << ",placesHandle = " << placesHandle
              << ", class = " << className
              << ", argument_size = " << argument_size
              << ", argument = " << (char *)argument
              << ", initPopulation = " << initPopulation
              << endl;
      MASS_base::log( convert.str( ) );
  }

  // load the construtor and destructor
  DllClass *agentsDllClass = new DllClass( className );
  MASS_base::dllMap.
    insert( map<int, DllClass*>::value_type( handle, agentsDllClass ) );

  // initialize currentAgentId and localPopulation
  currentAgentId = MASS_base::myPid * MAX_AGENTS_PER_NODE;
  localPopulation = 0;

  // instantiate just one agent to call its map( ) function
  Agent *protoAgent = (Agent *)( agentsDllClass->instantiate( argument ) );

  // retrieve the corresponding places
  DllClass *placesDllClass = MASS_base::dllMap[ placesHandle ];
  Places_base *curPlaces = MASS_base::placesMap[ placesHandle ];

  if(printOutput == true){
      convert.str( "" );
      convert << "Agets_base constructor: placesDillClass = " 
	      << (void *)placesDllClass
	      << " curPlaces = " << (void *)curPlaces;
      MASS_base::log( convert.str( ) );
  }
  for ( int i = 0; i < curPlaces->getPlacesSize( ); i++ ) {

    // scan each place to see how many agents it can create
    Place *curPlace = placesDllClass->places[i];

    if(printOutput == true){
        convert.str( "" );
        convert << "Agent_base constructor place[" << i << "]";
        MASS_base::log( convert.str( ) );
    }
    if (printOutput == true) {
	convert.str("");
	convert << "cur Place index = " << curPlace->index.size() << std::endl;
	MASS_base::log( convert.str() );
    }
    // create as many new agents as nColonists
    for ( int nColonists = 
	    protoAgent->map( initPopulation, curPlace->size, curPlace->index, curPlace );
	  nColonists > 0; nColonists--, localPopulation++ ) {
      
      // agent instanstantiation and initialization
      Agent *newAgent = (Agent *)(agentsDllClass->instantiate( argument ) );

      if(printOutput == true){
         convert.str( "" );
         convert << " newAgent[" << localPopulation << "] = " << (void *)newAgent;
         MASS_base::log( convert.str( ) );
      }

      newAgent->agentsHandle = handle;
      newAgent->placesHandle = placesHandle;
      newAgent->agentId = currentAgentId++;
      newAgent->parentId = -1; // no parent
      newAgent->place = curPlace;

      for ( int index = 0; index < int( curPlace->index.size( ) ); index++ )
	newAgent->index.push_back( curPlace->index[index] );
      newAgent->alive = true;

      // store this agent in the bag of agents
      agentsDllClass->agents->push_back( newAgent );

      // TODO: register newAgent into curPlace
      curPlace->agents.push_back( (MObject *)newAgent );
    }
  }
  delete protoAgent;
}

/**
* 
*/
Agents_base::~Agents_base( ) {
}

/**
 * 
 * @param functionId
 * @param argument
 * @param tid
 */
void Agents_base::callAll( int functionId, void *argument, int tid ) {

	//Set up the bag of agents
	DllClass *dllclass = MASS_base::dllMap[ handle ];

	//Create the bag for returning agents to be placed in
	vector<Agent*> *retBag = dllclass->retBag;
	int numOfOriginalVectors = Mthread::agentBagSize;
        ostringstream convert;

	while (true){
		//Create the index for this iteration
		int myIndex;

		//Lock the index assignment so no two threads will receive the same value
		pthread_mutex_lock(&MASS_base::request_lock);

		/**/		//Thread checking
		if(printOutput == true){
		    convert.str("");
		    convert << "Starting index value is: " << Mthread::agentBagSize;
		    MASS_base::log(convert.str());
		}
		myIndex=Mthread::agentBagSize--;


		//Error Checking
		if(printOutput == true){
		    convert.str("");
		    convert << "Thread[" << tid << "]: agent("<< myIndex << ") assigned";
		    MASS_base::log(convert.str());
		}

		pthread_mutex_unlock(&MASS_base::request_lock);

		//Continue to run until the assigning index becomes negative
		//(in which case, we've run out of agents)
		if(myIndex > 0){
		  if(printOutput == true){
		        convert.str("");
		  }			
			//Lock the assignment and removal of agents
			pthread_mutex_lock(&MASS_base::request_lock);
			Agent* tmpAgent = dllclass->agents->back();
			dllclass->agents->pop_back();
			
			if(printOutput == true){
			    convert << "Thread [" << tid << "]: agent(" << tmpAgent << ")[" << myIndex << "] was removed ";
			    convert << "fId = " << functionId << " argument " << argument; 
			    MASS_base::log( convert.str( ) );
			}
			pthread_mutex_unlock(&MASS_base::request_lock);
			
			//Use the Agents' callMethod to have it begin running
			tmpAgent->callMethod(functionId, argument); 

			if(printOutput == true){
			    convert.str( "" );
			    convert << "Thread [" << tid << "]: (" << myIndex << ") has called its method; ";
			    MASS_base::log( convert.str( ) );
			}
			//Puth the now running thread into the return bag
			pthread_mutex_lock(&MASS_base::request_lock);
			retBag->push_back(tmpAgent);

			if(printOutput == true){
			    convert.str( "" );
			    convert << "Thread [" << tid << "]: (" << myIndex << ") has been placed in the return bag; ";
			    convert << "Current Agent Bag Size is: " << Mthread::agentBagSize;
			    convert << " retBag.size = " << retBag->size( );
			    MASS_base::log(convert.str());
			}
			pthread_mutex_unlock(&MASS_base::request_lock);
		}
		//Otherwise, we are out of agents and should stop
		//trying to assign any more
		else{
			break;
		}
	}
	//Wait for the thread count to become zero
	Mthread::barrierThreads( tid );
	
	//Assign the new bag of finished agents to the old pointer for reuse
	if ( tid == 0 ) {
	  delete MASS_base::dllMap[ handle ]->agents;
	  MASS_base::dllMap[ handle ]->agents = MASS_base::dllMap[ handle ]->retBag;
	  Mthread::agentBagSize = numOfOriginalVectors;

	  if(printOutput == true){
	      convert.str("");
	      convert << "Agents_base:callAll: agents.size = " << MASS_base::dllMap[handle]->agents->size( ) << endl;
	      convert << "Agents_base:callAll: agentsBagSize = " << Mthread::agentBagSize;
	      MASS_base::log( convert.str( ) );
	  }
	}
}

/**
 * 
 * @param functionId
 * @param argument
 * @param arg_size
 * @param ret_size
 * @param tid
 */
void Agents_base::callAll( int functionId, void *argument, int arg_size,
			     int ret_size, int tid ) {

  //Set up the bag of agents
  DllClass *dllclass = MASS_base::dllMap[ handle ];
  
  //Create the bag for returning agents to be placed in
  vector<Agent*> *retBag = dllclass->retBag;
  int numOfOriginalVectors = Mthread::agentBagSize;
  ostringstream convert;
  
  while(true){
    // create the index for this iteration
    int myIndex;

    // Lock the index assginment so no tow threads will receive the same index
    pthread_mutex_lock(&MASS_base::request_lock);

    // Thread checking
    if(printOutput == true){
        convert.str( "" );
        convert << "Starting index value is: " << Mthread::agentBagSize;
        MASS_base::log( convert.str() );
    }

    myIndex=Mthread::agentBagSize--; // myIndex == agentId + 1

    //Error Checking
    if(printOutput == true){
        convert.str("");
        convert << "Thread[" << tid << "]: agent("<< myIndex << ") assigned";
        MASS_base::log(convert.str());
    }

    pthread_mutex_unlock(&MASS_base::request_lock);			
    
    //While there are still indexes left, continue to grab and execute threads
    //When all are executing, place into vector and wait for them to finish
    if(myIndex > 0){
      // compute where to store this agent's return value
      // note that myIndex = agentId + 1

      if(printOutput == true){
          convert.str( "" );
          convert << "Thread[" << tid << "]: agent("<< myIndex << "): MASS_base::currentReturns  = " << MASS_base::currentReturns
	          << " ret_size = " << ret_size;
          MASS_base::log(convert.str());
      }

      char *return_values = MASS_base::currentReturns + (myIndex - 1) * ret_size;
      
      if(printOutput == true){
          convert.str( "" );
          convert << "Thread[" << tid << "]: agent("<< myIndex << "): return_values = " << return_values;
          MASS_base::log(convert.str());
      }

      // Lock the assginment and removel of agents
      pthread_mutex_lock(&MASS_base::request_lock);

      Agent* tmpAgent = dllclass->agents->back();
      dllclass->agents->pop_back();

      if(printOutput == true){
          convert.str( "" );
          convert << "Thread [" << tid << "]: agent(" << myIndex << ") was removed = " << tmpAgent;
          MASS_base::log(convert.str());
      }

      pthread_mutex_unlock(&MASS_base::request_lock);

      //Use the Agents' callMethod to have it begin running
      if(printOutput == true){
          convert.str( "" );
      }
      void *retVal = tmpAgent->callMethod( functionId, 
					   (char *)argument + arg_size * (myIndex - 1) );
      memcpy( return_values, retVal, ret_size ); // get this callAll's return value

      if(printOutput == true){
          convert << "Thread [" << tid << "]: (" << myIndex << ") has called its method; ";
      }
      //Puth the now running thread into the return bag
      pthread_mutex_lock(&MASS_base::request_lock);
      retBag->push_back(tmpAgent);
      
      if(printOutput == true){
          convert << "Thread [" << tid << "]: (" << myIndex << ") has been placed in the return bag; ";
          convert << "Current Agent Bag Size is: " << Mthread::agentBagSize;
          convert << " retBag.size = " << retBag->size( );
          MASS_base::log(convert.str());
      }

      pthread_mutex_unlock(&MASS_base::request_lock);
      
    }
    //Otherwise, we are out of agents and should stop                                                      
    //trying to assign any more
    else{
      break;
    }
  }
  //Confirm all threads have finished
  Mthread::barrierThreads( tid );
  
  //Assign the new bag of finished agents to the old pointer for reuse
  if ( tid == 0 ) {
    delete MASS_base::dllMap[ handle ]->agents;
    MASS_base::dllMap[ handle ]->agents = MASS_base::dllMap[ handle ]->retBag;
    Mthread::agentBagSize = numOfOriginalVectors;

    if(printOutput == true){
        convert.str("");
        convert << "Agents_base:callAll: agents.size = " << MASS_base::dllMap[handle]->agents->size( ) << endl;
        convert << "Agents_base:callAll: agentsBagSize = " << Mthread::agentBagSize;
        MASS_base::log( convert.str( ) );
    }

  }
  
}

/**
 * 
 * @param src_index
 * @param dst_size
 * @param dest_dimension
 * @param dest_index
 */
void Agents_base::getGlobalAgentArrayIndex( vector<int> src_index,
				    int dst_size[], int dest_dimension,
				    int dest_index[] ){

 for (int i = 0; i < dest_dimension; i++ ) {
    dest_index[i] = src_index[i]; // calculate dest index

    if ( dest_index[i] < 0 || dest_index[i] >= dst_size[i] ) {
      // out of range
      for ( int j = 0; j < dest_dimension; j++ ) {
	// all index must be set -1
	dest_index[j] = -1;
      }
      return;
    }
  }

}

/**
 * 
 * @param tid
 */
void Agents_base::manageAll( int tid ) {
  
  //Create the dllclass to access our agents from, out agentsDllClass for 
  // agent instantiation, and our bag for Agent objects after they have 
  // finished processing
  ostringstream convert;
  DllClass *dllclass = MASS_base::dllMap[handle];
  DllClass *agentsDllClass = new DllClass( className );
  Places_base *evaluatedPlaces = MASS_base::placesMap[ placesHandle ];
  vector<Agent*> *retBag = dllclass->retBag;

  // Spawn, Kill, Migrate. Check in that order throughout the bag of agents 
  // sequentially.
  while( true ){

    pthread_mutex_lock(&MASS_base::request_lock);
    int agentSize = dllclass->agents->size();
    if ( agentSize == 0 ) {
      pthread_mutex_unlock(&MASS_base::request_lock);
      break;
    }
    //Grab the last agent and remove it for processing. Be sure to lock on 
    // removal
    Agent *evaluationAgent = dllclass->agents->back();
    dllclass->agents->pop_back();

    if(printOutput == true){
        convert.str("");
        convert << "Agents_base::manageALL: Thread " << tid << " picked up " 
	        << evaluationAgent->agentId;
        MASS_base::log(convert.str());
    }

    pthread_mutex_unlock(&MASS_base::request_lock);
    int argumentcounter = 0;

    //If the spawn's newChildren field is set to anything higher than zero
    //we need to create newChildren's worth of Agents in the current location.

    //Spawn() Check
    int childrenCounter = evaluationAgent->newChildren;

    if(printOutput == true){
        convert.str("");
        convert << "agent " << evaluationAgent->agentId
	        << "'s childrenCounter = " << childrenCounter;
        MASS_base::log(convert.str());
    }

    while( childrenCounter > 0 ){
      if(printOutput == true){
          convert.str("");
          convert << "Agent_base::manageALL: Thread " << tid 
	          << " will spawn a child of agent " << evaluationAgent->agentId
	          << "...arguments.size( ) = " 
	          << evaluationAgent->arguments.size( )
	          << ", argumentcounter = " << argumentcounter;
          MASS_base::log(convert.str());
      }

      // Beginning mutex lock when spawning a new agent.  If multiple threads
      // try to spawn agents at the same time, it will result in concurrent
      // access to Agents_base class variables.
      pthread_mutex_lock(&MASS_base::request_lock);
      
      Agent* addAgent = 
	// validate the correspondence of arguments and argumentcounter
	( int( evaluationAgent->arguments.size( ) ) > argumentcounter ) ?
	// yes: this child agent should receive an argument.
	(Agent *)(agentsDllClass->instantiate(evaluationAgent->
					      arguments[argumentcounter++])) :
	// no:  this child agent should not receive an argument.
	(Agent *)(agentsDllClass->instantiate( NULL ) );

      //Push the created agent into our bag for returns and update the counter
      //needed to keep track of our agents.

      retBag->push_back(addAgent);
      pthread_mutex_unlock(&MASS_base::request_lock);
      
      //Push the pointer copy into the current Agent's place location
      pthread_mutex_lock(&MASS_base::request_lock);
      evaluationAgent->place->agents.push_back((MObject*) addAgent);
      pthread_mutex_unlock(&MASS_base::request_lock);
      
      // initialize this child agent's attributes: 
      pthread_mutex_lock(&MASS_base::request_lock);
      addAgent->agentsHandle = evaluationAgent->agentsHandle;
      addAgent->placesHandle = evaluationAgent->placesHandle;
      addAgent->agentId = currentAgentId++;
      addAgent->index = evaluationAgent->index;
      addAgent->place = evaluationAgent->place;
      addAgent->parentId = evaluationAgent->agentId;
      //Decrement the newChildren counter once an Agent has been spawned
      evaluationAgent->newChildren--;
      childrenCounter--;
      pthread_mutex_unlock(&MASS_base::request_lock);
      
      if(printOutput == true){
          convert.str("");
          convert << "Agent_base::manageALL: Thread " << tid 
	          << " spawned a child of agent " << evaluationAgent->agentId
	          << " and put the child " << addAgent->agentId
	          << " child into retBag." ;
          MASS_base::log(convert.str());
      }
    }

    //Kill() Check
    if(printOutput == true){
        convert.str("");
        convert << "Agent_base::manageALL: Thread " << tid 
	        << " check " << evaluationAgent->agentId << "'s alive = " 
	        << evaluationAgent->alive;
        MASS_base::log(convert.str());
    }
    if(evaluationAgent->alive == false){
      
      //Get the place in which evaluationAgent is 'stored' in
      Place *evaluationPlace = evaluationAgent->place;
      
      // Move through the list of Agents to locate which to delete
      // Do so non-interruptively.
      pthread_mutex_lock(&MASS_base::request_lock);
      int evalPlaceAgents = evaluationPlace->agents.size();

      for( int i = 0; i < evalPlaceAgents; i++){
	
	//Type casting used so we can compare agentId's
	MObject *comparisonAgent = evaluationPlace->agents[i];
	Agent *convertedAgent = static_cast<Agent*>(comparisonAgent);

	// Check the Id against the ID of the agent to be removed. 
	// If it matches, remove it Lock
	if((evaluationAgent->agentId == convertedAgent->agentId) && 
	   (evaluationAgent->agentsHandle == convertedAgent->agentsHandle)){
	  evaluationPlace->agents.erase( evaluationPlace->agents.begin() + i );
	  
	  if(printOutput == true){
	      convert.str("");
	      convert << "Agent_base::manageALL: Thread " << tid 
		      << " deleted " << evaluationAgent->agentId 
		      << " from place[" << evaluationPlace->index[0]
		      << "][" << evaluationPlace->index[1] << "]";
	      MASS_base::log(convert.str());
	  }
	  break;
	}
      }

      pthread_mutex_unlock(&MASS_base::request_lock);

      //Delete the agent and its pointer to complete the removal
      //delete &evaluationAgent;
      delete evaluationAgent;
      continue;
    }
    
    //Migrate() check

    //Iterate over all dimensions of the agent to check its location
    //against that of its place. If they are the same, return back.
    int agentIndex = evaluationAgent->index.size();
    int destCoord[agentIndex];

    // compute its coordinate
    getGlobalAgentArrayIndex( evaluationAgent->index, evaluatedPlaces->size,
			      evaluatedPlaces->dimension, destCoord );

    if(printOutput == true){
        convert.str( "" );
        convert << "pthread_self[" << pthread_self( )
	        << "tid[" << tid << "]: calls from"
	        << "[" << evaluationAgent->index[0]
	        << "][" << evaluationAgent->index[1] << "]"
	        << " (destCoord[" << destCoord[0]
	        << "][" << destCoord[1] << "]"
	        << " evaluatedPlaces->size[" << evaluatedPlaces->size[0] 
	        << "][" << evaluatedPlaces->size[1] << "]";
        MASS_base::log(convert.str());
    }

    if( destCoord[0] != -1 ) { 
      // destination valid
      if(printOutput == true){
          convert.str( "" );
          convert << "getGlobalLinearIndexFromGlobalArrayIndex: evaluatedPlace = " << evaluatedPlaces
	          << " destCoard = " << destCoord;
          MASS_base::log(convert.str());
      }
      int globalLinearIndex = 
	evaluatedPlaces->getGlobalLinearIndexFromGlobalArrayIndex( destCoord,
						  evaluatedPlaces->size,
						  evaluatedPlaces->dimension );
      if(printOutput == true){
          convert.str( "" );
          convert << " linear = " << globalLinearIndex
	          << " lower = " << evaluatedPlaces->lower_boundary
	          << " upper = " << evaluatedPlaces->upper_boundary << ")";
          MASS_base::log(convert.str());
      }

      // Should remove the pointer object in the place that points to 
      // the migrting Agent
      Place *oldPlace = evaluationAgent->place;
      pthread_mutex_lock(&MASS_base::request_lock);
      // Scan old_place->agents to find this evaluationAgent's index.
      int oldIndex = -1;
      for(unsigned int i = 0; i < oldPlace->agents.size();i++){
	if(oldPlace->agents[i] == evaluationAgent){
	  oldIndex = i;
	  break;
	}	    
      }
      if(oldIndex != -1) {
	oldPlace->agents.erase( oldPlace->agents.begin() + oldIndex ); 
      }
      else {
	// should happen
	if(printOutput == true){
	  convert.str( "" );
	  convert << "evaluationAgent " << evaluationAgent->agentId 
		  << " couldn't been found in the old place!";
	  MASS_base::log(convert.str());
	}
	exit( -1 );
      }
      if(printOutput == true){
	convert.str( "" );
	convert << "evaluationAgent " << evaluationAgent->agentId 
		<< " was removed from the oldPlace["
		<< oldPlace->index[0] << "]["
		<< oldPlace->index[1] << "]";
	MASS_base::log(convert.str());
      }
      pthread_mutex_unlock(&MASS_base::request_lock);

      if ( globalLinearIndex >= evaluatedPlaces->lower_boundary &&
	   globalLinearIndex <= evaluatedPlaces->upper_boundary ) {
	// local destination

	// insert the migration Agent to a local destination place
	int destinationLocalLinearIndex 
	  = globalLinearIndex - evaluatedPlaces->lower_boundary;

	if(printOutput == true){
	    convert.str( "" );
	    convert << "destinationLocalLinerIndex = " 
		    << destinationLocalLinearIndex;
	    MASS_base::log(convert.str());
	}

	DllClass *places_dllclass = MASS_base::dllMap[ placesHandle ];
	evaluationAgent->place 
	  = places_dllclass->places[destinationLocalLinearIndex];

	evaluationAgent->index = evaluationAgent->place->index;

	if(printOutput == true){
	    convert.str( "" );
	    convert << "evaluationAgent->place = " 
		    << evaluationAgent->place;
	    MASS_base::log(convert.str());
	}

	pthread_mutex_lock(&MASS_base::request_lock);
	evaluationAgent->place->agents.push_back((MObject *)evaluationAgent);

	if(printOutput == true){
	    convert.str( "" );
	    convert << "evaluationAgent " << evaluationAgent->agentId 
		    << " was inserted into the destPlace["
		    << evaluationAgent->place->index[0] << "]["
		    << evaluationAgent->place->index[1] << "]";
	    MASS_base::log(convert.str());
	}

	pthread_mutex_unlock(&MASS_base::request_lock);

	//If not killed or migrated remotely, push Agent into the retBag  
	pthread_mutex_lock(&MASS_base::request_lock);
	retBag->push_back(evaluationAgent);

	if(printOutput == true){
	    convert.str( "" );
	    convert << "evaluationAgent " << evaluationAgent->agentId 
		    << " was pushed back into retBag";
	    MASS_base::log(convert.str());
	}

	pthread_mutex_unlock(&MASS_base::request_lock);

      } 
      else {
	// remote destination
	
	// find the destination node
	int destRank 
	  = evaluatedPlaces->getRankFromGlobalLinearIndex( globalLinearIndex );
	
	// create a request
	AgentMigrationRequest *request 
	  = new AgentMigrationRequest( globalLinearIndex, evaluationAgent );

	if(printOutput == true){
	    convert.str( "" );
	    convert << "AgentMigrationRequest *request = " << request;
	    MASS_base::log( convert.str( ) );
	}

	// enqueue the request to this node.map
	pthread_mutex_lock( &MASS_base::request_lock );
	MASS_base::migrationRequests[destRank]->push_back( request );
	
	if(printOutput == true){
	    convert.str( "" );
	    convert << "remoteRequest[" << destRank << "]->push_back:"
		    << " dst = " << globalLinearIndex;
	    MASS_base::log( convert.str( ) );
	}

	pthread_mutex_unlock( &MASS_base::request_lock );
      }
    } else {
      if(printOutput == true){
	  convert << " to destination invalid";
      }
    }
    if(printOutput == true){
        MASS_base::log( convert.str( ) );
    }	
  } // end of while( true )

  //When while loop finishes, all Agents reside in retBag. 
  // Need to hook back up.
  Mthread::barrierThreads( tid );

  //Assign the new bag of finished agents to the old pointer for reuse
  if ( tid == 0 ) {
    delete MASS_base::dllMap[ handle ]->agents;
    MASS_base::dllMap[ handle ]->agents = MASS_base::dllMap[ handle ]->retBag;
    Mthread::agentBagSize = MASS_base::dllMap[ handle ]->agents->size( );

    if(printOutput == true){
        convert.str("");
        convert << "Agents_base:manageAll: agents.size = " 
	        << MASS_base::dllMap[handle]->agents->size( ) << endl;
        convert << "Agents_base:manageAll: agentsBagSize = " 
	        << Mthread::agentBagSize;
        MASS_base::log( convert.str( ) );
    }
  }

  // all threads must barrier synchronize here.
  Mthread::barrierThreads( tid );
  if ( tid == 0 ) {
    
    if(printOutput == true){
        convert.str( "" );
        convert << "tid[" << tid << "] now enters processAgentMigrationRequest";
        MASS_base::log( convert.str( ) );
    }

    // the main thread spawns as many communication threads as the number of
    // remote computing nodes and let each invoke processAgentMigrationReq.
    
    // args to threads: rank, agentHandle, placeHandle, lower_boundary
    int comThrArgs[MASS_base::systemSize][4];
    pthread_t thread_ref[MASS_base::systemSize]; // communication thread id
    for ( int rank = 0; rank < MASS_base::systemSize; rank++ ) {
      
      if ( rank == MASS_base::myPid ) // don't communicate with myself
	continue;
      
      // set arguments 
      comThrArgs[rank][0] = rank;
      comThrArgs[rank][1] = handle; // agents' handle
      comThrArgs[rank][2] = evaluatedPlaces->handle;
      comThrArgs[rank][3] = evaluatedPlaces->lower_boundary;
      
      // start a communication thread
      if ( pthread_create( &thread_ref[rank], NULL, 
			   Agents_base::processAgentMigrationRequest, 
			   comThrArgs[rank] ) != 0 ) {
	MASS_base::log( "Agents_base.manageAll: failed in pthread_create" );
	exit( -1 );
      }

      if(printOutput == true){
          convert.str( "" );
          convert << "Agents_base.manageAll will start processAgentMigrationRequest thread["
	          << rank << "] = " << thread_ref[rank];
          MASS_base::log( convert.str( ) );
      }
    }

    // wait for all the communication threads to be terminated
    for ( int rank = MASS_base::systemSize - 1; rank >= 0; rank-- ) {

      if(printOutput == true){
          convert.str( "" );
          convert << "Agents_base.manageAll will join processAgentMigrationRequest A thread["
	          << rank << "] = " << thread_ref[rank] << " myPid = " << MASS_base::myPid;
          MASS_base::log( convert.str( ) );
      }
      if ( rank == MASS_base::myPid ) // don't communicate with myself
	continue;      
      
      if(printOutput == true){
          convert.str( "" );
          convert << "Agents_base.manageAll will join processAgentMigrationRequest B thread["
	          << rank << "] = " << thread_ref[rank];
          MASS_base::log( convert.str( ) );
      }

      pthread_join( thread_ref[rank], NULL );

      if(printOutput == true){
          convert.str( "" );
          convert << "Agents_base.manageAll joined processAgentMigrationRequest C thread["
	          << rank << "] = " << thread_ref[rank];
          MASS_base::log( convert.str( ) );
      }
    }
    localPopulation = MASS_base::dllMap[handle]->agents->size( );

    if(printOutput == true){
        convert.str( "" );
        convert << "Agents_base.manageAll completed: localPopulation = "
	        << localPopulation;
        MASS_base::log( convert.str( ) );
    }
  }
  else {
    if(printOutput == true){
        convert.str( "" );
        convert << "pthread_self[" << pthread_self( )
	        << "] tid[" << tid << "] skips processAgentMigrationRequest";
        MASS_base::log( convert.str( ) );
    }
  }
}

/**
 * 
 * @param param
 * @return 
 */
void *Agents_base::processAgentMigrationRequest( void *param ) {
  int destRank = ( (int *)param )[0];
  int agentHandle = ( (int *)param )[1];
  int placeHandle = ( (int *)param )[2];
  //  int my_lower_boundary = ( (int *)param )[3];

  vector<AgentMigrationRequest*>* orgRequest = NULL;
  ostringstream convert;

  if(printOutput == true){
      convert.str( "" );
      convert << "pthread_self[" << pthread_self( )
	      << "] rank[" << destRank << "]: starts processAgentMigrationRequest";
      MASS_base::log( convert.str( ) );
  }

  // pick up the next rank to process
  orgRequest = MASS_base::migrationRequests[destRank];

  // for debugging
  pthread_mutex_lock( &MASS_base::request_lock );
  if(printOutput == true){
      convert.str( "" );
      convert << "tid[" << destRank << "] sends an exhange request to rank: " 
	      << destRank << " size() = " << orgRequest->size( ) << endl;
      MASS_base::log( convert.str( ) );

      convert.str( "" );
      for ( int i = 0; i < int( orgRequest->size( ) ); i++ ) {
        convert << "send "
	        << (*orgRequest)[i]->agent << " to "
	        << (*orgRequest)[i]->destGlobalLinearIndex << endl;
      }
      MASS_base::log( convert.str( ) );
  }
  pthread_mutex_unlock( &MASS_base::request_lock );

  // now compose and send a message by a child
  Message messageToDest( Message::AGENTS_MIGRATION_REMOTE_REQUEST,
			 agentHandle, placeHandle, orgRequest );

  if(printOutput == true){
      convert.str( "" );
      convert << "tid[" << destRank << "] made messageToDest to rank: " 
	      << destRank; 
      MASS_base::log( convert.str( ) );
  }

  struct MigrationSendMessage rankNmessage;
  rankNmessage.rank = destRank;
  rankNmessage.message = &messageToDest;
  pthread_t thread_ref;
  pthread_create( &thread_ref, NULL, sendMessageByChild, &rankNmessage );

  // receive a message by myself
  Message *messageFromSrc = MASS_base::exchange.receiveMessage( destRank );
 
  // at this point, the message must be exchanged.
  pthread_join( thread_ref, NULL );

  if(printOutput == true){
      convert.str( "" );
      convert << "pthread id = " << thread_ref
	      << "pthread_join completed for rank[" 
	      << destRank << "] and will delete messageToDest: " << &messageToDest;
      MASS_base::log( convert.str( ) );

      convert.str( "" );
      convert << "Message Deleted";
      MASS_base::log( convert.str( ) );
  }

  // process a message
  vector<AgentMigrationRequest*>* receivedRequest 
    = messageFromSrc->getMigrationReqList( );

  int agentsHandle = messageFromSrc->getHandle( );
  int placesHandle = messageFromSrc->getDestHandle( );
  Places_base *dstPlaces = MASS_base::placesMap[ placesHandle ];
  DllClass *agents_dllclass = MASS_base::dllMap[ agentsHandle ];
  DllClass *places_dllclass = MASS_base::dllMap[ placesHandle ];

  if(printOutput == true){
      convert.str( "" );
      convert << "request from rank[" << destRank << "] = " << receivedRequest;
      convert << " size( ) = " << receivedRequest->size( );
      MASS_base::log( convert.str( ) );
  }

  // retrieve agents from receiveRequest
  while( receivedRequest->size( ) > 0 ) {
    AgentMigrationRequest *request = receivedRequest->back( );
    receivedRequest->pop_back( );
    int globalLinearIndex = request->destGlobalLinearIndex;
    Agent *agent = request->agent;

    // local destination
    int destinationLocalLinearIndex 
      = globalLinearIndex - dstPlaces->lower_boundary;
      
    if(printOutput == true){
        convert << " dstLocal = " << destinationLocalLinearIndex << endl;
    }

    Place *dstPlace = 
      (Place *)(places_dllclass->places[destinationLocalLinearIndex]);

    // push this agent into the place and the entire agent bag.
    agent->place = dstPlace;
    agent->index = dstPlace->index;
    pthread_mutex_lock(&MASS_base::request_lock);
    dstPlace->agents.push_back( agent );
    agents_dllclass->agents->push_back( agent );
    pthread_mutex_unlock(&MASS_base::request_lock);

    delete request;
  }

  if(printOutput == true){
      convert.str( "" );
      convert << "pthread_self[" << pthread_self( )
	      << "] retreive agents from rank[" << destRank << "]complated";
      MASS_base::log( convert.str( ) );
  }
  pthread_exit( NULL );
  return NULL;
}

/**
 * 
 * @param param
 * @return 
 */
void *Agents_base::sendMessageByChild( void *param ) {
  int rank = ((struct MigrationSendMessage *)param)->rank;

  ostringstream convert;
  if(printOutput == true){
      convert << "pthread_self[" << pthread_self( )
	      << "] sendMessageByChild to " << rank << " starts";
      MASS_base::log( convert.str( ) );
  }

  Message *message = (Message *)((struct MigrationSendMessage *)param)->message;
  MASS_base::exchange.sendMessage( rank, message );

  if(printOutput == true){
      convert.str( "" );
      convert << "pthread_self[" << pthread_self( )
	      << "] sendMessageByChild to " << rank << " finished";
      MASS_base::log( convert.str( ) );
  }
  pthread_exit( NULL );
  return NULL;
}






