#include "Place.h"
#include "MASS_base.h"
#include "limits.h"
#include <iostream>
#include <sstream> // ostringstream

/**
 * Returns the size of the matrix that consists of application-specific
 * places. Intuitively, size[0], size[1], and size[2] correspond to the size
 * of x, y, and z, or that of i, j, and k.
 * @return 
 */
vector<int> Place::getSizeVect(  ){
	return size;
}

/**
 * 
 * @param handle
 * @param offset
 * @return 
 */
Place *Place::findDstPlace( int handle, int offset[] ) {
  // compute the global linear index from offset[]
  Places_base *places = MASS_base::placesMap[ handle ];
  int neighborCoord[places->dimension];
  places->getGlobalNeighborArrayIndex( index, offset, places->size, 
				       places->dimension, neighborCoord );
  int globalLinearIndex 
    = places->getGlobalLinearIndexFromGlobalArrayIndex( neighborCoord,
							places->size,
							places->dimension );
  /*
  ostringstream convert;
  convert << "globalLinearIndex = " << globalLinearIndex << endl;
  MASS_base::log( convert.str( ) );
  */

  if ( globalLinearIndex == INT_MIN )
    return NULL;

  // identify the destination place
  DllClass *dllclass = MASS_base::dllMap[ handle ];
  int destinationLocalLinearIndex
    = globalLinearIndex - places->lower_boundary;

  Place *dstPlace = NULL;
  int shadow_index;
  if ( destinationLocalLinearIndex >= 0 && 
       destinationLocalLinearIndex < places->places_size )
    dstPlace = dllclass->places[ destinationLocalLinearIndex ];
  else if ( destinationLocalLinearIndex < 0 && 
       ( shadow_index = destinationLocalLinearIndex + places->shadow_size ) 
       >= 0 )
    dstPlace = dllclass->left_shadow[ shadow_index ];
  else if ( (shadow_index = destinationLocalLinearIndex - places->places_size)
	    >= 0
	    && shadow_index < places->shadow_size )
    dstPlace = dllclass->right_shadow[ shadow_index ];



  return dstPlace;
}

/**
 * 
 * @param handle
 * @param offset
 * @return 
 */
void *Place::getOutMessage( int handle, int offset[] ) {

  Place *dstPlace = findDstPlace( handle, offset );

  // return the destination outMessage
  return ( dstPlace != NULL ) ? dstPlace->outMessage : NULL;

}

/**
 * 
 * @param handle
 * @param offset
 * @param position 0 = North, 1 = East, 2 = South, 3 = West
 * @param value
 */
void Place::putInMessage( int handle, int offset[], int position, 
			  void *value ){

  Place *dstPlace = findDstPlace( handle, offset );

  // fill out the space if inMessages are empty
  for ( int i = 0; i <= position; i++ ) 
    if ( int( dstPlace->inMessages.size( ) ) 
	 <= i && dstPlace->inMessage_size > 0 )
      dstPlace->inMessages.push_back( malloc( dstPlace->inMessage_size ) );

  // write to the destination inMessage[position]
  if ( dstPlace != NULL && position < int( dstPlace->inMessages.size( ) ) ) 
    memcpy( dstPlace->inMessages[ position ], value, 
	    dstPlace->inMessage_size );
}
