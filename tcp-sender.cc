#include <stdio.h>
#include <unistd.h>
#include <string>
#include <poll.h>

#include "socket.hh"
#include "controller.hh"

using namespace std;
using namespace Network;

int main( int argc, char *argv[] )
{
  /* check arguments */
  bool debug = false;
  if ( argc == 4 && string( argv[ 3 ] ) == "debug" ) {
    debug = true;
  } else if ( argc == 3 ) {
    /* do nothing */
  } else {
    fprintf( stderr, "Usage: %s IP PORT [debug]\n", argv[ 0 ] );
    exit( 1 );
  }

  try {
    /* Fill in destination address from command line arguments */
    Address destination( argv[ 1 ] /* ip */, argv[ 2 ] /* port */ );

    fprintf( stderr, "Sending packets to %s:%d.\n",
	     destination.ip().c_str(), destination.port() );

    /* Create TCP socket for outgoing TCP segments. */
    Network::Socket sock( SOCK_STREAM );

    /* Block until you connect to remote server/receiver */
    sock.connect( destination );
    fprintf( stderr, "Connected successfully to receiver \n");

    /* Loop */
    while ( 1 ) {
      Packet x( destination, 0xDEADBEEF );
      sock.send( x );
      if (debug) {
        fprintf( stderr, "Sending one more TCP packet to %s:%d.\n",
	         destination.ip().c_str(), destination.port() );
      }
    }
  } catch ( const string & exception ) {
    /* We got an exception, so quit. */
    fprintf( stderr, "Exiting on exception: %s\n", exception.c_str() );
    exit( 1 );
  }
  return 0;
}
