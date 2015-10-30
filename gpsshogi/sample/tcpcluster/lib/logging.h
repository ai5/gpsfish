/* logging.h
 */
#ifndef GPSSHOGI_LOGGING_H
#define GPSSHOGI_LOGGING_H
#include <boost/lexical_cast.hpp>
#include <boost/asio/io_service.hpp>
#include <iosfwd>
namespace gpsshogi
{
  struct Logging
  {
    static void error(const std::string& msg);
    static void warn(const std::string& msg);
    static void notice(const std::string& msg);
    static void info(const std::string& msg, bool important=false);

    static void rebind(std::ostream *os);
    static void setQueue(boost::asio::io_service& queue);
    static const std::string datetime();
    static const std::string time();

    static void writeLine(std::ostream& os, const std::string& msg, bool important);
    static void writeLineTo(std::string time, std::string msg, bool to_file, bool important=false);
    static void schedule(const std::string& time, const std::string& msg,
			 boost::asio::io_service*, bool to_file=true,
			 bool important=false);

    static void startUdpLogging(const std::string& host_port,
				boost::asio::io_service& udp_queue);
    static void udpLine(std::string log);
    static void enableSlaveUdp(bool enable);
    static void slaveUdp(const std::string& name, std::string msg);
    static void quit();
    static std::string makeAndSetNewDirectory();
    static std::string path() { return directory+"/"; }
    static void setPrefix(std::string);
  private:
    static void setPrefixInQueue(std::string);
    static std::string directory;
  };
  template <class T> inline const std::string to_s(const T& a) 
  {
    return boost::lexical_cast<std::string>(a);
  }
}

#endif /* GPSSHOGI_LOGGING_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
