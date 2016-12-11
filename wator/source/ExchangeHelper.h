#ifndef EXCHANGEHELPER_H
#define EXCHANGEHELPER_H

#include <string>
#include <map>
#include "Socket.h"
#include "Message.h"

using namespace std;

/**
 *
 */
class ExchangeHelper {
 public:
  void establishConnection( int size, int rank, 
			    vector<string> hosts, int port );
  void sendMessage( int rank, Message *exchangeReq );
  Message *receiveMessage( int rank );
 private:
  static Socket *socket;
  static int *sockets;
};

#endif
