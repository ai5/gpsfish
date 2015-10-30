#include "myPublisher.h"
#include <boost/cstdint.hpp>
#include <boost/format.hpp>
#include <iostream>

MyPublisher::MyPublisher(zmq::context_t *_context,
                        const std::string& _host,
                        int _port,
                        bool is_bind)
    : context(_context),
      host(_host),
      port(_port),
      publisher(*context, ZMQ_PUSH)
{
  //publisher.bind("ipc://wdoorgraphic.ipc");
  std::string hostport = boost::str(boost::format("tcp://%s:%d") % host % port);
  const boost::uint64_t HWM = 1;
  publisher.setsockopt(ZMQ_HWM, &HWM, sizeof(HWM));
  if (is_bind)
    publisher.bind(hostport.c_str());
  else
    publisher.connect(hostport.c_str());
}

bool
MyPublisher::publish(const std::string& line)
{
  zmq::message_t message(line.size());
  memcpy(message.data(), line.data(), line.size());
  return publisher.send(message, ZMQ_NOBLOCK);
}

bool
MyPublisher::publish(const std::string& line,
                     const char *first,
                     size_t size)
{
  if (size == 0)
    return false;

  zmq::message_t message(line.size());
  memcpy(message.data(), line.data(), line.size());
  const bool rc = publisher.send(message, ZMQ_NOBLOCK | ZMQ_SNDMORE);
  if (!rc) {
    std::cerr << "Failed to send " << line << "\n";
    return false;
  }

  zmq::message_t out(size);
  memcpy(out.data(), first, size);
  return publisher.send(out, ZMQ_NOBLOCK);
}

bool
MyPublisher::publish(const std::string& line,
                     const binary_t& binary)
{
  if (binary.empty())
    return false;
  return publish(line, &binary[0], binary.size());
}

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
