#ifndef _GPS_MY_SUBSCRIBER_H
#define _GPS_MY_SUBSCRIBER_H

#include <zmq.hpp>
#include <string>
#include <vector>

class MySubscriber
{
  zmq::context_t *context;
  std::string host;
  int port;
  zmq::socket_t subscriber;
public:
  typedef std::vector<char> binary_t;

  /**
   * Constructor.
   * @param _context pointer to a context_t instance
   * @param _host host address
   * @param _port port number
   * @param is_bind if it is true, a socket starts up as a server; a socket
   *        connects to the host and port, otherwise. 
   */
  MySubscriber(zmq::context_t *_context,
               const std::string& _host,
               int _port,
               bool is_bind=false);
  int subscribe(std::string& s);
  int subscribe(std::string& finename, binary_t& binary);
};

#endif /* _GPS_MY_SUBSCRIBER_H */

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
