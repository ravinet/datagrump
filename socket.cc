#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <cassert>

#include "socket.hh"

using namespace std;
using namespace Network;

/* Default constructor */
Socket::Socket()
  : sock_(-1),
    timestamps_enabled( true ) {}

/* Construct socket of specific type */
Socket::Socket( int sock_type )
  : sock_( socket( AF_INET, sock_type, 0 ) ),
    timestamps_enabled( true )
{
  if ( sock_ < 0 ) {
    perror( "socket" );
    exit( 1 );
  }

  /* Ask for timestamps */
  int ts_opt = 1;
  if ( setsockopt( sock_, SOL_SOCKET, SO_TIMESTAMPNS, &ts_opt,
		   sizeof( ts_opt ) ) < 0 ) {
    perror( "setsockopt" );
    exit( 1 );
  }
}

/* Accept connection request */
Socket Socket::accept( void ) const
{
  int64_t acc = ::accept( sock_, nullptr, nullptr);
  if ( acc  < 0 ) {
    perror( "accept" );
    exit( 1 );
  }
  Socket tmp_socket;
  tmp_socket.set_fd( acc );
  return tmp_socket;
}

/* listen for requests */
void Socket::listen(int BACKLOG ) const
{
  if ( ::listen( sock_, BACKLOG) == -1) {
    perror( "listen" );
    exit( 1 );
  }
}

/* Connect to ip/port (typically used by client) */
Socket Socket::connect( const Address & addr ) const
{
  int64_t connect_fd = ::connect( sock_, (sockaddr *)&addr.sockaddr(),
		                  sizeof( addr.sockaddr() ) );
  if (connect_fd < 0) {
    fprintf( stderr, "Error connecting to %s\n", addr.str().c_str() );
    perror( "connect" );
    exit( 1 );
  }
  Socket tmp_socket;
  tmp_socket.set_fd( connect_fd );
  return tmp_socket;
}

/* Bind to port (typically used by server) */
void Socket::bind( const Address & addr ) const
{
  if ( ::bind( sock_, (sockaddr *)&addr.sockaddr(),
	       sizeof( addr.sockaddr() ) ) < 0 ) {
    fprintf( stderr, "Error binding to %s\n", addr.str().c_str() );
    perror( "bind" );
    exit( 1 );
  }
}

/* Send packet */
void Socket::send( Packet & packet ) const
{
  packet.set_send_timestamp();
  string payload( packet.str() );

  ssize_t bytes_sent = sendto( sock_, payload.data(),
			       payload.size(), 0,
			       (sockaddr *)&packet.addr().sockaddr(),
			       sizeof( packet.addr().sockaddr() ) );

  if ( bytes_sent != static_cast<ssize_t>( payload.size() ) ) {
    perror( "sendto" );
    throw string( "sendto error" );
  }
}

/* Receive a packet and associated timestamp */
Packet Socket::recv( void ) const
{
  static const int RECEIVE_MTU = 2048;

  /* receive source address, timestamp, and payload in msghdr structure */
  struct sockaddr_in packet_remote_addr;
  struct msghdr header;
  struct iovec msg_iovec;

  char msg_payload[ RECEIVE_MTU ];
  char msg_control[ RECEIVE_MTU ];

  /* receive source address */
  header.msg_name = &packet_remote_addr;
  header.msg_namelen = sizeof( packet_remote_addr );

  /* receive payload */
  msg_iovec.iov_base = msg_payload;
  msg_iovec.iov_len = RECEIVE_MTU;
  header.msg_iov = &msg_iovec;
  header.msg_iovlen = 1;

  /* receive timestamp */
  header.msg_control = msg_control;
  header.msg_controllen = RECEIVE_MTU;

  /* receive flags */
  header.msg_flags = 0;

  ssize_t received_len = recvmsg( sock_, &header, 0 );

  if ( received_len < 0 ) {
    perror( "recvmsg" );
    throw string( "recvmsg" );
  }

  if ( header.msg_flags & MSG_TRUNC ) {
    perror( "recvmsg" );
    throw string( "Received oversize datagram" );
  }

  /* verify presence of timestamp */
  if (timestamps_enabled) {
    struct cmsghdr *ts_hdr = CMSG_FIRSTHDR( &header );
    assert( ts_hdr );
    assert( ts_hdr->cmsg_level == SOL_SOCKET );
    assert( ts_hdr->cmsg_type == SO_TIMESTAMPNS );
 
    return Packet( Address( packet_remote_addr ),
		 string( msg_payload, received_len ),
                 *(struct timespec *)CMSG_DATA( ts_hdr ) );
  } else {
    struct timespec ts;
    return Packet( Address( packet_remote_addr ),
		 string( msg_payload, received_len ),
                 ts);

  }
}
