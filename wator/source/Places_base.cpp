#include "Places_base.h"
#include "MASS_base.h"
#include "Message.h"
#include "DllClass.h"
#include "limits.h"
#include <iostream>
#include <sstream> // ostringstream

//Used to enable or disable output in places
const bool printOutput = false;
//const bool printOutput = true;

/**
 * 
 * @param handle
 * @param className
 * @param boundary_width
 * @param argument
 * @param argument_size
 * @param dim
 * @param size
 */
Places_base::Places_base( int handle, string className, int boundary_width,
			  void *argument, int argument_size, 
			  int dim, int size[] )
  : handle( handle ), className( className ), dimension( dim ), 
    boundary_width( boundary_width ) {

  ostringstream convert;
  if ( printOutput == true ) {
      convert << "Places_base handle = " << handle 
	      << ", class = " << className
	      << ", argument_size = " << argument_size 
	      << ", argument = " << (char *)argument
	      << ", boundary_width = " << boundary_width
	      << ", dim = " << dim
	      << endl;
    MASS_base::log( convert.str( ) );
  }

  if ( size == NULL ) 
    // if given in "int dim, ..." format, init_all() must be called later 
    return;
  this->size = new int[dim];
  for ( int i = 0; i < dim; i++ )
    this->size[i] = size[i];

  init_all( argument, argument_size );
}

/**
 * 
 */
Places_base::~Places_base( ) {
  // destroy( places ); to be debugged
  DllClass *dllclass = MASS_base::dllMap[handle];
  for ( int i = 0; i < places_size; i++ )
    dllclass->destroy( dllclass->places[i] );

  if ( dllclass->left_shadow != NULL )
    for ( int i = 0; i < shadow_size; i++ )
      dllclass->destroy( dllclass->left_shadow[i] );

  if ( dllclass->right_shadow != NULL )
    for ( int i = 0; i < shadow_size; i++ )
      dllclass->destroy( dllclass->right_shadow[i] );

  dlclose( dllclass->stub );
}

/**
 * 
 * @param argument
 * @param argument_size
 */
void Places_base::init_all( void *argument, int argument_size ) {
  // For debugging
  ostringstream convert;
  if(printOutput == true){
      convert << "init_all handle = " << handle 
	      << ", class = " << className
	      << ", argument_size = " << argument_size 
	      << ", argument = " << (char *)argument
	      << ", dimension = " << dimension
	      << endl;
      for ( int i = 0; i < dimension; i++ )
        convert << "size[" << i << "] = " << size[i] << "  ";
      convert << endl;
      MASS_base::log( convert.str( ) );
  }

  // Print the current working directory
  /*
  char buf[200];
  getcwd( buf, 200 );
  convert.str( "" );
  convert << buf << endl;
  MASS_base::log( convert.str( ) );
  */

  // load the construtor and destructor
  DllClass *dllclass = new DllClass( className );
  MASS_base::dllMap.
    insert( map<int, DllClass*>::value_type( handle, dllclass ) );

  // calculate lower_boundary and upper_boundary
  int total = 1;
  for ( int i = 0; i < dimension; i++ )
    total *= size[i];
  int stripe = total / MASS_base::systemSize;

  lower_boundary = stripe * MASS_base::myPid;
  upper_boundary = ( MASS_base::myPid < MASS_base::systemSize - 1 ) ?
    lower_boundary + stripe - 1 : total - 1;
  places_size = upper_boundary - lower_boundary + 1;

  // instantiate Places objects within dlclass
  this->places_size = places_size;
  //  maintaining an entire set
  dllclass->places = new Place*[places_size]; 

  vector<int> index;
  index.reserve( dimension );

  // initialize all Places objects
  for ( int i = 0; i < places_size; i++ ) {
    // instanitate a new place
    dllclass->places[i] = (Place *)( dllclass->instantiate( argument ) );  
    dllclass->places[i]->size.reserve( dimension );
    for ( int j = 0; j < dimension; j++ )
      // define size[] and index[]
      dllclass->places[i]->size.push_back( size[j] ); 
    dllclass->places[i]->index 
      = getGlobalArrayIndex( lower_boundary + i );
  }

  // allocate the left/right shadows

  if ( boundary_width <= 0 ) {
    // no shadow space.
    shadow_size = 0;
    dllclass->left_shadow = NULL;
    dllclass->right_shadow = NULL;
    return;
  }

  shadow_size 
    = ( dimension == 1 ) ? boundary_width : total / size[0] * boundary_width;
  if ( printOutput == true ) {
    ostringstream convert;
    convert << "Places_base.shadow_size = " << shadow_size;
    MASS_base::log( convert.str( ) );
  }
  dllclass->left_shadow = ( MASS_base::myPid == 0 ) ? 
    NULL : new Place*[ shadow_size ];
  dllclass->right_shadow = ( MASS_base::myPid == MASS_base::systemSize - 1 ) ? 
    NULL : new Place*[ shadow_size ];    

  // initialize the left/right shadows
  for ( int i = 0; i < shadow_size; i++ ) {

    // left shadow initialization
    if ( dllclass->left_shadow != NULL ) {
      // instanitate a new place
      dllclass->left_shadow[i] 
	= (Place *)( dllclass->instantiate( argument ) );  
      dllclass->left_shadow[i]->size.reserve( dimension );
      for ( int j = 0; j < dimension; j++ )
	// define size[] and index[]
	dllclass->left_shadow[i]->size.push_back( size[j] ); 
      dllclass->left_shadow[i]->index 
	= getGlobalArrayIndex( lower_boundary - shadow_size + i );
      dllclass->left_shadow[i]->outMessage = NULL;
      dllclass->left_shadow[i]->outMessage_size = 0;
      dllclass->left_shadow[i]->inMessage_size = 0;
    }

    // right shadow initialization
    if ( dllclass->right_shadow != NULL ) {
      // instanitate a new place
      dllclass->right_shadow[i] 
	= (Place *)( dllclass->instantiate( argument ) );  
      dllclass->right_shadow[i]->size.reserve( dimension );
      for ( int j = 0; j < dimension; j++ )
	// define size[] and index[]
	dllclass->right_shadow[i]->size.push_back( size[j] ); 
      dllclass->right_shadow[i]->index 
	= getGlobalArrayIndex( upper_boundary + i );
      dllclass->right_shadow[i]->outMessage = NULL;
      dllclass->right_shadow[i]->outMessage_size = 0;
      dllclass->right_shadow[i]->inMessage_size = 0;
    }    
  }
}

/** @brief Converts a given plain single index into a multi-dimensional index.
  *  @param[in] singleIndex an index in a plain single dimesion that will be
   *                        converted in a multi-dimensional index.
   * @return a multi-dimensional index			   
 */
vector<int> Places_base::getGlobalArrayIndex( int singleIndex ) {
  vector<int> index;            // a multi-dimensional index
  index.reserve( dimension );
  vector<int>::iterator iter = index.begin();

  for ( int i = dimension - 1; i >= 0; i-- ) { 
    // calculate from lower dimensions
    index.insert( iter, 1, singleIndex % size[i] );
    singleIndex /= size[i];
  }

  return index;
}

/**
 * 
 * @param functionId
 * @param argument
 * @param tid
 */
void Places_base::callAll( int functionId, void *argument, int tid ) {
  int range[2];
  getLocalRange( range, tid );

  DllClass *dllclass = MASS_base::dllMap[ handle ];

  // debugging
  ostringstream convert;
  if ( printOutput == true ) {
    convert << "thread[" << tid << "] callAll functionId = " << functionId
	    << ", range[0] = " << range[0] << " range[1] = " << range[1]
	    << ", dllclass = " << (void *)dllclass;
    MASS_base::log( convert.str( ) );
  }

  if ( range[0] >= 0 && range[1] >= 0 ) {
    for ( int i = range[0]; i <= range[1]; i++ ) {
      if ( printOutput == true ) {
	convert.str( "" );
	convert << "thread[" << tid << "]: places[" << i 
		<< "] = " << dllclass->places[i] 
		<< ", vptr = " << *(int**)(dllclass->places[i]);
	MASS_base::log( convert.str( ) );
      }
      dllclass->places[i]->callMethod( functionId, argument ); // <-- seg fault
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
 * @return 
 */
void **Places_base::callAll( int functionId, void *argument, 
			     int arg_size, int ret_size, int tid ) {
  int range[2];
  getLocalRange( range, tid );

  // debugging
  ostringstream convert;
  if ( printOutput == true ) {
    convert << "thread[" << tid << "] callAll_return object functionId = " 
	    << functionId
	    << ", range[0] = " << range[0] << " range[1] = " << range[1]
  	  << ", return_size = " << ret_size;
    MASS_base::log( convert.str( ) );
  }

  DllClass *dllclass = MASS_base::dllMap[ handle ];
  char *return_values = MASS_base::currentReturns + range[0] * ret_size;
  if ( range[0] >= 0 && range[1] >= 0 ) {
    for ( int i = range[0]; i <= range[1]; i++ ) {
      if ( printOutput == true ) {
	convert.str( "" );
	convert << "thread[" << tid << "]: places[i] = " << dllclass->places[i];
	MASS_base::log( convert.str( ) );
      }
      memcpy( (void *)return_values, dllclass->places[i]->callMethod( functionId, 
					      (char *)argument + arg_size * i
					      ), ret_size );
      return_values += ret_size;
    }
  }

  return NULL;
}

/** @brief Returns the first and last of the range that should be allocated
  *        to a given thread
  *
  *  @param[i] tid an id of the thread that calls this function.
  *  @return an array of two integers: element 0 = the first and 
  *           element 1 = the last
*/
void Places_base::getLocalRange( int range[], int tid ) {

  int nThreads = MASS_base::threads.size( );
  int portion = places_size / nThreads; // a range to be allocated per thread
  int remainder = places_size % nThreads;

  if ( portion == 0 ) {
    // there are more threads than elements in the MASS.Places_based object
    if ( remainder > tid ) {
      range[0] = tid;
      range[1] = tid;
    }
    else {
      range[0] = -1;
      range[1] = -1;
    }
  }
  else {
    // there are more MASS.Places than threads
    int first = tid * portion;
    int last = ( tid + 1 ) * portion - 1;
    if ( tid < remainder ) {
      // add in remainders
      first += tid;
      last = last + tid + 1; // 1 is one of remainders.
    }
    else {
      // remainders have been assigned to previous threads
      first += remainder;
      last += remainder;
    }
    range[0] = first;
    range[1] = last;
  }
}

/**
 * 
 * @param dstPlaces
 * @param functionId
 * @param destinations
 * @param tid
 */
void Places_base::exchangeAll( Places_base *dstPlaces, int functionId, 
			       vector<int*> *destinations, int tid ) {
  int range[2];
  getLocalRange( range, tid );
  ostringstream convert;
  // debugging
  if(printOutput == true){
      convert << "thread[" << tid << "] exchangeAll functionId = " << functionId
	      << ", range[0] = " << range[0] << " range[1] = " << range[1];
      MASS_base::log( convert.str( ) );
  }

  DllClass *src_dllclass = MASS_base::dllMap[ handle ];
  DllClass *dst_dllclass = MASS_base::dllMap[ dstPlaces->handle ];

  if(printOutput == true){
      convert.str( "" );
      convert << "tid[" << tid << "]: checks destinations:";
      for ( int i = 0; i < int( destinations->size( ) ); i++ ) {
        int *offset = (*destinations)[i];
        convert << "[" << offset[0]
	        << "][" << offset[1] << "]  ";    
      }
      MASS_base::log( convert.str( ) );
  }
  // now scan all places within range[0] ~ range[1]
  if ( range[0] >= 0 && range[1] >= 0 ) {
    for ( int i = range[0]; i <= range[1]; i++ ) {
      // for each place
      Place *srcPlace = (Place *)(src_dllclass->places[i]);

      // check its neighbors
      for ( int j = 0; j < int( destinations->size( ) ); j++ ) {

	// for each neighbor
	int *offset = (*destinations)[j];
	int neighborCoord[dstPlaces->dimension];

	// compute its coordinate
	getGlobalNeighborArrayIndex( srcPlace->index, offset, dstPlaces->size,
				     dstPlaces->dimension, neighborCoord );
	if(printOutput == true){
	    convert.str( "" );
	    convert << "tid[" << tid << "]: calls from"
		    << "[" << srcPlace->index[0]
		    << "][" << srcPlace->index[1] << "]"
		    << " (neighborCord[" << neighborCoord[0]
		    << "][" << neighborCoord[1] << "]"
		    << " dstPlaces->size[" << dstPlaces->size[0] 
		    << "][" << dstPlaces->size[1] << "]";
	    }
	if ( neighborCoord[0] != -1 ) { 
	  // destination valid
	  int globalLinearIndex = 
	    getGlobalLinearIndexFromGlobalArrayIndex( neighborCoord,
						      dstPlaces->size,
						      dstPlaces->dimension );
	  if(printOutput == true){
	      convert << " linear = " << globalLinearIndex
		      << " lower = " << dstPlaces->lower_boundary
		      << " upper = " << dstPlaces->upper_boundary << ")";
	  }

	  if ( globalLinearIndex >= dstPlaces->lower_boundary &&
	       globalLinearIndex <= dstPlaces->upper_boundary ) {
	    // local destination
	    int destinationLocalLinearIndex 
	      = globalLinearIndex - dstPlaces->lower_boundary;
	    Place *dstPlace = 
	      (Place *)(dst_dllclass->places[destinationLocalLinearIndex]);
	    
	    if(printOutput == true){
	        convert << " to [" << dstPlace->index[0]
		        << "][" << dstPlace->index[1] << "]";
	    }
	    // call the destination function
	    void *inMessage = dstPlace->callMethod( functionId, 
						    srcPlace->outMessage );

	    // store this inMessage: 
	    // note that callMethod must return a dynamic memory space
	    srcPlace->inMessages.push_back( inMessage );

	    // for debug
	    if(printOutput == true){
	        convert << " inMessage = " 
		        << *(int *)(srcPlace->inMessages.back( ));
	    }
	  } else {
	    // remote destination

	    // find the destination node
	    int destRank 
	      = getRankFromGlobalLinearIndex( globalLinearIndex );

	    // create a request
	    int orgGlobalLinearIndex =
	      getGlobalLinearIndexFromGlobalArrayIndex( &(srcPlace->index[0]), 
							size,
							dimension );
	    RemoteExchangeRequest *request 
	      = new RemoteExchangeRequest( globalLinearIndex,
					   orgGlobalLinearIndex,
					   j, // inMsgIndex
					   srcPlace->inMessage_size,
					   srcPlace->outMessage,
					   srcPlace->outMessage_size,
					   false );

	    // enqueue the request to this node.map
	    pthread_mutex_lock( &MASS_base::request_lock );
	    MASS_base::remoteRequests[destRank]->push_back( request );

	    if(printOutput == true){
	        convert.str( "" );
	        convert << "remoteRequest[" << destRank << "]->push_back:"
		        << " org = " << orgGlobalLinearIndex
		        << " dst = " << globalLinearIndex
		        << " size( ) = " 
		        << MASS_base::remoteRequests[destRank]->size( );
	        MASS_base::log( convert.str( ) );
	    }
	    pthread_mutex_unlock( &MASS_base::request_lock );
	  }
	} else {
	  if(printOutput == true)
	    convert << " to destination invalid";
	}
	if(printOutput == true){
	    MASS_base::log( convert.str( ) );
	}	
      }
    }
  }

  // all threads must barrier synchronize here.
  Mthread::barrierThreads( tid );
  if ( tid == 0 ) {

    if(printOutput == true){
        convert.str( "" );
        convert << "tid[" << tid << "] now enters processRemoteExchangeRequest";
        MASS_base::log( convert.str( ) );
    }
    // the main thread spawns as many communication threads as the number of
    // remote computing nodes and let each invoke processRemoteExchangeReq.
    
    // args to threads: rank, srcHandle, dstHandle, functionId, lower_boundary
    int comThrArgs[MASS_base::systemSize][5];
    pthread_t thread_ref[MASS_base::systemSize]; // communication thread id
    for ( int rank = 0; rank < MASS_base::systemSize; rank++ ) {

      if ( rank == MASS_base::myPid ) // don't communicate with myself
	continue;

      // set arguments 
      comThrArgs[rank][0] = rank;
      comThrArgs[rank][1] = handle;
      comThrArgs[rank][2] = dstPlaces->handle;
      comThrArgs[rank][3] = functionId;
      comThrArgs[rank][4] = lower_boundary;

      // start a communication thread
      if ( pthread_create( &thread_ref[rank], NULL, 
			   Places_base::processRemoteExchangeRequest, 
			   comThrArgs[rank] ) != 0 ) {
	MASS_base::log( "Places_base.exchangeAll: failed in pthread_create" );
	exit( -1 );
      }
    }

    // wait for all the communication threads to be terminated
    for ( int rank = 0; rank < MASS_base::systemSize; rank++ ) {
      if ( rank == MASS_base::myPid ) // don't communicate with myself
	continue;      
      pthread_join( thread_ref[rank], NULL );
    }
  }
  else {
    if(printOutput == true){
        convert.str( "" );
        convert << "tid[" << tid << "] skips processRemoteExchangeRequest";
        MASS_base::log( convert.str( ) );
    }
  }

}

/**
 * 
 * @param param
 * @return 
 */
void *Places_base::processRemoteExchangeRequest( void *param ) {
  int destRank = ( (int *)param )[0];
  int srcHandle = ( (int *)param )[1];
  int destHandle_at_src = ( (int *)param )[2];
  int functionId = ( (int *)param )[3];
  int my_lower_boundary = ( (int *)param )[4];

  vector<RemoteExchangeRequest*>* orgRequest = NULL;
  ostringstream convert;

  if(printOutput == true){
      convert.str( "" );
      convert << "rank[" << destRank << "]: starts processRemoteExchangeRequest";
      MASS_base::log( convert.str( ) );
  }
  // pick up the next rank to process
  orgRequest = MASS_base::remoteRequests[destRank];

  // for debugging
  pthread_mutex_lock( &MASS_base::request_lock );

  if(printOutput == true){
      convert.str( "" );
      convert << "tid[" << destRank << "] sends an exhange request to rank: " 
	      << destRank << " size() = " << orgRequest->size( ) << endl;
      for ( int i = 0; i < int( orgRequest->size( ) ); i++ ) {
        convert << "send from "
	        << (*orgRequest)[i]->orgGlobalLinearIndex << " to "
	        << (*orgRequest)[i]->destGlobalLinearIndex << " at "
	        << (*orgRequest)[i]->inMessageIndex << " inMsgSize: " 
	        << (*orgRequest)[i]->inMessageSize << " outMsgSize: " 
	        << (*orgRequest)[i]->outMessageSize << endl;
      }
      MASS_base::log( convert.str( ) );
  }
  pthread_mutex_unlock( &MASS_base::request_lock );

  // now compose and send a message by a child
  Message *messageToDest 
    = new Message( Message::PLACES_EXCHANGE_ALL_REMOTE_REQUEST,
		   srcHandle, destHandle_at_src, functionId, orgRequest );

  struct ExchangeSendMessage rankNmessage;
  rankNmessage.rank = destRank;
  rankNmessage.message = messageToDest;
  pthread_t thread_ref;
  pthread_create( &thread_ref, NULL, sendMessageByChild, &rankNmessage );

  // receive a message by myself
  Message *messageFromSrc = MASS_base::exchange.receiveMessage( destRank );

  // at this point, 
  // at this point, the message must be exchanged.
  pthread_join( thread_ref, NULL );

  // process a message
  vector<RemoteExchangeRequest*>* receivedRequest 
    = messageFromSrc->getExchangeReqList( );

  int destHandle_at_dst = messageFromSrc->getDestHandle( );
  Places_base *dstPlaces = MASS_base::placesMap[ destHandle_at_dst ];
  DllClass *dst_dllclass = MASS_base::dllMap[ destHandle_at_dst ];

  if(printOutput == true){
      convert.str( "" );
      convert << "request from rank[" << destRank << "] = " << receivedRequest;
      convert << " size( ) = " << receivedRequest->size( );
  }

  // get prepared for a space to sotre return values
  int inMessageSizes = 0;
  for ( int i = 0; i < int( receivedRequest->size( ) ); i++ ) 
    inMessageSizes += (*receivedRequest)[i]->inMessageSize;
  char retVals[inMessageSizes];
  
  if(printOutput == true){
      convert << " retVals = " << (void *)retVals
	      << " total inMessageSizes = " << inMessageSizes << endl;
  }
  // for each place, call the corresponding callMethod( ).
  char *retValPos = retVals;
  for ( int i = 0; i < int( receivedRequest->size( ) ); i++ ) {

    if(printOutput == true){
        convert << "received from "
	        << (*receivedRequest)[i]->orgGlobalLinearIndex << " to "
	        << (*receivedRequest)[i]->destGlobalLinearIndex << " at "
	        << (*receivedRequest)[i]->inMessageIndex << " inMsgSize: " 
	        << (*receivedRequest)[i]->inMessageSize << " outMsgSize: "
	        << (*receivedRequest)[i]->outMessageSize 
	        << " dstPlaces->lower = " << dstPlaces->lower_boundary
	        << " dstPlaces->upper = " << dstPlaces->upper_boundary;
    }
    int globalLinearIndex = (*receivedRequest)[i]->destGlobalLinearIndex;
    void *outMessage = (*receivedRequest)[i]->outMessage;
    
    if ( globalLinearIndex >= dstPlaces->lower_boundary &&
	 globalLinearIndex <= dstPlaces->upper_boundary ) {
      // local destination
      int destinationLocalLinearIndex 
	= globalLinearIndex - dstPlaces->lower_boundary;
      
      if(printOutput == true)
          convert << " dstLocal = " << destinationLocalLinearIndex << endl;
      
      Place *dstPlace = 
	(Place *)(dst_dllclass->places[destinationLocalLinearIndex]);
      
      // call the destination function
      void *inMessage = dstPlace->callMethod( functionId, 
					      outMessage );
      memcpy(  retValPos, inMessage, (*receivedRequest)[i]->inMessageSize );
      retValPos += (*receivedRequest)[i]->inMessageSize;
    }
  }
  if(printOutput == true){
      MASS_base::log( convert.str( ) );
  }
  delete messageFromSrc;

  // send return values by a child thread
  Message *messageToSrc = 
    new Message( Message::PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT,
		 retVals, inMessageSizes );
  rankNmessage.rank = destRank;
  rankNmessage.message = messageToSrc;
  pthread_create( &thread_ref, NULL, sendMessageByChild, &rankNmessage );
  
  // receive return values by myself in parallel
  Message *messageFromDest = MASS_base::exchange.receiveMessage( destRank );

  // at this point, the message must be exchanged.
  pthread_join( thread_ref, NULL );
  delete messageToSrc;

  // store return values to the orignal places
  int argumentSize = messageFromDest->getArgumentSize( );
  char argument[argumentSize];
  messageFromDest->getArgument( argument );

  int pos = 0;
  DllClass *src_dllclass = MASS_base::dllMap[ srcHandle ];
  
  if(printOutput == true){
      convert.str( "" );
      convert << "return values: argumentSize = " << argumentSize
	      << " src_dllclass = " << src_dllclass
	      << " orgRequest->size( ) = " << orgRequest->size( );
      MASS_base::log( convert.str( ) );
  }
  
  for ( int i = 0; i < int( orgRequest->size( ) ); i++ ) {
    // local source
    int orgLocalLinearIndex
      = (*orgRequest)[i]->orgGlobalLinearIndex - my_lower_boundary;
    
    // locate a local place
    Place *srcPlace =	(Place *)(src_dllclass->places[orgLocalLinearIndex]);
    
    // store a return value to it
    char *inMessage = new char[srcPlace->inMessage_size];
    memcpy( inMessage, argument + pos, srcPlace->inMessage_size );
    pos += srcPlace->inMessage_size;
    
    // insert an item at inMessageIndex or just append it.
    if ( int( srcPlace->inMessages.size( ) ) 
	 > (*orgRequest)[i]->inMessageIndex )
      srcPlace->inMessages.insert( srcPlace->inMessages.begin( ) 
				   + (*orgRequest)[i]->inMessageIndex,
				   (void *)inMessage );
    else
      srcPlace->inMessages.push_back( (void *)inMessage );

    if ( printOutput == true ) {
      convert.str( "" );
      convert << "srcPlace[" << srcPlace->index[0]<< "][" 
	      << srcPlace->index[1] << "] inserted " 
	      << "at " << (*orgRequest)[i]->inMessageIndex;
      MASS_base::log( convert.str( ) );
    }
    
  }
  delete messageToDest; // messageToDest->orgReuqest is no longer used. delete it.
  delete messageFromDest;

  return NULL;
}

/**
 * 
 * @param param
 * @return NULL
 */
void *Places_base::sendMessageByChild( void *param ) {
  int rank = ((struct ExchangeSendMessage *)param)->rank;
  Message *message = (Message *)((struct ExchangeSendMessage *)param)->message;
  MASS_base::exchange.sendMessage( rank, message );
  return NULL;
}

/**
 * 
 */
void Places_base::exchangeBoundary( ) {
  if ( shadow_size == 0 ) { // no boundary, no exchange
    ostringstream convert;
    convert << "places (handle = " << handle 
	    << ") has NO boundary, and thus invokes NO exchange boundary";
    MASS_base::log( convert.str( ) );
    return;
  }

  pthread_t thread_ref = 0l;

  if ( printOutput == true ) {
    MASS_base::log( "exchangeBoundary starts" );
  }

  int param[2][4];
  if ( MASS_base::myPid < MASS_base::systemSize - 1 ) {
    // create a child in charge of handling the right shadow.
    param[0][0] = 'R';
    param[0][1] = handle;
    param[0][2] = places_size; // upper_boundary - lower_boundary + 1
    param[0][3] = shadow_size;
    if ( printOutput == true ) {
      ostringstream convert;
      convert << "exchangeBoundary: pthreacd_create( helper, R ) places_size=" 
	      << places_size;
      MASS_base::log( convert.str( ) );
    }
    pthread_create( &thread_ref, NULL, exchangeBoundary_helper, param[0] );
  }

  if ( MASS_base::myPid > 0 ) {
    // the main takes charge of handling the left shadow.
    param[1][0] = 'L';
    param[1][1] = handle;    
    param[1][2] = places_size; // upper_boundary - lower_boundary + 1
    param[1][3] = shadow_size;
    if ( printOutput == true ) {
      ostringstream convert;
      convert << "exchangeBoundary: main thread( helper, L ) places_size=" 
	      << places_size;
      MASS_base::log( convert.str( ) );
    }
    exchangeBoundary_helper( param[1] );
  }

  if ( thread_ref != 0l ) {
    // we are done with exchangeBoundary
    int error_code = pthread_join( thread_ref, NULL ); 
    if ( error_code != 0 ) { // if we remove this if-clause, we will get a segmentation fault! Why?
      ostringstream convert;
      convert << "exchangeBoundary: the main performs pthread_join with the child...error_code = " 
	      << error_code;
      MASS_base::log( convert.str( ) );
    }
  }
}

/**
 * 
 * @param param
 * @return 
 */
void *Places_base::exchangeBoundary_helper( void *param ) {
  // identifiy the boundary space;
  char direction = ( (int *)param )[0];
  int handle = ( (int *)param )[1];
  int places_size = ( (int *)param )[2];
  int shadow_size = ( (int *)param )[3];
  DllClass *dllclass = MASS_base::dllMap[handle];

  ostringstream convert;
  if ( printOutput == true ) {
      convert << "Places_base.exchangeBoundary_helper direction = " 
	      << direction
	      << ", handle = " << handle
	      << ", places_size = " << places_size 
	      << ", shadow_size = " << shadow_size
	//<< ", outMessage_size = " << outMessage_size
	      << endl;
    MASS_base::log( convert.str( ) );
  }

  Place **boundary = ( direction == 'L' ) ? 
    dllclass->places :                                // left boundary top
    dllclass->places + ( places_size - shadow_size ); // right boundary top

  Place **shadow = ( direction == 'L' ) ?
    dllclass->left_shadow :                           // left shadow top
    dllclass->right_shadow;                           // right shadow top

  // allocate an outBuffer to contain all outMessages in this boundary
  int outMessage_size = boundary[0]->outMessage_size;
  int outBuffer_size = shadow_size * outMessage_size;

  // allocate an inBuffer to contain all inMessage[3]s if direction=='L'
  // or to contain all inMessage[1]s if direction=='R'
  int inMessage_size = boundary[0]->inMessage_size;
  int inBuffer_size = shadow_size * inMessage_size;

  // allocate an outBuffer/inBuffer-combined continguous buffer
  char *buffer = (char *)( malloc( outBuffer_size + inBuffer_size ) );
  char *outBuffer = buffer;
  char *inBuffer = buffer + outBuffer_size;
  
  // copy all the outMessages into the outBuffer
  char *pos = outBuffer; // we shouldn't change the outBuffer pointer.
  for ( int i = 0; i < shadow_size; i++ ) {
    if ( printOutput == true ) {
      convert.str( "" );
      convert << "boundary[" << i << "] = " << boundary[i];
      convert << " outMessage = " << boundary[i]->outMessage;
      MASS_base::log( convert.str( ) );
    }
    memcpy( pos, boundary[i]->outMessage, outMessage_size );
    pos += outMessage_size;
  }

  // copy all the inMessages into the outBuffer
  pos = inBuffer; // we shouldn't change the inBuffer pointer.
  for ( int i = 0; i < shadow_size; i++ ) {
    if ( printOutput == true ) {
      convert.str( "" );
      convert << "shadow[" << i << "] = " << shadow[i];
      convert << " inMessages = " << shadow[i]->inMessages.size( );
      MASS_base::log( convert.str( ) ) ;
    }
    if ( ( direction == 'L' && shadow[i]->inMessages.size( ) >= 4 ) ||
	 ( direction == 'R' && shadow[i]->inMessages.size( ) >= 1 ) ) {
      memcpy( pos, shadow[i]->inMessages[ (direction == 'L' ) ? 3 : 1 ], 
	      inMessage_size );
      pos += inMessage_size;
    }
  }

  if ( printOutput == true ) {
    convert.str( "" );
    convert << "Places_base.exchangeBoundary_helper direction = " 
	    << direction
	    << ", outMessage_size = " << outMessage_size
	    << ", inMessage_size = " << inMessage_size
	    << endl;
    pos = outBuffer;
    convert << "outBuffer: ";
    for ( int i = 0; i < shadow_size; i++ ) {
      convert << *(int *)pos << " " << endl;
      pos += outMessage_size;
    }
    pos = inBuffer;
    convert << "inBuffer: ";
    for ( int i = 0; i < shadow_size; i++ ) {
      convert << *(int *)pos << " " << endl;
      pos += inMessage_size;
    }
    MASS_base::log( convert.str( ) );
  }

  // create a PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST message
  Message *messageToDest = 
    new Message( Message::PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST,
		 buffer, outBuffer_size + inBuffer_size );

  // compose a PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST message
  int destRank 
    = ( direction == 'L' ) ? MASS_base::myPid - 1 : MASS_base::myPid + 1;
  struct ExchangeSendMessage rankNmessage;
  rankNmessage.rank = destRank;
  rankNmessage.message = messageToDest;

  if ( printOutput == true ) {
    convert.str( "" );
    convert << "Places_base.exchangeBoundary_helper direction = " 
	    << direction
	    << ", rankNmessage.rank = " << rankNmessage.rank
	    << endl;
    MASS_base::log( convert.str( ) );
  }

  // send it to my neighbor with a child
  pthread_t thread_ref = 0l;
  pthread_create( &thread_ref, NULL, sendMessageByChild, &rankNmessage );

  // receive a PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST message from my neighbor
  Message *messageFromDest = MASS_base::exchange.receiveMessage( destRank );

  if ( printOutput == true ) {
    convert.str( "" );
    convert << "Places_base.exchangeBoundary_helper direction = " 
	    << direction
	    << ", messageFromDest = " << messageFromDest
	    << endl;
    MASS_base::log( convert.str( ) );
  }

  // wait for the child termination
  if ( thread_ref != 0l ) {
    if ( pthread_join( thread_ref, NULL ) == 0 ) {
      if ( printOutput == true ) {
	convert.str( "" );
	convert << "Places_base.exchangeBoundary_helper direction = " 
		<< direction
		<< ", sendMessageByChild terminated"
		<< endl;
	MASS_base::log( convert.str( ) );
      }
    }
  }

  // delete the message sent
  free( buffer ); // this is because messageToDest->argument_in_heap is false
  outBuffer = 0;
  inBuffer = 0;
  delete messageToDest;
  messageToDest = 0;

  // extract the message received and copy it to the corresponding shadow.
  shadow = ( direction == 'L' ) ? 
    dllclass->left_shadow : dllclass->right_shadow;
  outBuffer = (char *)( messageFromDest->getArgumentPointer( ) );

  // extract the message received and copy it to the corresponding boundary.
  boundary = ( direction == 'L' ) ? 
    dllclass->places :                                // left boundary top
    dllclass->places + ( places_size - shadow_size ); // right boundary top
  inBuffer = outBuffer + outBuffer_size;

  for ( int i = 0; i < shadow_size; i++ ) {
    // copy the outBuffer contents into the corresponding shadow
    if ( shadow[i]->outMessage_size == 0 ) {
      // first allocate a space
      shadow[i]->outMessage_size = outMessage_size;
      shadow[i]->outMessage = malloc( outMessage_size );
    }
    memcpy( shadow[i]->outMessage, outBuffer, outMessage_size );

    // copy the inBuffer contents into the corresponding boundary
    int position = ( direction == 'L' ) ? 3 : 1;
    /* at present, no automatic inMessage space allocation. 
       a user must allocate inMessage space if they want to send it.
    for ( int j = 0; j < 4; j++ )
      if ( int( boundary[i]->inMessages.size( ) )
	   <= j  && inMessage_size > 0 )
	boundary[i]->inMessages.push_back( malloc( inMessage_size ) );
    */
    if ( position < int( boundary[i]->inMessages.size( ) ) )
      memcpy( boundary[i]->inMessages[ position ], inBuffer,
	      inMessage_size );

    // check the contents
    if ( printOutput == true ) {
      convert.str( "" );
      convert << "Places_base.exchangeBoundary_helper direction = " 
	      << direction
	      << ", shadow[" << i << "]->outMessage = " 
	      << shadow[i]->outMessage
	      << ", outBuffer = " << *(int *)outBuffer
	      << ", boundary[" << i << "]->inMessages[" 
	      << position << "] = " 
	      << ( ( boundary[i]->inMessages.size( ) > (unsigned int)position ) ? 
		   boundary[i]->inMessages[position] : 0 )
	      << ", inBuffer = " << *(int *)inBuffer
	      << endl;
      MASS_base::log( convert.str( ) );
    }
    outBuffer += outMessage_size;
    inBuffer += inMessage_size;
  }  

  // delete the message received
  // note that messageFromDest->argument is deleted automatically.
  delete messageFromDest;

  return NULL;
}

/**
 * 
 * @param src_index
 * @param offset
 * @param dst_size
 * @param dest_dimension
 * @param dest_index
 */
void Places_base::getGlobalNeighborArrayIndex( vector<int>src_index, 
					       int offset[],
					       int dst_size[], 
					       int dest_dimension,
					       int dest_index[] ) {
  for (int i = 0; i < dest_dimension; i++ ) {
    dest_index[i] = src_index[i] + offset[i]; // calculate dest index

    if ( dest_index[i] < 0 || dest_index[i] >= dst_size[i] ) {
      // out of range
      for ( int j = 0; j < dest_dimension; j++ ) {
	// all index must be set -1
	dest_index[j] = -1;
	return;
      }
    }
  }
}

/**
 * 
 * @param index
 * @param size
 * @param dimension
 * @return 
 */
int Places_base::getGlobalLinearIndexFromGlobalArrayIndex( int index[], 
							   int size[],
							   int dimension ) {
  int retVal = 0;

  for ( int i = 0; i < dimension; i++ ) {
    if ( size[i] <= 0 )
      continue;
    if ( index[i] >= 0 && index[i] < size[i] ) {
      retVal = retVal * size[i];
      retVal += index[i];
    }
    else
      return INT_MIN; // out of space
  }

  return retVal;
}

/**
 * 
 * @param globalLinearIndex
 * @return 
 */
int Places_base::getRankFromGlobalLinearIndex( int globalLinearIndex ) {
  static int total = 0;
  static int stripe = 0;

  if ( total == 0 ) {
    // first time computation
    total = 1;
    for ( int i = 0; i < dimension; i++ )
      total *= size[i];
    stripe = total / MASS_base::systemSize;
  }

  int rank, scope;
  for ( rank = 0, scope = stripe ; rank < MASS_base::systemSize; 
	rank++, scope += stripe ) {
    if ( globalLinearIndex < scope )
      break;
  }

  return ( rank == MASS_base::systemSize ) ? rank - 1 : rank;
}
