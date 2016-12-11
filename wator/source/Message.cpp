#include "Message.h"
#include "MASS_base.h"
#include <iostream>
#include <sstream>   // ostringstream

//Used to toggle output in Messages
const bool printOutput = false;
//const bool printOutput = true;

/**
 * 
 */
Message::~Message( ) {
  if ( argument_in_heap == true && argument != NULL ) delete (char *)argument;
  if ( hosts != NULL ) delete hosts;
  if ( argument_in_heap == true && destinations != NULL ) delete destinations;
  if ( exchangeReqList != NULL ) {
    while ( exchangeReqList->size( ) > 0 ) {
      RemoteExchangeRequest *req = (*exchangeReqList)[0];
      exchangeReqList->erase( exchangeReqList->begin( ) );
      delete req;
    }
    // exchangeReqList is MASS_base::remoteRequests[rank]
    // this should not be deleted.
  }
  if ( migrationReqList != NULL ) {
    while ( migrationReqList->size( ) > 0 ) {

      AgentMigrationRequest *req = (*migrationReqList)[0];
      migrationReqList->erase( migrationReqList->begin( ) );
      delete req;
    }
    // migrationReqList is MASS_base::migrationRequests[rank]
    // this should not be deleted.
  }
}

/**
 * 
 * @param msg_size
 * @return 
 */
char *Message::serialize( int &msg_size ) {
  ostringstream convert;

  char *msg = NULL;
  char *pos = NULL;
  msg_size = sizeof( ACTION_TYPE );
  
  if(printOutput == true)
      cerr << "Message::serialize begin: action = " << action << endl;
  switch ( action ) {
  case EMPTY:
  case ACK:
    // calculate msg_size
    msg_size += sizeof( int );   // agent_population;

    if ( argument != NULL && argument_size > 0 ) {
      msg_size += sizeof( int );   // argument_size;
      msg_size += argument_size;   // void *argument;
    }

    // compose a msg
    pos = msg = new char[msg_size];
    *(ACTION_TYPE *)msg = action; pos += sizeof( ACTION_TYPE );// action
    *(int *)pos = agent_population; 
    pos += sizeof( int ); // agent_population

    if ( argument != NULL && argument_size > 0 ) {
      *(int *)pos = argument_size;  pos += sizeof( int );      // argument_size
      memcpy((void *)pos, argument,  argument_size );
    }
    break;

  case PLACES_INITIALIZE:
    // calculate msg_size
    msg_size += ( sizeof( int ) * ( size->size( ) + 1 ) ); // vector<int> size;
    msg_size += sizeof( int );     // int handle;
    msg_size += sizeof( int );     // classname.size( );
    msg_size += classname.size( ); // classname
    msg_size += sizeof( int );     // argument_size;
    msg_size += argument_size;     // void *argument
    msg_size += sizeof( int );     // int boundary_width;
    msg_size += hosts->size( );    // hosts->size( );
    for ( int i = 0; i < (int )hosts->size( ); i++ ) {
      msg_size += sizeof( int ); msg_size += (*hosts)[i].size( ); // hosts[i]
    }

    // compose a msg
    pos = msg = new char[msg_size];  
    *(ACTION_TYPE *)pos = action; // cerr<< *(ACTION_TYPE *)pos << endl; 
    pos += sizeof( ACTION_TYPE ); // action
    *(int *)pos = size->size( ); // cerr << *(int *)pos << endl; 
    pos += sizeof( int ); // size.size( );
    for ( int i = 0; i < ( int )size->size( ); i++ ) {
      *(int *)pos = (*size)[i];  
      // cerr << *(int *)pos << endl;   
      pos += sizeof( int ); // size[i]
    }
    *(int *)pos = handle; 
    //cerr << *(int *)pos << endl;        
    pos += sizeof( int ); // handle
    *(int *)pos = classname.size( ); 
    // cerr << *(int *)pos << endl;
    pos += sizeof( int ); // classname.size( )
    strncpy( pos, classname.c_str( ), classname.size( ) ); // classname
    //cerr << pos << endl;
    pos += classname.size( );
    *(int *)pos = argument_size;  
    // cerr << "argument_size = " << argument_size << endl;
    // cerr << "*(int *)pos = " << *(int *)pos << endl;
    pos += sizeof( int ); // argument_size
    memcpy( (void *)pos, argument, argument_size );      // argument
    pos += argument_size;
    *(int *)pos = boundary_width;                        // bounary_width
    pos += sizeof( int );
    *(int *)pos = hosts->size( );  pos += sizeof( int ); // hosts->size( );
    for ( int i = 0; i < ( int )hosts->size( ); i++ ) {
      *(int *)pos 
	= (*hosts)[i].size( ); pos += sizeof( int ); // hosts[i].size( )
      strncpy( pos, (*hosts)[i].c_str( ), (*hosts)[i].size( ) );
      pos += (*hosts)[i].size( );
    }
    break;

  case PLACES_CALL_ALL_VOID_OBJECT: 
  case PLACES_CALL_ALL_RETURN_OBJECT:
    // calculate msg_size
    msg_size += sizeof( int );  // int handle,
    msg_size += sizeof( int );  // int functionId
    msg_size += sizeof( int );  // argument_size;
    msg_size += sizeof( int );  // return_size
    msg_size += argument_size;  // void *argument

    // compose a msg
    pos = msg = new char[msg_size];
    *(ACTION_TYPE *)pos = action; pos += sizeof( ACTION_TYPE ); // action
    *(int *)pos = handle;         pos += sizeof( int );         // handle
    *(int *)pos = functionId;     pos += sizeof( int );         // functionId
    *(int *)pos = argument_size;  pos += sizeof( int );         // arg_size
    *(int *)pos = return_size;    pos += sizeof( int );         // return_size
    memcpy((void *)pos, argument, argument_size );              // argument
    break;

  case PLACES_CALL_SOME_VOID_OBJECT: break;

  case PLACES_EXCHANGE_ALL: 
    // calculate msg_size
    msg_size += sizeof( int ); // int handle,
    msg_size += sizeof( int ); // int dest_handle,
    msg_size += sizeof( int ); // int functionId
    msg_size += sizeof( int ); // int dimension
    msg_size += sizeof( int ); // destinations.size( );
    msg_size += sizeof( int ) * ( dimension * destinations->size( ) ); 

    // compose a message
    pos = msg = new char[msg_size];
    *(ACTION_TYPE *)pos = action; pos += sizeof( ACTION_TYPE ); // action
    *(int *)pos = handle;         pos += sizeof( int );         // handle
    *(int *)pos = dest_handle;    pos += sizeof( int );         // dest_handle
    *(int *)pos = functionId;     pos += sizeof( int );         // functionId
    *(int *)pos = dimension;      pos += sizeof( int );         // dimension
    *(int *)pos = destinations->size( ); pos += sizeof( int );  // dest->size()
    
    for ( int i = 0; i < int( destinations->size( ) ); i++ ) {
      int *dest = (*destinations)[i];
      for ( int j = 0; j < dimension; j++ ) {
	*(int *)pos = dest[j];    
	pos += sizeof( int ); // destination[i][j]
      }
    }
    break;

  case PLACES_EXCHANGE_ALL_REMOTE_REQUEST:
    // calculate msg_size
    msg_size += sizeof( int ); // int handle,
    msg_size += sizeof( int ); // int dest_handle,
    msg_size += sizeof( int ); // int functionId
    msg_size += sizeof( int ); // exchangeReqList->size( );
    
    if  ( int( exchangeReqList->size( ) ) > 0 ) {
      msg_size += 
	( sizeof( int ) * 5 + (*exchangeReqList)[0]->outMessageSize ) 
	* ( exchangeReqList->size( ) ); // rmtExReq
    }

    if(printOutput == true){
        convert.str( "" );
        convert << "PLACES_EXCHANGE_ALL_REMOTE_REQUEST to be sent:";
        MASS_base::log( convert.str( ) );
    }

    // compose a message
    pos = msg = new char[msg_size];
    *(ACTION_TYPE *)pos = action; pos += sizeof( ACTION_TYPE ); // action
    *(int *)pos = handle;         pos += sizeof( int );         // handle
    *(int *)pos = dest_handle;    pos += sizeof( int );         // dest_handle
    *(int *)pos = functionId;     pos += sizeof( int );         // functionId
    *(int *)pos = exchangeReqList->size( ); pos += sizeof( int ); // list->size

    if(printOutput == true){
        convert.str( "" );
        convert << " functionId = " << functionId 
	        << " exchangeReqList->size = " << exchangeReqList->size( );
        MASS_base::log( convert.str( ) );
    }

    for ( int i = 0; i < int( exchangeReqList->size( ) ); i++ ) { // rmtExReq
      *(int *)pos = (*exchangeReqList)[i]->destGlobalLinearIndex; 
      pos += sizeof( int );
      *(int *)pos = (*exchangeReqList)[i]->orgGlobalLinearIndex;  
      pos += sizeof( int );
      *(int *)pos = (*exchangeReqList)[i]->inMessageIndex;        
      pos += sizeof( int );
      *(int *)pos = (*exchangeReqList)[i]->inMessageSize;        
      pos += sizeof( int );     
      *(int *)pos = (*exchangeReqList)[i]->outMessageSize;        // outMsgSize
      pos += sizeof( int );
      memcpy( (void*)pos, (*exchangeReqList)[i]->outMessage,      // outMessage
	     (*exchangeReqList)[i]->outMessageSize );
      pos += (*exchangeReqList)[i]->outMessageSize;      
    }

    break;
  case AGENTS_MIGRATION_REMOTE_REQUEST:
    
    msg_size += sizeof( int );   // Agent Handle
    msg_size += sizeof( int );   // Place Handle
    msg_size += sizeof( int );   // migrationReqList->size( );
    
    if  ( int( migrationReqList->size( ) ) > 0 ) {
      for ( int i = 0; i < int( migrationReqList->size( ) ); i++ ) {
	msg_size += sizeof( int ) * 6;
	msg_size += (*migrationReqList)[i]->agent->migratableDataSize;
      }
    }

    if(printOutput == true){
        convert.str( "" );
        convert << "AGENTS_MIGRATION_REMOTE_REQUEST to be sent";
        MASS_base::log( convert.str( ) );
    }

    //Compose message
    pos = msg = new char[msg_size];
    *(ACTION_TYPE *)pos = action;            pos += sizeof( ACTION_TYPE ); // action type
    *(int *)pos = handle;                    pos += sizeof( int );         // agent handle
    *(int *)pos = dest_handle;               pos += sizeof( int );         // place handle
    *(int *)pos = migrationReqList->size();  pos += sizeof( int );         // MigrationReqListy->size
    
    if(printOutput == true){
        convert.str( "" );
        convert << "Agent handle: " << handle << " place handle: " << dest_handle << " size = "
	        << migrationReqList->size();
        MASS_base::log( convert.str() );
    }

    for ( int i = 0; i < int( migrationReqList->size( ) ); i++ ) { // each migrationReq
      *(int *)pos = (*migrationReqList)[i]->destGlobalLinearIndex;
      pos += sizeof( int );
      *(int *)pos = (*migrationReqList)[i]->agent->agentsHandle;
      pos += sizeof( int );
      *(int *)pos = (*migrationReqList)[i]->agent->placesHandle;
      pos += sizeof( int );
      *(int *)pos = (*migrationReqList)[i]->agent->agentId;
      pos += sizeof( int );
      *(int *)pos = (*migrationReqList)[i]->agent->parentId;
      pos += sizeof( int );     
      *(int *)pos = (*migrationReqList)[i]->agent->migratableDataSize;
      pos += sizeof( int );
      if ( (*migrationReqList)[i]->agent->migratableDataSize > 0 )
	memcpy( (void*)pos, (*migrationReqList)[i]->agent->migratableData,
		(*migrationReqList)[i]->agent->migratableDataSize );
      pos += (*migrationReqList)[i]->agent->migratableDataSize;
    }
    break;
    
  case PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT:
  case PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST:
    // calculate msg_size
    msg_size += sizeof( int ); // int inMessageSizes a.k.a. argument_size
    msg_size += argument_size; // retVals a.k.a. argument
    
    // compose a message
    pos = msg = new char[msg_size];
    *(ACTION_TYPE *)pos = action; pos += sizeof( ACTION_TYPE ); // action
    *(int *)pos = argument_size;  pos += sizeof( int );         // arg_size
    memcpy( pos, argument, argument_size );                      // argument
    
    break;
    
  case AGENTS_INITIALIZE:
    // calculate msg_size
    msg_size += sizeof( int );     // int agent_population
    msg_size += sizeof( int );     // int handle;
    msg_size += sizeof( int );     // int dest_handle a.k.a. placeHandle
    msg_size += sizeof( int );     // classname.size( );
    msg_size += classname.size( ); // classname
    msg_size += sizeof( int );     // argument_size;
    msg_size += argument_size;     // void *argument

    // compose a msg
    pos = msg = new char[msg_size];  
    *(ACTION_TYPE *)pos = action;    pos += sizeof( ACTION_TYPE ); // action
    *(int *)pos = agent_population;  pos += sizeof( int ); // agent_population
    *(int *)pos = handle;            pos += sizeof( int ); // handle
    *(int *)pos = dest_handle;       pos += sizeof( int ); // placeHandle
    *(int *)pos = classname.size( ); pos += sizeof( int ); // classname.size( )
    strncpy( pos, classname.c_str( ), classname.size( ) ); // classname
    pos += classname.size( );
    *(int *)pos = argument_size;     pos += sizeof( int ); // argument_size
    memcpy( (void *)pos, argument, argument_size );      // argument
    
    break;
    
  case AGENTS_CALL_ALL_VOID_OBJECT:
  case AGENTS_CALL_ALL_RETURN_OBJECT:
    msg_size += sizeof(int);	//int handle
    msg_size += sizeof(int);	//int functionId
    msg_size += sizeof(int);	//argument_size
    msg_size += sizeof(int);	//return_size
    msg_size += argument_size;	//void *argument
    
    pos = msg = new char[msg_size];
    *(ACTION_TYPE *)pos = action;	pos += sizeof(ACTION_TYPE);	//action
    *(int *)pos = handle;		pos += sizeof(int);		//handle
    *(int *)pos = functionId;	pos += sizeof(int);		//functionId
    *(int *)pos = argument_size;	pos += sizeof(int);		//arg_size
    *(int *)pos = return_size;	pos += sizeof(int);		//return_size
    memcpy((void *)pos, argument, argument_size);			//argument
    
    break;
  case AGENTS_MANAGE_ALL: 
  case PLACES_EXCHANGE_BOUNDARY:
    // calculate msg_size
    msg_size += sizeof( int );          // it handle;
    
    // compse a msg
    pos = msg = new char[msg_size];
    *(ACTION_TYPE *)pos = action;    pos += sizeof( ACTION_TYPE ); // action
    *(int *)pos = handle;            pos += sizeof( int ); // handle
    
    break;
    
  case FINISH:
    if(printOutput == true)
        cerr << "serialize: FINISH started" << endl;
    msg = new char[msg_size];
    *(ACTION_TYPE *)msg = action;
    if(printOutput == true)
      cerr << "serialize: FINISH ended" << endl;
    break;
    
  default:
    msg_size = 0;
    break;
  }
  if(printOutput == true)
      cerr << "Message::serialize end: action = " << *(ACTION_TYPE *)msg << endl;
  return msg;
}

/**
 * 
 * @param msg
 * @param msg_size
 */
void Message::deserialize( char *msg, int msg_size ) {
  char *cur = msg;
  int size_size = 0;
  int classname_size = 0;
  int hosts_size = 0;
  int destinations_size = 0;
  int exchangeReqListSize = 0;
  int migrationReqListSize = 0;
  DllClass *agentsDllClass = NULL;
  Agent *agent = NULL;
  AgentMigrationRequest *request = NULL;
  int destIndex = 0;

  ostringstream convert;
  if(printOutput == true){
      convert << "deserialize: action = " << *(ACTION_TYPE *)cur << endl;
      MASS_base::log( convert.str( ) );
  }

  switch( *(ACTION_TYPE *)cur ) {
  case EMPTY:
    action = EMPTY;
    return;
  case ACK:
    if(printOutput == true){
        convert.str( "" );
        convert << *(int *)cur << " ";
    }

    action = ACK; cur += sizeof( ACTION_TYPE );
    
    if(printOutput == true){
        convert << *(int *)cur << " ";
    }

    agent_population = *(int *)cur; cur += sizeof( int ); // agent_population

    if(printOutput == true){
        convert << "coming agent_pouplation = " << agent_population;
        MASS_base::log( convert.str( ) );
    }

    if ( msg_size > int( sizeof( ACTION_TYPE ) + sizeof( int ) ) ) {
      argument_size = *(int *)cur; cur += sizeof( int ); // argument_size
      argument_in_heap = ( ( argument = new char[argument_size] ) != NULL ); 
      /*
      convert.str( "" ); convert << "argument_size = " << argument_size;
      MASS_base::log( convert.str( ) );
      */
      memcpy( argument, (void *)cur, argument_size );     // argument
      cur += argument_size;
    }
    if(printOutput == true)
        cerr << "deserialize completed" << endl;
    return;

  case PLACES_INITIALIZE:
    action = PLACES_INITIALIZE; cur += sizeof( ACTION_TYPE );
    size_size = *(int *)cur; cur += sizeof( int ); // size.size( );
    /*
    convert.str( "" ); convert << "size = " << size_size << endl;
    MASS_base::log( convert.str( ) );
    */
    size  = new vector<int>;
    for ( int i = 0; i < size_size; i++ ) {
      /*
      convert.str( "" ); convert << *(int *)cur << endl;
      MASS_base::log( convert.str( ) );
      */ 
      size->push_back( *(int *)cur ); cur += sizeof( int ); // size[i];
    }
    handle = *(int *)cur; cur += sizeof( int );          // handle
    classname_size = *(int *)cur; cur += sizeof( int );  // classname_size
    classname = ""; classname.append( cur, classname_size ); // classname
    // MASS_base::log( classname.c_str( ) );

    cur += classname_size; 
    argument_size = *(int *)cur; cur += sizeof( int );   // argument_size
    argument_in_heap = ( ( argument = new char[argument_size] ) != NULL ); 
    /*
    convert.str( "" ); convert << "argument_size = " << argument_size << endl;
    MASS_base::log( convert.str( ) );
    */
    memcpy( argument, (void *)cur, argument_size );      // argument
    cur += argument_size;
    boundary_width = *(int *)cur; cur += sizeof( int );  // boundary_width
    hosts_size = *(int *)cur; cur += sizeof( int );      // hosts.size( );
    /*
    convert.str( "" ); convert << "host_size = " << hosts_size << endl;
    MASS_base::log( convert.str( ) );
    */
    hosts = new vector<string>;
    for ( int i = 0; i < hosts_size; i++ ) {
      int hostname_size = *(int *)cur; cur += sizeof( int ); // hosts[i].size()
      string hostname = ""; hostname.append( cur, hostname_size );
      cur += hostname_size;
      //MASS_base::log( hostname );
      hosts->push_back( hostname ); // hosts[i]
    }
    return;

  case PLACES_CALL_ALL_VOID_OBJECT:
  case PLACES_CALL_ALL_RETURN_OBJECT:
    action = *(ACTION_TYPE *)cur; cur += sizeof( ACTION_TYPE ); // action
    handle = *(int *)cur;         cur += sizeof( int );         // handle
    functionId = *(int *)cur;     cur += sizeof( int );         // functionId
    argument_size = *(int *)cur;  cur += sizeof( int );         // arg_size
    return_size = *(int *)cur;    cur += sizeof( int );         // return_size
    argument_in_heap = ( ( argument = new char[argument_size] ) != NULL ); 
    memcpy( argument, (void *)cur, argument_size );              // argument
    return;

  case PLACES_CALL_SOME_VOID_OBJECT: break;
  case PLACES_EXCHANGE_ALL:
    action = *(ACTION_TYPE *)cur; cur += sizeof( ACTION_TYPE ); // action
    handle = *(int *)cur;         cur += sizeof( int );         // handle
    dest_handle = *(int *)cur;    cur += sizeof( int );         // dest_handle
    functionId = *(int *)cur;     cur += sizeof( int );         // functionId
    dimension = *(int *)cur;      cur += sizeof( int );         // dimension
    destinations_size = *(int *)cur; cur += sizeof( int );      // dest->size()
    destinations = new vector<int*>;
    argument_in_heap = true;
    
    for ( int i = 0; i < destinations_size; i++ ) {
      int *dest = new int[dimension];
      for ( int j = 0; j < dimension; j++ ) {
	dest[j] = *(int *)cur;    cur += sizeof( int );    // destination[i][j]
      }
      destinations->push_back( dest );
    }
    return;


  case PLACES_EXCHANGE_ALL_REMOTE_REQUEST:
    action = *(ACTION_TYPE *)cur; cur += sizeof( ACTION_TYPE ); // action
    handle = *(int *)cur;         cur += sizeof( int );         // handle
    dest_handle = *(int *)cur;    cur += sizeof( int );         // dest_handle
    functionId = *(int *)cur;     cur += sizeof( int );         // functionId
    exchangeReqListSize = *(int *)cur; cur += sizeof( int );    // list->size
    exchangeReqList = new vector<RemoteExchangeRequest*>;       // list created
    argument_in_heap = true;

    if(printOutput == true){
        convert.str( "" );
        convert << "PLACES_EXCHANGE_ALL_REMOTE_REQUEST: "
	        << " action = " << action << " handle = " << handle
	        << " dest_handle = " << dest_handle 
	        << " functionId = " << functionId
	        << " exchangeReqListSize = " << exchangeReqListSize;
        MASS_base::log( convert.str( ) );
    }

    for ( int i = 0; i < exchangeReqListSize; i++ ) {
      int destIndex  = *(int *)cur; cur += sizeof( int );
      int orgIndex   = *(int *)cur; cur += sizeof( int );
      int inMsgIndex = *(int *)cur; cur += sizeof( int );
      int inMsgSize = *(int *)cur; cur += sizeof( int );
      int outMessageSize = *(int *)cur; cur += sizeof( int );   // outMsgSize
      char *outMessage = new char[outMessageSize];
      memcpy( outMessage, (void *)cur, outMessageSize );         // outMessage
      cur += outMessageSize;

      if(printOutput == true){
          convert.str( "" );
          convert << "PLACES_EXCHANGE_ALL_REMOTE_REQUEST:"
	          << " i = " << i << " destIndex = " << destIndex
	          << " orgIndex = " << orgIndex
	          << " inMsgIndex = " << inMsgIndex
	          << " inMsgSize = " << inMsgSize
	          << " outMessageSize = " << outMessageSize
	          << " outMessage = " << *(int *)outMessage;
          MASS_base::log( convert.str( ) );
      }

      RemoteExchangeRequest *request =
	new RemoteExchangeRequest( destIndex, orgIndex, inMsgIndex, inMsgSize,
				   outMessage,  outMessageSize, true );
      exchangeReqList->push_back( request );
    }
    return;

  case AGENTS_MIGRATION_REMOTE_REQUEST:

    action = *(ACTION_TYPE *)cur;            cur += sizeof( ACTION_TYPE ); // action type
    handle = *(int *)cur;                    cur += sizeof( int );         // agent handle
    dest_handle = *(int *)cur;               cur += sizeof( int );         // place handle
    migrationReqListSize = *(int *)cur;      cur += sizeof( int );         // MigrationReqListy->size
    migrationReqList = new vector<AgentMigrationRequest*>;                 // list created
    argument_in_heap = true;

    if(printOutput == true){
        convert.str( "" );
        convert << "Deserialize: Agent handle: " << handle << " place handle: " << dest_handle;
        MASS_base::log( convert.str() );
    }

    agentsDllClass = MASS_base::dllMap[handle];
    for ( int i = 0; i < migrationReqListSize; i++ ) { // each migrationReq
      destIndex = *(int *)cur;
      cur += sizeof( int );      
      agent = (Agent *)( agentsDllClass->instantiate( NULL ) );
      agent->agentsHandle = *(int *)cur;
      cur += sizeof( int );
      agent->placesHandle = *(int *)cur;
      cur += sizeof( int );
      agent->agentId = *(int *)cur;
      cur += sizeof( int );
      agent->parentId = *(int *)cur;
      cur += sizeof( int );     
      agent->migratableDataSize = *(int *)cur;
      cur += sizeof( int );
      agent->alive = true;
      agent->newChildren = 0;

      if(printOutput == true){
          convert.str( "" );
          convert << "Deserialize: agentId(" << agent << ")[" << agent->agentId << "] data size = "
	          << agent->migratableDataSize;
          MASS_base::log( convert.str() );

          convert.str( "" );
          convert << "Deserialize: agentId(" << agent << ")[" << agent->agentId << "] data = "
	          << (char *)cur;
          MASS_base::log( convert.str() );
      }

      if ( agent->migratableDataSize > 0 ) {
	if(printOutput == true)
	    MASS_base::log( "A" );

	// agent->migratableData = malloc( agent->migratableDataSize );
	agent->migratableData = (void *)(new char[agent->migratableDataSize] );

	if(printOutput == true){
            convert.str( "" );
            convert << "Deserialize: agentId(" << agent << ")[" << agent->agentId << "] malloc = "
	          << agent->migratableData;
            MASS_base::log( convert.str() );
	}

	memcpy( agent->migratableData, (void *)cur,
		agent->migratableDataSize );

	if(printOutput == true)
	    MASS_base::log( "C" );

	cur += agent->migratableDataSize;

	if(printOutput == true)
	    MASS_base::log( "D" );
      }

      if(printOutput == true){
          convert.str( "" );
          convert << "Deserialize: agentId(" << agent << ")[" << agent->agentId << "] migratableData = "
	          << agent->migratableData;
          MASS_base::log( convert.str() );
      }

      request = new AgentMigrationRequest( destIndex, agent );
      migrationReqList->push_back( request );
    }

    return;

  case PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT: 
  case PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST:
    if(printOutput == true){
        convert.str( "" );
        convert << "PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT: will deserialize"
	        << " argument_size = " << argument_size;
        MASS_base::log( convert.str( ) );
    }
    action = *(ACTION_TYPE *)cur; cur += sizeof( ACTION_TYPE ); // action
    argument_size = *(int *)cur;  cur += sizeof( int );         // arg_size
    
    argument_in_heap = ( ( argument = new char[argument_size] ) != NULL ); 
    memcpy( argument, (void *)cur, argument_size );              // argument

    if(printOutput == true){
        convert.str( "" );
        convert << "PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT: deserialization done"
	        << " argument_size = " << argument_size;
        MASS_base::log( convert.str( ) );
    }

    return;

  case AGENTS_INITIALIZE:
    action = *(ACTION_TYPE *)cur;   cur += sizeof( ACTION_TYPE ); // action
    agent_population = *(int *)cur; cur += sizeof( int );         // population
    handle = *(int *)cur;           cur += sizeof( int );         // handle
    dest_handle = *(int *)cur;      cur += sizeof( int );     // placesHandle
    classname_size = *(int *)cur;   cur += sizeof( int );     // classname_size
    classname = ""; classname.append( cur, classname_size );  // classname
    cur += classname_size; 
    argument_size = *(int *)cur;  cur += sizeof( int );         // arg_size
    argument_in_heap = ( ( argument = new char[argument_size] ) != NULL ); 
    memcpy( argument, (void *)cur, argument_size );              // argument

    if(printOutput == true){
        convert.str( "" ); 
        convert << "population = " << agent_population
	        << " handle = " << handle
	        << " placeHandle = " << dest_handle
	        << " classname = " << classname
	        << " argument_size = " << argument_size;
        MASS_base::log( convert.str( ) );
    }

    return;

  case AGENTS_CALL_ALL_VOID_OBJECT:
  case AGENTS_CALL_ALL_RETURN_OBJECT: 
	action = *(ACTION_TYPE *)cur;	cur += sizeof( ACTION_TYPE ); 	//action
	handle = *(int *)cur;		cur += sizeof( int );		//handle
	functionId = *(int *)cur;	cur += sizeof( int );		//functionId
	argument_size = *(int *)cur;	cur += sizeof( int );		//arg_size
	return_size = *(int *)cur;	cur += sizeof( int );		//return_size
	argument_in_heap = ( ( argument = new char[argument_size] ) != NULL);
	memcpy( argument, (void *)cur, argument_size);			//argument
	return;

   return;
  case AGENTS_MANAGE_ALL:
  case PLACES_EXCHANGE_BOUNDARY:
    action = *(ACTION_TYPE *)cur;	cur += sizeof( ACTION_TYPE );	//action
    handle = *(int *)cur;		cur += sizeof( int );		//handle

    return;

  case FINISH: 
    action = FINISH;
    return;

  default:
    break;
  }
}
