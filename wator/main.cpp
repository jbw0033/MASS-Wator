#include "MASS.h"
#include "Ocean.h"
#include "Fish.h"
#include "Shark.h"
#include "Timer.h"	//Timer
#include <stdlib.h> // atoi
#include <unistd.h>
#include <vector>

const int iterPerTurn = 9;							//Partitioned movespaces are 9 squares large

Timer timer;

int main( int argc, char *args[] ) {

	// check that all arguments are present 
	// exit with an error if they are not
	if ( argc != 10 ) {
		cerr << "usage: ./main username password machinefile port nProc nThreads numTurns sizeX sizeY" << endl;
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
	const int sizeX = atoi( args[8] );
	const int sizeY = atoi( args[9] );
	const int myFishPopulation = 4;	//Population{RMsize, RMSpawn}
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
	Places *ocean = new Places( 1, "Ocean", 1, msg, 6, 2, sizeX, sizeY );

	// define the destinations, which represent the Places 
	// adjacent to a particular place (represented by [0, 0].
	//        [0, 1]                 [ north]
	// [-1, 0][0, 0][1, 0]  == [west][origin][east]
	//        [0,-1]                 [ south]
	// Each X is represent by an array containing its coordinates.
	// Note that you can have an arbritrary number of destinations.  For example,
	// northwest would be [-1,1].
	vector<int*> destinations;
	int north[2] = {0, 1};  destinations.push_back( north );
	int east[2]  = {1, 0};  destinations.push_back( east );
	int south[2] = {0, -1}; destinations.push_back( south );
	int west[2]  = {-1, 0}; destinations.push_back( west );


	// prepare a message for the fish (this is argument below)
		msg = (char *) ("swim\0");

	// Create the agents.
	// Arguments are, in order:
	//    handle, className, *argument, argument_size, *places,  initPopulation
	Agents *fish = new Agents( 2, "Fish", msg, 5, ocean, myFishPopulation);
  
	ocean->callAll(Ocean::init_);				//initialize Oceans
  
	timer.start();
		
	cout << "There are " << fish->nAgents() << " fish in the ocean" << endl;
	ocean->callAll(Ocean::popCount_);
	for(int turn = 0; turn < numTurns; turn++)
	{
		fish->callAll( Fish::newTurn_ );	//Reset movecount and iteration to act
		
		//Iterations
		for(int i = 0; i < iterPerTurn; i++)
		{
			int* iter = new int;
			*iter = i;

			// reset the information on each place for the next location ordered move.
			ocean->callAll( Ocean::resetForNextTurn_ );

			fish->callAll( Fish::delegateMove_, (void*) iter, sizeof(int) );	//Delegate
			
			ocean->callAll(Ocean::displayPopAsOut_);	// display the current population
			ocean->exchangeBoundary();		// exchange shadow space information
			ocean->callAll(Ocean::delegateCalculation_);
			
			fish->callAll( Fish::moveDelegated_, (void*) iter, sizeof(int) );	//Read delegate and move

			fish->manageAll( );	//Actual move the agent. Here moves agents calculated each iteration
			
		}
//		if (printOutput && (numTurns >= 4) && (turn % (numTurns / 4) == 0)) {
//			int* currentTurn = new int;
//			*currentTurn = turn;
//			std::cerr << "Printing Agent Positions for turn " << turn << std::endl;
//			fish->callAll( Fish::printCurrentPositionWithID_, (void*) currentTurn, sizeof(int) );
//			std::cerr << "Finished printing agent positions" << std::endl;
//		}

		fish->callAll(Fish::createChild_, (void*) msg, 4);
	}
		
	std::cerr << "Agent ending positions after " << numTurns << std::endl;
	int* maxTurns = new int;
	*maxTurns = numTurns;
	fish->callAll( Fish::printCurrentPositionWithID_, (void*) maxTurns, sizeof(int) );
//	shark->callAll( Shark::printCurrentPositionWithID_, (void*) maxTurns, sizeof(int) );
	
	long elaspedTime_END =  timer.lap();
	printf( "\nEnd of simulation. Elasped time using MASS framework with %i processes and %i thread and %i turns :: %ld \n",nProc,nThr,numTurns, elaspedTime_END);
  
	MASS::finish( );
}
