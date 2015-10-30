#include "options.h"
#include "benchmark.h"
#include "gpsshogi/revision.h"

#include "osl/game_playing/gnuShogiClient.h"
#include "osl/game_playing/csaClient.h"
#include "osl/game_playing/bookPlayer.h"
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/speculativeSearchPlayer.h"
#include "osl/game_playing/csaLogger.h"
#include "osl/game_playing/weightTracer.h"
#include "osl/game_playing/recordTracer.h"
#include "osl/search/usiProxy.h"
#include "osl/rating/featureSet.h"
#include "osl/move_probability/featureSet.h"
#include "osl/book/openingBook.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/eval/progressEval.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"
#include "osl/search/searchRecorder.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <memory>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#ifdef __MINGW32__
#ifndef SIGUSR1
#define SIGUSR1 10
#endif
#include <windows.h>
#endif

using namespace osl;
using namespace osl::book;
using namespace osl::game_playing;

const char *ignore_time_in_csa_file = getenv("IGNORE_TIME_IN_CSA_FILE");

SearchOptions search_options;
EvalOptions eval_options;
MatchOptions match_options("gpsshogi");
OpeningOptions opening_options;
int sleep_seconds;

WeightedBook& makeWeightedBook()
{
  static WeightedBook book(OslConfig::openingBook(opening_options.normal_book_filename));
  return book;
}

std::shared_ptr<std::ofstream> makeOutput(bool ignore_id,
					    std::string filename, int id)
{
  if (! ignore_id)
  {
    std::string number = ".00";
    sprintf(&number[0], ".%02d", id % 100);
    filename += number;
  }
  std::shared_ptr<std::ofstream> result(new std::ofstream(filename.c_str()));
  return result;
}
SearchPlayer *makePlayer(const std::string& eval_type, bool use_alpha_beta)
{
    if (OslConfig::verbose())
      std::cerr << "using alpha beta\n";
#ifndef MINIMAL
    if (eval_type == "usi")
      return new UsiProxyPlayer();
    if (eval_type == "test3")
      return new AlphaBeta3OpenMidEndingEvalPlayer();
    if (eval_type == "test4")
      return new AlphaBeta4Player();
#endif
#ifndef MINIMAL
    if (eval_type == "test")
#endif
      return new AlphaBeta2OpenMidEndingEvalPlayer();
#ifndef MINIMAL
    if (eval_type != "progress")
      std::cerr << "unsupported eval type " << eval_type << "\n";
    return new AlphaBeta2ProgressEvalPlayer();
#endif
}

volatile int *force_resign_flag = 0;
void force_resign(int)
{
  if (force_resign_flag)
    *force_resign_flag = 1;
}

static bool game_started = false;
void play(int argc,char **argv) 
{
  int verbose = 0;
  int num_benchmark = 0;
  po::options_description other_options("Other options");
  other_options.add_options()
    ("benchmark", "test search performance")
    ("benchmark-single", "test search performance")
    ("benchmark-more", 
     po::value<int>(&num_benchmark)->default_value(0),
     "number of problems for benchmark")
    ("sleep-at-exit", 
     po::value<int>(&sleep_seconds)->default_value(0),
     "seconds of sleeping at exit")
    ("health-check", "test whether data files are properly placed")
    ("help,h", "produce help message")
    ("verbose,v", "produce verbose output")
    ("version", "show version info")
    ;
  po::options_description command_line_options;
  command_line_options.add(search_options.options).add(eval_options.options)
    .add(match_options.options).add(opening_options.options).add(other_options);
  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, command_line_options), vm);
    po::notify(vm);
    match_options.update(vm);
    opening_options.update(vm);
    verbose = vm.count("verbose");
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << command_line_options << std::endl;
    throw;
  }
  if (vm.count("health-check")) {
    bool ok = OslConfig::healthCheck();
    ok &= rating::StandardFeatureSet::healthCheck();
#ifndef GPSONE
    ok &= move_probability::StandardFeatureSet::healthCheck();
#endif
    if (! ok)
      throw std::runtime_error("health check failed");
    return;
  }
  if (vm.count("help")) {
    std::cout << command_line_options << std::endl;
    return;
  }
  if (vm.count("version")) {
    std::cout << "gpsshogi " << gpsshogi::gpsshogi_revision << "\n\n"
      << "Copyright (C) 2003-2011 Team GPS.\n";
    std::cout << "osl " << OslConfig::configuration() << '\n';
    return;
  }

  if (verbose)
    std::cerr << "start up in " << (double)::clock()/CLOCKS_PER_SEC
	      << "sec" << std::endl;
  if (match_options.csa_mode
      || vm.count("benchmark") || vm.count("benchmark-single")
      || num_benchmark) {
    OslConfig::setVerbose(true);
    OslConfig::showOslHome();
    ++verbose;
  }
  else {
    std::cout << "gpsshogi " << gpsshogi::gpsshogi_revision
	      << " (compatible with GNU Shogi)\n" << std::flush;
    // signal(SIGTERM, SIG_IGN);
    match_options.use_alpha_beta = true;
    search_options.search_limit = 1000;
  }
  if (verbose)
    OslConfig::setVerbose(verbose);
#ifdef OSL_SMP
  if (search_options.num_cpus > osl::OslConfig::MaxThreads) {
    std::cerr << "number of cpu exceeds max threads(" << osl::OslConfig::MaxThreads << ")\n";
    throw std::runtime_error("Number of cpus exceeds max thread failure");
  }
  osl::OslConfig::setNumCPUs(search_options.num_cpus);
  if (verbose)
    std::cerr << "use cpu " << search_options.num_cpus << "\n";
#endif
  osl::OslConfig::setMemoryUsePercent(search_options.memory_use_percent);
  if (eval_options.eval_type == "usi") {
    if (!eval_options.eval_filename.empty() || !eval_options.progress_filename.empty())
      throw std::logic_error("usi proxy has its own data file");
    if (eval_options.usi_proxy != "")
      search::UsiProxy::setUp(eval_options.usi_proxy);
  }
#ifndef MINIMAL
  if (eval_options.eval_random)
    OslConfig::setEvalRandom(eval_options.eval_random);
#endif
  
  const bool load_success 
#ifndef MINIMAL
    = (eval_options.eval_type == "test"
       || eval_options.eval_type == "test3"
       || eval_options.eval_type == "test4"
       || eval_options.eval_type == "usi"
#else
    = ( true
#endif
       ? (eval_options.eval_filename.empty() 
	  ? eval::ml::OpenMidEndingEval::setUp()
	  : eval::ml::OpenMidEndingEval::setUp(eval_options.eval_filename.c_str()))
#ifndef MINIMAL
       : (eval_options.eval_filename.empty() 
	  ? eval::ProgressEval::setUp()
	  : eval::ProgressEval::setUp(eval_options.eval_filename.c_str())));
#else
       : false);
#endif
  if ( !load_success)
  {
    std::cerr << eval_options.eval_filename << " read error\n";
    throw std::runtime_error("evaluation function setup failure");
  }
#ifndef MINIMAL
  if (eval_options.eval_type == "test"
      || eval_options.eval_type == "test3"
      || eval_options.eval_type == "test4"
      || eval_options.eval_type == "usi")
#endif
  {
    const bool load_success 
      = (eval_options.progress_filename.empty()
	 ? progress::ml::NewProgress::setUp()
	 : progress::ml::NewProgress::setUp(
	   eval_options.progress_filename.c_str()));
    if (! load_success)
    {
      std::cerr << "NewProgress data read error\n";
    throw std::runtime_error("NewProgress setup failure");
    }
  }
  if (vm.count("benchmark") || vm.count("benchmark-single")) {
    std::auto_ptr<SearchPlayer> player
      (makePlayer(eval_options.eval_type, match_options.use_alpha_beta));
    search_options.setConfig(*player);
    benchmark(*player, match_options.initial_csa_file, 
	      match_options.byoyomi ? match_options.byoyomi : 30);
    return;
  }
  if (num_benchmark) {
    std::unique_ptr<SearchPlayer> player
      (makePlayer(eval_options.eval_type, match_options.use_alpha_beta));
    search_options.setConfig(*player);
    benchmark_more(*player, num_benchmark, match_options.byoyomi ? match_options.byoyomi : 30);
    return;
  }

  std::unique_ptr<OpeningBookTracer> black_book, white_book;

  if (! opening_options.use_opening_book)
  {
    black_book.reset(new NullBook());
    white_book.reset(new NullBook());
  }
  else if (opening_options.kisen_id >= 0)
  {
    const RecordTracer tracer = 
      RecordTracer::kisenRecord(opening_options.kisen_filename.c_str(), 
				opening_options.kisen_id,
				opening_options.book_moves, verbose);
    black_book.reset(new RecordTracer(tracer));
    white_book.reset(new RecordTracer(tracer));
  }
  else if (!opening_options.csa_filename.empty())
  {
    CsaFileMinimal csa(opening_options.csa_filename);
    if (!(csa.initialState() == SimpleState(HIRATE)))
    {
      throw std::runtime_error("Initial state of CSA file must be HIRATE");
    }
    RecordTracer tracer(csa.moves(), verbose);
    black_book.reset(new RecordTracer(tracer));
    white_book.reset(new RecordTracer(tracer));
  }
  else
  {
    // normal book
    if (opening_options.determinate_level > 0)
    {
      black_book.reset(new DeterminateWeightTracer(makeWeightedBook(),
                                        verbose && match_options.sente, 
                                        opening_options.determinate_level));
      white_book.reset(new DeterminateWeightTracer(makeWeightedBook(),
                                        verbose && (! match_options.sente),
                                        opening_options.determinate_level));
    } else {
      black_book.reset(new WeightTracer(makeWeightedBook(),
                                        verbose && match_options.sente,
                                        opening_options.weight_coef_for_the_initial_move,
                                        opening_options.weight_coef));
      white_book.reset(new WeightTracer(makeWeightedBook(),
                                        verbose && (! match_options.sente),
                                        opening_options.weight_coef_for_the_initial_move,
                                        opening_options.weight_coef));
    }
  }

  std::unique_ptr<BookPlayer> black;
  std::unique_ptr<BookPlayer> white;
  if (match_options.random_player)
  {
    black.reset(new BookPlayer(black_book.release(), new RandomPlayer()));
    white.reset(new BookPlayer(white_book.release(), new RandomPlayer()));
  }
  else
  {
    std::unique_ptr<SearchPlayer> black_search
      (makePlayer(eval_options.eval_type, match_options.use_alpha_beta));
    std::unique_ptr<SearchPlayer> white_search
      (makePlayer(eval_options.eval_type, match_options.use_alpha_beta));

    black_search->setVerbose(match_options.sente ? verbose*2 : 0);
    search_options.setConfig(*black_search);
    black_search->enableSavePV(match_options.save_pv);

    white_search->setVerbose((! match_options.sente) ? verbose*2 : 0);
    search_options.setConfig(*white_search);
    white_search->enableSavePV(match_options.save_pv);
  
    if (! match_options.think_opponent_time)
    {
      black.reset(new BookPlayer(black_book.release(), black_search.release()));
      white.reset(new BookPlayer(white_book.release(), white_search.release()));
    }
    else
    {
      SpeculativeSearchPlayer *black_speculative
	= new SpeculativeSearchPlayer(BLACK, black_search.release());
      black.reset(new BookPlayer(black_book.release(), black_speculative));
      SpeculativeSearchPlayer *white_speculative
	= new SpeculativeSearchPlayer(WHITE, white_search.release());
      white.reset(new BookPlayer(white_book.release(), white_speculative));
    }
  }
  black->setBookLimit(opening_options.book_moves);
  white->setBookLimit(opening_options.book_moves);

  int record_id = 0;
  std::shared_ptr<std::ofstream> 
    output = makeOutput(match_options.csa_mode, 
			match_options.output_filename,
			record_id);
  std::auto_ptr<CsaLogger> logger(new CsaLogger(*output));

  std::unique_ptr<CuiClient> client;
#ifndef MINIMAL
  if (match_options.csa_mode)
#endif
  {
    CsaClient *c = new CsaClient(black.get(), white.get(), logger.release(), 
				 std::cin, std::cout);
    c->setShowMoveWithComment(match_options.send_move_with_comment);
    client.reset(c);
  }
#ifndef MINIMAL
  else {
    client.reset(new GnuShogiClient(black.get(), white.get(), logger.release(), 
				    std::cin, std::cout));
  }
#endif
  if (! ignore_time_in_csa_file)
    client->setTimeLeft(match_options.time_left, match_options.time_left);
  if (! match_options.initial_csa_file.empty())
    client->load(match_options.initial_csa_file.c_str(), verbose);
  if (ignore_time_in_csa_file)
    client->setTimeLeft(match_options.time_left, match_options.time_left);
  client->setByoyomi(match_options.byoyomi);
  
  if (match_options.sente)
  {
    client->setComputerPlayer(BLACK, true);
    client->setComputerPlayer(WHITE, false);
  }
  else
  {
    client->setComputerPlayer(BLACK, false);
    client->setComputerPlayer(WHITE, true);
  }
  force_resign_flag = client->stopFlag();
  signal(SIGUSR1, force_resign);

  /** Ready */
  game_started = true;
  while (std::cin)
  {
    if (match_options.csa_mode && ! match_options.initial_csa_file.empty())
      client->run();		// do not need initial state
    else
      client->run(match_options.black_name.c_str(), 
		  match_options.white_name.c_str());
    if (match_options.csa_mode)
      break;
    
    // log 取り替え
    ++record_id;
    std::shared_ptr<std::ofstream> 
      new_output = makeOutput(match_options.csa_mode, 
			      match_options.output_filename, record_id);
    client->resetLogger(new CsaLogger(*new_output)); // 古いloggerを利用不可にしてから
    output = new_output;	// destructor を呼ぶ
  }
}

void sleep_before_exit()
{
  std::cout << std::flush;
  std::cerr << std::flush;
  if (! game_started || sleep_seconds <= 0)
    return;
  std::cerr << "sleeping " << sleep_seconds << " seconds before exit\n";
#ifdef __MINGW32__
  Sleep(sleep_seconds);
#else
  sleep(sleep_seconds);
#endif
}

int main(int argc, char **argv)
{
  OslConfig::setUp();
  try
  {
    play(argc, argv);
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << "\n";
    sleep_before_exit();
    return 1;
  }
  catch (GnuShogiQuit&) {
    return 0;
  }
  catch (...)
  {
    std::cerr << "unknown exception \n";
    sleep_before_exit();
    return 1;
  }
  sleep_before_exit();
  return 0;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
