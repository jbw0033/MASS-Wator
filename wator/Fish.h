#ifndef FISH_H
#define FISH_H

#include "Agent.h"
#include "MASS_base.h"

using namespace std;

class Fish : public Agent {

public:
	// define functionId's that will 'point' to the functions they represent.
	static const int agentInit_ = 0;
	static const int createChild_ = 1;
	static const int killMe_ = 2;
	static const int calculateMoveset_ = 3;
	static const int delegateMove_ = 4;
	static const int moveDelegated_ = 5;
	static const int newTurn_ = 6;
	static const int printCurrentPositionWithID_ = 7;

	// define the cardinals, which represent the Places
	// adjacent to a particular place (represented by [0, 0].
	//        [0, 1]                 [ north]
	// [-1, 0][0, 0][1, 0]  == [west][origin][east]
	//        [0,-1]                 [ south]
	// Each X is represent by an array containing its coordinates.
	// Note that you can have an arbritrary number of destinations.  For example,
	// northwest would be [-1,1].
	static const int cardinals[5][2];

	/**
	 * Initialize a Fish object
	 */
	Fish( void *argument ) : Agent( argument ) {
		cout << "Fish BORN!! with ID " << agentId << endl;
	};

	/**
	 * the callMethod uses the function ID to determine which method to execute.
	 * It is assumed the arguments passed in contain everything those
	 * methods need to run.
	 */
	virtual void *callMethod( int functionId, void *argument ) {
		switch( functionId ) {
		case agentInit_: return agentInit( argument );
		case printCurrentPositionWithID_: return printCurrentWithID( argument );
		case createChild_: return createChild( argument );
		case killMe_: return killMe( argument );
		case calculateMoveset_ : return calculateMoveset( argument );
		case delegateMove_: return delegateMove( argument );
		case moveDelegated_: return moveDelegated( argument );
		case newTurn_: return newTurn();
		}
		return NULL;
	};

	/**
	 *	Overload for Agent::map
	 */
	///using Agent::map; --Overload. ERROR: Agent::map is not a polymorphic virtual function
	//virtual int map( int initPopulation, vector<int> size, vector<int> index, Place* curPlace);
	//NEEDS TO BE VIRTUAL
	//int map( int initPopulation, vector<int> size, vector<int> index, Place* curPlace);
	virtual int map( int initPopulation, vector<int> size, vector<int> index, Place* curPlace);

private:
	void *printCurrentWithID( void *argument );
	void *agentInit( void *argument );
	void *createChild( void *argument );
	void *killMe( void *argument );
	void *calculateMoveset( void *arguments);
	void *delegateMove(void* arguments);							//Delegate random neighbor reservation to my Land
	void *moveDelegated(void* arguments);						//Move to the randomly reserved neighboring square
	void *newTurn();								//Reset migration data for the new turn

	std::vector<int>	moveset;					//Moveset of directions to randomly walk
	int movesLeft;								//Moves remaining in the turn
	int iterationToAct;							//Which iteration in the turn to act 	#partion-space
	int newChildTime;
};

#endif

