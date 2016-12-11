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
 * callalltest causes all Land's to report their location and contents.
 */
void *Land::callalltest( void *argument ) {
  ostringstream convert;
  
  double *ret_val = new double;
  *ret_val = *(int *)argument * 10.0;
  
  if(printOutput == true){
      convert << "callalltest[" << index[0] << "][" << index[1] << "] out of ["
	      << size[0] << "][" << size[1] << "]" << arg << ", " 
	      << *(int  *)argument
	      << " *(double *)ret_val = " << *ret_val;
      MASS_base::log( convert.str( ) );
  }
  return ret_val;
}

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
	
	 //Set up 0th inMessage box for reservation. #doesn't help
	 //inMessage_size = sizeof( int );
	 //inMessages.clear();																//clear, or else messages out of date
	
	//Debug logging: Expects all land are now blind of occupancy of neighbors
	//ostringstream convert;
	//convert << "resetForNextTurn:  Land[" << index[0] << "][" << index[1] << "]'s north = " << neighborPop[0] << ", east = " << neighborPop[1] << ", south = " << neighborPop[2] << ", west = " << neighborPop[3];
	//MASS_base::log( convert.str( ) );
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
	
	//Debug logging: Expected 400 or nxn number of lands have a single agent on them
	//ostringstream convert;
	//convert << "displayPopAsOut:  Land[" << index[0] << "][" << index[1] << "] has the population of " << *((int *)outMessage);
	//MASS_base::log( convert.str( ) );
	
	return NULL;
}

/** (void)																#Visual logging confirmed 160710
*	Record the outmessage of neighbors as population
*	pre: neighborPop is initialized and contains 4 -1s
*/
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
	
	//Debug logging: Expects id to match the agentId on corresponding coordinate
	//ostringstream convert;
	//convert << "setMyAgent: Land[" << index[0] << "][" << index[1] << "] agent is (" << myAgent << ")";
	//MASS_base::log( convert.str( ) );
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
		/*
		//int position = 0;				//Default, write to 0				//ERROR: PutInMessages() breaks if put into if-else statements
		if(nextMove[0] < index[0])			//West
			putInMessage( 1, nextMove, 3, arg );	//Put a reservation message into the 3rd inMessage of my selected neighbor //ERROR: Does not work as documented: cannot cross ranks
		else if(nextMove[0] > index[0])		//East
			putInMessage( 1, nextMove, 1, arg );	//Put a reservation message into the 1st inMessage of my selected neighbor //ERROR: Does not work as documented: cannot cross ranks
		else
			putInMessage( 1, nextMove, 0, arg );	//Put a reservation message into the 0th inMessage of my selected neighbor //ERROR: Does not work as documented: cannot cross ranks*/
		//putInMessage( 1, nextMove, 0, arg );
		
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
	/*
	//inMessage_size = sizeof( int );
	int *reservation = NULL;
	if(!inMessages.empty())
	{
		
		vector<int> reservations;
		//int max = (inMessages.size() > NUM_NEIGHBORS) ? NUM_NEIGHBORS : inMessages.size();
		for(int i = 0; i < (int)inMessages.size(); i++)
		{
			if(inMessages[i] != NULL)
			{
				reservations.push_back(*(int*)inMessages[i]);
			}
		}
		if(!reservations.empty())
		{
			int r = rand() % reservations.size();
			*reservation = reservations[r];
		}
		
		//reservation = (int *)(inMessages[5]);		//Read reservation from in message if any	 //ERROR: Does not work as documented: cannot allocate cross ranks
		
		//DEBUG
		ostringstream convert;convert << "Read a " << *reservation;MASS_base::log( convert.str( ) );
	}
	
	//Set corresponding reservation status in outMessage;
	if(reservation == NULL)
	{
		outMessage_size = sizeof( int );
		outMessage = new int( );
		*(int *)outMessage = 0;
	}
	else
	{
		outMessage_size = sizeof( int );
		outMessage = new int( );
		*(int *)outMessage = *reservation;
		//inMessages[0] = NULL;
		
		
		//clear #doesn't help
		//int *ptr = new int;
		// *ptr = 0;
		//inMessages[0] = (void*)ptr;
	}*/
	
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
	
	//Debug logging: Expects neighbor populations and choose an unoccupied neighbor for nextMove
	/*
	ostringstream convert;
	convert << "delegateCalculation:  Land[" << index[0] << "][" << index[1] << "]'s north = " << neighborPop[0] << ", east = " << neighborPop[1] << ", south = " << neighborPop[2] << ", west = " << neighborPop[3];
	if(!(nextMove[0] == 0 && nextMove[1] == 0))
		convert << " and thus will select the neighbor[" << (index[0] + nextMove[0]) << ", " << (index[1] + nextMove[1]) << "]";
	else
		convert << "and thus could not make selection.";
	MASS_base::log( convert.str( ) );*/
	
	return NULL;
}