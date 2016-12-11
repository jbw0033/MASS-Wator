#ifndef MESSAGE_H
#define MESSAGE_H
#define VOID_HANDLE -1

#include <string>
#include <string.h>
#include <vector>
#include "RemoteExchangeRequest.h"
#include "AgentMigrationRequest.h"

using namespace std;

class Message
{
public:

  /**
   * ACTION_TYPE
   * A list of actions assigned to numbers.
   */
  enum ACTION_TYPE
  {
    EMPTY, // 0 
    FINISH, // 1
    ACK, // 2

    PLACES_INITIALIZE, // 3
    PLACES_CALL_ALL_VOID_OBJECT, // 4
    PLACES_CALL_ALL_RETURN_OBJECT, // 5
    PLACES_CALL_SOME_VOID_OBJECT,
    PLACES_EXCHANGE_ALL, // 7
    PLACES_EXCHANGE_ALL_REMOTE_REQUEST, // 8
    PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT, // 9
    PLACES_EXCHANGE_BOUNDARY, // 10
    PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST, // 11

    AGENTS_INITIALIZE, // 12
    AGENTS_CALL_ALL_VOID_OBJECT, // 13
    AGENTS_CALL_ALL_RETURN_OBJECT, // 14
    AGENTS_MANAGE_ALL, // 15
    AGENTS_MIGRATION_REMOTE_REQUEST // 16
  };

  /**
   * PLACES_INITIALIZE
   * @param action
   * @param size
   * @param handle
   * @param classname
   * @param argument
   * @param arg_size
   * @param boundary_width
   * @param hosts
   */
  Message (ACTION_TYPE action,
           vector<int> *size, int handle, string classname, void *argument,
           int arg_size, int boundary_width, vector<string> *hosts) :
  action (action), size (size),
  handle (handle), dest_handle (VOID_HANDLE),
  functionId (0), classname (classname),
  argument (argument), argument_size (arg_size), return_size (0),
  argument_in_heap (false), hosts (hosts), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (boundary_width),
  exchangeReqList (NULL),
  migrationReqList (NULL) { };

  /**
   * PLACES_CALL_ALL_VOID_OBJECT,
   * PLACES_CALL_ALL_RETURN_OBJECT,
   * AGENTS_CALL_ALL_VOID_OBJECT,
   * AGENTS_CALL_ALL_RETURN_OBJECT
   * @param action
   * @param handle
   * @param functionId
   * @param argument
   * @param arg_size
   * @param ret_size
   */
  Message (ACTION_TYPE action,
           int handle, int functionId, void *argument, int arg_size,
           int ret_size) :
  action (action), size (0),
  handle (handle), dest_handle (VOID_HANDLE),
  functionId (functionId), classname (""),
  argument (argument), argument_size (arg_size), return_size (ret_size),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL) { };

  /**
   * PLACES_EXCHANGE_ALL
   * @param action
   * @param handle
   * @param dest_handle
   * @param functionId
   * @param destinations
   * @param dimension
   */
  Message (ACTION_TYPE action,
           int handle, int dest_handle, int functionId,
           vector<int*> *destinations, int dimension) :
  action (action), size (0),
  handle (handle), dest_handle (dest_handle),
  functionId (functionId), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (destinations),
  dimension (dimension), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL) { };

  /**
   * PLACES_EXCHANGE_ALL_REMOTE_REQUEST
   * @param action
   * @param handle
   * @param dest_handle
   * @param functionId
   * @param exchangeReqList
   */
  Message (ACTION_TYPE action,
           int handle, int dest_handle, int functionId,
           vector<RemoteExchangeRequest*> *exchangeReqList) :
  action (action), size (0),
  handle (handle), dest_handle (dest_handle),
  functionId (functionId), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (exchangeReqList), migrationReqList (NULL) { };

  /**
   * PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT and 
   * PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST
   * @param action
   * @param retVals
   * @param retValsSize
   */
  Message (ACTION_TYPE action, char *retVals, int retValsSize) :
  action (action), size (0),
  handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
  functionId (0), classname (""),
  argument (retVals), argument_size (retValsSize), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL) { };

  /**
   * AGENTS_INITIALIZE
   * @param action
   * @param initPopulation
   * @param handle
   * @param placeHandle
   * @param className
   * @param argument
   * @param argument_size
   */
  Message (ACTION_TYPE action, int initPopulation, int handle,
           int placeHandle, string className, void *argument,
           int argument_size) :
  action (action), size (0),
  handle (handle), dest_handle (placeHandle),
  functionId (0), classname (className),
  argument (argument), argument_size (argument_size), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (initPopulation), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL) { };

  /**
   * AGENTS_MANAGE_ALL and PLACES_EXCHANGE_BOUNDARY
   * @param action
   * @param handle
   * @param dummy
   */
  Message (ACTION_TYPE action, int handle, int dummy) :
  action (action), size (0),
  handle (handle), dest_handle (handle),
  functionId (0), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL) { };

  /**
   * AGENTS_MIGRATION_REMOTE_REQUEST
   * @param action
   * @param agentHandle
   * @param placeHandle
   * @param migrationReqList
   */
  Message (ACTION_TYPE action, int agentHandle, int placeHandle,
           vector<AgentMigrationRequest*> *migrationReqList) :
  action (action), size (0),
  handle (agentHandle), dest_handle (placeHandle),
  functionId (0), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (migrationReqList) { };

  /**
   * FINISH
   * ACK
   * @param action
   */
  Message (ACTION_TYPE action) :
  action (action), size (NULL),
  handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
  functionId (0), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL) { };

  /**
   * ACK used for PLACES_CALL_ALL_RETURN_OBJECT
   * @param action
   * @param argument
   * @param arg_size
   */
  Message (ACTION_TYPE action, void *argument, int arg_size) :
  action (action), size (NULL),
  handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
  functionId (0), classname (""),
  argument (argument), argument_size (arg_size), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL) { };


  /**
   * ACK used for AGENTS_CALL_ALL_RETURN_OBJECT
   * @param action
   * @param argument
   * @param arg_size
   * @param localPopulation
   */
  Message (ACTION_TYPE action, void *argument, int arg_size, int localPopulation) :
  action (action), size (NULL),
  handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
  functionId (0), classname (""),
  argument (argument), argument_size (arg_size), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (localPopulation), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL) { };

  /**
   * ACK used for AGENTS_INITIALIZE and AGENTS_CALL_ALL_VOID_OBJECT
   * @param action
   * @param localPopulation
   */
  Message (ACTION_TYPE action, int localPopulation) :
  action (action), size (NULL),
  handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
  functionId (0), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (localPopulation), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL) { };

  
  /**
   * EMPTY
   */
  Message () :
  action (EMPTY), size (NULL),
  handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
  functionId (0), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL) { };

  /**
   * 
   */
  ~Message (); // delete argument and hosts.

  char *serialize (int &size);
  void deserialize (char *msg, int size);

  /**
   * Get the action
   * @return action
   */
  ACTION_TYPE getAction ()
  {
    return action;
  };

  /**
   * Get the size
   * @return *size
   */
  vector<int> getSize ()
  {
    return *size;
  };

  /**
   * Get the handle
   * @return handle
   */
  int getHandle ()
  {
    return handle;
  };

  /**
   * Get the destination handle
   * @return dest_handle
   */
  int getDestHandle ()
  {
    return dest_handle;
  };

  /**
   * Get the functionId
   * @return functionId
   */
  int getFunctionId ()
  {
    return functionId;
  };

  /**
   * Get the class name
   * @return classname
   */
  string getClassname ()
  {
    return classname;
  };

  /**
   * Check if argument is valid
   * @return (argument != NULL)
   */
  bool isArgumentValid ()
  {
    return ( argument != NULL);
  };

  /**
   * Get the argument via memcpy
   */
  void getArgument (void *arg)
  {
    memcpy (arg, argument, argument_size);
  };

  /**
   * Get the argument pointer
   * @return argument
   */
  void *getArgumentPointer ()
  {
    return argument;
  };

  /**
   * Get the argument size
   * @return argument_size
   */
  int getArgumentSize ()
  {
    return argument_size;
  };

  /**
   * Get the return size
   * @return return_size
   */
  int getReturnSize ()
  {
    return return_size;
  };

  /**
   * Get the Boundary Width
   * @return boundary_width
   */
  int getBoundaryWidth ()
  {
    return boundary_width;
  };

  /**
   * Get the Agent Populations
   * @return agent_population
   */
  int getAgentPopulation ()
  {
    return agent_population;
  };

  /**
   * Get the hosts
   * @return *hosts
   */
  vector<string> getHosts ()
  {
    return *hosts;
  };

  /**
   * Get the destinations
   * @return destinations
   */
  vector<int*> *getDestinations ()
  {
    return destinations;
  };

  /**
   * Get the Remote Exchange Request List
   * @return exchangeReqList
   */
  vector<RemoteExchangeRequest*> *getExchangeReqList ()
  {
    return exchangeReqList;
  };

  /**
   * Get the Agent Migration Request List
   * @return migrationReqList
   */
  vector<AgentMigrationRequest*> *getMigrationReqList ()
  {
    return migrationReqList;
  };

protected:
  ACTION_TYPE action;
  vector<int> *size;
  int handle;
  int dest_handle;
  int functionId;
  string classname; // classname.so must be located in CWD.
  void *argument;
  int argument_size;
  int return_size;
  bool argument_in_heap;
  vector<string> *hosts; // all hosts participated in computation
  vector<int*> *destinations; // all destinations of exchangeAll
  int dimension;
  int agent_population;
  int boundary_width;
  vector<RemoteExchangeRequest*> *exchangeReqList;
  vector<AgentMigrationRequest*> *migrationReqList;
};

#endif
