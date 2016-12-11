#include "Fish.h"
#include "MASS_base.h"
#include <math.h>      // ceil
#include "Ocean.h"

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
//	Fish::popCount = 0;
	newChildTime = 5;
  cout << "agentInit[Fish " << agentId << "] called, argument = " << (char *)argument;
  return NULL;
}

/**
*	Initialize the cardinals array
*/
const int Fish::cardinals[5][2] = { {0,0}, {1,0}, {0,1}, {-1,0}, {0,-1} };

void *Fish::printCurrentWithID( void *argument) {
	int* turnPtr = (int*)argument;

	cout << "Turn Fish " << *turnPtr << " agent " << agentId << " current Location = [" << place->index[0] << "][" << place->index[1] << "]" << endl;
	return NULL;
}

/**
 * creates a child agent from a parent agent.
 */
void *Fish::createChild( void *argument ) {
  int nChildren = 0;
  vector<void *> *arguments = new vector<void *>;
  arguments->push_back( argument );
  if ( Fish::newChildTime < 1) { // only spawn a child if the fish has been alive 5 time steps
    nChildren = 1;
    newChildTime = 5;
    spawn( nChildren, *arguments, 5 );
  }
  delete arguments;

  cout << "child in " << newChildTime << " steps ";
  cout << "createChild[Fish " << agentId << "] called to spawn " << nChildren << " childern "<< endl;

  return NULL;
}

/**
 * Kill an unneeded agent.
 */
void *Fish::killMe( void *argument ) {
	if(place->agents.size() > 1) {
		kill( );
		cout << "killMe[Fish " << agentId << "] called";
	}
  return NULL;
}

/**
*	Overload for Agent::map -- ERROR: Agent::map is not a polymorphic function
Randomwalk spawns a single agent in spawn area for all defined spawn coordinates*/

int Fish::map( int initPopulation, vector<int> size, vector<int> index, Place* curPlace)
{

	cout << "Fish " << agentId << " is inside map function." << endl;

	if (size.size() != index.size()) return 0;
	for (int i = 0; (unsigned)i < size.size(); i++) {

		int spotCount = (index[0] * size[0]) + index[1];
		int goalCount = (size[0] * size[1]) / initPopulation;

		if(spotCount % goalCount == 0 && index[0] + index[1] != 0) {
			return 1;
		}
		else if((size[0] * size[1]) % initPopulation == 0 && spotCount + 1 == size[0] * size[1]) {
			return 1;
		}
	}

	return 0;
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
//	iterationToAct = 1;	//Select based on location-- partition by x % 3 + y % 3 * 3 = {0 to 8}

	int x = index[0];
		int y = index[1];
		iterationToAct = x % 3 + 3 * (y % 3);

	cout << "Fish Agent " << agentId << " In newTurn" << endl;

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

	cout << "Fish Agent " << agentId << " in delegate move to " << place->index[0] << " " << place->index[1] << endl;

	return NULL;
}

/** (void)
*	Move to the randomly reserved neighboring square
*/
void *Fish::moveDelegated( void *argument )
{

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

		cout << "moveDelegated: Fish Agent-" << agentId << " at [" << place->index[0];
		cout << ", " << place->index[1] << "] to move to [" << x << ", ";
		cout << y << "] a displacement of [" << nextMove[0] << ", " << nextMove[1] << "]" << endl;
	}
	else {
		cout << "moveDelegated: Fish Agent-" << agentId << " at [" << place->index[0] << ", " << place->index[1] << "] could not move!" << endl;
	}

	newChildTime--;


	return NULL;
}
