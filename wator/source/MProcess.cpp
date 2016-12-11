#include <stdlib.h> // atoi
#include <unistd.h> // chdir, sleep (just for debugging)
#include <sys/stat.h> // mkdir
#include <sys/types.h> // mkdir, opendir, closedir
#include <dirent.h>    // opendir, closedir
#include <sstream>     // ostringstream
#include "MProcess.h"
#include "MASS_base.h"
#include "Mthread.h"

//Toggles output for MProcess
const bool printOutput = false;
//const bool printOutput = true;

/**
 * 
 * @param name
 * @param myPid
 * @param nProc
 * @param nThr
 * @param port
 */
MProcess::MProcess( char *name, int myPid, int nProc, int nThr, int port ) {
  this->hostName = new string( name );
  this->myPid = myPid;
  this->nProc = nProc;
  this->nThr = nThr;
  this->port = port;
  MASS_base::initMASS_base( name, myPid, nProc, port );
  
  // Create a logger
  DIR *dir = NULL;
  if ( ( dir =  opendir( MASS_LOGS ) ) == NULL )
    mkdir( MASS_LOGS, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
  else
    closedir( dir );

  MASS_base::initializeThreads( nThr );
}

/**
 * 
 */
void MProcess::start( ) {
  MASS_base::log( "MProcess started" );

  // retrieve the client socket ipaddress
  int master_ip_size = 0;
  read( 0, &master_ip_size, sizeof( int ) );
  char master_ip[master_ip_size + 1];
  bzero( master_ip, master_ip_size + 1 );
  read( 0, &master_ip, master_ip_size );

  // Create a bare socket to the master
  Socket socket( port );
  this->sd = socket.getClientSocket( master_ip );

  // Synchronize with the master node first.
  sendAck( );

  bool alive = true;
  while( alive ) {
    // receive a new message from the master
    Message *m = receiveMessage( );

    ostringstream convert;
    if(printOutput == true){
        convert << "A new message received: action = " << m->getAction( ) << endl;
        MASS_base::log( convert.str( ) );
    }

    // get prepared for the following arguments for PLACES_INITIALIZE
    vector<int> size;            // size[]
    vector<string> hosts; hosts.clear( ); 
    int argument_size = 0;       // argument[argument_size];
    char *argument = NULL;
    Places_base *places = NULL;  // new Places
    Agents_base *agents = NULL;  // new Agents

    // retrieve an argument
    argument_size = m->getArgumentSize( );
    argument = ( argument_size > 0 ) ? new char[argument_size] : NULL;
    m->getArgument( argument );

    if ( m != NULL ) {
      switch( m->getAction( ) ) {
      case Message::ACK:
	sendAck( );
	break;

      case Message::EMPTY:
	if(printOutput == true)
	    MASS_base::log( "EMPTY received!!!!" );
	sendAck( );
	break;

      case Message::FINISH:
	Mthread::resumeThreads( Mthread::STATUS_TERMINATE );
	// confirm all threads are done with finish
	Mthread::barrierThreads( 0 );
	sendAck( );
	alive = false;
	if(printOutput == true)
	     MASS_base::log( "FINISH received and ACK sent" );
	break;

      case Message::PLACES_INITIALIZE:
	if(printOutput == true)
	    MASS_base::log( "PLACES_INITIALIZE received" );
	// create a new Places
	size = m->getSize( );

	places = new Places_base( m->getHandle( ), m->getClassname( ),
				  m->getBoundaryWidth( ),
				  argument, argument_size,
				  size.size( ), &size[0] );

	for ( int i = 0; i < int( m->getHosts( ).size( ) ); i++ )
	  hosts.push_back( m->getHosts( )[i] );
	// establish all inter-node connections within setHosts( )
	MASS_base::setHosts( hosts );
	
	MASS_base::placesMap.
	  insert( map<int, Places_base*>::value_type( m->getHandle( ), 
						      places ) );
	sendAck( );
	if(printOutput == true)
	    MASS_base::log( "PLACES_INITIALIZE completed and ACK sent" );
	break;

      case Message::PLACES_CALL_ALL_VOID_OBJECT:
	if(printOutput == true)
	    MASS_base::log( "PLACES_CALL_ALL_VOID_OBJECT received" );

	// retrieve the corresponding places
	MASS_base::currentPlaces = MASS_base::placesMap[ m->getHandle( ) ];
	MASS_base::currentFunctionId = m->getFunctionId( );
	MASS_base::currentArgument = (void *)argument;
	MASS_base::currentArgSize = argument_size;
	MASS_base::currentMsgType = m->getAction( );

	// resume threads to work on call all.
	Mthread::resumeThreads( Mthread::STATUS_CALLALL );

	// 3rd arg: 0 = the main thread id
	MASS_base::
	  currentPlaces->callAll( m->getFunctionId( ), (void *)argument, 0 );
	
	// confirm all threads are done with places.callAll
	Mthread::barrierThreads( 0 );
	
	sendAck( );
	break;

      case Message::PLACES_CALL_ALL_RETURN_OBJECT:
	if(printOutput == true)
	    MASS_base::log( "PLACES_CALL_ALL_RETURN_OBJECT received" );
	// retrieve the corresponding places
	MASS_base::currentPlaces = MASS_base::placesMap[ m->getHandle( ) ];
	MASS_base::currentFunctionId = m->getFunctionId( );
	MASS_base::currentArgument = (void *)argument;

	if(printOutput == true) {
	  ostringstream convert;
	  convert << "check 1 places_size = " << MASS_base::currentPlaces->places_size;
	  MASS_base::log( convert.str( ) );
	}

	MASS_base::currentArgSize 
	  = argument_size / MASS_base::currentPlaces->places_size;

	if(printOutput == true)
	    MASS_base::log( "check 2" );

	MASS_base::currentRetSize = m->getReturnSize( );
	MASS_base::currentMsgType = m->getAction( );
	MASS_base::currentReturns 
	  = new char[MASS_base::currentPlaces->places_size 
		     * MASS_base::currentRetSize];

	// resume threads to work on call all.
	Mthread::resumeThreads( Mthread::STATUS_CALLALL );

	// 3rd arg: 0 = the main thread id

	MASS_base::
	  currentPlaces->callAll( MASS_base::currentFunctionId,
				  MASS_base::currentArgument,
				  MASS_base::currentArgSize,
				  MASS_base::currentRetSize,
				  0 );

	// confirm all threads are done with places.callAll with return objects
	Mthread::barrierThreads( 0 );
	
	if(printOutput == true){
	    convert.str( "" );
	    convert << "PLACES_CALL_ALL_RETURN_OBJECT checking currentReturns";
	}

	sendReturnValues( (void *)MASS_base::currentReturns, 
			  MASS_base::currentPlaces->places_size,
			  MASS_base::currentRetSize );
	delete MASS_base::currentReturns;
	break;

      case Message::PLACES_CALL_SOME_VOID_OBJECT:
	break;

      case Message::PLACES_EXCHANGE_ALL:
	if(printOutput == true){
	    convert.str( "" );
	    convert << "PLACES_EXCHANGE_ALL recweived handle = " 
		    << m->getHandle( ) << " dest_handle = " << m->getDestHandle();
	    MASS_base::log( convert.str( ) );
	}

	// retrieve the corresponding places
	MASS_base::currentPlaces = MASS_base::placesMap[ m->getHandle( ) ];
	MASS_base::destinationPlaces =MASS_base::placesMap[m->getDestHandle()];
	MASS_base::currentFunctionId = m->getFunctionId( );
	MASS_base::currentDestinations = m->getDestinations( );

	// reset requestCounter by the main thread
	MASS_base::requestCounter = 0;

	// for debug
	MASS_base::showHosts( );

	// resume threads to work on call all.
	Mthread::resumeThreads( Mthread::STATUS_EXCHANGEALL );	

	// exchangeall implementation
	MASS_base::
	  currentPlaces->exchangeAll( MASS_base::destinationPlaces,
				      MASS_base::currentFunctionId, 
				      MASS_base::currentDestinations, 0 );

	// confirm all threads are done with places.exchangeall.
	Mthread::barrierThreads( 0 );
	if(printOutput == true)
	    MASS_base::log( "barrier done" );

	sendAck( );
	if(printOutput == true)
	    MASS_base::log( "PLACES_EXCHANGE_ALL completed and ACK sent" );
	break;

      case Message::PLACES_EXCHANGE_BOUNDARY:
	if(printOutput == true){
	    convert.str( "" );
	    convert << "PLACES_EXCHANGE_BOUNDARY received handle = " 
		    << m->getHandle( );
	    MASS_base::log( convert.str( ) );
	}

	// retrieve the corresponding places
	MASS_base::currentPlaces = MASS_base::placesMap[ m->getHandle( ) ];

	// for debug
	MASS_base::showHosts( );

	// exchange boundary implementation
	MASS_base::
	  currentPlaces->exchangeBoundary( );

	sendAck( );
	if(printOutput == true)
	    MASS_base::log( "PLACES_EXCHANGE_BOUNDARY completed and ACK sent");
	break;

      case Message::PLACES_EXCHANGE_ALL_REMOTE_REQUEST:
      case Message::PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT:
      case Message::PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST:
	break;

      case Message::AGENTS_INITIALIZE:
	if(printOutput == true)
	    MASS_base::log( "AGENTS_INITIALIZE received" );

	agents = new Agents_base( m->getHandle( ), m->getClassname( ),
				  argument, argument_size,
				  m->getDestHandle( ), 
				  m->getAgentPopulation( ) );
	
	MASS_base::agentsMap.
	  insert( map<int, Agents_base*>::value_type( m->getHandle( ), 
						      agents ) );
	sendAck( agents->localPopulation );
	if(printOutput == true)
	    MASS_base::log( "AGENTS_INITIALIZE completed and ACK sent" );
	break;

      case Message::AGENTS_CALL_ALL_VOID_OBJECT:
	if(printOutput == true)
	    MASS_base::log("AGENTS_CALL_ALL_VOID_OBJECT received" );
	MASS_base::currentAgents = MASS_base::agentsMap[m->getHandle() ];
	MASS_base::currentFunctionId = m->getFunctionId();
	MASS_base::currentArgument = (void *)argument;
	MASS_base::currentArgSize = argument_size;
	MASS_base::currentMsgType = m->getAction();
	
	Mthread::agentBagSize = MASS_base::dllMap[m->getHandle()]->agents->size();
	MASS_base::dllMap[m->getHandle()]->retBag = new vector<Agent*>;

	// resume threads to work on call all
	Mthread::resumeThreads(Mthread::STATUS_AGENTSCALLALL );

	MASS_base::currentAgents->callAll(m->getFunctionId(), (void *)argument, 0);

	// confirm all threads are done with agents.callAll
	Mthread::barrierThreads( 0 );
	if(printOutput == true)
	    MASS_base::log( "barrier done" );

	sendAck( MASS_base::currentAgents->localPopulation );
	break;

      case Message::AGENTS_CALL_ALL_RETURN_OBJECT:
	if(printOutput == true)
	    MASS_base::log("AGENTS_CALL_ALL_RETURN_OBJECT received");
	MASS_base::currentAgents = MASS_base::agentsMap[m->getHandle()];
	MASS_base::currentFunctionId = m->getFunctionId();
	MASS_base::currentArgument = (void *)argument;
	MASS_base::currentArgSize = ( MASS_base::currentAgents->localPopulation> 0 ) ?
          argument_size / MASS_base::currentAgents->localPopulation : 0;
	MASS_base::currentRetSize = m->getReturnSize();
	MASS_base::currentMsgType = m->getAction();
	MASS_base::currentReturns = new char[MASS_base::currentAgents->localPopulation * MASS_base::currentRetSize];

	Mthread::agentBagSize = MASS_base::dllMap[m->getHandle()]->agents->size();
	MASS_base::dllMap[m->getHandle()]->retBag = new vector<Agent*>;

	// resume threads to work on call all with return objects
	Mthread::resumeThreads(Mthread::STATUS_AGENTSCALLALL);

	MASS_base::currentAgents->callAll( MASS_base::currentFunctionId,
						MASS_base::currentArgument,
						MASS_base::currentArgSize,
						MASS_base::currentRetSize,
						0 );

	// confirm all threads are done with agnets.callAll with return objects
	Mthread::barrierThreads( 0 );
	if(printOutput == true)
	    MASS_base::log( "barrier done" );

	sendReturnValues( (void *)MASS_base::currentReturns,
			  MASS_base::currentAgents->localPopulation,
			  MASS_base::currentRetSize, 
			  MASS_base::currentAgents->localPopulation );

	delete MASS_base::currentReturns;
	
	break;
      case Message::AGENTS_MANAGE_ALL:
	if(printOutput == true)
	    MASS_base::log("AGENTS_MANAGE_ALL received");
	MASS_base::currentAgents = MASS_base::agentsMap[m->getHandle()];

	Mthread::agentBagSize = MASS_base::dllMap[m->getHandle()]->agents->size();
	MASS_base::dllMap[m->getHandle( )]->retBag = new vector<Agent*>;
	Mthread::resumeThreads(Mthread::STATUS_MANAGEALL);

	MASS_base::currentAgents->manageAll( 0 ); // 0 = the main thread id

	// confirm all threads are done with agents.manageAll.
	Mthread::barrierThreads( 0 );
	if(printOutput == true){
	    convert.str( "" );
	    convert << "sendAck will send localPopulation = " << MASS_base::currentAgents->localPopulation;
	    MASS_base::log( convert.str( ) );
	}

	sendAck( MASS_base::currentAgents->localPopulation );

	break;
	
      case Message:: AGENTS_MIGRATION_REMOTE_REQUEST:
	break;
	
      }
      delete m;
    }
    
  }
}

/**
 * 
 */
void MProcess::sendAck( ) {
  Message *msg = new Message( Message::ACK );
  sendMessage( msg );
  delete msg;
}

/**
 * 
 * @param localPopulation
 */
void MProcess::sendAck( int localPopulation ) {
  Message *msg = new Message( Message::ACK, localPopulation );
  ostringstream convert;
  if(printOutput == true){
      convert << "msg->getAgentPopulation = " << msg->getAgentPopulation( );
      MASS_base::log( convert.str( ) );
  }
  sendMessage( msg );
  delete msg;
}

/**
 * 
 * @param argument
 * @param nPlaces
 * @param return_size
 */
void MProcess::sendReturnValues( void *argument, int nPlaces,
				int return_size ) {
  Message *msg = new Message( Message::ACK, argument, 
			      nPlaces * return_size );
  sendMessage( msg );
  delete msg;
}

/**
 * 
 * @param argument
 * @param nAgents
 * @param return_size
 * @param localPopulation
 */
void MProcess::sendReturnValues( void *argument, int nAgents,
				 int return_size, int localPopulation ) {
  Message *msg = new Message( Message::ACK, argument, 
			      nAgents * return_size, localPopulation );
  sendMessage( msg );
  delete msg;
}

/**
 * 
 * @param msg
 */
void MProcess::sendMessage( Message *msg ) {
  int msg_size = 0;
  char *byte_msg = msg->serialize( msg_size );

  write( sd, (void *)&msg_size, sizeof( int ) );  // send a message size
  write( sd, byte_msg, msg_size );                // send a message body
}

/**
 * 
 * @return 
 */
Message *MProcess::receiveMessage( ) {
  int size = -1;
  int nRead = 0;
  if ( read( sd, (void *)&size, sizeof( int ) ) > 0 ) {// receive a message size

    ostringstream convert;
    if(printOutput == true){
        convert << "receiveMessage: size = " << size << endl;
        MASS_base::log( convert.str( ) );
    }

    char *buf = new char[size];
    for ( nRead = 0;
	  ( nRead += read( sd, buf + nRead, size - nRead) ) < size; );
    Message *m = new Message( );
    m->deserialize( buf, size );
    return m;
  } else {
    if(printOutput == true)
        MASS_base::log( "receiveMessage error" );
    exit( -1 );
  }
}

/**
 * main MASS function that launches MProcess
 * @param argc
 * @param argv
 * @return 
 */
int main( int argc, char* argv[] ) {
  // receive all arguments
  char *cur_dir = argv[1];
  char *hostName = argv[2];
  int myPid = atoi( argv[3] );
  int nProc = atoi( argv[4] );
  int nThr = atoi( argv[5] );
  int port = atoi( argv[6] );

  // set the current working directory to where the master node is running.
  chdir( cur_dir );

  // launch an MProcess at each slave node.
  MProcess process( hostName, myPid, nProc, nThr, port );
  process.start( );
}

