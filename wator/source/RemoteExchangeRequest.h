#ifndef REMOTEEXCHANGEREQUEST_H
#define REMOTEEXCHANGEREQUEST_H

class RemoteExchangeRequest {
 friend class Places_base;
 friend class Message;
 public:
  RemoteExchangeRequest( int destIndex, int orgIndex, int inMsgIndex,
			 int inMsgSize, 
			 void *outMsg, int outMsgSize, bool in_heap ) :
    destGlobalLinearIndex( destIndex ), orgGlobalLinearIndex( orgIndex ),
    inMessageIndex( inMsgIndex ), inMessageSize( inMsgSize ),
    outMessage( outMsg ), 
    outMessageSize( outMsgSize ), outMessage_in_heap( in_heap ) {};

  ~RemoteExchangeRequest( ) {
    if ( outMessage_in_heap == true && outMessage != NULL ) {
      delete (char *)outMessage;
    }
  };

  int destGlobalLinearIndex;
  int orgGlobalLinearIndex;
  int inMessageIndex;
  int inMessageSize;
  void *outMessage;
  int outMessageSize;
  bool outMessage_in_heap;
};

#endif
