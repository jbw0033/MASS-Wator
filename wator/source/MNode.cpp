#include "MNode.h"
#include "MASS_base.h"
#include <sstream> // ostringstream

//Used to toggle output for MNode
const bool printOutput = false;
//const bool printOutput = true;

using namespace std;

/**
* 
*/
void MNode::closeMainConnection( ) {
}

/**
 * 
 * @param m
 */
void MNode::sendMessage( Message *m ) {
  int size;
  char *buf = m->serialize( size );
  if(printOutput == true)
      cerr << "size = " << size << endl;
  if ( write( sd, (char *)&size, sizeof( int ) ) > 0 ) {
    if ( write( sd, buf, size ) > 0 ) {
      return;
    }
  }
  MASS_base::log( "sendMessage error" );
  exit( -1 );
}

/**
 * 
 * @return 
 */
Message *MNode::receiveMessage( ) {
  int size = -1;
  int nRead = 0;
  if ( ( nRead = read( sd, (char *)&size, sizeof( int ) ) ) > 0 ) {
    if(printOutput == true)
        cerr << "nRead = " << nRead << ", size = " << size << endl;
    char *buf = new char[size];
    for ( nRead = 0; 
	  ( nRead += read( sd, buf + nRead, size - nRead) ) < size; );
    if(printOutput == true)
        cerr << "nRead = " << nRead << endl;
    if ( size > 4 ){
      if(printOutput == true)
          cerr << "*(int *)(buf + 4) = " << *(int *)(buf + 4) << endl;
    }
    Message *m = new Message( );
    m->deserialize( buf, size );
    return m;
  }
  else if ( nRead <= 0 ) {
    ostringstream convert;
    convert << "receivMessage error from rank[" << pid << "] at " 
	    << hostName;
    MASS_base::log( convert.str( ) );
    exit( -1 );
  }
  return NULL;
}

