#ifndef MASS_H
#define MASS_H

#include <vector>             // vector
#include "MASS_base.h"
#include "Utilities.h"
#include "Ssh2Connection.h"
#include "MNode.h"
#include "Places.h"
#include "Agents.h"

#define LIBSSH2_PORT 22

/**
 * 
 */
class MASS : public MASS_base {
  friend class Places;
  friend class Agents;
 public:
  static void init( char *args[], int nProc, int nThr );
  static void finish( );
 private:
  static Utilities util;
  static vector<MNode*> mNodes;
  static void barrier_all_slaves( ) 
    { barrier_all_slaves( NULL, 0, 0, NULL ); };
  static void barrier_all_slaves( int localAgents[] )
    { barrier_all_slaves( NULL, 0, 0, localAgents ); };
  static void barrier_all_slaves( char *return_values, int stripe, 
				  int arg_size )
    { barrier_all_slaves( return_values, stripe, arg_size, NULL ); };
  static void barrier_all_slaves( char *return_values, int stripe, 
				  int arg_size, int localAgents[] );
};

#endif
