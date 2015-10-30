#include "tcpcluster.h"
#include <boost/asio.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <iostream>
#include <string>
#include <stdexcept>

using namespace boost::asio::ip;
void getline_or_throw(tcp::socket& socket,
		      boost::asio::streambuf& buffer,
		      std::string& line) 
{
  boost::asio::read_until(socket, buffer, '\n');
  std::istream response_stream(&buffer);
  if (! getline(response_stream, line))
    throw std::runtime_error("disconected");
}
void write_line(tcp::socket& socket, const std::string& msg) 
{
  assert(msg.find('\n') == msg.npos);
  boost::asio::streambuf buffer;
  std::ostream os(&buffer);
  os << msg << '\n';
  boost::asio::write(socket, buffer);
}

void run(tcp::socket& socket)
{
  std::cerr << "## connection from " << socket.remote_endpoint() << "\n";
  boost::asio::streambuf buffer;
  std::string command, response;
  while ((std::cerr << "##--- ready" << "\n" << std::flush)
	 && getline(std::cin, command) && ! command.empty()) {
    if (command[0] != '#' && command[0] != '<') {
      write_line(socket, command);
      continue;
    }
    if (command == "<") {
      getline_or_throw(socket, buffer, response);
      std::cout << response << "\n" << std::flush;
    }
    else if (command[0] == '<') {
      std::string arg = command.substr(1);
      boost::algorithm::trim(arg);
      std::cerr << "##read until ^" << arg << "\n";
      do {
	getline_or_throw(socket, buffer, response);
	std::cout << response << "\n";
      } while (response.find(arg) != 0);
      std::cout << std::flush;
    }
    else if (command == "#quit") {
      write_line(socket, "quit");
      sleep(1);
      break;
    }
    else {
      std::cerr << "## unknown command\n" << std::flush;
    }
  }
  std::cerr << "## end\n";
}

int main()
{
  boost::asio::io_service io;
  try {
    tcp::acceptor acc(io, tcp::endpoint(tcp::v4(), gpsshogi::cluster_port));
    std::cerr << "server: start waiting\n";
    for (;;) {
      std::cerr << "### waiting usi client\n";
      tcp::socket socket(io);
      acc.accept(socket);
      try {
	run(socket);
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
