/* eval.cc
 */
#include "gpsshogi/recorddb/recordDb.h"
#include "gpsshogi/recorddb/evalDb.h"
#include "gpsshogi/recorddb/facade.h"
#include "osl/state/numEffectState.h"
#include "osl/eval/ml/openMidEndingEval.h"
#include "osl/progress/ml/newProgress.h"
#include "osl/search/alphaBeta2.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/searchState2.h"
#include "osl/search/moveWithComment.h"
#include "osl/checkmate/dualDfpn.h"
#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include <boost/thread/thread.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <tbb/concurrent_queue.h>
#include <iostream>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;

// ./eval --init 1 --in recorddb.tch --out evaldb.tch --checkmate 10000 // init and add data of raw evaluation
// ./eval --eval 1 --limit 400 --db evaldb.tch // add data of alphabetasearch with threshold upto 400
// ./eval --show-all 1 --db evaldb.tch
// ./eval --draw 1 --db evaldb.tch --limit 4

unsigned int threshold, verbose, checkmate_limit, num_threads;
bool initialize_mode, eval_mode, show_all_mode, draw_mode;
int limit;
std::string src, dst, dbname;
void initialize();
void evaluate();
void show_all();
void draw();
int main(int argc, char **argv)
{
  po::options_description options;
  options.add_options()
    ("initialize", po::value<bool>(&initialize_mode)->default_value(0),
     "initialize evaldb")
    ("in", po::value<std::string>(&src)->default_value(""),
     "filename for source database")
    ("out", po::value<std::string>(&dst)->default_value(""),
     "filename for destination database")
    ("checkmate,c", po::value<unsigned int>(&checkmate_limit)->default_value(0),
     "checkmate node count to ignore")
    ("eval-mode", po::value<bool>(&eval_mode)->default_value(0),
     "add data into evaldb")
    ("db", po::value<std::string>(&dbname)->default_value(""),
     "filename for source database")
    ("limit,l", po::value<int>(&limit)->default_value(1000),
     "threshold for reallization probability in search")
    ("show-all-mode", po::value<bool>(&show_all_mode)->default_value(0),
     "show data in evaldb")
    ("draw-mode", po::value<bool>(&draw_mode)->default_value(1),
     "show data in evaldb")
    ("num-threads,N", po::value<unsigned int>(&num_threads)->default_value(1),
     "set number of thread parallelization")
    ("verbose,v", po::value<unsigned int>(&verbose)->default_value(0),
     "set verbose level")
    ("help,h", "Show help message");

  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }
  if (vm.count("help") 
      || (! initialize_mode && ! eval_mode && ! show_all_mode && !draw_mode)
      || (initialize_mode && (src == "" || dst == ""))
      || (! initialize_mode && (dbname == "")))
  {
    std::cerr << "Usage: " << argv[0] << " [options] inputfile outputfile" << std::endl;
    std::cout << options << std::endl;
    return 1;
  }
  eval::ml::OpenMidEndingEval::setUp();
  progress::ml::NewProgress::setUp();

  if (initialize_mode)
    initialize();
  else if (eval_mode)
    evaluate();
  else if (show_all_mode)
    show_all();
  else if (draw_mode)
    draw();
}

tbb::concurrent_queue<std::pair<record::MiniBoardChar50,SquareInfo> > read_queue;
tbb::concurrent_queue<std::pair<record::MiniBoardChar50,EvalInfo> > search_queue;
tbb::concurrent_queue<boost::tuple<record::MiniBoardChar50,EvalInfo,bool> > write_queue;
volatile size_t added = 0;

struct Converter
{
  void operator()()
  {
    typedef eval::ml::OpenMidEndingEval ome_eval_t;
    const int pawn = ome_eval_t::captureValue(newPtypeO(WHITE, PAWN))/2;
    // typedef search::SearchState2::checkmate_t checkmate_t;
    typedef DualDfpn checkmate_t;
    std::unique_ptr<checkmate_t> checkmate(new checkmate_t);
    std::unique_ptr<search::SimpleHashTable> table(new search::SimpleHashTable(1000000, 400));
    search::CountRecorder recorder;
    std::pair<record::MiniBoardChar50, SquareInfo> data;
    for (size_t i=0; true; ++i)
    {
      read_queue.pop(data);
      if (data.first == record::MiniBoardChar50())
	break;
      NumEffectState state(data.first.toSimpleState(BLACK));
      // test checkmate
      if (state.inCheck())
      {
	if (checkmate->isLosingState(checkmate_limit, state,
				     HashKey(state), PathEncoding(state.turn())))
	  continue;
      }
      else
      {
	Move best_move;
	if (checkmate->isWinningState(checkmate_limit, state,
				      HashKey(state), PathEncoding(state.turn()),
				      best_move))
	  continue;
      }
      // add to db
      EvalInfo eval;
      {
	EvalInfo::Info *info = eval.add_info();
	info->set_depth(-1);
	info->set_eval(ome_eval_t(state).value()*100/pawn);
      }

      write_queue.push(boost::make_tuple(data.first, eval, true));
      if (checkmate->mainNodeCount() >= 1000000)
	checkmate.reset(new checkmate_t);
      if (table->size() > table->capacity()/2)
	table.reset(new search::SimpleHashTable(1000000, 400));
    }
  }
};
struct Writer
{
  EvalDB &dbout;
  size_t db_size;
  Writer(EvalDB& db, size_t size) : dbout(db), db_size(size)
  {
  }
  void operator()()
  { 
    boost::progress_display progress(db_size, std::cerr);
    boost::tuple<record::MiniBoardChar50, EvalInfo, bool> data;
    for (size_t i=0; true; ++i, ++progress) 
    {
      write_queue.pop(data);
      if (data.get<0>() == record::MiniBoardChar50())
	break;
      if (data.get<2>() == false)
	continue;
      dbout.put(data.get<0>().toString(), data.get<1>());
      if (i % 1024 == 0)
	dbout.optimize();
      ++added;
    }
    progress += progress.expected_count() - progress.count();
  }
};
void initialize()
{
  RecordDB dbin(src.c_str(), true);
  EvalDB dbout(dst.c_str(), false);

  std::cerr << "estimate = " << dbin.size() << "\n";
  read_queue.set_capacity(1<<20);

  size_t db_size = dbin.size();
  dbin.initIterator();
  record::MiniBoardChar50 key((SimpleState(HIRATE)));
  SquareInfo value;
  boost::thread writer((Writer(dbout, db_size)));
  boost::ptr_vector<boost::thread> converters;
  for (size_t i=0; i<num_threads; ++i)
    converters.push_back(new boost::thread(Converter()));

  size_t all = 0;
  time_point start = clock::now();
  while (dbin.next(key, value)) 
  {
    ++all;
    read_queue.push(std::make_pair(key, value));
    if (all >= db_size)
      break;
  }
  std::cerr << "finish input\n";
  for (size_t i=0; i<num_threads; ++i)
    read_queue.push(std::make_pair(record::MiniBoardChar50(), SquareInfo()));
  for (size_t i=0; i<num_threads; ++i)
    converters[i].join();
  write_queue.push(boost::make_tuple(record::MiniBoardChar50(), EvalInfo(), false));
  writer.join();
  
  std::cerr << "read " << all << " wrote " << added << "\n";
  std::cerr << "elapsed " << start.elapsedSeconds() << " seconds\n";
}

struct Searcher
{
  void operator()()
  {
    typedef eval::ml::OpenMidEndingEval ome_eval_t;
    typedef search::SearchState2::checkmate_t checkmate_t;
    // typedef DualDfpn checkmate_t;
    std::unique_ptr<checkmate_t> checkmate(new checkmate_t);
    std::unique_ptr<search::SimpleHashTable> table(new search::SimpleHashTable(1000000, 400));
    search::CountRecorder recorder;
    std::pair<record::MiniBoardChar50, EvalInfo> data;
    for (size_t i=0; true; ++i)
    {
      search_queue.pop(data);
      if (data.first == record::MiniBoardChar50())
	break;
      NumEffectState state(data.first.toSimpleState(BLACK));
      // add to db

      // quiesce
      bool modified = false;
      bool has_data = false;
      for (int i=0; i<data.second.info_size(); ++i)
	if (data.second.info(i).depth() == 4)
	{
	  has_data = true;
	  break;
	}
      if (! has_data)
      {
	modified = true;
	search::AlphaBeta2<ome_eval_t> search(state, *checkmate, &*table, recorder);
	search::MoveWithComment additional_info;
	Move best_move = search.computeBestMoveIteratively
	  (0, 200, 400, 1000000, search::TimeAssigned(milliseconds(60*1000)),
	   &additional_info);
	EvalInfo::Info *qinfo = data.second.add_info();
	qinfo->set_depth(4);
	qinfo->set_eval(additional_info.value);
	qinfo->set_best_move(best_move.intValue());
	qinfo->set_node_count(search.nodeCount());
      }
      for (int depth=400; depth<=limit; depth+=200)
      {
	has_data = false;
	for (int i=0; i<data.second.info_size(); ++i)
	  if (data.second.info(i).depth() == depth)
	{
	  has_data = true;
	  break;
	}
	if (has_data)
	  continue;
	modified = true;
	search::AlphaBeta2<ome_eval_t> search(state, *checkmate, &*table, recorder);
	search::MoveWithComment additional_info;
	Move best_move = search.computeBestMoveIteratively
	  (depth, 200, 400, 1000000, search::TimeAssigned(milliseconds(60*1000)),
	   &additional_info);
	EvalInfo::Info *info = data.second.add_info();
	info->set_depth(depth);
	info->set_eval(additional_info.value);
	info->set_best_move(best_move.intValue());
	info->set_node_count(search.nodeCount());
      }

      write_queue.push(boost::make_tuple(data.first, data.second, modified));
      if (checkmate->mainNodeCount() >= 100000)
	checkmate.reset(new checkmate_t);
      if (table->size() > table->capacity()/2)
	table.reset(new search::SimpleHashTable(1000000, 400));
    }
  }
};

void evaluate()
{
  EvalDB db(dbname.c_str(), false);
  std::cerr << "db size = " << db.size() << "\n";
  search_queue.set_capacity(1<<20);

  db.initIterator();
  record::MiniBoardChar50 key;
  EvalInfo value;

  size_t db_size = db.size();
  boost::thread writer((Writer(db, db_size)));
  boost::ptr_vector<boost::thread> searchers;
  for (size_t i=0; i<num_threads; ++i)
    searchers.push_back(new boost::thread(Searcher()));
  time_point start = clock::now();

  while (db.next(key, value)) 
  {
    NumEffectState state(key.toSimpleState(BLACK));
    search_queue.push(std::make_pair(key, value));
  }
  std::cerr << "finish input\n";
  for (size_t i=0; i<num_threads; ++i)
    search_queue.push(std::make_pair(record::MiniBoardChar50(), EvalInfo()));
  for (size_t i=0; i<num_threads; ++i) 
    searchers[i].join();
  write_queue.push(boost::make_tuple(record::MiniBoardChar50(), EvalInfo(), false));
  writer.join();
  
  std::cerr << "elapsed " << start.elapsedSeconds() << " seconds\n";

}
void show_all()
{
  EvalDB db(dbname.c_str(), true);
  db.initIterator();
  record::MiniBoardChar50 key;
  EvalInfo value;
  while (db.next(key, value)) 
  {
    NumEffectState state(key.toSimpleState(BLACK));
    std::cout << state << value.DebugString();
  }
}

bool has_eval(const EvalInfo& info, int& eval)
{
  for (int i=0; i<info.info_size(); ++i)
  {
    if (info.info(i).depth() == limit)
    {
      eval = info.info(i).eval();
      return true;
    }
  }
  return false;
}
void draw()
{
  EvalDB db(dbname.c_str(), true);
  boost::progress_display progress(db.size(), std::cerr);
  db.initIterator();
  record::MiniBoardChar50 key;
  EvalInfo value;
  // [-2050,-1951],[-1950,-1851],...,[-50,50],...[1951,2050]
  // 
  CArray<int,41> wins, losses;
  wins.fill(0);
  losses.fill(0);
  double total = 0;
  for (; db.next(key, value); ++progress)
  {
    int eval;
    if (has_eval(value, eval))
    {
      int win, loss, gps_win, gps_loss, bonanza_win, bonanza_loss;
      gpsshogi::recorddb::query(key.toSimpleState(), win, loss, gps_win, gps_loss, bonanza_win, bonanza_loss);
      total += win + loss;
      if (abs(eval) <= 2050)
      {
	if (eval > 0)
	  --eval;
	const size_t index = (eval+2050)/100;
	wins[index] += win;
	losses[index] += loss;
      }
    }
  }
  for (size_t i=0; i<wins.size(); ++i)
  {
    if (wins[i] + losses[i] < 100)
      continue;
    std::cout << static_cast<int>(i)*100 - 2050
	      << " " << wins[i] << " " << losses[i]
	      << " " << (wins[i] + losses[i])/total
	      << "\n";
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
