/* order-by-search
 */
#include "osl/record/csaRecord.h"
#include "osl/record/csaIOError.h"
#include "osl/state/numEffectState.h"
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/weightTracer.h"
#include "osl/game_playing/bookPlayer.h"
#include "osl/eval/ml/openMidEndingEval.h"
#include "osl/progress/ml/newProgress.h"
#include "osl/move_generator/legalMoves.h"
#include "osl/container/moveLogProbVector.h"
#include "osl/rating/ratingEnv.h"
#include "osl/rating/featureSet.h"
#include "osl/apply_move/applyMove.h"
#include "osl/misc/pointerQueue.h"
#include "osl/oslConfig.h"
#include "osl/sennichite.h"
#include <boost/thread/thread.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
namespace po = boost::program_options;
#include <string>
#include <sstream>
#include <iostream>
#ifdef _WIN32
#  include <malloc.h>
#endif
using namespace osl;

int seconds=1, skip_moves=0, num_cpus = OslConfig::concurrency();

struct InputData
{
  NumEffectState state;
  MoveVector ignore_moves;
  Move last_move;
  int order;
  InputData(const NumEffectState& src, int o) 
    : state(src), order(o)
  {
  }
};

struct OutputData
{
  Move move, last_move;
  int value, order;
  OutputData(Move m, Move l, int v, int o)
    : move(m), last_move(l), value(v), order(o)
  {
  }
};

osl::misc::PointerQueue<InputData> queue_in;
osl::misc::PointerQueue<OutputData> queue_out;

struct ThreadRun
{
  int tid;
  explicit ThreadRun(int id) : tid(id) {}
  void operator()() const
    __attribute__((noinline))
#ifdef _WIN32
    __attribute__((force_align_arg_pointer))
#endif
  {
    game_playing::AlphaBeta2OpenMidEndingEvalPlayer player;
    player.setTableLimit(std::numeric_limits<size_t>::max(), 200);
    player.setNodeLimit(std::numeric_limits<size_t>::max());

    while (1) {
      std::shared_ptr<InputData> in = queue_in.pop_front();
      if (!in)
	break;
      game_playing::GameState gstate(in->state);
      if (! in->ignore_moves.empty())
	player.setRootIgnoreMoves(&(in->ignore_moves), false);
      else
	player.setRootIgnoreMoves(0, false);
      const osl::milliseconds b(seconds*1000);
      osl::search::TimeAssigned msec(b, b);
      MoveWithComment ret = player.selectBestMoveInTime(gstate, msec);
      
      std::shared_ptr<OutputData> out 
	(new OutputData(ret.move, in->last_move, ret.value, in->order));
      queue_out.push_back(out);
    }
  }
};

void genmove_probability(const NumEffectState& state, int limit, MoveLogProbVector& out)
{
  static const rating::StandardFeatureSet& feature_set
    = rating::StandardFeatureSet::instance();
  rating::RatingEnv env;
  progress::ml::NewProgress progress(state);
  env.make(state, state.pin(state.turn()), state.pin(alt(state.turn())),
	   progress.progress16());
  feature_set.generateLogProb(state, env, limit, out);
}

int order_by_probability(const MoveLogProbVector& moves, Move next)
{
  int order = moves.size();
  if (const MoveLogProb *p = moves.find(next))
    order = (p - &*moves.begin());
  return order;
}

const int N = 6;
const CArray<int,N> Width = {{ 2, 4, 8, 16, 32, 64 }};
CArray<int,7> order_by_search(const NumEffectState& state,
			      const MoveLogProbVector& moves, Move next)
{
  const int sign = -sign(state.turn());
  MoveLogProbVector values(moves.size());
  for (size_t i=0; i<moves.size(); ++i) {
    std::shared_ptr<InputData> job(new InputData(state, i));
    ApplyMoveOfTurn::doMove(job->state, moves[i].getMove());
    job->last_move = moves[i].getMove();
    queue_in.push_back(job);
  }
  for (size_t i=0; i<moves.size(); ++i) {
    std::shared_ptr<OutputData> result = queue_out.pop_front();
    values[result->order] = MoveLogProb(result->last_move, result->value*sign);
  }
  CArray<MoveLogProb,N> others;
  size_t i=0;
  for (; i<Width.size(); ++i) {
    const int n = Width[i];
    assert(n > 1);
    if (n > (int)moves.size())
      break;
    MoveVector ignore_moves;
    for (int j=0; j<n; ++j)
      ignore_moves.push_back(values[j].getMove());
    if (ignore_moves.isMember(next))
      break;
    std::shared_ptr<InputData> job(new InputData(state, i));
    job->ignore_moves = ignore_moves;
    queue_in.push_back(job);
  }
  for (size_t j=0; j<i; ++j) {
    std::shared_ptr<OutputData> result = queue_out.pop_front();
    others[result->order] = MoveLogProb(result->move, result->value*sign);
  }
  CArray<int, N+1> ret;
  for (size_t i=0; i<Width.size(); ++i) {
    std::cerr << "n = " << Width[i] << "\n";
    const int n = std::min(Width[i]-1, (int)values.size());
    assert(n <= (int)values.size());
    MoveLogProbVector range(values.begin(), values.begin()+n);
    if (others[i].getMove().isNormal())
      range.push_back(others[i]);
    range.sortByProbability();
    if (1) {
      BOOST_FOREACH(MoveLogProb m, range) {
	std::cerr << "  " << record::csa::show(m.getMove()) << ' ' << m.getLogProb();
	if (m.getMove() == next) break;
      }
    }
    std::cerr << "\n";
    ret[i] = order_by_probability(range, next);
  }
  values.sortByProbability();
  ret[N] = order_by_probability(values, next);
  return ret;
}

void run_position(size_t i, const NumEffectState& state, Move next)
{
  MoveLogProbVector moves;
  genmove_probability(state, 2000, moves);
  int order_p = order_by_probability(moves, next);
  CArray<int,7> order_s = order_by_search(state, moves, next);
  std::cout << i+1 << ' ' << record::csa::show(next) << ' ' << moves.size()
	    << ' ' << order_p+1;
  BOOST_FOREACH(int order, order_s)
    std::cout << ' ' << order+1;
  std::cout << "\n";  
}

void run_record(const std::string& filename) 
{
  std::cerr << filename << "\n";
  CsaFile file(filename);
  NumEffectState state = file.getInitialState();
  Record record = file.getRecord();
  vector<Move> moves = record.getMoves();
  for (size_t i=0; i<moves.size(); ++i) {
    if ((int)i >= skip_moves) {
      run_position(i, state, moves[i]);
    }
    ApplyMoveOfTurn::doMove(state, moves[i]);
  }  
}

int main(int argc, char **argv)
{
  po::options_description other_options("Other options");
  other_options.add_options()
    ("seconds,S",
     po::value<int>(&seconds)->default_value(seconds),
     "seconds for search")
    ("num-cpus,N",
     po::value<int>(&num_cpus)->default_value(num_cpus),
     "for parallel analyses, not for parallel search")
    ("moves,m",
     po::value<int>(&skip_moves)->default_value(skip_moves),
     "skip first n moves")
    ("help", "produce help message")
    ;
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("search-file", po::value<std::vector<std::string> >());

  po::options_description command_line_options;
  command_line_options.add(other_options).add(hidden);
  po::options_description visible_options("All options");
  visible_options.add(other_options);
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
  eval::ml::OpenMidEndingEval::setUp();
  progress::ml::NewProgress::setUp();
  rating::StandardFeatureSet::instance();
  std::cerr << "seconds " << seconds << "\n";
  boost::ptr_vector<boost::thread> threads;
  for (int i=0; i<num_cpus; ++i)
    threads.push_back(new boost::thread(ThreadRun(i)));

  BOOST_FOREACH(const std::string& filename, filenames) {
    run_record(filename);
  }
  std::cerr << "finish\n";
  queue_in.quit();
  for (int i=0; i<num_cpus; ++i)
    threads[i].join();
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

