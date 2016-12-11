#include "MASS.h"
#include "Land.h"
#include "Nomad.h"
#include "Timer.h"	//Timer
#include <stdlib.h> // atoi
#include <unistd.h>
#include <vector>

//Used to toggle output for Main
const bool printOutput = false;
const int iterPerTurn = 9;							//Partitioned movespaces are 9 squares large

Timer timer;

int main( int argc, char *args[] ) {

  // check that all arguments are present 
  // exit with an error if they are not
  if ( argc != 12 ) {
    cerr << "usage: ./main username password machinefile port nProc nTh ..." 
	 << endl;
    cerr << "argc == " << argc << endl;
    for ( int i = 0; i < argc ; i++) {
	cerr << "args[" << i << "] = " << args[i] << endl;
    }
    return -1;
  }
  
  // get the arguments passed in
  char *arguments[4];
  arguments[0] = args[1]; // username
  arguments[1] = args[2]; // password
  arguments[2] = args[3]; // machinefile
  arguments[3] = args[4]; // port
  int nProc = atoi( args[5] ); // number of processes
  int nThr = atoi( args[6] );  // number of threads
  
  const int numTurns = atoi( args[7] );	//Run this simulation for numTurns
  const bool unitTest = ( *args[8] == 'y' ) ? true : false;	//Enable or disable unit testing
  const bool collision_free_lib = ( *args[9] == 'y' ) ? true : false;	//Enable or disable collision detection on MASS library, default is app level collision
  const int sizeX = atoi( args[10] );
	const int sizeY = atoi( args[11] );			//Random map's area
  const int rmSpawn = 0.2;												//Spawn proportion to total area by dimension
  const int myPopulation = sizeX * sizeY * rmSpawn * rmSpawn;			//Population{RMsize, RMSpawn}
  static const int totalSize = sizeX * sizeY;							//Total size of map
  
  // initialize MASS with the machine information,
  // number of processes, and number of threads
  MASS::init( arguments, nProc, nThr ); 
  // prepare a message for the places (this is argument below)
  char *msg = (char *)("hello\0"); // should not be char msg[]
  
  /*  THIS SECTION OF CODE DEALS ONLY WITH PLACES  */
  
  // Create the places.
  // Arguments are, in order:
  //    handle, className, boundary_width, argument, argument_size, dim, ...
  Places *land = new Places( 1, "Land", 1, msg, 7, 2, sizeX, sizeY );
  vector<int*> destinations;
  int north[2] = {0, 1};  destinations.push_back( north );
  int east[2]  = {1, 0};  destinations.push_back( east );
  int south[2] = {0, -1}; destinations.push_back( south );
  int west[2]  = {-1, 0}; destinations.push_back( west );

  // Create the agents.
  // Arguments are, in order:
  //    handle, className, *argument, argument_size, *places,  initPopulation
  Agents *nomad = (collision_free_lib) ?
				new Agents( 100, "Nomad", msg, 7, land, myPopulation ):
				new Agents( 2, "Nomad", msg, 7, land, myPopulation );
  
  land->callAll(Land::init_);				//initialize Lands
	
	timer.start();
	
	/*PARTITION-SPACE COLLISION AVOID RANDOMWALK at the app level*/				//#partition-space
	if(!collision_free_lib)
	{
	
	for(int turn = 0; turn < numTurns; turn++)
	{

		ostringstream convert;
		convert << std::endl << "TURN: " << turn << std::endl;
		MASS_base::log( convert.str() );
		nomad->callAll( Nomad::newTurn_ );										//Reset movecount and iteration to act
		//Iterations
		for(int i = 0; i < iterPerTurn; i++)
		{
			int* iter = new int;
			*iter = i;
			land->callAll( Land::resetForNextTurn_ );							//Clear migration data from previous iteration
			nomad->callAll( Nomad::delegateMove2_, (void*) iter, sizeof(int) );				//Delegate
			land->callAll(Land::displayPopAsOut_);
			land->exchangeBoundary();
			land->callAll(Land::delegateCalculation_);
			nomad->callAll( Nomad::moveDelegated2_, (void*) iter, sizeof(int) );	//Read delegate and move
			nomad->manageAll( );												//Actual move the agent. Here moves agents calculated each iteration

		}
		
		//UNIT-TEST: No two Nomads exist on same land
		if(unitTest)
		{
			bool assertion = true;
			int hash[10000] = {0};						//must be the same size as map
			
			int agent_callargs[myPopulation];
		    int *retvals = (int *) nomad->callAll( Nomad::ravelledxy_, (void *)agent_callargs, sizeof( int ), sizeof( int ) );
			for ( int i = 0; i < myPopulation; i++ )
			{
				if(hash[retvals[i]] != 0)
				{
					assertion = false;
					break;
				}
				else
					hash[retvals[i]] = 1;
			}
			
			delete retvals;
			
			if(assertion)
				MASS_base::log( "UNIT TEST PASSED" );
			else
				MASS_base::log( "UNIT TEST FAILED" );
		}
	}
	}

	/*THIS SECTION DOES RANDOMWALK without collision detection FOR SOME TURNS*/ //#library-collision-detect
	else
	{
		for(int i = 0; i < numTurns; i++)
	{
	if (i % (numTurns / 4) == 0 && printOutput) {
		std::cerr << "TURN " << i << " AGENT POSITIONS: " << std::endl;
		nomad->callAll( Nomad::printCurrentWithID_ );
		std::cerr << "DONE PRINTING AGENT POSITIONS" << std::endl;
	}
     nomad->callAll( Nomad::randomMove_ );
     nomad->manageAll( );
	 //Assert that no two Nomads exist on the same Land
	 if(unitTest)
		{
			bool assertion = true;
			int* hash = new int[totalSize];
			
			int agent_callargs[myPopulation];
		    int *retvals = (int *) nomad->callAll( Nomad::ravelledxy_, (void *)agent_callargs, sizeof( int ), sizeof( int ) );
			for ( int j = 0; j < myPopulation; j++ )
			{
				if(hash[retvals[j]] != 0)
				{
					assertion = false;
					break;
				}
				else
					hash[retvals[j]] = 1;
			}
			
			delete retvals;
			
			ostringstream convert;
			if(assertion)
				convert << "UNIT TEST PASSED turn " << i;
			else
				convert << "UNIT TEST FAILED turn " << i;

			MASS_base::log( convert.str( ) );
			delete hash;
		}
	}
	
	}
  
  long elaspedTime_END =  timer.lap();
  printf( "\nEnd of simulation. Elasped time using MASS framework with %i processes and %i thread and %i turns :: %ld \n",nProc,nThr,numTurns, elaspedTime_END);
  
  MASS::finish( );
}
#ifndef Land_H
#define Land_H

#include <string.h>
#include "Place.h"

using namespace std;

class Land : public Place {
public:

	static const int NUM_NEIGHBORS = 4;									//A land has 4 neighbors
  
  // define functionId's that will 'point' to the functions they represent.
  static const int init_ = 0;
  static const int exchangetest_ = 2;
  static const int checkInMessage_ = 3;
  static const int printOutMessage_ = 4;
  static const int printShadow_ = 5;
  static const int popCount_ = 6;
  static const int recordAsNeighborPop_ = 7;
  static const int resetForNextTurn_ = 8;
  static const int displayPopAsOut_ = 9;
  static const int recordNeighborPopByOut_ = 10;
  static const int setMyAgent_ = 11;
  static const int getMigrationReservation_ = 12;
  static const int makeReservation_ = 13;
  static const int showReservation_ = 14;
  static const int checkReservation_ = 15;
  static const int reservationExchange_ = 16;
  static const int showReservation2_ = 17;
  static const int delegateCalculation_ = 18;
  
  /**
   * Initialize a Land object by allocating memory for it.
   */
  Land( void *argument ) : Place( argument ) {
    bzero( arg, sizeof( arg ) );
    strcpy( arg, (char *)argument );
  };
  
  /**
   * the callMethod uses the function ID to determine which method to execute.
   * It is assumed the arguments passed in contain everything those 
   * methods need to run.
   */
  virtual void *callMethod( int functionId, void *argument ) {
    switch( functionId ) {
    case init_: return init( argument );
    case exchangetest_: return exchangetest( argument );
    case checkInMessage_: return checkInMessage( argument );
    case printOutMessage_: return printOutMessage( argument );
    case printShadow_: return printShadow( argument );
	case popCount_: return popCount( argument );
	case recordAsNeighborPop_: return recordAsNeighborPop();
	case resetForNextTurn_: return resetForNextTurn();
	case displayPopAsOut_: return displayPopAsOut();
	case recordNeighborPopByOut_: return recordNeighborPopByOut();
	case setMyAgent_: return setMyAgent( argument );
	case getMigrationReservation_: return getMigrationReservation();
	case makeReservation_: return makeReservation();
	case showReservation_: return showReservation();
	case checkReservation_: return checkReservation();
	case reservationExchange_: return reservationExchange();
	case showReservation2_: return showReservation2();
	case delegateCalculation_: return delegateCalculation();
    }
    return NULL;
  };

private:
  char arg[100];
  void *init( void *argument );
  void *exchangetest( void *argument );
  void *checkInMessage( void *argument );
  void *printOutMessage( void *argument );
  void *printShadow( void *argument );
  void *popCount( void *argument );												//Return the population of this land
  void *recordAsNeighborPop();									//Record the inmessage to neighborPop
  void *resetForNextTurn();										//Reset for the next turn
  void *displayPopAsOut();										//Set the outmessage as population of this land
  void *recordNeighborPopByOut();								//Record the outmessage of neighbors as population
  void *setMyAgent( void *argmuent );							//Save the active current agentID for fast retrieval in migration calculation
  void *getMigrationReservation();								//returns int[2] of next move, as relative coordinate for migration, null if no rservation
  
  //Reservation process, 3 functions
  void *makeReservation();										//Makes a reservation on behalf of its agent by putInMessage to neighbor the ID of its agent
  void *showReservation();										//Shows the reservation status as 0 or agentID of the agent that can move to this place for the next turn
  void *checkReservation();										//Checks the reservation status of the neighbor for the its reservation status
  
  //PartitionMove
  void *delegateCalculation();									//Based on neighbor population, calculate nextMove and save it 		#partition-space
  
  void *reservationExchange();									//Exchange all version of making reservation.
  void *showReservation2();										//Exchange version of showReservation
  
  vector<int*> cardinals;										//Vector form of cardinals
  static const int neighbor[4][2];								//Array form of cardinals
  //Migration Data//
  int myAgent;													//Stores the id of my agent temporarily
  int nextMove[2];												//Stores the next move for my agent to migrate
  bool reserved;												//Status of whether next move is valid reservation or not
  int neighborPop[4];											//An array to store int population of neighbors
};

#endif
#include "Land.h"
#include "MASS_base.h"
#include <sstream>     // ostringstream

//Used to toggle output for Wave2d
//const bool printOutput = false;
 const bool printOutput = true;

extern "C" Place* instantiate( void *argument ) {
  return new Land( argument );
}

extern "C" void destroy( Place *object ) {
  delete object;
}


/**
 * Initializes a Land object.
 */
void *Land::init( void *argument ) {
  
  //Define cardinals
  int north[2]  = {0, 1};  cardinals.push_back( north );
  int east[2] = {1, 0};  cardinals.push_back( east );
  int south[2]  = {0, -1}; cardinals.push_back( south );
  int west[2] = {-1, 0}; cardinals.push_back( west );
  
  resetForNextTurn();					//Initialize by clearing the migration status
  
  return NULL;
}

  const int Land::neighbor[4][2] = {{0,1}, {1,0}, {0,-1}, {-1,0}};

/**
 * exchangetest causes each place to exchange information with it's neighbors.
 */
void *Land::exchangetest( void *argument ) {
  ostringstream convert;

  if(printOutput == true){
      convert << "exchangetest[" << index[0] << "][" << index[1] << "] out of ["
	      << size[0] << "][" << size[1] << "] received " << *(int *)argument;
      MASS_base::log( convert.str( ) );
  }

  int *retVal = new int;
  *retVal = *(int *)argument * 10000 + index[0] * 100 + index[1];
  
  return retVal;
}

/**
 * Logs any inMessages associated with the Place.
 */
void *Land::checkInMessage( void *argument ) {

  ostringstream convert;

  if(printOutput == true){
      convert << "checkInMessage[" << index[0] << "][" << index[1] 
	      << "] out of ["
	      << size[0] << "][" << size[1]
	      << "] inMessages.size = " << inMessages.size( )
	      << " received ";
      for ( int i = 0; i < int( inMessages.size( ) ); i++ )
        convert << " [" << i << "] = " << *(int *)(inMessages[i]);
  
      MASS_base::log( convert.str( ) );
  }

  return NULL;
}

/**
 */
void *Land::printOutMessage( void *argument ) {
  ostringstream convert;
  convert << "printOutMessage Land[" << index[0] << "][" << index[1] 
	  << "]'s outMessage = " << *(int *)outMessage;
  MASS_base::log( convert.str( ) );

  return NULL;
}

/**
 * Prints out the neighbors defined below if they exist.
 */
void *Land::printShadow( void *argument ) {
  int shadow[4];
  int north[2] = {0, 1};
  int east[2] = {1, 0}; 
  int south[2] = {0, -1};
  int west[2] = {-1, 0};
  int *ptr = (int *)getOutMessage( 1, north );
  shadow[0] = ( ptr == NULL ) ? -1 : *ptr;
  ptr = (int *)getOutMessage( 1, east );
  shadow[1] = ( ptr == NULL ) ? -1 : *ptr;
  ptr = (int *)getOutMessage( 1, south );
  shadow[2] = ( ptr == NULL ) ? -1 : *ptr;
  ptr = (int *)getOutMessage( 1, west );
  shadow[3] = ( ptr == NULL ) ? -1 : *ptr;

  ostringstream convert;
  convert << "printShadow:  Land[" << index[0] << "][" << index[1] 
	  << "]'s north = " << shadow[0] << ", east = " << shadow[1]
	  << ", south = " << shadow[2] << ", west = " << shadow[3];
  MASS_base::log( convert.str( ) );

  return NULL;
}

/** (int)
*	Returns the population of this land
*	@return:	AN INT, containing population of local agents of this Land
*/
void *Land::popCount( void *argument )
{ 
	int *retVal = new int;
	*retVal = agents.size();
	return retVal;
}

/** (void)
*	Record inmessage data into neighborPop
*	@pre:	Inmessage must contain integers
*/
void *Land::recordAsNeighborPop()
{
	int max = ((int)inMessages.size() > NUM_NEIGHBORS ? NUM_NEIGHBORS : inMessages.size());
	for(int i = 0; i < max; i++)
	{
		neighborPop[i] = *((int *)inMessages[i]);
	}
	return NULL;
}

/** (void)
*	Resets migration data for the next turn
*/
void *Land::resetForNextTurn()
{
	for(int i = 0; i < NUM_NEIGHBORS; i++)
		neighborPop[i] = -1;
	
	myAgent = -1;
	nextMove[0] = 0; nextMove[1] = 0;
	reserved = false;
	
	outMessage = NULL;
	outMessage_size = 0;
	return NULL;
}

/** (void)
*	Set the outmessage as population of this land
*/
void *Land::displayPopAsOut()
{
	outMessage_size = sizeof( int );
    outMessage = new int( );
    *(int *)outMessage = (int)agents.size();
	
	return NULL;
}

void *Land::recordNeighborPopByOut()
{
	//Only need to record neighborpopulation if the land has an agent who has delegated the Land to do so
	if(agents.size() == 0)
		return NULL;
	
	int North[2] = {0, 1};
    int East[2]  = {1, 0};
    int South[2] = {0, -1};
    int West[2]  = {-1, 0};
	
	/**/
	int *ptr = (int *)getOutMessage( 1, North );
    neighborPop[0] = ( ptr == NULL ) ? 0 : *ptr;
    ptr = (int *)getOutMessage( 1, East );
    neighborPop[1] = ( ptr == NULL ) ? 0 : *ptr;
    ptr = (int *)getOutMessage( 1, South );
    neighborPop[2] = ( ptr == NULL ) ? 0 : *ptr;
    ptr = (int *)getOutMessage( 1, West );
    neighborPop[3] = ( ptr == NULL ) ? 0 : *ptr;
	
	//Debug logging: Expects neighbor populations
	ostringstream convert;
	convert << "recordNeighborPopByOut:  Land[" << index[0] << "][" << index[1] << "]'s north = " << neighborPop[0] << ", east = " << neighborPop[1] << ", south = " << neighborPop[2] << ", west = " << neighborPop[3];
	MASS_base::log( convert.str( ) );
	return NULL;
}

/** (void)									#Visual logging confirmed 160710
*	Record my active agent by ID
*	@param:	argument is an int, the id of my agent
*/
void *Land::setMyAgent( void *argument )
{
	//Can only set agentID if agent resides on the Land
	if(agents.size() == 0)
		return NULL;
	
	myAgent = *(int *)argument;
	
	return NULL;
}

/** (int *)									#UNTESTED
*	Return the relative coordinate of successfully reserved place if any. Returns NULL if no reservations, and {0, 0} if reservation failed.
*	This is so the Nomad can retrieve its next move from the Land it is currently on
*/
void *Land::getMigrationReservation()
{
	int *retVal;
	if(nextMove[0] == 0 && nextMove[1] == 0)
		retVal = NULL;
	else
	{
		retVal = new int[2];
		if(!reserved)
		{
			retVal[0] = 0; retVal[1] = 0;
		}
		retVal[0] = nextMove[0];
		retVal[1] = nextMove[1];
	}
	
	//TO DEBUG LOG
	return retVal;
}

/** (void)									#DOES NOT WORK: putInMessage() fails accross ranks
*	Makes a reservation on behalf of its agent by putInMessage to neighbor the ID of its agent.
*	First, it calculates and stores a neighbor to reserve.
*	Then it attempts to put its agent's ID into the neighbor's inMessage.
*	@pre:	Neighbors have been polled for population status
*	@post:	Creates reservation on a selected neighbor
*/
void *Land::makeReservation()
{
	//Can only make reservation if delegated to do so
	if(agents.size() == 0)
		return NULL;
	
	vector<int> moveset;
	for(int i = 0; i < (int)cardinals.size(); i++)
	{
		if(neighborPop[i] == 0)
			moveset.push_back(i);
	}
	if(!moveset.empty())
	{
		int r = rand() % moveset.size();
		nextMove[0] = neighbor[moveset[r]][0];
		nextMove[1] = neighbor[moveset[r]][1];
	}
	
	
	//If any neighbor selected, reserve
	if(!(nextMove[0] == 0 && nextMove[1] == 0))
	{
		int *arg = new int;
		*arg = myAgent;
	}
	
	//Debug logging: Expects to reserve a space on an unoccupied land
	ostringstream convert;
	if(!(nextMove[0] == 0 && nextMove[1] == 0))
		convert << "makeReservation: Land[" << index[0] << "][" << index[1] << "] reserving on [" << (index[0] + nextMove[0]) << "][" << (index[1] + nextMove[1]) << "]";
	else
		convert << "makeReservation: Land[" << index[0] << "][" << index[1] << "] could not make reservation";
	MASS_base::log( convert.str( ) );
	
	//TO DEBUG LOG
	return NULL;
}

/** (void)									#DOES NOT WORK: putInMessage() fails accross ranks
*	Shows the reservation status as 0 or agentID of the agent that can move to this place for the next turn
*	First, it reads from inMessages to see if any reservations have been made.
*	@post:	The reservation made will be set as outMessage, or 0 if it hasn't been made. Also clears the inMessages[0] used for reservation channel
*/
void *Land::showReservation()
{
	ostringstream DEBUG;												//DEBUG OUTPUT
	
	vector<int> reservations;
	int* cur;
	
	for(int i = 0; i < (int)inMessages.size(); i++)
	{
		cur = (int *)(inMessages[i]);
		
		//Add whatever is trying to reserve on me
		if(cur != NULL)
		{
			if(*cur != 0)
				reservations.push_back(*cur);
			DEBUG << *cur << ", ";
		}
		else
			DEBUG << "null, ";
		
		
	}
	
	outMessage_size = sizeof( int );
	outMessage = new int( );
	//If empty, outMessage is 0
	if(reservations.empty())
	{
		*(int *)outMessage = 0;
	}
	//Else, outMessage is a randomly selected reservation
	else
	{
		int r = rand() % reservations.size();
		*(int *)outMessage = reservations[r];
	}
	
	//Debug logging: Expects to have read a reservation of whichever agentID wishing to migrate to this Land or 0 if none
	ostringstream convert;
	convert << "showReservation: Land[" << index[0] << "][" << index[1] << "] is reserved (" << *(int *)outMessage << "). InMessage was { " << DEBUG.str() <<"}";
	MASS_base::log( convert.str( ) );
	
	return NULL;
}

/** (void)									#Probably works 160711, not sure about successful reservation
*	Checks the reservation status of the neighbor for the its reservation status, by reading from the neighbor's outMessage
*	@post:	Sets the reserve status to true if its reservation is successful
*/
void *Land::checkReservation()
{
	//Can only check reservation if delegated to do so
	if(agents.size() == 0)
		return NULL;
	
	//If any neighbor selected, check reservation status
	if(!(nextMove[0] == 0 && nextMove[1] == 0))
	{
		int *ptr = (int *)getOutMessage( 1, nextMove );
		if(ptr != NULL && *ptr == myAgent)
		{
			reserved = true;
		}
	}
	
	//Debug logging: Correctly shows whether or not the random move was successfully reserved
	ostringstream convert;
	if(reserved)
		convert << "checkReservation: Land[" << index[0] << "][" << index[1] << "] successfully reserved on [" << (index[0] + nextMove[0]) << "][" << (index[1] + nextMove[1]) << "]";
	else if(!(nextMove[0] == 0 && nextMove[1] == 0))
		convert << "checkReservation: Land[" << index[0] << "][" << index[1] << "] could not reserve on [" << (index[0] + nextMove[0]) << "][" << (index[1] + nextMove[1]) << "]";
	else
		convert << "checkReservation: Land[" << index[0] << "][" << index[1] << "] could not decide a reservation.";
	MASS_base::log( convert.str( ) );
	
	return NULL;
}

/** (int*)									#DOESN'T WORK: ExchangeAll() return values will not cross boundaries
*	Exchange all version of making reservation, where next move was previously calculated.
*	@return:Returns int[3] {nextMove[0], nextMove[1], AgentID} if I have reservation to make, else NULL
*	@pre:	makeReservation must have been previously been called, or else no reservations will be made
*	@post:	Sets the reserve status to true if its reservation is successful
*/
void *Land::reservationExchange()
{
	int *retVal = NULL;
	//Return only if a next move is to be made
	if(!(nextMove[0] == 0 && nextMove[1] == 0))
	{
		retVal = new int[3];
		retVal[0] = index[0] + nextMove[0];
		retVal[1] = index[1] + nextMove[1];
		retVal[2] = myAgent;
	}
	
	//Debug logging: Should be almost identical to setMyAgent and makeReservation() combined
	ostringstream convert;
	if(retVal != NULL)
		convert << "reservationExchange: Land[" << index[0] << "][" << index[1] << "] reservation message {" << retVal[0] << ", " << retVal[1] << ", " << retVal[2] <<"}";
	else
		convert << "reservationExchange: Land[" << index[0] << "][" << index[1] << "] reservation message: No Reservation";
	MASS_base::log( convert.str( ) );
	return retVal;
}

/** (void)									#DOESN'T WORK: ExchangeAll() return values will not cross boundaries
*	Exchange all version of showing reservation
*	Shows the reservation status as 0 or agentID of the agent that can move to this place for the next turn
*	First, it reads from inMessages to see if any reservations have been made, and randomly selects one of them for display, or 0 if none exist.
*	@post:	The reservation made will be set as outMessage, or 0 if it hasn't been made.
*/
void *Land::showReservation2()
{
	//Read what's for reservation
	vector<int> reservations;
	int* cur;
	
	for(int i = 0; i < (int)inMessages.size(); i++)
	{
		cur = (int *)(inMessages[i]);
		
		//Add whatever is trying to reserve on me
		if(cur != NULL && cur[0] == index[0] && cur[1] == index[1])
		{
			reservations.push_back(cur[2]);
		}
	}
	
	outMessage_size = sizeof( int );
	outMessage = new int( );
	//If empty, outMessage is 0
	if(reservations.empty())
	{
		*(int *)outMessage = 0;
	}
	//Else, outMessage is a randomly selected reservation
	else
	{
		int r = rand() % reservations.size();
		*(int *)outMessage = reservations[r];
	}
	
	//Debug logging: Expects to have read a reservation of whichever agentID wishing to migrate to this Land or 0 if none
	ostringstream convert;
	convert << "showReservation2: Land[" << index[0] << "][" << index[1] << "] is reserved (" << *(int *)outMessage << ")";
	MASS_base::log( convert.str( ) );
	
	return NULL;
}

/** (void)																#LOG visually confirmed 160728
*	Record the outmessage of neighbors as population
*	pre: neighborPop is initialized and contains 4 -1s, outmessage of Lands are population
*/
void *Land::delegateCalculation()
{
	//Only calculate if delegated
	if(agents.size() == 0 || myAgent == -1)
		return NULL;
	
	//Record neighbor population as before
	int North[2] = {0, 1};
    int East[2]  = {1, 0};
    int South[2] = {0, -1};
    int West[2]  = {-1, 0};
	
	int *ptr = (int *)getOutMessage( 1, North );
    neighborPop[0] = ( ptr == NULL ) ? 0 : *ptr;
    ptr = (int *)getOutMessage( 1, East );
    neighborPop[1] = ( ptr == NULL ) ? 0 : *ptr;
    ptr = (int *)getOutMessage( 1, South );
    neighborPop[2] = ( ptr == NULL ) ? 0 : *ptr;
    ptr = (int *)getOutMessage( 1, West );
    neighborPop[3] = ( ptr == NULL ) ? 0 : *ptr;
	
	//Calculate nextmove as before
	vector<int> moveset;
	for(int i = 0; i < (int)cardinals.size(); i++)
	{
		if(neighborPop[i] == 0)
			moveset.push_back(i);
	}
	if(!moveset.empty())
	{
		int r = rand() % moveset.size();
		nextMove[0] = neighbor[moveset[r]][0];
		nextMove[1] = neighbor[moveset[r]][1];
	}
	
	return NULL;
}#ifndef NOMAD_H
#define NOMAD_H

#include "Agent.h"
#include "MASS_base.h"
#include <sstream> // ostringstream

//Used to toggle output in Nomad.h
const bool printOut = false;
//const bool printOut = true;

using namespace std;

class Nomad : public Agent {

 
 public:
  // define functionId's that will 'point' to the functions they represent.
  static const int agentInit_ = 0;
  static const int somethingFun_ = 1;
  static const int createChild_ = 2;
  static const int killMe_ = 3;
  static const int move_ = 4;
  static const int addData_ = 6;
  static const int move2_ = 7;
  static const int randomMove_ = 8;
  static const int calculateMoveset_ = 9;
  static const int delegateMove_ = 10;
  static const int moveDelegated_ = 11;
  static const int newTurn_ = 12;
  static const int delegateMove2_ = 13;
  static const int moveDelegated2_ = 14;
  static const int ravelledxy_ = 15;
  static const int printCurrentWithID_ = 16;

  static const int DEFAULT = -223;
  int y_size = DEFAULT;
  int x_size = DEFAULT;

  static const int cardinals[5][2];
  
   /**
   * Initialize a Nomad object
   */
  Nomad( void *argument ) : Agent( argument ) {
    if(printOut == true)
        MASS_base::log( "BORN!!" );
  };
  
  /**
   * the callMethod uses the function ID to determine which method to execute.
   * It is assumed the arguments passed in contain everything those 
   * methods need to run.
   */
  virtual void *callMethod( int functionId, void *argument ) {
    switch( functionId ) {
    case agentInit_: return agentInit( argument );
    case somethingFun_: return somethingFun( argument );
    case createChild_: return createChild( argument );
    case killMe_: return killMe( argument );
    case move_: return move( argument );
    case addData_: return addData( argument );
    case move2_: return move2( argument );
    case randomMove_ : return randomMove( argument );
    case printCurrentWithID_ : return printCurrentWithID( argument);
    case calculateMoveset_ : return calculateMoveset( argument );
	case delegateMove_: return delegateMove();
	case moveDelegated_: return moveDelegated();
	case newTurn_: return newTurn();												//#partion-space
	case delegateMove2_: return delegateMove2( argument );							//#partion-space
	case moveDelegated2_: return moveDelegated2( argument );						//#partion-space
	case ravelledxy_: return ravelledxy( argument );								//#get-raveled-coordinate
    }
    return NULL;
  };
  
  //int map( int initPopulation, vector<int> size, vector<int> index, Place* curPlace);
  virtual int map( int initPopulation, vector<int> size, vector<int> index, Place* curPlace);

 private:
  void *printCurrentWithID( void *argument );

  void *agentInit( void *argument );
  void *somethingFun( void *argument );
  void *createChild( void *argument );
  void *killMe( void *argument );
  void *move( void *argument );
  void *addData( void *argument );
  void *move2( void *argument );
  void *randomMove( void *arguments );
  void *calculateMoveset( void *arguments);
  void *delegateMove();							//Delegate random neighbor reservation to my Land
  void *moveDelegated();						//Move to the randomly reserved neighboring square
  void *newTurn();								//Reset migration data for the new turn
  void *delegateMove2( void *argument );		//Delegate move, partitioned movespace version taking Iteration# as argument
  void *moveDelegated2( void *argument );		//moveDelegated, partitioned movespace version taking Iteration# as argument
  void *ravelledxy( void *argument );			//Get the ravelled xy coordinate of the nomad, y-major #get-raveled-coordinate

  std::vector<int>	moveset;					//Moveset of directions to randomly walk
  int movesLeft;								//Moves remaining in the turn
  int iterationToAct;							//Which iteration in the turn to act 	#partion-space
};

#endif
#include "Nomad.h"
#include "Land.h"
#include "MASS_base.h"
#include <sstream>     // ostringstream
#include <math.h>      // ceil

//Toggles output for user program
//const bool printOutput = false;
const bool printOutput = true;

extern "C" Agent* instantiate( void *argument ) {
  return new Nomad( argument );
}

extern "C" void destroy( Agent *object ) {
  delete object;
}

/**
 * initialize a Nomad with the given argument.
 */
void *Nomad::agentInit( void *argument ) {
  ostringstream convert;
  if(printOutput == true){
      convert << "agentInit[" << agentId << "] called, argument = " << (char *)argument;
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

/**
*	Initialize the cardinals array
*/
const int Nomad::cardinals[5][2] = { {0,0}, {1,0}, {0,1}, {-1,0}, {0,-1} };

/**
 * Prints out a message.
 */
void *Nomad::somethingFun( void *argument ) {
  ostringstream convert;
  if(printOutput == true){
      convert << "somethingFun[" << agentId << "] called, argument = " << (char *)argument;
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

void *Nomad::printCurrentWithID( void *argument) {
	ostringstream convert;
	convert << "TOLD_TO_PRINT_PLACE, id = " << agentId << " current Location = [" << place->index[0] << "][" << place->index[1] << "]" << endl;
	MASS_base::log(convert.str());
	return NULL;
}

/**
 * creates a child agent from a parent agent.
 */
void *Nomad::createChild( void *argument ) {
  int nChildren = 0;
  vector<void *> *arguments = new vector<void *>;
  arguments->push_back( argument );
  arguments->push_back( argument );
  if ( agentId % 2 == 0 ) { // only spawn a child if my agent id is even
    nChildren = 2;
    spawn( nChildren, *arguments, 15 );
  }
  delete arguments;

  ostringstream convert;
  if(printOutput == true){
      convert << "createChild[" << agentId << "] called to spawn " << nChildren;
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

/**
 * Kill an unneeded agent.
 */
void *Nomad::killMe( void *argument ) {
  if ( agentId % 2 != 0 ) {
    kill( );
  }

  ostringstream convert;
  if(printOutput == true){
      convert << "killMe[" << agentId << "] called";
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

/**
 * Have an agent move to a new place.
 */
void *Nomad::move( void *argument ) {
  if ( agentId % 5 == 0 ) {
    vector<int> dest;
    dest.push_back( 0 );
    dest.push_back( 0 );
    migrate( dest );
  }

  ostringstream convert;
  if(printOutput == true){
      convert << "migrate[" << agentId << "] called";
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

/**
 * Add data to an agent to carry.
 */
void *Nomad::addData( void *argument ) {
  migratableDataSize = 24;
  migratableData = (void *)(new char[migratableDataSize]);
  bzero( migratableData, migratableDataSize );
  
  ostringstream convert;
  if(printOutput == true){
      convert << "my agent id = " << agentId;
  }

  memcpy( migratableData, (void *)( convert.str( ).c_str( ) ), 
          migratableDataSize );

  if(printOutput == true){
      convert << " dataSize = " << migratableDataSize;
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

/**
 * Another sample movement, this time to a specific destination.
 */
void *Nomad::move2( void *argument ) {

  int x = ( place->index[0] + 20 ) % 100;
  vector<int> dest;
  dest.push_back( x );
  dest.push_back( 0 );
  //int c = 0;
  //dest.push_back( cardinals[c][1] );
  migrate( dest );  

  ostringstream convert;
  if(printOutput == true){
      convert << "my agent(" << agentId << ") will move from "
	      << "[" << place->index[0] << "][" << place->index[1]
	      << "] to [" << x << "][0]";
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

/**
*	Overload for Agent::map -- ERROR: Agent::map is not a polymorphic function
Randomwalk spawns a single agent in spawn area for all defined spawn coordinates*/

int Nomad::map( int initPopulation, vector<int> size, vector<int> index, Place* curPlace)
{
	if (size.size() != index.size() ) return 0;
	for (int i = 0; i < size.size(); i++) {
		double max_in_i_demension = (size[i] / 2.0) + (size[i] * 0.1);
		double min_in_i_demension = (size[i] / 2.0) - (size[i] * 0.1);
		if (index[i] < min_in_i_demension || index[i] >= max_in_i_demension) {
			return 0;
		}
	}
	if (printOutput == true) {
		ostringstream convert;
		convert << "creating agent at " << index[0];
		for (int i = 1; i < size.size(); i++) {
			convert << ", " << index[i];
		}
		convert << std::endl;
		MASS_base::log( convert.str() );
	}
	return 1;
}

/**
 * Random move, walking the nomad in a random direction
 */
void *Nomad::randomMove( void *argument ) {

	if (x_size == DEFAULT || y_size == DEFAULT) {
		vector<int> sizeVectForPlace =  place->getSizeVect();
		x_size = sizeVectForPlace[0];
		y_size = sizeVectForPlace[1];
	}

	//Fill moveset and move
	moveset.clear();
	moveset.push_back(1);
	moveset.push_back(2);
	moveset.push_back(3);
	moveset.push_back(4);

	vector<int> dest;
	int x = -1, y = -1;
	do {
		if(moveset.size() == 0){
			//std::cerr << "Agent " << agentId << " cannot move from: " << place->index[0] << "," << place->index[1] << std::endl;
			return NULL;
		}
		int r = rand() % moveset.size();						//randomly walk in one of possible directions
		x = place->index[0] + cardinals[moveset[r]][0];
		y = place->index[1] + cardinals[moveset[r]][1];
		if(x < 0 || y < 0 || x >= x_size || y >= y_size) {
			moveset.erase(moveset.begin() + r);
		}
	}while(x < 0 || y < 0 || x >= x_size || y >= y_size);
	dest.push_back( x );
  	dest.push_back( y );
	migrate(dest);

  ostringstream convert;
  convert << "my agent(" << agentId << ") will walk from " << "[" << place->index[0] << "][" << place->index[1] << "] to [" << x << "][" << y << "]";
//std::cerr << convert.str() << std::endl;
  if(printOutput) { MASS_base::log( convert.str( ) ); }
  return NULL;
}

/** Populates moveset with a list of cardinals this agent can move to.
*	Provided the agent's current location and which space to move in.
*/
void *Nomad::calculateMoveset( void *argument ) {
	moveset.clear();
	moveset.push_back(1);
	moveset.push_back(2);
	moveset.push_back(3);
	moveset.push_back(4);
	return NULL;
}

/** (void)					#Visual logging confirmed 160710
*	Provide my Place myID so that it can reserve my next move
*/
void *Nomad::delegateMove()
{
	int *id = new int;
	*id = agentId;
	place->callMethod(Land::setMyAgent_, id);
	
	return NULL;
}

/** (void)					#UNTESTED
*	Move to the randomly reserved neighboring square
*/
void *Nomad::moveDelegated()
{
	ostringstream convert;
	
	vector<int> dest;
	int x = 0, y = 0;
	int *nextMove = (int*)(place->callMethod(Land::getMigrationReservation_, NULL));
	if(nextMove != NULL)
	{
		x = place->index[0] + nextMove[0];
		y = place->index[1] + nextMove[1];
		dest.push_back( x );
		dest.push_back( y );
		migrate(dest);
		
		convert << "Agent-" << agentId << " at [" << index[0] << ", " << index[1] << "] to move to [" << x << ", " << y << "]";
	}
	else
		convert << "Agent-" << agentId << " at [" << index[0] << ", " << index[1] << "] could not move!";

    MASS_base::log( convert.str( ) );
	
	return NULL;
}

/** (void)					#Visual logging confirmed 160728
*	Reset migration data for the new turn, including calculation for which iteration to act
*/
void *Nomad::newTurn()
{
	movesLeft = 1;
	
	int x = index[0];
	int y = index[1];
	iterationToAct = x % 3 + 3 * (y % 3);									//Select based on location-- partition by x % 3 + y % 3 * 3 = {0 to 8}
	
	return NULL;
}

/** (void)					#Visual logging confirmed 160728
*	Provide my Place myID so that it can reserve my next move
*	@param- argument:		An integer for the partition counter
*/
void *Nomad::delegateMove2( void *argument )
{
	//Check if iteration to act
	int *partitionCounter = (int*)argument;
	if(iterationToAct != *partitionCounter)
		return NULL;
	
	int *id = new int;
	*id = agentId;
	place->callMethod(Land::setMyAgent_, id);
	
	return NULL;
}

/** (void)					#Visual logging confirmed 160728
*	Move to the randomly reserved neighboring square
*/
void *Nomad::moveDelegated2( void *argument )
{
	//Check if iteration to act
	int *partitionCounter = (int*)argument;
	if(iterationToAct != *partitionCounter)
		return NULL;
	
	ostringstream convert;
	
	vector<int> dest;
	int x = 0, y = 0;
	int *nextMove = (int*)(place->callMethod(Land::getMigrationReservation_, NULL));
	if(nextMove != NULL)
	{
		x = place->index[0] + nextMove[0];
		y = place->index[1] + nextMove[1];
		dest.push_back( x );
		dest.push_back( y );
		migrate(dest);
		
		convert << "moveDelegated2: Agent-" << agentId << " at [" << place->index[0] << ", " << place->index[1] << "] to move to [" << x << ", " << y << "] a displacement of [" << nextMove[0] << ", " << nextMove[1] << "]";
	}
	else
		convert << "moveDelegated2: Agent-" << agentId << " at [" << place->index[0] << ", " << place->index[1] << "] could not move!";
    if(printOutput) { MASS_base::log( convert.str( ) ); }
	
	return NULL;
}

/** (int)								#untested
 * Return the ravelled xy coordinate of the agent
 */
void *Nomad::ravelledxy( void *argument ) {
  int *ret_val = new int;
  *ret_val = place->index[0] + 100 * place->index[1];
  return ret_val;
}
