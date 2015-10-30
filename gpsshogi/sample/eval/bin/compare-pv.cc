#include "pvFile.h"
#include "pvVector.h"
#include "eval/eval.h"
#include "eval/evalFactory.h"

#include "osl/numEffectState.h"
#include "osl/state/historyState.h"
#include "osl/move_classifier/shouldPromoteCut.h"
#include "osl/record/kisen.h"
#include "osl/eval/evalTraits.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"
#include "osl/stat/average.h"

#include <boost/program_options.hpp>
#include <functional>
#include <algorithm>
#include <iostream>

namespace po = boost::program_options;
using namespace osl;

std::unique_ptr<gpsshogi::Eval> my_eval;
int evaluate(const NumEffectState& src, const gpsshogi::PVVector& pv)
{
  HistoryState state(src);
  for (size_t i=0; i<pv.size(); ++i)
    state.makeMove(pv[i]);
  return my_eval->eval(state.state());
}

class PVCompare
{
protected:
  const NumEffectState *state;
public:
  virtual ~PVCompare()
  {
  }
  virtual void init(const NumEffectState& state, const std::vector<Move>&, int next) { this->state = &state; }
  virtual void add(const gpsshogi::PVVector& pv)=0;
  virtual void finish() {}
};

class PassIsBetter : public PVCompare
{
  gpsshogi::PVVector pv_main, pv_pass;
  Move next_move;
public:
  void init(const NumEffectState& state, const std::vector<Move>& moves, int next) { 
    this->state = &state;
    next_move = moves[next];
    pv_main.clear(); pv_pass.clear(); 
  }
  void add(const gpsshogi::PVVector& pv)
  {
    if (pv.empty())
      return;
    if (pv[0].isPass())
      pv_pass = pv;
    else if (pv[0] == next_move)
      pv_main = pv;
  }
  void finish() 
  {
    if (pv_main.empty() || pv_pass.empty())
      return;
    const int eval_main = evaluate(*state, pv_main);
    const int eval_pass = evaluate(*state, pv_pass);
    if (eval::betterThan(state->turn(), eval_pass, eval_main)) {
      std::cout << *state;
      std::cout << eval_main << ' ' << pv_main
		<< eval_pass << ' ' << pv_pass << "\n";
    }
  }
};

class MoveAfterPass : public PVCompare
{
  gpsshogi::PVVector pv_pass;
  Move next2_move;
public:
  void init(const NumEffectState& state, const std::vector<Move>& moves, int next) { 
    this->state = &state;
    next2_move = Move();
    if (next+1 < (int)moves.size())
      next2_move = moves[next+1];
    pv_pass.clear(); 
  }
  void add(const gpsshogi::PVVector& pv)
  {
    if (pv.empty())
      return;
    if (pv[0].isPass())
      pv_pass = pv;
  }
  void finish() 
  {
    if (!next2_move.isNormal() || pv_pass.size() < 2)
      return;
    if (pv_pass[1] != next2_move) {
      std::cout << *state;
      std::cout << next2_move << "\n" << pv_pass << "\n";
    }
  }
};

class EvalRange : public PVCompare
{
  std::vector<double> values;
  double selected_value;
  CArray<Average,16> best2, best10, best20, median, worst, selected;
  Player turn;
  Move next_move;
public:
  ~EvalRange() 
  {
  }
  void init(const NumEffectState& state, const std::vector<Move>& moves, int next) 
  { 
    this->state = &state;
    values.clear();
    turn = state.turn();
    next_move = moves[next];
  }
  void add(const gpsshogi::PVVector& pv)
  {
    const double eval = evaluate(*state, pv)
      *eval::delta(turn)*100.0/my_eval->pawnValue();
    values.push_back(eval);
    if (!pv.empty() && pv[0] == next_move)
      selected_value = eval;
  }
  void finish() 
  {    
    if (values.size() < 20)
      return;
    std::sort(values.begin(), values.end(), std::greater<int>());
    const int p = progress::ml::NewProgress(*state).progress16().value();
    best2[p].add(values[0]-values[1]);
    best10[p].add(values[0]-values[9]);
    best20[p].add(values[0]-values[19]);
    median[p].add(values[0]-values[values.size()/2]);
    worst[p].add(values[0]-values.back());
    selected[p].add(values[0]-selected_value);
    if ((best2[0].numElements() % 1024) == 0) {
      for (int i=0; i<16; ++i) {
	std::cout << i << ' ' << best2[i].average()
		  << ' ' << best10[i].average()
		  << ' ' << best20[i].average()
		  << ' ' << median[i].average() << ' ' << worst[i].average()
		  << ' ' << selected[i].average() << "\n" << std::flush;
      }
    }
  }
};

class EqualValue : public PVCompare
{
  typedef std::vector<int> vector_t;
  vector_t values;
  int selected_value, cur_move;
  Player turn;
  Move next_move;
  Average mean_count, mean_order, mean_siblings, mean_legals;
  std::vector<Average> stage_count, stage_order, stage_siblings, stage_legals;
public:
  EqualValue()
  {
    const int length = 200;
    stage_count.resize(length);
    stage_order.resize(length);
    stage_siblings.resize(length);
    stage_legals.resize(length);
  }
  ~EqualValue()
  {
    std::cout << "all eqcount " << mean_count.average()
	      << " order " << mean_order.average()
	      << " siblings " << mean_siblings.average()
	      << " legals " << mean_legals.average()
	      << " total " << mean_count.numElements() << "\n";
    for (size_t i=0; i<stage_count.size(); ++i) {
      std::cout << i << ' ' << stage_count[i].average()
		<< ' ' << stage_order[i].average()
		<< ' ' << stage_siblings[i].average()
		<< ' ' << stage_legals[i].average()
		<< ' ' << stage_count[i].numElements()
		<< "\n";
    }
    std::cout << std::flush;
  }
  void add(int cur_move, int count, int order, int siblings)
  {
    mean_count.add(count);
    mean_order.add(order);
    mean_siblings.add(siblings);
    if (cur_move < (int)stage_count.size()) {
      stage_count[cur_move].add(count);
      stage_order[cur_move].add(order);
      stage_siblings[cur_move].add(siblings);
    }
  }
  void init(const NumEffectState& state, const std::vector<Move>& moves, int next) 
  { 
    this->state = &state;
    values.clear();
    turn = state.turn();
    cur_move = next;
    next_move = moves[next];
    MoveVector all;
    state.generateLegal(all);
    mean_legals.add(all.size());
    if (cur_move < (int)stage_legals.size())
      stage_legals[cur_move].add(all.size());
  }
  void add(const gpsshogi::PVVector& pv)
  {
    const int eval = evaluate(*state, pv);
    values.push_back(eval);
    if (!pv.empty() && pv[0] == next_move)
      selected_value = eval;
  }
  void finish() 
  {    
    if (values.size() <= 1)
      return;
    std::sort(values.begin(), values.end()); // bad move first
    vector_t::iterator p = std::find(values.begin(), values.end(), selected_value);
    const int count = std::count(p, values.end(), selected_value);
    const int order = (turn == WHITE)
      ? (p - values.begin()) : (values.size() - (p - values.begin() + 1));
    if (0)
      std::cout << cur_move << ' ' << count << ' ' << order << ' ' << values.size() << "\n";
    add(cur_move, count, order, values.size());
  }
};

int main(int argc, char **argv)
{
  std::string eval_type, eval_data, kisen_filename, predicate_name;
  std::vector<std::string> pv_filenames;
  bool show_record_move, quiet;
  size_t num_records;
  
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("num-records,n",
     po::value<size_t>(&num_records)->default_value(0),
     "number of records to be analyzed (all if 0)")
    ("predicate",
     boost::program_options::value<std::string>(&predicate_name)->
     default_value("pass_is_better"),
     "Predicate to use.  Valid options are pass_is_better, "
     "move_after_pass, and eval_range")
    ("kisen-file,k",
     po::value<std::string>(&kisen_filename)->default_value(""),
     "Kisen filename corresponding to pv file")
    ("show-record-move",
     po::value<bool>(&show_record_move)->default_value(false),
     "show record move in addition to position when predicate matches")
    ("eval,e",
     po::value<std::string>(&eval_type)->default_value(std::string("piece")),
     "evaluation function (king or piece)")
    ("eval-data",
     po::value<std::string>(&eval_data)->default_value(""))
    ("quiet,q",
     po::value<bool>(&quiet)->default_value(false),
     "counting only.  do not show positions matched.")
    ;
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("pv-file", po::value<std::vector<std::string> >(),
     "filename containing PVs");
  po::options_description all_options;
  all_options.add(command_line_options).add(hidden);
  po::positional_options_description p;
  p.add("pv-file", -1);
  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).
	      options(all_options).positional(p).run(), vm);
    po::notify(vm);
    if (vm.count("pv-file")) 
    {
      pv_filenames = vm["pv-file"].as<std::vector<std::string> >();
      std::cerr << "file(s):";
      for (std::string file: pv_filenames)
	std::cerr << ' ' << file;
      std::cerr << "\n";
    }
    else
    {
      std::cerr << "PV file wasn't specified" << std::endl;
      return 1;
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  OslConfig::setUp();
  if (!osl::eval::ml::OpenMidEndingEval::setUp()) {
    std::cerr << "OpenMidEndingEval set up failed";
    // fall through as this might not be fatal depending on eval type used
  }
  if (!osl::progress::ml::NewProgress::setUp()) {
    std::cerr << "NewProgress set up failed";
    // fall through as this might not be fatal depending on eval type used
  }
  my_eval.reset(gpsshogi::EvalFactory::newEval(eval_type));
  if (! my_eval) {
    std::cerr << "unknown eval type " << eval_type << "\n";
    throw std::runtime_error("unknown eval type");
  }
  if (!eval_data.empty())
    if (! my_eval->load(eval_data.c_str()))
      std::cerr << "load failed " << eval_data << "\n";

  osl::record::KisenFile kisen(kisen_filename);

  std::unique_ptr<PVCompare> compare;
  if (predicate_name == "pass_is_better")
  {
    compare.reset(new PassIsBetter);
  }
  else if (predicate_name == "eval_range")
  {
    compare.reset(new EvalRange);
  }
  else if (predicate_name == "move_after_pass")
  {
    compare.reset(new MoveAfterPass);
  }
  else if (predicate_name == "equal_value")
  {
    compare.reset(new EqualValue);
  }
  else
  {
    std::cerr << "Unknown predicate "  << predicate_name;
    return 1;
  }

  std::vector<osl::Move> moves;
  osl::NumEffectState state(kisen.initialState());
  size_t num_processed = 0;
  for (size_t i = 0; i < pv_filenames.size(); ++i)
  {
    gpsshogi::PVFileReader pr(pv_filenames[i].c_str());
    int record, position;
    int cur_record = -1;
    int cur_position = 0;
    while (pr.newPosition(record, position))
    {
      if (record != cur_record)
      {
	cur_record = record;
	moves = kisen.moves(cur_record);
	cur_position = 0;
	state = NumEffectState(kisen.initialState());
	if (record < 0)
	{
	  continue;
	}
	if (++num_processed >= num_records && num_records > 0)
	  break;
      }
      assert(position > cur_position || cur_position == 0);
      while (position > cur_position)
      {
	state.makeMove(moves[cur_position]);
	++cur_position;
      } 
      if (cur_position >= (int)moves.size())
	continue;
      
      compare->init(state, moves, cur_position);
      
      gpsshogi::PVVector pv;
      while (pr.readPv(pv))
      {
	compare->add(pv);
	pv.clear();
      }
      compare->finish();
    }
    if (num_processed >= num_records && num_records > 0)
      break;
  }
  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
