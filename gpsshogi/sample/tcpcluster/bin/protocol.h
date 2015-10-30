/* protocol.h
 */
#ifndef GPSSHOGI_PROTOCOL_H
#define GPSSHOGI_PROTOCOL_H
#include <boost/serialization/serialization.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <string>
#include <iostream>
#include <stdexcept>
#include <cassert>

const char *server_version = "gpsshogi distrubuted learn 0.1";
const int default_port = 8888;
const char *client_ok = "client ok";
const char *bye = "bye";

struct SearchConfig
{
  bool compare_pass;
  int normal_depth, quiesce_depth, search_window, max_progress;
  std::string eval_type;
  SearchConfig() : normal_depth(2)
  {
  }
private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, unsigned int version)
  {
    ar & compare_pass;
    ar & normal_depth;
    ar & quiesce_depth;
    ar & search_window;
    ar & max_progress;
    ar & eval_type;
  }
};

// 0. handshake
// s<-c server_version
// c->s client name
// s<-c client ok
// 1. job request
// c->s job_request prev_version
// s<-c a1. same config + search + records => 2
// s<-c a2. same config + differentiate + records => 3
// s<-c b. new config + data => 1.
// s<-c c. sleep seconds => 1.

inline void write_line(std::ostream& os, const std::string& msg) 
{
  assert(msg.find('\n') == msg.npos);
  os << msg << "\n" << std::flush;
}
inline void getline_or_throw(std::istream& is, std::string& line) 
{
  if (! getline(is, line))
    throw std::runtime_error("disconected");
}
inline void throw_protocol_error()
{
  throw std::runtime_error("protocol error");
}
inline void expect(std::istream& is, const std::string& msg, int line_number)
{
  std::string line;
  getline_or_throw(is, line);
  if (line != msg) {
    std::cerr << "expect failed " << line << " != " << msg
	      << " at " << line_number << "\n";
    throw_protocol_error();
  }
  std::cerr << "ok " << msg << "\n";
}

#define EXPECT(x,y) expect(x, y, __LINE__)


#endif /* GPSSHOGI_PROTOCOL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
