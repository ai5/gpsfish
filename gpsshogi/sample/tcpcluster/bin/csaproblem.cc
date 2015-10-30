#include "coordinator.h"
#include "searchNode.h"
#include "searchTree.h"
#include "logging.h"
#include "osl/state/historyState.h"
#include "osl/record/csaRecord.h"
#include "osl/usi.h"
#include <boost/asio.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <vector>

namespace gpsshogi
{
  class ProblemStream : public UpStream
  {
    osl::NumEffectState state;
    std::vector<std::string> filenames;
    std::vector<osl::Move> expected, output;
    int msec;
    bool last_position;
    size_t cur, ok;
    std::mutex mutex;
    std::condition_variable condition;
  public:
    ProblemStream(const std::vector<std::string>& f, int m, bool l)
      : filenames(f), msec(m), last_position(l), cur(0), ok(0)
    {
    }
    ~ProblemStream()
    {
    }
    void start()
    {
      std::thread thread(&ProblemStream::run, this);
      thread.detach();
    }
    void run()
    {
      coordinator->waitReady();
      while (output.size() < filenames.size()) {
	std::this_thread::sleep_for(std::chrono::seconds(1));
	nextProblem();
	std::unique_lock<std::mutex> lk(mutex);
	while (output.size() < expected.size())
	  condition.wait(lk);
      }
      std::this_thread::sleep_for(std::chrono::seconds(5));
      Logging::notice("done "+to_s(ok)+" / "+to_s(output.size()));
      io->post(std::bind(&Coordinator::prepareQuit, coordinator));
    }
    void nextProblem()
    {
      while (cur < filenames.size()) {
	io->post(std::bind(&Coordinator::showStatus, coordinator,
			     boost::ref(std::cerr)));
	std::string filename = filenames[cur++];
	Logging::notice("next problem "+filename);
	try {
	  osl::CsaFile file(filename);
	  state.copyFrom(file.initialState());
	  const auto moves=file.moves();
	  osl::Move solution
	    = (!last_position && moves.size()) ? moves[0] : osl::Move();
	  expected.push_back(solution);
	  std::string usi = osl::usi::show(state);
	  if (last_position && moves.size()) {
	    usi += " moves";
	    for (osl::Move m: moves) {
	      state.makeMove(m);
	      usi += " "+osl::usi::show(m);
	    }
	  }
	  if (usi.find("position") != 0)
	    usi = "position "+usi;
	  std::cerr << usi << "\n";
	  io->post(std::bind(&Coordinator::handleGo, coordinator,
			       cur, usi, msec));
	  return;
	}
	catch (std::exception& e) {
	  Logging::error("skip " + filename + " "+e.what());
	  expected.push_back(osl::Move());
	  output.push_back(osl::Move());
	}
	catch (...) {
	  Logging::error("skip " + filename);
	  expected.push_back(osl::Move());
	  output.push_back(osl::Move());
	}
      }
    }
    void outputSearchProgress(int position_id, const std::string& msg)
    {
      InterimReport report, info;
      report.updateByInfo(msg,-1);
      osl::HistoryState copy(state);
      info.set(copy.state().turn(), report);
      std::cerr << std::setfill(' ') << std::setw(5)
		<< info.bestValue()*100/usi_pawn_value
		<< ' ' << SearchNode::toCSA(copy, info.joinPV()) << "\n";
    }
    void outputSearchResult(int position_id, const std::string& msg)
    {
      osl::Move move;
      try {
	std::vector<std::string> elements;
	boost::algorithm::split(elements, msg, boost::algorithm::is_any_of(" "));
	move = osl::usi::strToMove(elements[1], state);
      }
      catch (...) {
	Logging::error("parse failed: " + msg);
      }
      std::lock_guard<std::mutex> lk(mutex);
      output.push_back(move);
      if (move == expected.back())
	++ok;
      std::cout << filenames[cur-1] << " " << osl::csa::show(move)
		<< " " << osl::csa::show(expected.back()) << " "
		<< (move == expected.back() ? "OK" : "NG") << "\n";
      condition.notify_all();
    }
  };
}


int main(int argc, char **argv)
{
  unsigned int expected_slaves;
  int parallel_io = 2, slave_tlp = 0, seconds;
  bool slave_udp_log = false, last_position = false;
  std::string udp_log = "", slave_log_dir="", accept_multi_slaves;
  int draw_value_cp;
  bool human_rounding=false;

  namespace po = boost::program_options;
  po::options_description options("Options");
  options.add_options()
    ("slaves",
     po::value<unsigned int>(&expected_slaves)->default_value(1),
     "wait until specified number of slaves become ready")
    ("io-threads,N", po::value<int>(&parallel_io)->default_value(2),
     "parallel io.")
    ("udp-logging", po::value<std::string>(&udp_log)->default_value(""), "host:port for udp logging")
    ("slave-udp-log", po::value<bool>(&slave_udp_log)->default_value(false), "send udp log for slave-io")
    ("slave-log-directory",
     po::value<std::string>(&slave_log_dir)->default_value("io"),
     "specify where slave logs to go (\"\" for disable logging)")
    ("slave-tlp",
     po::value<int>(&slave_tlp)->default_value(0),
     "specify number of threads for all slaves, 0 for default")
    ("byoyomi,B",
     po::value<int>(&seconds)->default_value(30),
     "seconds for each problem")
    ("last-position", 
     po::value<bool>(&last_position)->default_value(false),
     "search after position all moves played")
    ("accept-multiple-slaves-filename",
     po::value<std::string>(&accept_multi_slaves)->default_value(""),
     "filename of ip address where multiple slaves may login from, even when slave-tlp == 0")
    ("draw-value-cp",
     po::value<int>(&draw_value_cp)->default_value(0),
     "preference of draw in centi pawn (prefers if positive)")
    ("human-rounding",
     po::value<bool>(&human_rounding)->default_value(0),
     "specify true when time is measured in minutes, droping seconds)")
    ("help,h", "produce help message")
    ;
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("search-file", po::value<std::vector<std::string> >());

  po::options_description all_options;
  all_options.add(options).add(hidden);
  po::positional_options_description p;
  p.add("search-file", -1);

  po::variables_map vm;
  std::vector<std::string> filenames;
  try
  {
    store(po::command_line_parser(argc, argv).
	  options(all_options).positional(p).run(), vm);
    notify(vm);
    if (vm.count("help")) {
      std::cout << options << std::endl;
      return 0;
    }
    filenames = vm["search-file"].as<std::vector<std::string> >();
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    throw;
  }

  gpsshogi::SearchTree::setDrawValueCP(draw_value_cp);
  gpsshogi::SearchTree::setHumanRounding(human_rounding);
  if (accept_multi_slaves != "")
    gpsshogi::SlaveManager::acceptMultipleSlaves(accept_multi_slaves);
  
  // option upstream(tcp or stdio), threading
  gpsshogi::Coordinator world
    (new gpsshogi::ProblemStream(filenames, seconds*1000, last_position),
     expected_slaves, parallel_io, udp_log, slave_udp_log, slave_log_dir, slave_tlp);
  try {
    world.start();
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
