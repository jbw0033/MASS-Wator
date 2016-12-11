#include "Fish.h"
#include "MASS_base.h"
#include <sstream>     // ostringstream
#include <math.h>      // ceil
#include "Ocean.h"

//Toggles output for user program
//const bool printOutput = false;
const bool printOutput = true;

extern "C" Agent* instantiate( void *argument ) {
  return new Fish( argument );
}

extern "C" void destroy( Agent *object ) {
  delete object;
}

/**
 * initialize a Fish with the given argument.
 */
void *Fish::agentInit( void *argument ) {
  ostringstream convert;
  if(printOutput == true){
//      convert << "agentInit[Fish " << agentId << "] called, argument = " << (char *)argument;
//      MASS_base::log( convert.str( ) );
  }

  cout << "agentInit[Fish " << agentId << "] called, argument = " << (char *)argument;
  return NULL;
}

/**
*	Initialize the cardinals array
*/
const int Fish::cardinals[5][2] = { {0,0}, {1,0}, {0,1}, {-1,0}, {0,-1} };

/**
 * Prints out a message.
 */
void *Fish::somethingFun( void *argument ) {
  ostringstream convert;
  if(printOutput == true){
//      convert << "somethingFun[Fish " << agentId << "] called, argument = " << (char *)argument;
//      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

void *Fish::printCurrentWithID( void *argument) {
	int* turnPtr = (int*)argument;
	ostringstream convert;
//	convert << "Turn Fish " << *turnPtr << " agent " << agentId << " current Location = [" << place->index[0] << "][" << place->index[1] << "]" << endl;
//	MASS_base::log(convert.str());

	cout << "Turn Fish " << *turnPtr << " agent " << agentId << " current Location = [" << place->index[0] << "][" << place->index[1] << "]" << endl;
	return NULL;
}

/**
 * creates a child agent from a parent agent.
 */
void *Fish::createChild( void *argument ) {
//  int nChildren = 0;
//  vector<void *> *arguments = new vector<void *>;
//  arguments->push_back( argument );
//  arguments->push_back( argument );
//  if ( agentId % 2 == 0 ) { // only spawn a child if my agent id is even
//    nChildren = 2;
//    spawn( nChildren, *arguments, 15 );
//  }
//  delete arguments;

//  ostringstream convert;
//  if(printOutput == true){
//      convert << "createChild[Fish " << agentId << "] called to spawn " << nChildren;
//      MASS_base::log( convert.str( ) );
//  }
  return NULL;
}

/**
 * Kill an unneeded agent.
 */
void *Fish::killMe( void *argument ) {
  if ( agentId % 2 != 0 ) {
    kill( );
  }

  ostringstream convert;
  if(printOutput == true){
//      convert << "killMe[Fish " << agentId << "] called";
//      MASS_base::log( convert.str( ) );
  }

  cout << "killMe[Fish " << agentId << "] called";
  return NULL;
}

//int *Fish::getId(void *argument) {
//	return agentId;
//}

/**
 * Call all agents and have them report their arguments.
 */
void *Fish::callalltest( void *argument ) {
  ostringstream convert;
  if(printOutput == true){
//      convert << "callalltest: fish agent(" << agentId;
//      MASS_base::log( convert.str( ) );
  }
  double *ret_val = new double;
  *ret_val = *(int *)argument * 10.0;

  if(printOutput == true){
      convert.str( "" );
//      convert << "callalltest: fish agent(" << agentId << "): argument = "
//              << *(int  *)argument
//              << " *(double *)ret_val = " << *ret_val;
//      MASS_base::log( convert.str( ) );
  }
  return ret_val;
}


/**
 * Add data to an agent to carry.
 */
void *Fish::addData( void *argument ) {
  migratableDataSize = 24;
  migratableData = (void *)(new char[migratableDataSize]);
  bzero( migratableData, migratableDataSize );
  
  ostringstream convert;
  if(printOutput == true){
//      convert << "my fish agent id = " << agentId;
  }

  memcpy( migratableData, (void *)( convert.str( ).c_str( ) ), 
          migratableDataSize );

  if(printOutput == true){
//      convert << " dataSize = " << migratableDataSize;
//      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

/**
*	Overload for Agent::map -- ERROR: Agent::map is not a polymorphic function
Randomwalk spawns a single agent in spawn area for all defined spawn coordinates*/

int Fish::map( int initPopulation, vector<int> size, vector<int> index, Place* curPlace)
{

	ostringstream convert;
	if (printOutput) {
//		convert << "inside map function";
//		MASS_base::log(convert.str());
	}
	if (size.size() != index.size() ) return 0;
	for (int i = 0; (unsigned)i < 1; i++) {

		int sizeI = size[i];
		int indexI = index[i];

		double max_in_i_demension = (static_cast<double>(sizeI) / 2.0) + (static_cast<double>(sizeI) * 0.1);
		double min_in_i_demension = (static_cast<double>(sizeI) / 2.0) - (static_cast<double>(sizeI) * 0.1);

		if (static_cast<double>(indexI) < min_in_i_demension || static_cast<double>(indexI) >= max_in_i_demension) {
			return 0;
		}
	}
	if (printOutput == true) {
//		convert.str("");
//		convert << "creating fish agent at " << index[0];
//		for (int i = 1; i < size.size(); i++) {
//			convert << ", " << index[i];
//		}
//		convert << std::endl;
//		MASS_base::log( convert.str() );
	}

	cout << "Population should be " << initPopulation;
	cout << "creating fish agent at " << index[0] << " " << index[1] << endl;
	return 1;
}

/** Populates moveset with a list of cardinals this agent can move to.
*	Provided the agent's current location and which space to move in.
*/
void *Fish::calculateMoveset( void *argument ) {
	moveset.clear();
	moveset.push_back(1);
	moveset.push_back(2);
	moveset.push_back(3);
	moveset.push_back(4);
	return NULL;
}

/** (void)
*	Reset migration data for the new turn, including calculation for which iteration to act
*/
void *Fish::newTurn()
{
	movesLeft = 1;
	
	int x = index[0];
	int y = index[1];
	iterationToAct = x % 3 + 3 * (y % 3);	//Select based on location-- partition by x % 3 + y % 3 * 3 = {0 to 8}
	
	cout << "Fish Agent" << agentId << "In newTurn" << endl;

	return NULL;
}

/** (void)					#Visual logging confirmed 160728
*	Provide my Place myID so that it can reserve my next move
*	@param- argument:		An integer for the partition counter
*/
void *Fish::delegateMove( void *argument )
{
	//Check if iteration to act
	int *partitionCounter = (int*)argument;
	if(iterationToAct != *partitionCounter)
		return NULL;
	
	int *id = new int;
	*id = agentId;
	place->callMethod(Ocean::setMyAgent_, id);
	
	cout << "Fish Agent " << agentId << "in delegate move" << endl;

	return NULL;
}

/** (void)
*	Move to the randomly reserved neighboring square
*/
void *Fish::moveDelegated( void *argument )
{
	//Check if iteration to act
	int *partitionCounter = (int*)argument;
	if(iterationToAct != *partitionCounter) {
		return NULL;
	}
	
	ostringstream convert;
	if (printOutput) {
//		MASS_base::log("inside moveDelegated");
	}
	vector<int> dest;
	int x = 0, y = 0;
	int *nextMove = (int*)(place->callMethod(Ocean::getMigrationReservation_, NULL));
	if(nextMove != NULL)
	{
		x = place->index[0] + nextMove[0];
		y = place->index[1] + nextMove[1];
		dest.push_back( x );
		dest.push_back( y );
		migrate(dest);
		
//		convert << "moveDelegated: Fish Agent-" << agentId << " at [" << place->index[0];
//		convert << ", " << place->index[1] << "] to move to [" << x << ", ";
//		convert << y << "] a displacement of [" << nextMove[0] << ", " << nextMove[1] << "]";
	}
	else {
//		convert << "moveDelegated: Fish Agent-" << agentId << " at [" << place->index[0] << ", " << place->index[1] << "] could not move!";
	}
	if(printOutput) { 
		MASS_base::log( convert.str( ) ); 
	}
	
	return NULL;
}

/** (int)								#untested
 * Return the ravelled xy coordinate of the agent
 */
void *Fish::ravelledxy( void *argument ) {
  int *ret_val = new int;
  *ret_val = place->index[0] + 100 * place->index[1];
  return ret_val;
}
