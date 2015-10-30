#include "mySubscriber.h"
#include <boost/format.hpp>

MySubscriber::MySubscriber(zmq::context_t *_context,
                           const std::string& _host,
                           int _port,
                           bool is_bind)
  : context(_context),
    host(_host),
    port(_port),
    subscriber(*context, ZMQ_PULL)
{
  //subscriber.connect("ipc://wdoorgraphic.ipc");
  std::string hostport = boost::str(boost::format("tcp://%s:%d") % host % port);
  if (is_bind)
    subscriber.bind(hostport.c_str());
  else
    subscriber.connect(hostport.c_str());
  //static const char *filter = "png ";
  //subscriber.setsockopt(ZMQ_SUBSCRIBE, filter, strlen (filter));
}

int
MySubscriber::subscribe(std::string& s)
{
  zmq::message_t msg;
  subscriber.recv(&msg);
  s.assign(static_cast<char*>(msg.data()), 0, msg.size());
}

int
MySubscriber::subscribe(std::string& filename, binary_t& binary)
{
  // first, a file name
  subscribe(filename);

  // then, binary content
  int64_t more;
  size_t more_size = sizeof (more);
  zmq_getsockopt (subscriber, ZMQ_RCVMORE, &more, &more_size);
  assert(more);
  if (more) {
    zmq::message_t msg;
    subscriber.recv(&msg);
    binary.reserve(msg.size());
    const char *first = static_cast<char*>(msg.data());
    const char *last  = first + msg.size();
    binary.insert(binary.end(), first, last);
  }
}

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
