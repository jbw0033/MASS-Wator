#include "Agent.h"
#include "MASS_base.h"
#include <iostream>
#include <sstream>     // ostringstream

/**
 * Spawns a “numAgents’ of new agents, as passing arguments[i] (with arg_size) 
 * to the i-th new agent upon a next call to Agents.manageAll( ).
 * More specifically, spawn( ) changes the calling agent’s newChildren.
 * @param numAgents
 * @param arguments
 * @param arg_size
 */
void Agent::spawn( int numAgents, vector<void*> arguments, int arg_size){

	//Only want to make changes if the number to be created is above zero
	if(numAgents > 0){
		newChildren = numAgents;
		this->arguments = arguments;
	}
}

/**
 * Initiates an agent migration upon a next call to Agents.manageAll( ). More
 * specifically, migrate( ) updates the calling agent’s index[].
 * @param index
 * @return 
 */
bool Agent::migrate( vector<int> index ){
	vector<int> sizeVectors;
	sizeVectors = place->getSizeVect();
	for(int i = 0; (unsigned)i < sizeVectors.size(); i++){
		if(index[i] >= 0 && index[i] < sizeVectors[i])
			continue;
		else
			return false;
	}
	this->index = index;
	return true;
}
