#ifndef _GPS_MY_PUBLISHER_H
#define _GPS_MY_PUBLISHER_H

#include <zmq.hpp>
#include <string>
#include <vector>

/**
 * PUSH messages
 */
class MyPublisher
{
  zmq::context_t *context;
  std::string host;
  int port;
  zmq::socket_t publisher;
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
  MyPublisher(zmq::context_t *_context,
              const std::string& _host,
              int _port,
              bool is_bind=false);
  bool publish(const std::string& line);
  bool publish(const std::string& line, const char *first, size_t size);
  bool publish(const std::string& line, const binary_t& binary);
};

#endif /* _GPS_MY_PUBLISHER_H */

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
