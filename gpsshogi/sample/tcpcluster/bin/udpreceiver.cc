#include "tcpcluster.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <iomanip>

int main()
{
  try
  {   
    using boost::asio::ip::udp;
    boost::asio::io_service io_service;
    udp::socket socket(io_service, udp::endpoint
		       (udp::v4(), gpsshogi::udp_log_port));

    std::array<char, 1024> recv_buf;
    while (true) {
      udp::endpoint remote_endpoint;
      boost::system::error_code error;
      
      const size_t len = socket.receive_from
	(boost::asio::buffer(recv_buf), remote_endpoint, 0, error);
      if (error && error != boost::asio::error::message_size)
	throw boost::system::system_error(error);
      boost::posix_time::ptime now
	= boost::posix_time::second_clock::local_time();
      std::cout << boost::posix_time::to_iso_extended_string(now) << " ";
      std::cout << remote_endpoint << " ";
      std::cout.write(recv_buf.data(), len);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
