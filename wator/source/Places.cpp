#include "Places.h"
#include "MASS.h"
#include "Message.h"
#include "Mthread.h"
#include <iostream>
#include <dlfcn.h> // dlopen, dlsym, and dlclose

//Used to toggle comments from Places.cpp
const bool printOutput = false;
//const bool printOutput = true;

/**
 * Instantiates a shared array with "size[]" from the "className" class as
 * passing an argument to the "className" constructor. This array is
 * associated with a user-given handle that must be unique over
 * machines.
 * dimensions are numerated in the "..." format.
 * @param handle - A unique identifer that designates a group of places.  
 *                 Must be unique over all machines.
 * @param className - the user implemented class the places are constructed from
 * @param argument
 * @param argument_size
 * @param dim
 * @param ...
 */
Places::Places( int handle, string className, void *argument, 
		int argument_size, int dim, ... )
  : Places_base( handle, className, 0, argument, argument_size, dim, NULL ) {

  size = new int[dim];
  // Extract each dimension's length
  va_list list;
  va_start( list, dim );

  for ( int i = 0; i < dim; i++ ) {
    size[i] = va_arg( list, int );
  }
  va_end( list );

  init_all( argument, argument_size ); // explicitly call Places_base.init_all
  init_master( argument, argument_size, 0 );
}

/**
 * Instantiates a shared array with "size[]" from the "className" class as
 * passing an argument to the "className" constructor. This array is
 * associated with a user-given handle that must be unique over
 * machines.
 * @param handle - A unique identifer that designates a group of places.  
 *                 Must be unique over all machines.
 * @param className - the user implemented class the places are constructed from
 * @param argument
 * @param argument_size
 * @param dim
 * @param size
 */
Places::Places( int handle, string className, void *argument,
		int argument_size, int dim, int size[] )
  : Places_base( handle, className, 0, argument, argument_size, dim, size ) {

  // init_all called within Places_base
  init_master( argument, argument_size, 0 );
}

/**
 * Places constructor that creates places with a given dimension.
 * @param handle - A unique identifer that designates a group of places.  
 *                 Must be unique over all machines.
 * @param className - the user implemented class the places are constructed from
 * @param boundary_width
 * @param argument
 * @param argument_size
 * @param dim
 * @param ...
 */
Places::Places( int handle, string className, int boundary_width, 
		void *argument, int argument_size,  int dim, ... )
  : Places_base( handle, className, boundary_width, argument, argument_size, 
		 dim, NULL ) {
  
  size = new int[dim];
  // Extract each dimension's length
  va_list list;
  va_start( list, dim );

  for ( int i = 0; i < dim; i++ ) {
    size[i] = va_arg( list, int );
  }
  va_end( list );

  init_all( argument, argument_size ); // explicitly call Places_base.init_all
  init_master( argument, argument_size, boundary_width );
}

/**
 * Places constructor that creates places with a given dimension and size.
 * @param handle - A unique identifer that designates a group of places.
 *                 Must be unique over all machines.
 * @param className - the user implemented class the places are constructed from
 * @param boundary_width
 * @param argument
 * @param argument_size
 * @param dim
 * @param size
 */
Places::Places( int handle, string className, int boundary_width,
		void *argument,	int argument_size, int dim, int size[] )
  : Places_base( handle, className, boundary_width, argument, argument_size, 
		 dim, size ) {

  // init_all called within Places_base
  init_master( argument, argument_size, boundary_width );
}

/**
 * Initializes the places with the given arguments and boundary width.
 * @param argument
 * @param argument_size
 * @param boundary_width
 */
void Places::init_master( void *argument, int argument_size, 
			  int boundary_width ) {

  // convert size[dimension] to vector<int>
  vector<int> *size_vector = new vector<int>(dimension);
  size_vector->assign( size, size + dimension );
  
  // create a list of all host names;  
  // the master IP name
  char localhost[100];
  bzero( localhost, 100 );
  gethostname( localhost, 100 );
  vector<string> hosts;
  hosts.push_back( *( new string( localhost ) ) );

  // all the slave IP names
  for ( int i = 0; i < int( MASS::mNodes.size( ) ); i++ ) {
    hosts.push_back( MASS::mNodes[i]->getHostName( ) );
  }

  // create a new list for message
  vector<string> *tmp_hosts = new vector<string>( hosts );


  Message *m = new Message( Message::PLACES_INITIALIZE, size_vector,
			    handle, className,
			    argument, argument_size, boundary_width, 
			    tmp_hosts );
  
  // send a PLACES_INITIALIZE message to each slave
  for ( int i = 0; i < int( MASS::mNodes.size( ) ); i++ ) {
    MASS::mNodes[i]->sendMessage( m );

    if(printOutput == true){
        cerr << "PLACES_INITIALIZE sent to " << i << endl;
    }
  }
  delete m;

  // establish all inter-node connections within setHosts( )
  MASS_base::setHosts( hosts );

  // register this places in the places hash map
  MASS_base::placesMap.
    insert( map<int, Places_base*>::value_type( handle, this ) );

  // Synchronized with all slave processes
  MASS::barrier_all_slaves( );
}

/**
 * Calls the method specified with functionId of all array elements. Done
 * in parallel among multi-processes/threads.
 * @param functionId
 */
void Places::callAll( int functionId ) {
  ca_setup( functionId, NULL, 0, 0, Message::PLACES_CALL_ALL_VOID_OBJECT );
}

/**
 * Calls the method specified with functionId of all array elements as
 * passing an argument to the method. Done in parallel among multi-
 * processes/threads.
 * @param functionId
 * @param argument
 * @param arg_size
 */
void Places::callAll( int functionId, void *argument, int arg_size ) {

  if(printOutput == true){
      cerr << "callAll void object" << endl;
  }

  ca_setup( functionId, argument, arg_size, 0, // ret_size = 0
	    Message::PLACES_CALL_ALL_VOID_OBJECT );
}

/**
 * Calls the method specified with functionId of all array elements as
 * passing arguments[i] to element[i]’s method, and receives a return
 * value from it into (void *)[i] whose element’s size is return_size. Done 
 * in parallel among multi-processes/threads. In case of a multi-
 * dimensional array, "i" is considered as the index when the array is
 * flattened to a single dimension.
 * @param functionId
 * @param argument
 * @param arg_size
 * @param ret_size
 * @return 
 */
void *Places::callAll( int functionId, void *argument[], int arg_size,
		       int ret_size ) {

  if(printOutput == true){
      cerr << "callAll return object" << endl;
  }

  return ca_setup( functionId, (void *)argument, arg_size, ret_size,
		   Message::PLACES_CALL_ALL_RETURN_OBJECT );
}

/**
 * 
 * @param functionId
 * @param argument
 * @param arg_size
 * @param ret_size
 * @param type
 * @return 
 */
void *Places::ca_setup( int functionId, void *argument,
			int arg_size, int ret_size,
			Message::ACTION_TYPE type ) {
  // calculate the total argument size for return-objects
  int total = 1; // the total number of place elements
  for ( int i = 0; i < dimension; i++ )
    total *= size[i];
  int stripe = total / MASS_base::systemSize;

  // send a PLACES_CALLALL message to each slave
  Message *m = NULL;
  for ( int i = 0; i < int( MASS::mNodes.size( ) ); i++ ) {
    // create a message
    if ( type == Message::PLACES_CALL_ALL_VOID_OBJECT )
      m = new Message( type, this->handle, functionId, argument, arg_size,
		       ret_size );
    else { // PLACES_CALL_ALL_RETURN_OBJECT
      m = new Message( type, this->handle, functionId, 
		       // argument body
		       (char *)argument + arg_size * stripe * ( i + 1 ),
		       // argument size
		       ( i == int( MASS::mNodes.size( ) ) - 1 ) ?
		       arg_size * ( total - stripe * ( i + 1 ) ) : // + rmdr
		       arg_size * stripe,
		       ret_size ); // no remainder   

      if(printOutput == true){
          cerr << "Places.callAll: arg_size = " << arg_size 
	       << " stripe = " << stripe << " i + 1 = " << (i + 1) << endl;
      }
      /*
      int *data = (int *)((char *)argument + arg_size * stripe * ( i + 1 ));
      for ( int i = 0; i < stripe; i++ )
	cerr << *(data + i) << endl;
      */
    }

    // send it
    MASS::mNodes[i]->sendMessage( m );

    if(printOutput == true){
        cerr << "PLACES_CALL_ALL " << m->getAction( ) <<  " sent to " << i << endl;
    }

    // make sure to delete it
    delete m;
  }

  // retrieve the corresponding places
  MASS_base::currentPlaces = this;
  MASS_base::currentFunctionId = functionId;
  MASS_base::currentArgument = argument;
  MASS_base::currentArgSize = arg_size;
  MASS_base::currentMsgType = type;
  MASS_base::currentRetSize = ret_size;
  MASS_base::currentReturns = 
    ( type == Message::PLACES_CALL_ALL_VOID_OBJECT ) ?
    NULL :
    new char[total * MASS_base::currentRetSize]; // prepare an entire return space

  // resume threads
  Mthread::resumeThreads( Mthread::STATUS_CALLALL );

  // callall implementation
  if ( type == Message::PLACES_CALL_ALL_VOID_OBJECT )
    Places_base::callAll( functionId, argument, 0 ); // 0 = the main thread id
  else
    Places_base::callAll( functionId, (void *)argument, arg_size, ret_size, 0);

  // confirm all threads are done with callAll.
  Mthread::barrierThreads( 0 );
  
  // Synchronized with all slave processes
  if ( type == Message::PLACES_CALL_ALL_VOID_OBJECT 
	|| type == Message::PLACES_CALL_ALL_RETURN_OBJECT )
    MASS::barrier_all_slaves( MASS_base::currentReturns, stripe, 
			      MASS_base::currentRetSize );
  else
    MASS::barrier_all_slaves( );    

  return (void *)MASS_base::currentReturns;
}

/**
 * Calls from each of all cells to the method specified with functionId of
 * all destination cells, each indexed with a different Vector element.
 * Each vector element, say destination[] is an array of integers where
 * destination[i] includes a relative index (or a distance) on the coordinate
 * i from the current caller to the callee cell. The caller cell’s outMessage
 * is a continuous set of arguments passed to the callee’s method. The
 * caller’s inMessages[] stores values returned from all callees. More
 * specifically, inMessages[i] maintains a set of return values from the i th
 * callee.
 * @param dest_handle
 * @param functionId
 * @param destinations
 */
void Places::exchangeAll( int dest_handle, int functionId, 
			  vector<int*> *destinations ) {

  // send a PLACES_EXCHANGE_ALL message to each slave
  Message *m = new Message( Message::PLACES_EXCHANGE_ALL, 
			    this->handle, dest_handle, functionId, 
			    destinations, this->dimension );

  if(printOutput == true){
      cerr << "dest_handle = " << dest_handle << endl;
  }

  for ( int i =0; i < int ( MASS::mNodes.size( ) ); i++ ) {
    MASS::mNodes[i]->sendMessage( m );
  }
  delete m;
 
  // retrieve the corresponding places
  MASS_base::currentPlaces = this;
  MASS_base::destinationPlaces = MASS_base::placesMap[dest_handle];
  MASS_base::currentFunctionId = functionId;
  MASS_base::currentDestinations = destinations;

  // reset requestCounter by the main thread
  MASS_base::requestCounter = 0;

  // for debug
  MASS_base::showHosts( );

  // resume threads
  Mthread::resumeThreads( Mthread::STATUS_EXCHANGEALL );

  // exchangeall implementation
  Places_base::exchangeAll( MASS_base::destinationPlaces,
			    functionId, 
			    MASS_base::currentDestinations, 0 );

  // confirm all threads are done with exchangeAll.
  Mthread::barrierThreads( 0 );

  // Synchronized with all slave processes
  MASS::barrier_all_slaves( );
}

/**
 * 
 */
void Places::exchangeBoundary( ) {

  // send a PLACES_EXCHANGE_BOUNDARY message to each slave
  Message *m = new Message( Message::PLACES_EXCHANGE_BOUNDARY, this->handle, 
			    0 ); // 0 is dummy

  for ( int i =0; i < int ( MASS::mNodes.size( ) ); i++ ) {
    MASS::mNodes[i]->sendMessage( m );
  }
  delete m;
 
  // retrieve the corresponding places
  MASS_base::currentPlaces = this;

  // for debug
  MASS_base::showHosts( );

  // exchange boundary implementation
  Places_base::exchangeBoundary( );

  // Synchronized with all slave processes
  MASS::barrier_all_slaves( );
}
