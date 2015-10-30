#include "board.h"
#include "commands.h"
#ifndef MINIMAL_GPSSHELL
#  include "SReadline.h"
#endif
#include "book.h"

#include "osl/oslConfig.h"
#include "osl/eval/progressEval.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/misc/filePath.h"
#include "osl/record/csaRecord.h"
#include "osl/record/record.h"
#include "gpsshogi/revision.h"

#ifndef MINIMAL_GPSSHELL
#  include <Poco/Net/HTTPStreamFactory.h>
#ifdef ENABLE_REDIS
#  include <glog/logging.h>
#endif
#endif
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef MINIMAL_GPSSHELL
using namespace swift;
#endif
    

/*========== Global variables ==========*/

boost::filesystem::path cache_dir;
std::unique_ptr<gpsshell::Board> board;
std::unique_ptr<gpsshell::Book> the_book;
// For MINIMAL_GPSSHELL, redis_server_host, redis_server_port and
// redis_password should be empty, zero and empty respectively.
std::string redis_server_host;
int redis_server_port=0;
std::string redis_password;
    

/*========== Classes and Funcitons ==========*/



/*========== MAIN ==========*/

void printUsage(std::ostream& out, 
                char **argv,
                const boost::program_options::options_description& command_line_options)
{
  out << 
    "Usage: " << argv[0] << " [options]" << "\n"
      << command_line_options 
      << std::endl;
}

int parseCommandLine(int argc, char **argv, 
                     boost::program_options::variables_map& vm)
{
  namespace bp = boost::program_options;
  bp::options_description command_line_options;
  command_line_options.add_options()
    ("color",   bp::value<std::string>(),
                "Specify three comma-separated colors to show colorful pieces for BLACK, WHITE and the last move, respectively. eX. --color blue,brown,red")
    ("stdin",   bp::value<std::string>(),
                "Read a file from a specified type from the stdin.")
    ("eval-data",   bp::value<std::string>(),
                "data for evaluation function.")
#ifndef MINIMAL_GPSSHELL
#ifdef ENABLE_REDIS
    ("redis-host",     bp::value<std::string>(),
                       "IP of the redis server")
    ("redis-password", bp::value<std::string>(),
                       "password to connect to the redis server")
    ("redis-port",     bp::value<int>(),
                       "port number of the redis server")
#endif
#endif
    ("version", "Show version information")
    ("help,h",  "Show help message");
  bp::positional_options_description p;

  try
  {
    bp::store(
      bp::command_line_parser(
	argc, argv).options(command_line_options).positional(p).run(), vm);
    bp::notify(vm);
    if (vm.count("help"))
    {
      printUsage(std::cout, argv, command_line_options);
      return 0;
    }
    if (vm.count("version")) 
    {
      std::string name = "gpsshell ";
#ifdef OSL_SMP
      name += "(smp) ";
#endif
      std::cout << name << gpsshogi::gpsshogi_revision << "\n\n"
        << gpsshogi::gpsshogi_copyright << "\n";
      return 0;
    }
#ifndef MINIMAL_GPSSHELL
#ifdef ENABLE_REDIS
    if (vm.count("redis-host")) {
      redis_server_host.assign(vm["redis-host"].as<std::string>());
    } else if (getenv("GPS_REDIS_HOST")) {
      redis_server_host.assign(getenv("GPS_REDIS_HOST"));
    }
    if (vm.count("redis-port")) {
      redis_server_port = vm["redis-port"].as<int>();
    } else if (getenv("GPS_REDIS_PORT")) {
      redis_server_port = boost::lexical_cast<int>(getenv("GPS_REDIS_PORT"));
    }
    if (vm.count("redis-password")) {
      redis_password.assign(vm["redis-password"].as<std::string>());
    } else if (getenv("GPS_REDIS_PASSWORD")) {
      redis_password.assign(getenv("GPS_REDIS_PASSWORD"));
    }
#endif
#endif
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << "\n"
	      << e.what() << std::endl;
    printUsage(std::cerr, argv, command_line_options);
    return 1;
  }

  if (vm.count("color"))
  {
    const std::string color = vm["color"].as<std::string>();
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(",");
    tokenizer tokens(color, sep);
    tokenizer::iterator each = tokens.begin();
    assert(each != tokens.end());
    board->setBlackColor(osl::record::Color::colorFor(*each++));
    assert(each != tokens.end());
    board->setWhiteColor(osl::record::Color::colorFor(*each++));
    assert(each != tokens.end());
    board->setLastMoveColor(osl::record::Color::colorFor(*each++));
    assert(each == tokens.end());
  }
  return 1;
}

void processGpsshellHome(boost::filesystem::path& readline_home)
{
  std::string GPSSHELL_HOME = "/tmp/gpsshell"; // default value
  if (getenv("HOME"))
    GPSSHELL_HOME = std::string(getenv("HOME")) + "/.gpsshell";
  const boost::filesystem::path home(GPSSHELL_HOME);
  if (!boost::filesystem::exists(home))
    boost::filesystem::create_directories(home);

  // re-create download cache 
  cache_dir = boost::filesystem::path(GPSSHELL_HOME) / "download_cache";
  if (boost::filesystem::exists(cache_dir))
    boost::filesystem::remove_all(cache_dir);
  boost::filesystem::create_directories(cache_dir);
  
  readline_home = home / "gpsshell";
}

#ifndef MINIMAL_GPSSHELL
template <class SReadline>
void setCommands(SReadline& reader, 
                 gpsshell::MySession *session)
{
  setCommandsMain(session);
  // Now register the completers.
  // Actually it is possible to re-register another set at any time
  reader.RegisterCompletions(session->getContainer());
}
#endif

int main(int argc, char **argv)
{
#ifndef MINIMAL_GPSSHELL
  Poco::Net::HTTPStreamFactory::registerFactory();
#ifdef ENABLE_REDIS
  google::InitGoogleLogging(argv[0]);
#endif
#endif
  namespace bp = boost::program_options;
  bp::variables_map vm;
  if (!parseCommandLine(argc, argv, vm))
    exit(1);

  osl::eval::ProgressEval::setUp();
  if (vm.count("eval-data"))
    osl::OpenMidEndingEval::setUp(vm["eval-data"].as<std::string>().c_str());
  else
    osl::OpenMidEndingEval::setUp();
  osl::NewProgress::setUp();
  board.reset(new gpsshell::Board());
  osl::OslConfig::setUp();
  
  boost::filesystem::path readline_home;
  processGpsshellHome(readline_home);
  gpsshell::MySession session;
#ifndef MINIMAL_GPSSHELL
  const static size_t max_stored_commands = 32;
  SReadline reader(osl::misc::file_string(readline_home), max_stored_commands);
  setCommands(reader, &session);
#else
  setCommandsMain(&session);
#endif

  if (vm.count("stdin"))
  {
    const std::string& file_type = vm["stdin"].as<std::string>();
    if ("csa" == file_type)
    {
      std::string line ;
      osl::SimpleState work;
      osl::Record record;
      size_t prev_move = 0, prev_time = 0;
      while (getline(std::cin, line)) {
	osl::CsaFile::parseLine(work, record, line);
	if (record.moves().size() > prev_move) {
	  session.move({ "move", osl::csa::show(record.moves().back()) });
	  prev_move = record.moves().size();
	}
	if (record.times.size() > prev_time) {
	  std::cout << "time: " << record.times.back() << std::endl;
	  prev_time = record.times.size();
	}
      }
      return 0;
    }
    else
    {
      std::cerr << "Not supported file type: " << file_type << std::endl;
      return 1;
    }
  }

  const char *path = osl::OslConfig::openingBook(); // default 
  the_book.reset(new gpsshell::Book(path));
  board->showState();

  bool end_of_input = false; // true when we should exit
  std::vector<std::string> tokens; // List of the entered tokens
  while(true)
  {
    // We get the list of tokens
#ifdef MINIMAL_GPSSHELL
    std::cerr << "> " << std::flush;
    tokens.clear();
    std::string line, word;
    std::getline(std::cin, line);
    std::istringstream is(line);
    while (is >> word) 
      tokens.push_back(word);
#else
    reader.GetLine( "> ", tokens, end_of_input );
#endif

    if (end_of_input)
    {
      std::cout << "End of the session. Exiting." << std::endl;
      break;
    }
#ifndef MINIMAL_GPSSHELL
    if (tokens.empty())
    {
      // if no command, try the last command.
      std::vector<std::string> history;
      reader.GetHistory(history);
      std::cout << "history size " << history.size() << std::endl;
      if (!history.empty())
      {
        const std::string& last_command = history.back();
        SplitTokens(last_command, tokens);
      }
    }
#endif
    if (tokens.front() == "exit" || tokens.front() == "quit")
      break;

    session.executeCommand(tokens);
  } // while

  return 0;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
