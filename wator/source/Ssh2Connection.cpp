#include "Ssh2Connection.h"

/**
 * Read from the SSH2 connection
 * @param buf
 * @param size
 * @return 
 */
int Ssh2Connection::read( char *buf, int size ) {
  return ( int )libssh2_channel_read( channel, buf, size );
}

/**
 * Write to the SSH2 connection
 * @param buf
 * @param size
 * @return 
 */
int Ssh2Connection::write( char *buf, int size ) {
  return ( int )libssh2_channel_write( channel, buf, size );
}

