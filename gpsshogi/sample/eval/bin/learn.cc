#include "loss.h"
#include "eval/eval.h"
#include "eval/evalFactory.h"
#include "eval/progress.h"
#include "l1Ball.h"
#include "analyzer.h"
#include "pvFile.h"
#include "pvVector.h"
#include "pvGenerator.h"
#include "rotateRecord.h"
#include "osl/record/csaRecord.h"
#include "osl/record/kisen.h"
#include "osl/numEffectState.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"
#include "osl/misc/milliSeconds.h"
#include "osl/bits/binaryIO.h"
#include "osl/stat/histogram.h"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/program_options.hpp>
#include <thread>
#include <future>
#include <tuple>
#include <algorithm>
#include <vector>
#include <algorithm>
#include <valarray>
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <unistd.h>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;
typedef std::valarray<double> valarray_t;

void analyze(const std::string& output_prefix, 
	     size_t start_id, size_t num_records,
	     const std::string& initial_file,
	     int cross_start,
	     int max_progress,
	     bool high_rating_only);

std::unique_ptr<gpsshogi::Eval> my_eval;
std::string eval_type, loss_function;
size_t num_iteration, num_iteration_internal;
double lambda;
size_t min_rating = 1500, min_frequency, min_frequency_record;
double search_window, search_window_for_validation;
std::vector<std::string> kisen_filenames;
std::vector<std::string> csa_filenames;
double scaling_factor, eta_increment, sigmoid_asymmetry;
size_t num_cpus;
std::string pv_base;
bool smd, fix_step_size, weight_midgame, weight_winner, weight_by_branches;
bool accelerate_minor_features, clear_infrequent, clear_infrequent_by_record;
bool parent_child_smoothing;
int second_cross_start;
int watch_threshold, watch_threshold_seed;
double endgame_margin, decay_sibling_weight, aggressive_penalty_coef, high_king_weight;
bool rotate_record_mode;

std::unique_ptr<RotateRecord> rotate_record;

/** 進行度が後半まで分布するよう重みを加える */
double force_progress;

int main(int argc, char **argv)
{
  nice(20);
  
  size_t kisen_start, num_records;
  std::string output_prefix, initial_value, priority_record_text;
  po::options_description options("all_options");
  int cross_start;
  int max_progress;
  bool high_rating_only, compare_pass;
  int normal_depth, quiesce_depth, use_percent, limit_sibling;

  options.add_options()
    ("num-records,n",
     po::value<size_t>(&num_records)->default_value(0),
     "number of records to be analyzed (all if 0)")
    ("num-cpus,N",
     po::value<size_t>(&num_cpus)->default_value(1),
     "number cpus to be used")
    ("num-iteration,m",
     po::value<size_t>(&num_iteration)->default_value(1),
     "number of toplevel iterations including search")
    ("num-iteration-internal,M",
     po::value<size_t>(&num_iteration_internal)->default_value(50),
     "maximum number of internal iterations")
    ("output,o",
     po::value<std::string>(&output_prefix)->default_value("./"),
     "directory to write files (e.g., tmp/)")
    ("kisen-start",
     po::value<size_t>(&kisen_start)->default_value(0),
     "start id of kisen records")
    ("kisen-file,k", po::value<std::vector<std::string> >(),
     "filename for records to be analyzed")
    ("csa-file", po::value<std::vector<std::string> >(&csa_filenames)->multitoken(),
     "filename for records to be analyzed")
    ("eval,e",
     po::value<std::string>(&eval_type)->default_value(std::string("piece")),
     "evaluation function (piece, rich0, rich1)")
    ("window",
     po::value<double>(&search_window)->default_value(8),
     "search window relative to pawn value")
    ("vwindow",
     po::value<double>(&search_window_for_validation)->default_value(16),
     "search window for validation, relative to pawn value")
    ("lambda",
     po::value<double>(&lambda)->default_value(100),
     "lambda for l1 regularization")
    ("initial-value-file,f",
     po::value<std::string>(&initial_value)->default_value(""),
     "File with initial eval values")
    ("scaling-factor,s",
     po::value<double>(&scaling_factor)->default_value(0),
     "Scaling factor of initial eval values (ignored if 0)")
    ("max-progress",
     po::value<int>(&max_progress)->default_value(16),
     "When non-negative, only use states where progress is less than this "
      "value.")
    ("high-rating-only",
     po::value<bool>(&high_rating_only)->default_value(false),
      "When true only consider plays where both player have at least "
      "1500 rating value")
    ("cross-validation-start",
     po::value<int>(&cross_start)->default_value(200000),
     "Start ID of record in kisen file to do cross validation")
    ("second-cross-validation-start",
     po::value<int>(&second_cross_start)->default_value(-1),
     "Start ID of record in kisen file to do another cross validation if >= 0")
    ("eta-increment",
     po::value<double>(&eta_increment)->default_value(1.2),
     "scale to modify eta")
    ("loss-function,l",
     po::value<std::string>(&loss_function)->default_value(std::string("log")),
     "loss function (log, hinge, exp, sigmoid, halfsigmoid)")
#ifndef L1BALL_NO_SORT
    ("smd",
     po::value<bool>(&smd)->default_value(0),
     "enable stochastic meta descent if 1")
    ("fix-step-size",
     po::value<bool>(&fix_step_size)->default_value(0),
     "fix step size (not recommended) (only effective when smd == 0)")
    ("sigmoid-asymmetry",
     po::value<double>(&sigmoid_asymmetry)->default_value(1.0),
     "asymmetry (only effective when the loss function is sigmoid)")
#endif
    ("limit-greater-sibling",
     po::value<int>(&limit_sibling)->default_value(0),
     "consider only at most n moves that have higher evaluation values than that of recorded move for each position")
    ("accelerate-minor-features",
     po::value<bool>(&accelerate_minor_features)->default_value(0),
     "try larger step size in update the weights of less frequent features (only effective when smd == 0)")
    ("frequency-min",
     po::value<size_t>(&min_frequency)->default_value(200),
     "do not modify the weights of the features whose frequency is less than 200 times")
    ("frequency-record-min",
     po::value<size_t>(&min_frequency_record)->default_value(10),
     "do not modify the weights of the features that appear in less than 10 records")
    ("clear-infrequent-features",
     po::value<bool>(&clear_infrequent)->default_value(0),
     "set the weights to 0 for features appeared less than 200 times")
    ("clear-infrequent-features-by-record-count",
     po::value<bool>(&clear_infrequent_by_record)->default_value(0),
     "set the weights to 0 for features appeared less than 10 records")
    ("rotate-record",
     po::value<bool>(&rotate_record_mode)->default_value(false),
     "rotate records")
    ("watch-threshold",
     po::value<int>(&watch_threshold_seed)->default_value(-10),
     "report details about features whose absolute value is greater than this, or negative value for top-k values")
    ("depth",
     po::value<int>(&normal_depth)->default_value(2),
     "adjust quiescence depth")
    ("quiesce-depth",
     po::value<int>(&quiesce_depth)->default_value(4),
     "adjust quiescence depth")
    ("record-coverage",
     po::value<int>(&use_percent)->default_value(100),
     "randomlly skip (100 - this)% of specified records")
    ("compare-pass",
     po::value<bool>(&compare_pass)->default_value(true),
     "include pass in sibling moves to be compared unless in check")
    ("weight-midgame",
     po::value<bool>(&weight_midgame)->default_value(true),
     "give more importance to instances in midgame than those in opening or endgame")
    ("weight-by-branches",
     po::value<bool>(&weight_by_branches)->default_value(false),
     "give a weight proportional to the number of pairs")
    ("weight-winner",
     po::value<bool>(&weight_winner)->default_value(false),
     "give more importance to winner's move")
    ("endgame-margin",
     po::value<double>(&endgame_margin)->default_value(0.0),
     "endgame margin (relative to pawn) especially for hinge-loss")
    ("aggressive-penalty",
     po::value<double>(&aggressive_penalty_coef)->default_value(1.0),
     "give more penalty to aggressive moves (when > 1.0)")
    ("high-king-coef",
     po::value<double>(&high_king_weight)->default_value(1.0),
     "give more weight in positions with king outside his area (when > 1.0)")
    ("decay-sibling-weight",
     po::value<double>(&decay_sibling_weight)->default_value(1.0),
     "decrease learning weight for siblings based on the difference of evaluation (need --limit-sibling=n for sort siblings")
    ("priority-record-text",
     po::value<std::string>(&priority_record_text)->default_value(""),
     "specify a filename contaiting record ids that will be given priority when --record-coverage x < 100.")
    ("force-progress",
     po::value<double>(&force_progress)->default_value(0.0),
     "experimental feature only for effectprogress")
    ("parent-child-smoothing",
     po::value<bool>(&parent_child_smoothing)->default_value(false),
     "add penality for overestimation")
    ("help", "produce help message")
    ;

  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);
    if (vm.count("kisen-file"))
      kisen_filenames = vm["kisen-file"].as<std::vector<std::string> >();
    else
      kisen_filenames.push_back("../../../data/kisen/01.kif");
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }
  if (vm.count("help") || (fix_step_size && smd) 
      || (rotate_record_mode && (kisen_filenames.size() != 1 || second_cross_start != -1))
      || (decay_sibling_weight < 1.0 && limit_sibling == 0)) {
    std::cerr << options << std::endl;
    return 0;
  }
  if (num_cpus == 0 || (int)num_cpus > std::thread::hardware_concurrency()) {
    std::cerr << "do you really have so many cpus? " << num_cpus << "\n";
    return 1;
  }    
  if (! high_rating_only)
    min_rating = 0;

  OslConfig::setUp();
  if (!osl::eval::ml::OpenMidEndingEval::setUp()) {
    std::cerr << "OpenMidEndingEval set up failed";
    // fall through as this might not be fatal depending on eval type used
  }
  if (!osl::progress::ml::NewProgress::setUp()) {
    std::cerr << "NewProgress set up failed";
    // fall through as this might not be fatal depending on eval type used
  }
  PVGenerator::setLimitSibling(limit_sibling);
  PVGenerator::setQuiesceDepth(quiesce_depth);
  PVGenerator::setNormalDepth(normal_depth);
  PVGenerator::setUsePercent(use_percent);
  PVGenerator::setComparePass(compare_pass);
  if (loss_function == "hinge")
    PVGenerator::setWindowAsymmetry(4);
  if (rotate_record_mode) {
    std::string work_filename = "working.kif";
    rotate_record.reset(new RotateRecord(kisen_filenames[0], num_records, work_filename, kisen_start));
    kisen_start = 0;
    kisen_filenames[0] = work_filename;
    cross_start = 0;
  }
  if (! priority_record_text.empty()) {
    std::ifstream is(priority_record_text.c_str());
    if (! is) {
      std::cerr << "open failed " << priority_record_text << "\n";
      return 1;
    }
    std::vector<int> priority;
    int value;
    while (is >> value)
      priority.push_back(value);
    KisenAnalyzer::setPriorityRecord(priority);
    std::cerr << "priority record " << priority.size()
	      << " in " << priority_record_text << "\n";
  } 
  analyze(output_prefix, kisen_start, num_records,
	  initial_value, cross_start, 
	  max_progress, high_rating_only);
}

const int MaxThreads = 64;
static PVGenerator::Result results[MaxThreads];
static KisenAnalyzer::RecordConfig record_configs[MaxThreads];

static int position_randomness = 1;

valarray_t gradient[MaxThreads], Hv[MaxThreads], v_for_Hv;
stat::Average errors[MaxThreads];
std::unique_ptr<Histogram> progress_count[MaxThreads];
std::unique_ptr<Histogram> progress_count_prev;
std::vector<int> progress_count_sum;
/** feature の出現率 */
valarray_t frequency[MaxThreads];
/** feature の出現回数 */
std::vector<size_t> count;
/** feature の出現棋譜数. [0] を合計に代用 */
valarray_t count_by_record[MaxThreads];
valarray_t variance, reduce_scale;
long long total[MaxThreads];
long long aggressive_penalty_moves[MaxThreads];
/** 監視対象の重み */
std::vector<int> watch_list;

void init_progress_count()
{
  for (size_t i=0; i<num_cpus; ++i)
    progress_count[i].reset(new Histogram(1, my_eval->maxProgress()));
}
void merge_progress_count()
{
  progress_count_prev.reset(new Histogram(1, my_eval->maxProgress()));
  for (size_t i=0; i<num_cpus; ++i)
    progress_count_prev->merge(*progress_count[i]);
  progress_count_sum.resize(my_eval->maxProgress());
  for (int i=0; i<my_eval->maxProgress(); ++i) {
    progress_count_sum[i] = progress_count_prev->frequency(i);
    if (i>0)
      progress_count_sum[i] += progress_count_sum[i-1];
  }
}
void write_progress_count(const std::string& text_name)
{
  std::ofstream os(text_name.c_str());
  progress_count_prev->show(os);
}

struct ProcessPVBase : public misc::Align16New
{
  int thread_id, min_count;
  ProcessPVBase(int tid, int count) 
    : thread_id(tid), min_count(count)
  {
  }
  virtual ~ProcessPVBase() {}
  void operator()();
  virtual void beginPosition(double turn_coef, MoveData& selected) const {}
  virtual void beginPositionProgress(double turn_coef, MoveData& selected) const {}
  virtual void processInstance(double turn_coef, MoveData& selected, MoveData& sibling,
			       double weight, double progress) const=0;
  virtual void processInstanceProgress(double turn_coef, MoveData& selected, MoveData& sibling,
				       double weight, double progress) const {}
  virtual void endPosition() const {}
  virtual void finishRecord() const {}
  int curPosition() const { return cur_position; }
  const std::vector<Move>& curMoves() const { return moves; }
  Move curMove() const { return moves[cur_position]; }
  Player turn() const { return state.turn(); }
private:
  int cur_record=-1, cur_position=-1;
  NumEffectState state;
  std::vector<Move> moves;
};

void ProcessPVBase::operator()()
{
  std::string filename = PVGenerator::pv_file(pv_base, thread_id);

  int num_pv = 0, num_position = 0;
  const std::string kisen_filename = record_configs[thread_id].kisen_filename;
  KisenFile kisen_file(kisen_filename.c_str());

  PVFileReader pr(filename.c_str());
  int record, position;
  PVVector pv_best, pv;

  cur_record = cur_position = -1;
  state = kisen_file.initialState();

  while (pr.newPosition(record, position)) {
    ++num_position;
    if (record != cur_record) {
      cur_record = record;
      if (record >= 0)
      {
        moves = kisen_file.moves(cur_record);
        state = NumEffectState(kisen_file.initialState());
      }
      else
      {
        CsaFile csa(record_configs[thread_id].csa_filenames[-record - 1]);
        state = csa.initialState();
        moves = csa.moves();
      }
      cur_position = 0;
      finishRecord();
    }
    assert(position > cur_position || cur_position == 0);
    while (position > cur_position) {
      state.makeMove(moves[cur_position]);
      ++cur_position;
    }
    
    bool has_best_move = pr.readPv(pv);
    assert(has_best_move);
    if (!has_best_move)
      return;
    MoveData selected, sibling, progress_selected, progress_sibling;
    // analyze_leaf
    {
      NumEffectState leaf_state(state);
      Analyzer::makeLeaf(leaf_state, pv);
      my_eval->features(leaf_state, selected);
      if (force_progress)
	my_eval->featuresProgress(leaf_state, progress_selected);
    }
    const int turn_coef = (state.turn() == BLACK) ? 1 : -1;
    beginPosition(turn_coef, selected);
    if (force_progress)
      beginPositionProgress(turn_coef, progress_selected);
    int num_sibling = 0;
    std::vector<PVVector> pvs;
    while (pr.readPv(pv)) {
      pvs.push_back(pv);
    }
    const double progress_move_number = 1.0*cur_position/moves.size(); // [0,1]
    double weight = 1.0;
    if (weight_midgame)
      weight = 1.0 - abs(0.5-progress_move_number);
    if (weight_winner && moves.size() < 256 
	&& (((int)moves.size() % 2) == (cur_position % 2))) // moves[size] corresponds to toryo
      weight = std::min(0.5, weight);
    if (weight_by_branches)
      weight /= std::max(20, (int)pvs.size());
    if (record < 0) {
      weight *= 5;
    }
    if (state.kingSquare(BLACK).y() < 7 || state.kingSquare(WHITE).y() > 3)
      weight *= high_king_weight;
    for (const auto& pv: pvs) {
      ++num_pv; ++num_sibling;
      // analyze_leaf
      {
	NumEffectState leaf_state(state);
	Analyzer::makeLeaf(leaf_state, pv);
	my_eval->features(leaf_state, sibling);
	if (force_progress)
	  my_eval->featuresProgress(leaf_state, progress_sibling);
      }
      processInstance(turn_coef, selected, sibling, weight, progress_move_number);
      if (force_progress)
	processInstanceProgress(turn_coef, progress_selected,
				progress_sibling, weight, progress_move_number);
    }
    endPosition();
  }
  finishRecord();
}

struct ProcessPVCount : public ProcessPVBase
{
  mutable InstanceData instance;
  mutable std::vector<size_t> count_work;
  explicit ProcessPVCount(int tid) : ProcessPVBase(tid, 0), count_work(my_eval->dimension())
  {
  }
  void beginPosition(double /*turn_coef*/, MoveData& selected) const
  {
    progress_count[thread_id]->add(selected.progress);
#ifdef L1BALL_NO_SORT
    std::sort(selected.diffs.begin(), selected.diffs.end());
#endif
  }
  void processInstance(double, MoveData&, MoveData&, double, double) const;
  void finishRecord() const;
};

void ProcessPVCount::processInstance(double turn_coef, MoveData& selected, MoveData& sibling, double, double) const
{
  progress_count[thread_id]->add(sibling.progress);
  assert(std::is_sorted(selected.diffs.begin(), selected.diffs.end()));
  std::sort(sibling.diffs.begin(), sibling.diffs.end());
  instance.clear();
  Analyzer::makeInstanceSorted(turn_coef, selected.diffs, sibling.diffs,
			       count, min_count, instance);
  if (instance.index.empty())
    return;
  total[thread_id]++;
  for (size_t i=0; i<instance.index.size(); ++i) {
    if (i && instance.index[i] == instance.index[i-1]) // unique?
      continue;
    frequency[thread_id][instance.index[i]] += 1.0;
    count_work[instance.index[i]]++;
  }
}

void ProcessPVCount::finishRecord() const
{
  for (size_t i=0; i<count_work.size(); ++i) {    
    if (count_work[i] == 0)
      continue;
    count_by_record[thread_id][i] += 1.0;
    count_work[i] = 0;
  }
}

/* ------------------------------------------------------------------------- */

struct ProcessPV : public ProcessPVBase
{
  const valarray_t& w;
  bool make_hv;
#ifdef L1BALL_NO_SORT
  mutable double agsum, adot;
  mutable MoveData selected;
#else
  mutable MoveData instance;
#endif
  mutable double sibling_weight;
  mutable int instance_count, prev_position=-100;
  ProcessPV(int tid, const valarray_t& iw, bool hv) 
    : ProcessPVBase(tid, min_frequency), w(iw), make_hv(hv),
      sibling_weight(1.0), instance_count(0)
  {
  }
  void beginPosition(double turn_coef, MoveData& selected) const;
  void beginPositionProgress(double turn_coef, MoveData& selected) const;
  void processInstance(double, MoveData&, MoveData&, double, double) const;
  void processInstanceProgress(double turn_coef, MoveData& selected, MoveData& sibling,
			       double weight, double progress) const;
  void endPosition() const;
};

void ProcessPV::beginPosition(double turn_coef, MoveData& selected) const { 
  sibling_weight = 1.0;
  instance_count = 0;
#ifdef L1BALL_NO_SORT
  if (parent_child_smoothing && prev_position +1 == curPosition()) {
    // parent-child smoothing
    agsum = 0;
    const MoveData& parent = this->selected, &child = selected;
    // parent < child: ng (an overestimation in the parent move)
    // parent > child: ok (maybe a blunder)
    if (parent.value*turn_coef < child.value*turn_coef) {
      double loss = LogLoss::addGradientSep
	(w, adot, parent.diffs, child.diffs, turn_coef, count, min_count,
	 gradient[thread_id], agsum);
      LogLoss::addGradientSep(parent.diffs, count, min_count, agsum, gradient[thread_id]);
    }
  }
  agsum = 0; 
  prev_position = curPosition();
  this->selected = selected;
  adot = ValarrayUtil::dot(this->selected.diffs, w, count, min_count);
#endif
}

void ProcessPV::beginPositionProgress(double turn_coef, MoveData& selected) const { 
  if (force_progress) {
    sparse_vector_t empty_a;
    double agsum_dummy;
    double progress = ValarrayUtil::dot(selected.diffs, w, count, min_count);
    double error = std::max(0.0, my_eval->maxProgress()/scaling_factor-progress);
    HingeLoss::addGradientSep(w, error, empty_a,
			      selected.diffs, -1, count, min_count,
			      gradient[thread_id], agsum_dummy, force_progress);
    errors[thread_id].add(error*force_progress);
  }
}

void ProcessPV::endPosition() const
{
#ifdef L1BALL_NO_SORT
  if (loss_function == "sigmoid")
    SigmoidLoss::addGradientSep(selected.diffs, count, min_count, agsum, gradient[thread_id]);
  else if (loss_function == "halfsigmoid")
    HalfSigmoidLoss::addGradientSep(selected.diffs, count, min_count, agsum, gradient[thread_id]);
  else if (loss_function == "hinge")
    HingeLoss::addGradientSep(selected.diffs, count, min_count, agsum, gradient[thread_id]);
  else
    LogLoss::addGradientSep(selected.diffs, count, min_count, agsum, gradient[thread_id]);
#endif
}

void ProcessPV::processInstance(double turn_coef, MoveData& selected, MoveData& sibling, double instance_weight, double progress_move_number) const
{
  double loss = 0;
  double weight = instance_weight*sibling_weight;
  ++instance_count;
  const bool better_than_selected = turn_coef*(sibling.value-this->selected.value) > 0;
#ifdef L1BALL_NO_SORT
  if (better_than_selected && sibling.progress > 128+this->selected.progress) {
    weight *= aggressive_penalty_coef;
    aggressive_penalty_moves[thread_id]++;
  }
  double margin = 0.0;
  if (endgame_margin) 
    margin = progress_move_number*endgame_margin*my_eval->pawnValue()/scaling_factor;
  if (loss_function == "sigmoid")
    loss = SigmoidLoss::addGradientSep(w, adot, this->selected.diffs, sibling.diffs, turn_coef, count, min_count,
				       gradient[thread_id], agsum, weight,
				       margin);
  else if (loss_function == "halfsigmoid")
    loss = HalfSigmoidLoss::addGradientSep(w, adot, this->selected.diffs, sibling.diffs, turn_coef, count, min_count,
				       gradient[thread_id], agsum, weight,
				       margin);
  else if (loss_function == "hinge")
    loss = HingeLoss::addGradientSep(w, adot, this->selected.diffs, sibling.diffs, turn_coef, count, min_count,
				       gradient[thread_id], agsum, weight,
				       margin);
  else
    loss = LogLoss::addGradientSep(w, adot, this->selected.diffs, sibling.diffs, turn_coef, count, min_count,
				   gradient[thread_id], agsum, weight,
				   margin);
  if (better_than_selected)
    instance_weight *= decay_sibling_weight;
#else
  instance.clear();
  Analyzer::makeInstanceSorted(turn_coef, selected, sibling, count, min_count, instance);
  if (instance.index.empty())
    return;

  if (loss_function == "sigmoid") {
    if (make_hv)
      loss = SigmoidLoss::addGradient(w, instance, gradient[thread_id], v_for_Hv, Hv[thread_id], sigmoid_asymmetry);
    else
      loss = SigmoidLoss::addGradient(w, instance, gradient[thread_id], sigmoid_asymmetry);
  }
  else if (loss_function == "hinge") {
    if (make_hv)
      loss = HingeLoss::addGradient(w, instance, gradient[thread_id], v_for_Hv, Hv[thread_id]);
    else
      loss = HingeLoss::addGradient(w, instance, gradient[thread_id]);
  }
  else if (loss_function == "exp") {
    if (make_hv)
      loss = ExpLoss::addGradient(w, instance, gradient[thread_id], v_for_Hv, Hv[thread_id]);
    else
      loss = ExpLoss::addGradient(w, instance, gradient[thread_id]);
  }
  else {
    assert(loss_function == "log");
    if (make_hv)
      loss = LogLoss::addGradient(w, instance, gradient[thread_id], v_for_Hv, Hv[thread_id]);
    else
      loss = LogLoss::addGradient(w, instance, gradient[thread_id]);
  }
#endif
  errors[thread_id].add(loss);
}

void ProcessPV::processInstanceProgress(double turn_coef, MoveData& selected, MoveData& sibling, double instance_weight, double progress_move_number) const
{
  if (force_progress) {
    sparse_vector_t empty_a;
    double agsum_dummy;
    double progress = ValarrayUtil::dot(sibling.diffs, w, count, min_count);
    double error = std::max(0.0, my_eval->maxProgress()/scaling_factor-progress);
    HingeLoss::addGradientSep(w, error, empty_a,
			      sibling.diffs, -1, count, min_count,
			      gradient[thread_id], agsum_dummy, force_progress);
    errors[thread_id].add(error*force_progress);
  }
}

/* ------------------------------------------------------------------------- */
class GradientDescentUtil
{
protected:
  double prev_error, min_error;
  int prev_i;

  GradientDescentUtil()
  {
    prev_error = min_error = 1e8;
    prev_i = -1;
  }
  void iterationHead(int i, const valarray_t& w, double prev_error);
  void makeGradient(const valarray_t& w, valarray_t& gradient, double& error, bool make_hv);
};

void GradientDescentUtil::makeGradient(const valarray_t& w, valarray_t& out, double& error, bool make_hv)
{
  for (size_t i=0; i<num_cpus; ++i) {
    gradient[i].resize(w.size());
    gradient[i] = 0.0;
    if (make_hv) {
      Hv[i].resize(w.size());
      Hv[i] = 0.0;
    }
    errors[i].clear();
    aggressive_penalty_moves[i] = 0;
  }
  std::vector<std::future<void>> threads;
  for (size_t i=0; i<num_cpus; ++i)
    threads.push_back(std::async(std::launch::async, ProcessPV(i, w, make_hv)));
  for (size_t i=0; i<num_cpus; ++i)
    threads[i].get();

  out.resize(w.size());
  out = 0.0;
  for (size_t i=0; i<num_cpus; ++i) {
    out += gradient[i];
    if (i) {
      errors[0].merge(errors[i]);
      aggressive_penalty_moves[0] += aggressive_penalty_moves[i];
    }
  }
  error = errors[0].average();
}

void GradientDescentUtil::iterationHead(int i, const valarray_t& w, double error)
{
  if (i == 0 || prev_i == i) 
    return;
  prev_i = i;
  if (i == 1 || error < min_error) {
    min_error = error;
    std::cerr << '*';
  }
  else if (error < prev_error)
    std::cerr << '+';
  else 
    std::cerr << '-';
  prev_error = error;
  if (i % 10 == 0)
    std::cerr << error;
}

class GradientDescent : public gpsshogi::L1Ball, GradientDescentUtil
{
public:
  void iterationHead(int i, const valarray_t& w, double prev_error)
  {
    GradientDescentUtil::iterationHead(i, w, prev_error);
  }
  void makeGradient(const valarray_t& w, valarray_t& gradient, double& error)
  {
    GradientDescentUtil::makeGradient(w, gradient, error, false);
  }
  void initializeEta(valarray_t& eta, double eta0) const
  {
    if (! accelerate_minor_features)
      return gpsshogi::L1Ball::initializeEta(eta, eta0);
    eta = eta0*2;
    eta *= (0.5 - variance);
  }
  static double sign(double a) { return (a >= 0.0) ? 1.0 : -1.0; }
  void reduceEta(valarray_t& eta, int update_failed, const valarray_t& prev_sign, const valarray_t& gradient) const
  {
    if (! accelerate_minor_features)
      return gpsshogi::L1Ball::reduceEta(eta, update_failed, prev_sign, gradient);
    eta *= reduce_scale;
    if (update_failed > 1) {
      if (update_failed > 2)
	eta *= 0.25;
      else
	eta *= 0.5;
      return;
    }
    for (size_t i=0; i<eta.size(); ++i) {
      if (prev_sign[i] != sign(gradient[i]))
	eta[i] *= 0.5;
    }
  }
};

/* ------------------------------------------------------------------------- */
class GradientDescentSMD : public gpsshogi::L1BallSMD, GradientDescentUtil
{
public:
  void iterationHead(int i, const valarray_t& w, double prev_error)
  {
    GradientDescentUtil::iterationHead(i, w, prev_error);
  }
  void makeGradient(const valarray_t& w, valarray_t& gradient, double& error,
		    const valarray_t& v, valarray_t& hv_out)
  {
    v_for_Hv.resize(v.size());
    v_for_Hv = v;
    GradientDescentUtil::makeGradient(w, gradient, error, true);
    hv_out.resize(w.size());
    hv_out = 0.0;
    for (size_t i=0; i<num_cpus; ++i) {
      hv_out += Hv[i];
    }
  }
};

/* ------------------------------------------------------------------------- */
void count_all()
{
  for (size_t i=0; i<num_cpus; ++i) {
    frequency[i].resize(my_eval->dimension());
    frequency[i] = 0.0;
    total[i] = 0;
    count_by_record[i].resize(my_eval->dimension());
    count_by_record[i] = 0.0;
  }
  init_progress_count();
  std::vector<std::future<void>> threads;
  for (size_t i=0; i<num_cpus; ++i) {
    threads.push_back(std::async(std::launch::async, ProcessPVCount(i)));
  }
  for (size_t i=0; i<num_cpus; ++i) {
    threads[i].get();
    if (i == 0)
      continue;
    frequency[0] += frequency[i];
    total[0] += total[i];
    count_by_record[0] += count_by_record[i];
  }
  merge_progress_count();
  count.resize(frequency[0].size());
  copy(&frequency[0][0], &frequency[0][0]+frequency[0].size(), count.begin());
  frequency[0] /= total[0];
  std::cerr << "  frequency max " << frequency[0].max() << " ave " << frequency[0].sum()/frequency[0].size()
	    << " #instance " << total[0] << "\n";
  int ignored_by_record_count = 0;
  for (size_t i=0; i<frequency[0].size(); ++i) {
    frequency[0][i] = std::min(0.5, frequency[0][i]);
    if (count_by_record[0][i] < min_frequency_record && count[i] >= min_frequency) {
      ++ignored_by_record_count;
      count[i] = 0;
    }
    if (count[i] < min_frequency)
      frequency[0][i] = 0;
  }
  std::cerr << "  ignored_by_record_count " << ignored_by_record_count << "\n";
  variance.resize(my_eval->dimension());
  variance = frequency[0];
  variance *= (1.0 - frequency[0]);
  reduce_scale.resize(variance.size());
  reduce_scale = variance;
  reduce_scale /= sqrt(L1Ball::l2norm(reduce_scale));
  double scale = (1.0 - 1.0/4)/reduce_scale.max();
  reduce_scale *= scale;
  reduce_scale = 1.0 - reduce_scale;
}

/* ------------------------------------------------------------------------- */
      
static double eta = 1e-9;
void learn(valarray_t& w)
{
  valarray_t old_w = w;
  std::unique_ptr<L1BallBase> solver;
  if (smd)
    solver.reset(new GradientDescentSMD);
  else {
    GradientDescent *l = new GradientDescent;
    if (fix_step_size)
      l->setFixStepSize(true);
    solver.reset(l);
  }
  solver->setMeanAbsWeight(lambda);
  solver->setTolerance(1e-8);
  solver->setEta0(eta);
  solver->setEtaIncrement(eta_increment);
  solver->setMaxIteration(num_iteration_internal);
  solver->solve(w);
  eta = L1Ball::l1norm(solver->lastEta())/solver->lastEta().size()/2.0;
  std::cerr << " finished " << solver->lastError() << " " << eta
	    << "(" << solver->lastEta().max() << ")" << "\n";
  old_w -= w;
  std::cerr << "changed " << old_w.min() << ' ' << old_w.max() << "\n";
}

struct AggregateStream
{
  std::ostream& os0, &os1;
  AggregateStream(std::ostream& i0, std::ostream& i1) : os0(i0), os1(i1)
  {
  }
  template <class T> AggregateStream& operator<<(const T& t) 
  {
    os0 << t;
    os1 << t;
    return *this;
  }
  void flush()
  {
    os0 << std::flush;
    os1 << std::flush;
  }
};

double sigmoid_alpha()
{
  return 3.0/my_eval->pawnValue();
}

std::tuple<double,double,double> run_cross_validation(int cross_start, int cross_num, int max_progress, bool allow_skip)
{
  const KisenAnalyzer::OtherConfig config = 
    {
      search_window_for_validation, sigmoid_alpha(),
      max_progress, position_randomness,
      min_rating, my_eval.get()
    };
  std::vector<std::future<void>> threads;
      
  KisenAnalyzer::RecordConfig cross_configs[MaxThreads];
  KisenAnalyzer::distributeJob(num_cpus, &cross_configs[0], cross_start, cross_num,
			       kisen_filenames, min_rating,
                               csa_filenames);
  for (size_t j=0; j<num_cpus; ++j) {
    cross_configs[j].allow_skip_in_cross_validation = allow_skip;
    threads.push_back(std::async(std::launch::async, Validator(cross_configs[j], config, results+j)));
  }
  stat::Average werror, toprated, toprated_strict;
  for (size_t j=0; j<num_cpus; ++j) {
    threads[j].get();
    werror.merge(results[j].werrors);
    toprated.merge(results[j].toprated);
    toprated_strict.merge(results[j].toprated_strict);
  }
  return std::make_tuple(werror.average(), toprated.average(), toprated_strict.average());
}

void analyze(const std::string& output_prefix, 
	     size_t kisen_start, size_t num_records,
	     const std::string& initial_file,
	     int cross_start, 
	     int max_progress,
	     bool high_rating_only)
{
  my_eval.reset(gpsshogi::EvalFactory::newEval(eval_type));
  if (my_eval == NULL) {
    std::cerr << "unknown eval type " << eval_type << "\n";
    throw std::runtime_error("unknown eval type");
  }
  std::cerr << "dimension " << my_eval->dimension() << " max-active " << my_eval->maxActive() << " pawn value " << my_eval->pawnValue() << "\n";
  if (!initial_file.empty()) {
    std::cerr << "loading " << initial_file << "\n";
    bool ok = my_eval->load(initial_file.c_str());
    if (! ok)
      std::cerr << "load failed\n";
  }

  std::string log_filename = output_prefix + eval_type + "-learn-log.txt";
  std::ofstream olog(log_filename.c_str());
  AggregateStream os(std::cerr, olog);

  pv_base = output_prefix + "pv";

  valarray_t w(my_eval->dimension()), old_w(my_eval->dimension()), scaled_w(my_eval->dimension());
  my_eval->saveWeight(&old_w[0]);
  if (scaling_factor) 
  {
    w = old_w;
    w *= 1.0/scaling_factor;
    double mean_abs_value = L1Ball::l1norm(w)/w.size();
    std::cerr << "initial w max " << w.max() << " min " << w.min() << " mean abs value " << mean_abs_value << "\n";
    if (mean_abs_value > lambda) {
      w *= lambda/mean_abs_value*127/128;
      std::cerr << "adjusted w max " << w.max() << " min " << w.min() << " mean abs value " << L1Ball::l1norm(w)/w.size() << "\n";
    }
  }
  else
    scaling_factor = 1024;
  for (size_t i=0; i<num_iteration+1; ++i) {
    os << "iteration " << i << "\n";
    time_t now = time(0);
    os << " weight analyses for watch " << ctime(&now);
    {
      if (watch_threshold_seed >= 0)
	watch_threshold = watch_threshold_seed;
      else 
      {
	valarray_t tmp(my_eval->dimension());
	my_eval->saveWeight(&tmp[0]);
	for (size_t i = 0; i < tmp.size(); ++i)
	{
	  tmp[i] = std::abs(tmp[i]);
	}
	if (my_eval->lambdaStart() < tmp.size())
	{
	  std::sort(&tmp[my_eval->lambdaStart()], &tmp[0]+tmp.size());
	  const int k = std::min(
	    (int)(my_eval->dimension() - my_eval->lambdaStart()),
	    -watch_threshold_seed + 1);
	  watch_threshold = tmp[my_eval->dimension()-k];
	}
	else
	{
	  watch_threshold = 1 << 20;
	}
      }
      watch_list.clear();
      for (size_t i=my_eval->lambdaStart(); i<my_eval->dimension(); ++i) {
	if (abs(my_eval->flatValue(i)) > watch_threshold) {
	  os << " " << std::setw(6) << i << " " << std::setw(3) << my_eval->flatValue(i);
	  std::tuple<std::string, int, int> name_index_dim = my_eval->findFeature(i);
	  os << " " << std::setw(14) << std::get<0>(name_index_dim)
	     << " "  << std::setw(6) << std::get<1>(name_index_dim)
	     << " / " << std::setw(6) << std::get<2>(name_index_dim) << "  ";
	  os << my_eval->describe(std::get<0>(name_index_dim), std::get<1>(name_index_dim))
	     << "\n";
	  watch_list.push_back(i);
	}
      }
    }
    os << " cross validation " << ctime(&now);
    std::tuple<double, double, double> error
      = run_cross_validation(cross_start, num_records, max_progress, i == 0 || ! rotate_record);
    os << "\n  mean errors in search " << std::get<0>(error) 
       << " top " << std::get<1>(error)*100.0
       << "% (" << std::get<2>(error)*100.0 << "%)" << "\n";
    os.flush();

    if (second_cross_start >= 0) {
      assert(! rotate_record);
      time_t now = time(0);
      os << " cross validation (2) " << ctime(&now);
      std::tuple<double, double, double> error
	= run_cross_validation(second_cross_start, num_records, max_progress, true);
      os << "\n  mean errors in search " << std::get<0>(error) 
	 << " top " << std::get<1>(error)*100.0
	 << "% (" << std::get<2>(error)*100.0 << "%)" << "\n";
      os.flush();
    }

    if (i == num_iteration)
      break;

    if (i && rotate_record) {
      now = time(0);
      os << " rotate record " << ctime(&now);
      std::vector<std::tuple<int,double> > all;
      for (size_t j=0; j<num_cpus; ++j)
	all.insert(all.end(), results[j].all_errors.begin(), results[j].all_errors.end());
      rotate_record->rotate(all, std::get<0>(error));
    }

    now = time(0);
    os << " start search " << ctime(&now);
    time_point timer = clock::now();
    double total_node_count = 0.0, skip_by_rating = 0;
    {
      double consumed = ::clock();
      const KisenAnalyzer::OtherConfig config = 
	{
	  search_window, sigmoid_alpha(),
	  max_progress, (int)(position_randomness+osl::time_seeded_random()),
	  min_rating, my_eval.get()
	};
      std::vector<std::future<void>> threads;
      KisenAnalyzer::distributeJob(num_cpus, &record_configs[0], kisen_start, num_records,
				   kisen_filenames, min_rating, csa_filenames);
      for (size_t j=0; j<num_cpus; ++j) {
	threads.push_back(std::async(std::launch::async,
				     PVGenerator(pv_base, 
						 record_configs[j], config, results+j)));
      }
      stat::Average werror, toprated, toprated_strict;
      for (size_t j=0; j<num_cpus; ++j) {
	threads[j].get();
	werror.merge(results[j].werrors);
	toprated.merge(results[j].toprated);
	toprated_strict.merge(results[j].toprated_strict);
	total_node_count += results[j].node_count;
	skip_by_rating += results[j].skip_by_rating;
      }
      consumed = ::clock() - consumed;
      std::cerr << "\n  " << total_node_count << " nodes " << elapsedSeconds(timer) << " sec "
		<< total_node_count / elapsedSeconds(timer) << " nodes/sec\n";
      std::cerr << "  cpu time " << consumed / CLOCKS_PER_SEC 
		<< "  skip by rating " << skip_by_rating << "\n";
      os << "\n  mean errors in search (training data) " << werror.average()
	 << " top " << toprated.average()*100.0
	 << "% (" << toprated_strict.average()*100.0 << "%)" << "\n";
      os.flush();
    }
    
    now = time(0);
    os << " count " << ctime(&now);
    count_all();
    {
      // write progress histogram
      std::ostringstream ss;
      ss << output_prefix << eval_type + "-progress-histogram-" << i << ".txt";
      write_progress_count(ss.str());
    }
    if (! watch_list.empty()) {
      os << " watch list\n";
      for (size_t i=0; i<watch_list.size(); ++i) {
	const size_t index = watch_list[i];
	os << "  " << index << " "
	   << std::setw(14) << std::get<0>(my_eval->findFeature(index)) 
	   << " count " << std::setw(5) << count[index]
	   << " record " << std::setw(5) << count_by_record[0][index] << "\n";
      }
    }
    if (clear_infrequent) {
      size_t cleared = 0;
      for (size_t i=0; i<count.size(); ++i) {
	if (count[i] < min_frequency && w[i]) {
	  ++cleared;
	  w[i] = 0;
	}
      }
      os << "  cleared " << cleared << "\n";
    }
    if (clear_infrequent_by_record) {
      size_t cleared = 0;
      for (size_t i=0; i<count.size(); ++i) {
	if (count_by_record[0][i] < min_frequency_record && w[i]) {
	  ++cleared;
	  w[i] = 0;
	}
      }
      os << "  cleared " << cleared << "\n";
    }
    now = time(0);
    os << " start update " << ctime(&now);
    double consumed = ::clock();
    learn(w);
    consumed = ::clock() - consumed;
    std::cerr << "  cpu time " << consumed / CLOCKS_PER_SEC << "\n";

    my_eval->setWeight(&w[0]);
    my_eval->showSummary(std::cerr);
    my_eval->showSummary(olog);
    {
      // write text
      std::ostringstream ss;
      ss << output_prefix << eval_type + "-learn-" << i << ".txt";
      std::string text_name = ss.str();
      my_eval->save(text_name.c_str());
      // write binary
      FILE *fp = fopen(text_name.c_str(), "r");
      if (fp) {
	std::ostringstream ss;
	ss << output_prefix << eval_type + "-learn-" << i << ".bin";
	std::ofstream os(ss.str().c_str());
	std::vector<int> data(my_eval->dimension());
	for (int& d: data)
	  fscanf(fp, "%d", &d);
	fclose(fp);
	osl::misc::BinaryWriter::write(os, data);
      }
    }
    my_eval->saveWeight(&scaled_w[0]);	// after scaling
    if (! watch_list.empty()) {
      os << " watch list\n";
      for (size_t i=0; i<watch_list.size(); ++i) {
	const size_t index = watch_list[i];
	const bool warn = abs(scaled_w[index]) > abs(old_w[index]);
	os << " !"[warn] << " " << index << " " << std::get<0>(my_eval->findFeature(index)) 
	   << " " << old_w[index] << " => " << scaled_w[index] << "\n";
      }
    }
    old_w -= scaled_w;
    old_w = abs(old_w);
    os << "weight change: max " << old_w.max() << " ave. " << old_w.sum()/old_w.size()
       << "\n";
    old_w = scaled_w;
    os << "aggressive moves " << aggressive_penalty_moves[0] << "\n";
    
    size_t target = PAWN;
    while (target < scaled_w.size() && (scaled_w[target] == 0 || w[target] == 0))
      ++target;
    if (target < scaled_w.size())
      scaling_factor = scaled_w[target] / w[target];
    else
      scaling_factor = 128;
    const int num_non_zero = w.size() - std::count(&w[0], &w[0]+w.size(), 0);
    os << "scaling factor " << scaling_factor << "  ";
    os << "#non-zero weights " << num_non_zero << "  ";
    os << "mean abs weights/pawn " << L1Ball::l1norm(scaled_w)/num_non_zero/scaled_w[target]
       << "\n";
    os.flush();
  }
  
  time_t now = time(0);
  os << ctime(&now);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
