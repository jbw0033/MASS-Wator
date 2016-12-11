#ifndef PLACES_BASE_H
#define PLACES_BASE_H

#include <string>
#include <vector>
#include "Place.h"

using namespace std;

class Places_base {
  friend class MProcess;
  friend class Agents_base;
  friend class Place;
 public:
  Places_base( int handle, string className, int boundary_width, 
	       void *argument, int argument_size, int dim, int size[] );
  ~Places_base( );

  void callAll( int functionId, void *argument, int tid );
  void **callAll( int functionId, void *argument, int arg_size, 
		  int ret_size, int tid );
  void exchangeAll( Places_base *dstPlaces, int functionId, 
		    vector<int*> *destinations, int tid );
  void exchangeBoundary( );      // called from Places.exchangeBoundary( ) 

  int getHandle( ) { return handle; };
  int getPlacesSize( ) { return places_size; };

 protected:
  const int handle;              // handle 
  const string className;
  const int dimension;
  
  int lower_boundary;
  int upper_boundary;
  int places_size;
  int *size;
  int shadow_size;
  int boundary_width;
  
  void init_all( void *argument, int argument_size );
  vector<int> getGlobalArrayIndex( int singleIndex );
  void getLocalRange( int range[], int tid );
  static void *processRemoteExchangeRequest( void *param );
  void getGlobalNeighborArrayIndex( vector<int> src_index, int offset[],
				    int dst_size[], int dst_dimension,
				    int *dest_index );
  int getGlobalLinearIndexFromGlobalArrayIndex( int dest_index[],
						int dest_size[],
						int dest_dimension );
  int getRankFromGlobalLinearIndex( int globalLinearIndex );
  static void *exchangeBoundary_helper( void *param );  
  static void *sendMessageByChild( void *param );
  struct ExchangeSendMessage {
    int rank;
    void *message;
  };
};

#endif
