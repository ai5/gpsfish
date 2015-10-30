#include "options.h"

#include "osl/search/alphaBeta2.h"
#include "osl/search/alphaBeta3.h"
#include "osl/search/alphaBeta4.h"
#include "osl/search/usiProxy.h"
#include "osl/search/searchRecorder.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/search/moveWithComment.h"
#include "osl/misc/perfmon.h"
#include "osl/record/kanjiPrint.h"
#include "osl/record/csaRecord.h"
#include "osl/record/kakinoki.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/eval/pieceEval.h"
#include "osl/eval/progressEval.h"
#include "osl/progress.h"
#include "osl/moveLogProb.h"
#include "osl/misc/log/htmlPerformanceLog.h"
#include "osl/misc/log/textPerformanceLog.h"
#include "osl/hash/hashRandom.h"

#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <libgen.h>

using namespace osl;

template <class AlphaBetaType>
void processFile(const char *filename);

SearchOptions search_options;
EvalOptions eval_options;
AnalyzeOptions analyze_options("alphabeta");

size_t go_forward=0;
int num_correct=0, num_total=0;
int seconds;
int verbose = 2;
bool show_pv = false;
unsigned int multi_pv;
std::unique_ptr<misc::log::PerformanceLog> logger;
int main(int argc, char **argv)
{
  OslConfig::setVerbose(true);
  OslConfig::showOslHome();
  OslConfig::setUp();

  std::string html_filename;

  po::options_description other_options("Other options");
  other_options.add_options()
    ("help", "produce help message")
    ("self-play,c", "play continuously by self-play")
    ("forward,f",
     po::value<size_t>(&go_forward)->default_value(0),
     "make moves from given position before search")
    ("html-filename,O",
     po::value<std::string>(&html_filename)->default_value(std::string("alphabetastat")),
     "html filename for search results")
    ("seconds,S",
     po::value<int>(&seconds)->default_value(120),
     "seconds for search")
    ("show-pv",
     po::value<bool>(&show_pv)->default_value(false),
     "Output PV of best move and correct move if exist.")
    ("multi-pv",
     po::value<unsigned int>(&multi_pv)->default_value(0),
     "Conduct multi-pv search if greater than zero (only effective in search with AlphaBeta2).")
    ("verbose,v", 
     po::value<int>(&verbose)->default_value(2),
     "set verboseness")
    ;
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("search-file", po::value<std::vector<std::string> >());

  po::options_description command_line_options;
  command_line_options.add(search_options.options).add(eval_options.options)
    .add(analyze_options.options).add(other_options).add(hidden);
  po::options_description visible_options("All options");
  visible_options.add(search_options.options).add(eval_options.options)
    .add(analyze_options.options).add(other_options);

  po::positional_options_description p;
  p.add("search-file", -1);

  po::variables_map vm;
  std::vector<std::string> filenames;
  try {
    po::store(po::command_line_parser(argc, argv).
	      options(command_line_options).positional(p).run(), vm);
    notify(vm);
    if (vm.count("help")) {
      std::cerr << "Usage: " << argv[0] << " [options] files" << std::endl;
      std::cout << visible_options << std::endl;
      return 0;
    }
    filenames = vm["search-file"].as<std::vector<std::string> >();
  }
  catch (std::exception& e) {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0] << " [options] files" << std::endl;
    std::cerr << visible_options << std::endl;
    return 1;
  }
  osl::OslConfig::setMemoryUsePercent(search_options.memory_use_percent);
#ifndef MINIMAL
  if (eval_options.eval_random)
    OslConfig::setEvalRandom(eval_options.eval_random);
#endif
  const bool use_alphabeta3 = (eval_options.eval_type == "test3");
  const bool use_alphabeta4 = (eval_options.eval_type == "test4");
  const bool use_usiproxy = (eval_options.eval_type == "usi");
  const bool use_test_eval = (eval_options.eval_type == "test");
  if (use_usiproxy) {
    if (!eval_options.eval_filename.empty() || !eval_options.progress_filename.empty())
      throw std::logic_error("usi proxy has its own data file");
    if (eval_options.usi_proxy != "")
      search::UsiProxy::setUp(eval_options.usi_proxy);
  }

  if (use_test_eval || use_alphabeta3 || use_alphabeta4 || use_usiproxy)
  {
    const bool load_success 
      = eval_options.eval_filename.empty() 
      ? eval::ml::OpenMidEndingEval::setUp()
      : eval::ml::OpenMidEndingEval::setUp(eval_options.eval_filename.c_str());
    if (! load_success)
    {
      std::cerr << eval_options.eval_filename << " read error\n";
      return 1;
    }

    {
      const bool load_success 
	= (eval_options.progress_filename.empty()
	   ? progress::ml::NewProgress::setUp()
	   : progress::ml::NewProgress::setUp(
	     eval_options.progress_filename.c_str()));
      if (! load_success)
      {
	std::cerr << "NewProgress data read error\n";
	return 1;
      }
    }
  }
  // always load progress eval
#ifndef MINIMAL
  {
    const bool load_success 
      = eval_options.eval_filename.empty() 
      ? eval::ProgressEval::setUp()
      : eval::ProgressEval::setUp(eval_options.eval_filename.c_str());
    if (! load_success)
    {
      std::cerr << eval_options.eval_filename << " read error\n";
      return 1;
    }
  }
#endif
  if (html_filename == "-") {
    logger.reset(new misc::log::TextPerformanceLog());
  }
  else {
    std::ostringstream ss;
    ss << html_filename << search_options.search_limit << ".html";
    html_filename = ss.str();
    logger.reset(new misc::log::HtmlPerformanceLog(html_filename.c_str(), "alphabetastat"));
  }
#ifdef OSL_SMP
  osl::OslConfig::setNumCPUs(search_options.num_cpus);
#endif
  for (size_t i=0; i<filenames.size(); ++i)
  {
    const char *filename = filenames[i].c_str();
#ifndef MINIMAL
    if (use_alphabeta3)
      processFile<AlphaBeta3>(filename);
    else if (use_alphabeta4)
      processFile<AlphaBeta4>(filename);
    else if (use_usiproxy) 
      processFile<search::UsiProxy>(filename);
    else if (use_test_eval)
#endif
      processFile<AlphaBeta2OpenMidEndingEval>(filename);
#ifndef MINIMAL
    else
      processFile<AlphaBeta2ProgressEval>(filename);
#endif
    std::cerr << "performance " << num_correct << " / " << num_total << "\n";
  }
}

void showPVWithMove(const char *name, const Move move, const MoveWithComment& pv)
{
  if (! pv.moves.empty())
    std::cout << "' " << name << "\t" << pv.value << "\t";
  else
    std::cout << "\t";
  std::cout << csa::show(move) << " ";
  for (size_t i = 0; i < pv.moves.size(); ++i)
  {
    std::cout << csa::show(pv.moves[i]) << " ";
  }
  std::cout << std::endl;
}

void showPVWithMove(const char *name, const SimpleHashTable &table,
		    const HashKey& key, const Move move)
{  
  MoveVector pv; 
  const HashKey new_key = key.newHashWithMove(move);
  table.getPV(new_key, pv);
  const SimpleHashRecord *record = table.find(new_key);
  if (record && record->hasLowerBound(0))
    std::cout << "' " << name << "\t" << record->lowerBound() << "\t";
  else
    std::cout << "\t";
  std::cout << csa::show(move) << " ";
  for (size_t i = 0; i < pv.size(); ++i)
  {
    std::cout << csa::show(pv[i]) << " "; 
  }
  std::cout << std::endl;
}

template <class AlphaBetaType>
void processFile(const char *filename)
{
#if EXTRA_DEBUG
  static osl::SearchRecorder recorder(analyze_options.log_filename.c_str());
  recorder.setLogMargin(analyze_options.log_margin);
#else
  static osl::CountRecorder recorder;
#endif
  record::KanjiPrint printer(std::cerr);

#ifndef MINIMAL
  if (OslConfig::evalRandom()) {
    const PtypeO white_pawn = newPtypeO(WHITE,PAWN);
    const int pawn_value = AlphaBetaType::eval_t::captureValue(white_pawn)/2;
    HashRandom::setUp(1.0*OslConfig::evalRandom()*pawn_value/100);
  }
#endif  
  std::cerr << "\n\nloading " << filename << std::flush;

  NumEffectState sstate;
  Record r;
  if (std::string(filename).rfind(".kif") == std::string(filename).size()-4)
  {
    KakinokiFile file(filename);
    sstate.copyFrom(NumEffectState(file.initialState()));
    r = file.load();
  }
  else
  {
    CsaFile file(filename);
    sstate.copyFrom(NumEffectState(file.initialState()));
    r = file.load();
  }
  const auto moves=r.moves();

  MoveStack history;
  go_forward = std::min(go_forward, moves.size());
  for (unsigned int i=0; i<go_forward; ++i)
  {
    history.push(moves[i]);
    sstate.makeMove(moves[i]);
  }

  if (OslConfig::isMemoryLimitEffective()) 
  {
    search_options.table_size = std::numeric_limits<int>::max();
    search_options.node_limit = std::numeric_limits<int>::max();
  }
  
  NumEffectState state(sstate);
  std::cerr << " done\n" << std::flush;
#ifdef EXTRA_DEBUG
  recorder.stream() << "\nTesting" << filename
		    << " with limit " << search_options.search_limit 
		    << " table_size " << search_options.table_size << "\n"
		    << state << "\n";
#endif
  Move best_move;
  search::SearchState2::checkmate_t checker;
  unsigned int nodes, qnodes;

  SimpleHashTable table(search_options.table_size,
			search_options.table_record_limit);
  table.setVerbose(verbose);

#ifndef MINIMAL
  if (verbose > 1) {
    eval::ProgressEval eval(state);
    std::cerr << state << "eval " << eval.value()
	      << " progress " << eval.progress32().value() << "\n";
  }
#endif
  const time_point started = clock::now();
    
  AlphaBetaType searcher(state,checker,&table,recorder);
  searcher.setNextIterationCoefficient(3.0);
  searcher.setHistory(history);
  if (multi_pv)
    searcher.enableMultiPV(multi_pv);
  MoveWithComment pv;
  best_move = searcher.computeBestMoveIteratively
    (search_options.search_limit, 
     search_options.deepening_step, 
     search_options.initial_limit, 
     search_options.node_limit,
     search::TimeAssigned(milliseconds(seconds*1000)),
     &pv);
  nodes = recorder.nodeCount();
  qnodes = recorder.quiescenceCount();
  double consumed = elapsedSeconds(started);

  std::cerr << best_move << "\n";
  
  if (go_forward < moves.size())
  {
    std::cerr << " correct move: " << moves[go_forward] << "\n";
    std::cerr << "search result: " << best_move << "\n";

    const HashKey key(state);
    const SimpleHashRecord *record = table.find(key);
    int depth = 0;
    if (record) {
      if (record->lowerLimit() > record->upperLimit())
	depth = record->lowerLimit();
      else
	depth = record->upperLimit();
    }
    logger->record(basename(const_cast<char*>(filename)), 
		   moves[go_forward], best_move, nodes, qnodes,
		   consumed, depth);
    const char *name = basename(const_cast<char*>(filename));
    if (show_pv)
    {
      if (best_move != moves[go_forward])
	showPVWithMove(name, table, key, moves[go_forward]);
      showPVWithMove(name, best_move, pv);
    }
    if (best_move == moves[go_forward])
      ++num_correct;
    ++num_total;
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
