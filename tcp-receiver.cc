#include <stdio.h>

#include "socket.hh"

using namespace Network;
using namespace std;

int main( int argc, char *argv[] )
{
  /* check arguments */
  if ( argc != 2 ) {
    fprintf( stderr, "Usage: %s PORT\n", argv[ 0 ] );
    exit( 1 );
  }

  try {
    /* Create TCP socket for incoming TCP segments. */
    Network::Socket sock( SOCK_STREAM );

    /* Bind to TCP port. */
    sock.bind( Address( "0" /* all IP addresses */,
			argv[ 1 ] /* port */ ) );

    fprintf( stderr, "Binding on port %s...\n", argv[ 1 ] );

    /* Put socket in "listening" mode */
    sock.listen( 1 ); /* BACKLOG of 1 */

    /* Block until you can accept a connection */
    auto remote_socket = sock.accept();
    fprintf( stderr, "Accepted connection successfully from sender \n" );

    /* Loop */
    while ( 1 ) {
      Packet received_packet = remote_socket.recv();
      printf( "Received TCP packet here  %s \n", received_packet.str().c_str() );
    }
  } catch ( const string & exception ) {
    /* We got an exception, so quit. */
    fprintf( stderr, "Exiting on exception: %s\n", exception.c_str() );
    exit( 1 );
  }

  return 0;
}
