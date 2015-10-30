#include "loss.h"
#include "eval/eval.h"
#include "eval/evalFactory.h"
#include "eval/progress.h"
#include "rand012.h"
#include "l1Ball.h"
#include "analyzer.h"
#include "pvFile.h"
#include "pvVector.h"
#include "pvGenerator.h"
#include "rotateRecord.h"
#include "osl/record/kisen.h"
#include "osl/numEffectState.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"
#include "osl/misc/milliSeconds.h"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/program_options.hpp>
#include <thread>
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

void learn_main(const std::string& output_prefix, 
		size_t start_id, size_t num_records,
		int cross_start,
		int max_progress);
void analyze_gradient(const std::string& output_prefix,
		      size_t kisen_start, size_t num_records);
void analyze_loss(const std::string& output_prefix,
		  size_t kisen_start, size_t num_records,
		  const std::string& gradient_filename,
		  size_t num_records_loss_analyses);

std::unique_ptr<gpsshogi::Eval> my_eval;
std::string eval_type;
size_t num_iteration, num_iteration_internal;
double lambda = 100;
size_t min_rating = 0;
double search_window_abs, search_window_for_validation_abs;
std::vector<std::string> kisen_filenames;
double scaling_factor, sigmoid_scaling_pawn;
size_t num_cpus;
std::string pv_base;
int watch_threshold, watch_threshold_seed;

std::unique_ptr<RotateRecord> rotate_record;

int main(int argc, char **argv)
{
  nice(20);
  
  size_t kisen_start, num_records, num_records_loss_analyses;
  std::string output_prefix;
  std::string initial_value, loss_analyses;
  po::options_description options("all_options");
  int cross_start;
  int max_progress=16;
  int search_depth, quiesce_depth;
  bool analyze_pv;

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
    ("eval,e",
     po::value<std::string>(&eval_type)->default_value(std::string("piece")),
     "evaluation function (piece, rich0, rich1)")
    ("sigmoid-scaling-pawn",
     po::value<double>(&sigmoid_scaling_pawn)->default_value(3),
     "scaling factor for sigmoid loss function, relative to pawn value")
    ("window-abs",
     po::value<double>(&search_window_abs)->default_value(256),
     "absolute search window")
    ("vwindow-abs",
     po::value<double>(&search_window_for_validation_abs)->default_value(256),
     "absolute search window for validation")
    ("initial-value-file,f",
     po::value<std::string>(&initial_value)->default_value(""),
     "File with initial eval values")
    ("cross-validation-start",
     po::value<int>(&cross_start)->default_value(200000),
     "Start ID of record in kisen file to do cross validation")
    ("watch-threshold",
     po::value<int>(&watch_threshold_seed)->default_value(-10),
     "report details about features whose absolute value is greater than this, or negative value for top-k values")
    ("search-depth",
     po::value<int>(&search_depth)->default_value(0),
     "adjust normal search depth")
    ("quiesce-depth",
     po::value<int>(&quiesce_depth)->default_value(-1),
     "adjust quiescence depth")
    ("analyze-pv",
     po::value<bool>(&analyze_pv)->default_value(0),
     "analyze existing pv")
    ("loss-analyses",
     po::value<std::string>(&loss_analyses)->default_value(""),
     "analyze loss in existing pv, given filename of gradient as argument")
    ("num-records-loss-analyses",
     po::value<size_t>(&num_records_loss_analyses)->default_value(0),
     "number of records to be analyzed in --loss-analyses (all if 0)")
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
  if (vm.count("help")) {
    std::cerr << options << std::endl;
    return 0;
  }
  if (num_cpus == 0 || (int)num_cpus > std::thread::hardware_concurrency()) {
    std::cerr << "do you really have so many cpus? " << num_cpus << "\n";
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
  PVGenerator::setNormalDepth(search_depth);
  PVGenerator::setQuiesceDepth(quiesce_depth);
  my_eval.reset(gpsshogi::EvalFactory::newEval(eval_type));
  if (my_eval == NULL) {
    std::cerr << "unknown eval type " << eval_type << "\n";
    throw std::runtime_error("unknown eval type");
  }
  std::cerr << "dimension " << my_eval->dimension() << " max-active " << my_eval->maxActive() << " pawn value " << my_eval->pawnValue() << "\n";
  if (!initial_value.empty()) {
    std::cerr << "loading " << initial_value << "\n";
    bool ok = my_eval->load(initial_value.c_str());
    if (! ok)
      std::cerr << "load failed\n";
  }
  pv_base = output_prefix + "pv";
  if (analyze_pv)
    analyze_gradient(output_prefix, kisen_start, num_records);
  else if (loss_analyses != "")
    analyze_loss(output_prefix, kisen_start, num_records, loss_analyses,
		 num_records_loss_analyses);
  else
    learn_main(output_prefix, kisen_start, num_records,
	       cross_start, max_progress);
}

const int MaxThreads = 64;
static PVGenerator::Result results[MaxThreads];
static KisenAnalyzer::RecordConfig record_configs[MaxThreads];

static int position_randomness = 1;

valarray_t gradient[MaxThreads], Hv[MaxThreads], v_for_Hv;
stat::Average errors[MaxThreads];
/** feature の出現率 */
valarray_t frequency[MaxThreads];
/** feature の出現回数 */
std::vector<size_t> count;
/** feature の出現棋譜数. [0] を合計に代用 */
valarray_t count_by_record[MaxThreads];
long long total[MaxThreads];
/** 監視対象の重み */
std::vector<int> watch_list;

struct ProcessPVBase
{
  int thread_id, min_count;
  ProcessPVBase(int tid, int count) 
    : thread_id(tid), min_count(count)
  {
  }
  virtual ~ProcessPVBase() {}
  void operator()() const;
  virtual void beginPosition(double turn_coef, MoveData& selected) const {}
  virtual void processInstance(double turn_coef, MoveData& selected, MoveData& sibling) const=0;
  virtual void endPosition() const {}
  virtual void finishRecord() const {}
};

void ProcessPVBase::operator()() const
{
  std::string filename = PVGenerator::pv_file(pv_base, thread_id);

  int num_pv = 0, num_position = 0;
  const std::string kisen_filename = record_configs[thread_id].kisen_filename;
  KisenFile kisen_file(kisen_filename.c_str());

  PVFileReader pr(filename.c_str());
  int record, position;
  PVVector pv_best, pv;

  int cur_record=-1, cur_position=-1;
  NumEffectState state(kisen_file.initialState());
  std::vector<Move> moves;

  while (pr.newPosition(record, position)) {
    ++num_position;
    if (record != cur_record) {
      cur_record = record;
      moves = kisen_file.moves(cur_record);
      finishRecord();
      cur_position = 0;
      state = NumEffectState(kisen_file.initialState());
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
    MoveData selected, sibling;
    Analyzer::analyzeLeaf(state, pv, *my_eval, selected, true);
    const int turn_coef = (state.turn() == BLACK) ? 1 : -1;
    beginPosition(turn_coef, selected);
    int num_sibling = 0;
    while (pr.readPv(pv)) {
      ++num_pv; ++num_sibling;
      Analyzer::analyzeLeaf(state, pv, *my_eval, sibling, true);
      processInstance(turn_coef, selected, sibling);
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
    std::sort(selected.diffs.begin(), selected.diffs.end());
  }
  void processInstance(double, MoveData&, MoveData&) const;
  void finishRecord() const;
};

void ProcessPVCount::processInstance(double turn_coef, MoveData& selected, MoveData& sibling) const
{
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
double sigmoid_alpha()
{
  return 1.0/(search_window_abs/7.0);
}

struct ProcessPV : public ProcessPVBase
{
  std::shared_ptr<valarray_t> w;
  mutable double agsum, adot;
  mutable MoveData selected;
  ProcessPV(int tid, const valarray_t& iw) 
    : ProcessPVBase(tid, 0), w(new valarray_t(iw))
  {
    *w *= sigmoid_alpha(); // iw[PAWN]*sigmoid_scaling_pawn; //  
  }
  void beginPosition(double turn_coef, MoveData& selected) const;
  void processInstance(double, MoveData&, MoveData&) const;
  void endPosition() const;
};

void ProcessPV::beginPosition(double turn_coef, MoveData& selected) const { 
  agsum = 0; 
  this->selected = selected;
  adot = ValarrayUtil::dot(this->selected.diffs, *w, count, min_count);
}

void ProcessPV::endPosition() const
{
  SigmoidLoss::addGradientSep(selected.diffs, count, min_count, agsum, gradient[thread_id]);
}

void ProcessPV::processInstance(double turn_coef, MoveData& selected, MoveData& sibling) const
{
  double loss = 0;
  loss = SigmoidLoss::addGradientSep(*w, adot, this->selected.diffs, sibling.diffs, turn_coef, count, min_count,
				       gradient[thread_id], agsum);
  errors[thread_id].add(loss);
}

/* ------------------------------------------------------------------------- */
struct LossOfPV
{
  int thread_id, limit_records;
  LossOfPV(int tid, size_t limit) : thread_id(tid), limit_records(limit)
  {
  }
  void operator()() const;
};

void LossOfPV::operator()() const
{
  std::string filename = PVGenerator::pv_file(pv_base, thread_id);

  int num_pv = 0, num_position = 0;
  const std::string kisen_filename = record_configs[thread_id].kisen_filename;
  KisenFile kisen_file(kisen_filename.c_str());

  PVFileReader pr(filename.c_str());
  int record, position;
  PVVector pv_best, pv;

  int cur_record=-1, cur_position=-1, record_done=0;
  NumEffectState state(kisen_file.initialState());
  std::vector<Move> moves;

  while (pr.newPosition(record, position)) {
    ++num_position;
    if (record != cur_record) {
      cur_record = record;
      moves = kisen_file.moves(cur_record);
      cur_position = 0;
      state = NumEffectState(kisen_file.initialState());
      ++record_done;
      if (limit_records > 0 && record_done >= limit_records)
	break;
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
    int selected_value = Analyzer::leafValue(state, pv, *my_eval);
    const int turn_coef = (state.turn() == BLACK) ? 1 : -1;
    int num_sibling = 0;
    while (pr.readPv(pv)) {
      ++num_pv; ++num_sibling;
      int value = Analyzer::leafValue(state, pv, *my_eval);
      double loss = SigmoidUtil::alphax((value - selected_value)*turn_coef,
					sigmoid_alpha());
      errors[thread_id].add(loss);
    }
  }
}
/* ------------------------------------------------------------------------- */
class GradientDescent : public gpsshogi::Rand012
{
  double prev_error, min_error;
  int prev_i;
public:
  GradientDescent()
  {
    prev_error = min_error = 1e8;
    prev_i = -1;
  }
  void iterationHead(int i, const valarray_t& w, double error)
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
    if (i % 50 == 49)
      std::cerr << std::endl << ' ';
    if (i % 50 == 49) {
      for (int i=0; i<std::min(16, (int)w.size()); ++i)
	std::cerr << "  " << w[i];
      std::cerr << "\n";
    }
  }
  void makeGradient(const valarray_t& w, valarray_t& out, double& error)
  {
    for (size_t i=0; i<num_cpus; ++i) {
      gradient[i].resize(w.size());
      gradient[i] = 0.0;
      errors[i].clear();
    }
    boost::ptr_vector<std::thread> threads;
    for (size_t i=0; i<num_cpus; ++i)
      threads.push_back(new std::thread(ProcessPV(i, w)));
    for (size_t i=0; i<num_cpus; ++i)
      threads[i].join();

    out.resize(w.size());
    out = 0.0;
    for (size_t i=0; i<num_cpus; ++i) {
      out += gradient[i];
      if (i)
	errors[0].merge(errors[i]);
    }
    error = errors[0].average();
  }
  double computeLoss(const valarray_t& w, size_t limit_records)
  {
    for (size_t i=0; i<num_cpus; ++i) {
      gradient[i].resize(w.size());
      gradient[i] = 0.0;
      errors[i].clear();
    }
    boost::ptr_vector<std::thread> threads;
    for (size_t i=0; i<num_cpus; ++i)
      threads.push_back(new std::thread(LossOfPV(i, limit_records/num_cpus)));
    for (size_t i=0; i<num_cpus; ++i)
      threads[i].join();

    for (size_t i=0; i<num_cpus; ++i) {
      if (i)
	errors[0].merge(errors[i]);
    }
    return errors[0].average();
  }
  double lastError() const { return prev_error; }
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
  boost::ptr_vector<std::thread> threads;
  for (size_t i=0; i<num_cpus; ++i)
    threads.push_back(new std::thread(ProcessPVCount(i)));
  for (size_t i=0; i<num_cpus; ++i) {
    threads[i].join();
    if (i == 0)
      continue;
    frequency[0] += frequency[i];
    total[0] += total[i];
    count_by_record[0] += count_by_record[i];
  }
  count.resize(frequency[0].size());
  copy(&frequency[0][0], &frequency[0][0]+frequency[0].size(), count.begin());
  frequency[0] /= total[0];
  std::cerr << "  frequency max " << frequency[0].max() << " ave " << frequency[0].sum()/frequency[0].size()
	    << " #instance " << total[0] << "\n";
  int ignored_by_record_count = 0;
  for (size_t i=0; i<frequency[0].size(); ++i) {
    frequency[0][i] = std::min(0.5, frequency[0][i]);
  }
  std::cerr << "  ignored_by_record_count " << ignored_by_record_count << "\n";
}

/* ------------------------------------------------------------------------- */
      
void learn(valarray_t& w)
{
  GradientDescent solver;

  solver.setMaxIteration(num_iteration_internal);
  solver.solve(w);

  std::cerr << " finished " << solver.lastError() << "\n";
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

std::tuple<double,double,double> run_cross_validation(int cross_start, int cross_num, int max_progress, bool allow_skip)
{
  const KisenAnalyzer::OtherConfig config = 
    {
      search_window_for_validation_abs / my_eval->pawnValue(), sigmoid_alpha(),
      max_progress, position_randomness,
      min_rating, my_eval.get()
    };
  boost::ptr_vector<std::thread> threads;
      
  std::vector<std::string> csa_files;
  KisenAnalyzer::RecordConfig cross_configs[MaxThreads];
  KisenAnalyzer::distributeJob(num_cpus, &cross_configs[0], cross_start, cross_num,
			       kisen_filenames, min_rating, csa_files);
  for (size_t j=0; j<num_cpus; ++j) {
    cross_configs[j].allow_skip_in_cross_validation = allow_skip;
    threads.push_back(new std::thread(Validator(cross_configs[j], config, results+j)));
  }
  stat::Average werror, toprated, toprated_strict;
  size_t total_node_count = 0, siblings = 0;
  for (size_t j=0; j<num_cpus; ++j) {
    threads[j].join();
    werror.merge(results[j].werrors);
    toprated.merge(results[j].toprated);
    toprated_strict.merge(results[j].toprated_strict);
    total_node_count += results[j].node_count;
    siblings += results[j].siblings;
  }
  std::cerr << "positions " << werror.numElements()
	    << " siblings " << siblings
	    << " node count " << total_node_count << "\n";
  return std::make_tuple(werror.average(), toprated.average(), toprated_strict.average());
}

void analyze_gradient(const std::string& output_prefix,
		      size_t kisen_start, size_t num_records)
{
  std::vector<std::string> csa_files;
  KisenAnalyzer::distributeJob(num_cpus, &record_configs[0], kisen_start, num_records,
			       kisen_filenames, min_rating, csa_files);

  valarray_t w(my_eval->dimension()), gradient(w.size());
  my_eval->saveWeight(&w[0]);
  GradientDescent solver;
  double error = 0;
  time_t now = time(0);
  std::cerr << "make gradient " << ctime(&now);
  solver.makeGradient(w, gradient, error);
  now = time(0);
  std::cerr << "error = " << error << " writing gradient" << ctime(&now);
  {
    std::string log_filename = output_prefix + eval_type + "-rand012-analysis.txt";
    std::ofstream os(log_filename.c_str());
    os << "loss " << std::setprecision(10) << error << "\n";
  }
  {
    std::string gradient_filename = output_prefix + eval_type + "-rand012-gradient.txt";
    std::ofstream os(gradient_filename.c_str());
    os << std::setprecision(10);
    for (size_t i=0; i<gradient.size(); ++i)
      os << gradient[i] << "\n";
  }  
}

void analyze_loss(const std::string& output_prefix,
		  size_t kisen_start, size_t num_records,
		  const std::string& gradient_filename,
		  size_t num_records_loss_analyses)
{
  std::vector<std::string> csa_files;
  KisenAnalyzer::distributeJob(num_cpus, &record_configs[0], kisen_start, num_records,
			       kisen_filenames, min_rating, csa_files);

  valarray_t w(my_eval->dimension()), gradient(w.size()), stable_w(w.size());
  my_eval->saveWeight(&w[0]);
  {
    std::ifstream is(gradient_filename.c_str());
    for (size_t i=0; i<gradient.size(); ++i)
      is >> gradient[i];
    if (!is)
      throw std::runtime_error("read error "+gradient_filename);
  }
  {
    std::unique_ptr<Eval> eval(EvalFactory::newEval(eval_type));
    eval->saveWeight(&stable_w[0]);
  }
  
  GradientDescent solver;
  time_t now = time(0);
  std::cerr << "comute loss " << ctime(&now);
  double loss = solver.computeLoss(w, num_records_loss_analyses);
  now = time(0);
  std::cerr << "error =" << loss << ctime(&now);
  std::cout << "0 0 0 0 " << std::setprecision(10) << loss << "\n" << std::flush;

  valarray_t modified = w;
  for (int i=0; i<2048; ++i) {
    int id = osl::random() % w.size();
    while ((w[id] == 0 && stable_w[id] == 0) || gradient[id] == 0)
      id = osl::random() % w.size();
    for (int d=1; d<=8; d*=2) {
      modified[id] = w[id] + d*((gradient[id]>0) ? -1 : 1);
      my_eval->setWeightScale(&modified[0], 1.0);
      
      double loss = solver.computeLoss(modified, num_records_loss_analyses);
      std::cout << id << ' ' << d << ' ' << gradient[id] << ' ' << w[id]
		<< ' ' << loss << "\n" << std::flush;
    }
    modified[id] = w[id];
  }
  my_eval->setWeightScale(&w[0], 1.0);
}

void learn_main(const std::string& output_prefix, 
		size_t kisen_start, size_t num_records,
		int cross_start, 
		int max_progress)
{
  std::string log_filename = output_prefix + eval_type + "-rand012-log.txt";
  std::ofstream olog(log_filename.c_str());
  AggregateStream os(std::cerr, olog);
 
  valarray_t w(my_eval->dimension()), old_w(my_eval->dimension()), scaled_w(my_eval->dimension());
  my_eval->saveWeight(&old_w[0]);
  w = old_w;
  for (size_t i=0; i<num_iteration+1; ++i) {
    os << "iteration " << i << "\n";
    time_t now = time(0);
    os << " weight analyses for watch " << ctime(&now);
    if (eval_type != "progresseval")
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
	  os << "  " << i << " " << my_eval->flatValue(i);
	  std::tuple<std::string, int, int> name_index_dim = my_eval->findFeature(i);
	  os << "  " << std::get<0>(name_index_dim) << " " << std::get<1>(name_index_dim)
	     << " / " << std::get<2>(name_index_dim)<< "\n";
	  watch_list.push_back(i);
	}
      }
    }
    os << " cross validation " << ctime(&now);
    std::tuple<double, double, double> error
      = run_cross_validation(cross_start, num_records, max_progress, false);
    os << "\n  mean errors in search " << std::get<0>(error) 
       << " top " << std::get<1>(error)*100.0
       << "% (" << std::get<2>(error)*100.0 << "%)" << "\n";
    os.flush();

    if (i == num_iteration && i > 0)
      break;

    now = time(0);
    os << " start search " << ctime(&now);
    time_point timer = clock::now();
    unsigned long long total_node_count = 0.0, skip_by_rating = 0, siblings = 0;
    {
      double consumed = ::clock();
      const KisenAnalyzer::OtherConfig config = 
	{
	  search_window_abs / my_eval->pawnValue(), sigmoid_alpha(),
	  max_progress, position_randomness+osl::random(),
	  min_rating, my_eval.get()
	};
      std::vector<std::string> csa_files;
      boost::ptr_vector<std::thread> threads;
      KisenAnalyzer::distributeJob(num_cpus, &record_configs[0], kisen_start, num_records,
				   kisen_filenames, min_rating, csa_files);
      for (size_t j=0; j<num_cpus; ++j) {
	threads.push_back(new std::thread(PVGenerator(pv_base, 
							record_configs[j], config, results+j)));
      }
      stat::Average werror, toprated, toprated_strict;
      for (size_t j=0; j<num_cpus; ++j) {
	threads[j].join();
	werror.merge(results[j].werrors);
	toprated.merge(results[j].toprated);
	toprated_strict.merge(results[j].toprated_strict);
	total_node_count += results[j].node_count;
	skip_by_rating += results[j].skip_by_rating;
	siblings += results[j].siblings;
      }
      consumed = ::clock() - consumed;
      std::cerr << "\n  " << total_node_count << " nodes " << elapsedSeconds(timer) << " sec "
		<< total_node_count / elapsedSeconds(timer) << " nodes/sec\n";
      std::cerr << "  cpu time " << consumed / CLOCKS_PER_SEC 
		<< "  skip by rating " << skip_by_rating << "\n";
      os << "\n  mean errors in search (training data) " << werror.average()
	 << " top " << toprated.average()*100.0
	 << "% (" << toprated_strict.average()*100.0 << "%)"
	 << " positions " << werror.numElements()
	 << " siblings " << siblings
	 << "\n";
      os.flush();
    }
    if (i == num_iteration)
      break;
    
    now = time(0);
    os << " count " << ctime(&now);
    count_all();
    if (! watch_list.empty()) {
      os << " watch list\n";
      for (size_t i=0; i<watch_list.size(); ++i) {
	const size_t index = watch_list[i];
	os << "  " << index << " " << std::get<0>(my_eval->findFeature(index)) 
	   << " count " << count[index] << " record " << count_by_record[0][index] << "\n";
      }
    }
    now = time(0);
    os << " start rand012 " << ctime(&now);
    double consumed = ::clock();
    learn(w);
    consumed = ::clock() - consumed;
    std::cerr << "  cpu time " << consumed / CLOCKS_PER_SEC << "\n";

    my_eval->setWeightScale(&w[0], 1.0);
    my_eval->showSummary(std::cerr);
    my_eval->showSummary(olog);
    {
      std::ostringstream ss;
      ss << output_prefix << eval_type + "-rand012-" << i << ".txt";
      my_eval->save(ss.str().c_str());
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
