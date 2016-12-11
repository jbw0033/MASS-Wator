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
  static const int callalltest_ = 1;
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
    case callalltest_: return callalltest( argument );
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
  void *callalltest( void *argument );
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
