#include "protocol.h"

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <stdexcept>

using namespace boost::asio;
void run(std::iostream& s)
{
  std::cerr << "[connect start]\n";
  write_line(s, server_version);
  std::string line;
  {
    // name
    getline_or_throw(s, line);
    std::istringstream is(line);
    std::string client, name;
    if (! (is >> client >> name))
      throw_protocol_error();
    write_line(s, client_ok);
    std::cerr << "name = " << name << "\n";
  }
  getline_or_throw(s, line);
  if (line != bye)
    throw_protocol_error();
  std::cerr << "[connect end]\n";
}

int main()
{
  io_service io;
  try {
    ip::tcp::acceptor acc(io, ip::tcp::endpoint(ip::tcp::v4(), default_port));
    std::cerr << "server: start waiting\n";
    for (;;) {
      ip::tcp::iostream s;
      acc.accept(*s.rdbuf());
      try {
	run(s);
      }
      catch (std::exception& e) {
	std::cerr << e.what() << "\n";
      }
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
  return 0;    
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
