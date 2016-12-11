#ifndef PLACE_H
#define PLACE_H

#include "MObject.h"
#include <vector>
#include <iostream>

using namespace std;

class Place : MObject {
  friend class Places_base;
  friend class Agents_base;
  friend class Agent;
 public:
   
  /**
   * Is the default constructor. A contiguous space of arguments is passed
   * to the constructor.
   * @param argument
   */
  Place( void *argument ) : outMessage( NULL ), outMessage_size( 0 ), 
    inMessage_size( 0 ) { inMessages.clear( ); };
 
  vector<int> getSizeVect( );
  
  /**
   * Is called from Places.callAll( ), callSome( ), exchangeAll( ), and
ex * exchangeSome( ), and invoke the function specified with functionId as
pa * passing arguments to this function. A user-derived Place class must
im * implement this method.
   * @param functionId
   * @param argument
   * @return 
   */
  virtual void *callMethod( int functionId, void *argument ) = 0;
  ~Place( ) { };		//SHOULD BE VIRTUAL


  // VARIABLES
  
/**
  * Defines the size of the matrix that consists of application-specific
  * places. Intuitively, size[0], size[1], and size[2] correspond to the size
  * of x, y, and z, or that of i, j, and k.
  */
  vector<int> size;
 /**  
  * Is an array that maintains each place’s coordinates. Intuitively,
  * index[0], index[1], and index[2] correspond to coordinates of x, y, and
  * z, or those of i, j, and k. 
  */
  vector<int> index;
  
 /** Stores a set arguments to be passed to a set of remote-cell functions
  * that will be invoked by exchangeAll( ) or exchangeSome( ) in the
  * nearest future. The argument size must be specified with
  * outMessage_size. 
  */
  void *outMessage;
  
 /** Receives a return value in inMessages[i] from a function call made to
  * the i-th remote cell through exchangeAll( ) and exchangeSome( ).
  * Each element size must be specified with inMessage_size. 
  */
  vector<void*> inMessages;
  
 /**
  * Defines the size of outMessage.
  */
  int outMessage_size;
  
 /**
  * Defines the size of inMessage.
  */
  int inMessage_size;
  
  /** Includes all the agents residing locally on this place. */
  vector<MObject*> agents; 

 protected:
  void *getOutMessage( int handle, int index[] );
  void putInMessage( int handle, int index[], int position, void *value );

 private:
  Place *findDstPlace( int handle, int index[] );
};

#endif
