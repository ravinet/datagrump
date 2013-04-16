#ifndef SOCKET_HH
#define SOCKET_HH

#include "address.hh"
#include "packet.hh"

namespace Network {
/* Socket class. Wraps socket() */
  class Socket {
  private:
    /* the socket file descriptor */
    int sock_;
    
    /* bool that indicates if timestamps are enabled */
    bool timestamps_enabled;

  public:
    /* Default constructor */
    Socket();
    
    /* Construct socket of specific type */
    Socket( int sock_type );

    /* Accept connection request */
    Socket accept( void ) const;

    /* listen for requests */
    void listen(int BACKLOG ) const;

    /* Bind to port (typically used by server) */
    void bind( const Address & addr ) const;

    /* Connect to ip/port (typically used by client) */
    Socket connect( const Address & addr ) const;

    /* Send packet */
    void send( Packet & packet ) const;

    /* Receive a packet */
    Packet recv( void ) const;

    /* Getter for underlying socket fd */
    int fd( void ) const { return sock_; }

    /* Setter for underlying socket fd */
    void set_fd( int sock ) { sock_ = sock; }

    /* Disable timestamps */
    void disable_timestamps( void ) { timestamps_enabled = false; }
  };
}

#endif
