#ifndef MTHREAD_H
#define MTHREAD_H

#include "MASS_base.h"
#include <pthread.h>

class Mthread {
 public:
     
/**
  * Status Type
  * A list of possible statuses
  */
  enum STATUS_TYPE { STATUS_READY,          // 0
                     STATUS_TERMINATE,      // 1
                     STATUS_CALLALL,        // 2
		     STATUS_EXCHANGEALL,    // 3
		     STATUS_AGENTSCALLALL,  // 4
		     STATUS_MANAGEALL	    // 5
  };

  static void init( );
  static void *run( void *param );
  static void resumeThreads( STATUS_TYPE new_status );
  static void barrierThreads( int tid );

  static pthread_mutex_t lock;
  static pthread_cond_t barrier_ready;
  static pthread_cond_t barrier_finished;
  static int barrier_count;
  static STATUS_TYPE status;

  static int threadCreated;
  static int agentBagSize;
};

#endif
