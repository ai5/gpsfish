#include "eval/eval.h"
#include "eval/evalFactory.h"
#include "eval/progress.h"
#include "loss.h"
#include "l1Ball.h"
#include "analyzer.h"
#include "pvFile.h"
#include "pvGenerator.h"
#include "osl/rating/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "osl/rating/group.h"
#include "osl/rating/group/captureGroup.h"
#include "osl/rating/group.h"
#include "osl/record/kisen.h"
#include "osl/numEffectState.h"
#include "osl/misc/milliSeconds.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress/effect5x3.h"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/program_options.hpp>
#include <memory>
#include <thread>
#include <vector>
#include <algorithm>
#include <valarray>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
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

std::unique_ptr<osl::rating::FeatureSet> features;
std::string eval_type, loss_function;
size_t num_iteration, num_iteration_internal;
double lambda;
size_t min_rating = 1500;
double search_window, search_window_for_validation;
std::vector<std::string> kisen_filenames;
double scaling_factor, eta_increment;
size_t num_cpus;
bool smd, fix_step_size;
size_t mini_batch_size;
size_t kisen_start, num_records;
int cross_start;

namespace osl 
{
  namespace rating
  {   
    class TestFeatures : public FeatureSet
    {
    public:
      TestFeatures()
      {
	add(new rating::CaptureGroup());
	add(new rating::TakeBackGroup());
	addFinished();
      }
    };
  }
}

int main(int argc, char **argv)
{
  nice(20);
  
  std::string output_prefix;
  std::string initial_value;
  po::options_description options("all_options");
  int max_progress;
  bool high_rating_only, limit_sibling_10;

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
     po::value<std::string>(&output_prefix)->default_value("./tmp"),
     "directory to write files (e.g., tmp/)")
    ("kisen-start",
     po::value<size_t>(&kisen_start)->default_value(0),
     "start id of kisen records")
    ("kisen-file,k", po::value<std::vector<std::string> >(),
     "filename for records to be analyzed")
    ("lambda",
     po::value<double>(&lambda)->default_value(100),
     "lambda for l2 regularization")
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
     po::value<bool>(&high_rating_only)->default_value(true),
      "When true only consider plays where both player have at least "
      "1500 rating value")
    ("cross-validation-start",
     po::value<int>(&cross_start)->default_value(200000),
     "Start ID of record in kisen file to do cross validation")
    ("loss-function,l",
     po::value<std::string>(&loss_function)->default_value(std::string("log")),
     "loss function (log, hinge, exp, sigmoid)")
    ("eta-increment",
     po::value<double>(&eta_increment)->default_value(1.2),
     "scale to modify eta")
    ("smd",
     po::value<bool>(&smd)->default_value(0),
     "enable stochastic meta descent if 1")
    ("mini-batch-size",
     po::value<size_t>(&mini_batch_size)->default_value(0),
     "update weights for every 100.0/mini-batch-size% of instances are processed if mini_batch_size > 0")
    ("limit-greater-sibling10",
     po::value<bool>(&limit_sibling_10)->default_value(0),
     "consider only at most 10 moves that have higher evaluation values than that of recorded move for each position")
    ("fix-step-size",
     po::value<bool>(&fix_step_size)->default_value(0),
     "fix step size (only effective when smd == 0")
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
  if (vm.count("help") || (fix_step_size && smd)) {
    std::cerr << options << std::endl;
    return 0;
  }
  if (num_cpus == 0 || num_cpus > 8) {
    std::cerr << "do you really have so many cpus? " << num_cpus << "\n";
    return 1;
  }    
  if (! high_rating_only)
    min_rating = 0;

  // instantiate
  features.reset(new osl::rating::StandardFeatureSet);
  // features.reset(new rating::TestFeatures);

  analyze(output_prefix, kisen_start, num_records,
	  initial_value, cross_start, 
	  max_progress, high_rating_only);
}

const int MaxThreads = 64;

valarray_t gradient[MaxThreads], Hv[MaxThreads], v_for_Hv;
stat::Average errors[MaxThreads];
stat::Average top_rated[16][MaxThreads];
size_t skip[MaxThreads];
int mini_batch_id = 0;

struct MakeGradient
{
  int thread_id;
  const valarray_t& w;
  bool make_hv;

  MakeGradient(int tid, const valarray_t& iw, bool hv) : thread_id(tid), w(iw), make_hv(hv)
  {
  }
  static void match(const NumEffectState& state, const RatingEnv& env, Move m, std::vector<int>& out) 
  {
    out.clear();
    for (size_t g=0; g<features->groupSize(); ++g) {
      const int found = features->group(g).findMatch(state, m, env);
      if (found < 0) 
	continue;
      out.push_back(found + features->range(g).first);
    }
    std::sort(out.begin(), out.end());
  }
  void operator()() const;
  static void
  makeInstance(const std::vector<int>& selected, const std::vector<int>& other, InstanceData& instance)
  {
    instance.clear();
    instance.y = -1.0;
    auto p = other.begin();
    auto q = selected.begin();

    while (p != other.end() && q != selected.end()) {
      const int index = std::min(*p, *q);
      double diff;		// sibling[i] - selected[i];
      if (*p == *q) {
	diff = 0;
	++p, ++q;
	continue;
      }
      else if (*p < *q) {
	diff = 1;
	++p;
      }
      else {
	assert(*p > *q);
	diff = -1;
	++q;
      }
      instance.index.push_back(index);
      instance.value.push_back(diff);
    }
    while (p != other.end()) {
      const int index = *p;
      const double diff = 1;
      ++p;
      instance.index.push_back(index);
      instance.value.push_back(diff);
    }
    while (q != selected.end()) {
      const int index = *q;
      const double diff = -1;
      ++q;
      instance.index.push_back(index);
      instance.value.push_back(diff);
    }    
  }
};

void MakeGradient::operator()() const
{
  // const std::string kisen_filename = record_configs[thread_id].kisen_filename;
  const std::string kisen_filename = kisen_filenames[0];
  KisenFile kisen_file(kisen_filename.c_str());
  KisenIpxFile ipx(kisen_file.ipxFileName());

  skip[thread_id] = 0;
  const int width = num_records/num_cpus;
  const size_t last = (thread_id+1 == (int)num_cpus) ? num_records : width*(thread_id+1);
  for (size_t i=width*thread_id; i<last; ++i) {

    if (ipx.rating(i, BLACK) < min_rating 
	|| ipx.rating(i, WHITE) < min_rating) {
      ++skip[thread_id];
      continue;
    }

    NumEffectState state(kisen_file.initialState());
    std::vector<Move> moves = kisen_file.moves(i);
    rating::RatingEnv env;
    env.make(state);

    InstanceData instance;
    std::vector<int> best_match, match;
    for (size_t j=0; j+1<moves.size(); ++j) {
      const Move best_move = moves[j];
      MoveVector all_moves;
      state.generateLegal(all_moves);
      if (! all_moves.isMember(best_move))
	continue;
      this->match(state, env, best_move, best_match);
      for (size_t k=0; k<all_moves.size(); ++k) {
	const Move m = all_moves[k];
	if (best_move == m)
	  continue;
	this->match(state, env, m, match);
	makeInstance(best_match, match, instance);
	if (instance.index.empty())
	  continue;
	
	double loss = 0;
	if (loss_function == "sigmoid") {
	  if (make_hv)
	    loss = SigmoidLoss::addGradient(w, instance, gradient[thread_id], v_for_Hv, Hv[thread_id]);
	  else
	    loss = SigmoidLoss::addGradient(w, instance, gradient[thread_id]);
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
	errors[thread_id].add(loss);
      }
      state.makeMove(best_move);
      env.update(state, best_move);
    }
  }
}

/* ------------------------------------------------------------------------- */
class L1BallLogregUtil
{
protected:
  double prev_error, min_error;
  int prev_i;

  L1BallLogregUtil()
  {
    prev_error = min_error = 1e8;
    prev_i = -1;
  }
  void iterationHead(int i, const valarray_t& w, double prev_error);
  void makeGradient(const valarray_t& w, valarray_t& gradient, double& error, bool make_hv);
};

void L1BallLogregUtil::makeGradient(const valarray_t& w, valarray_t& out, double& error, bool make_hv)
{
  for (size_t i=0; i<num_cpus; ++i) {
    gradient[i].resize(w.size());
    gradient[i] = 0.0;
    if (make_hv) {
      Hv[i].resize(w.size());
      Hv[i] = 0.0;
    }
    errors[i].clear();
  }

  boost::ptr_vector<std::thread> threads;
  for (size_t i=0; i<num_cpus; ++i)
    threads.push_back(new std::thread(MakeGradient(i, w, make_hv)));
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

void L1BallLogregUtil::iterationHead(int i, const valarray_t& w, double error)
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
}

class L1BallLogreg : public gpsshogi::L1Ball, L1BallLogregUtil
{
public:
  void iterationHead(int i, const valarray_t& w, double prev_error)
  {
    L1BallLogregUtil::iterationHead(i, w, prev_error);
  }
  void makeGradient(const valarray_t& w, valarray_t& gradient, double& error)
  {
    L1BallLogregUtil::makeGradient(w, gradient, error, false);
  }
};

/* ------------------------------------------------------------------------- */
class L1BallSMDLogreg : public gpsshogi::L1BallSMD, L1BallLogregUtil
{
public:
  void iterationHead(int i, const valarray_t& w, double prev_error)
  {
    L1BallLogregUtil::iterationHead(i, w, prev_error);
  }
  void makeGradient(const valarray_t& w, valarray_t& gradient, double& error,
		    const valarray_t& v, valarray_t& hv_out)
  {
    ++mini_batch_id;
    v_for_Hv.resize(v.size());
    v_for_Hv = v;
    L1BallLogregUtil::makeGradient(w, gradient, error, true);
    hv_out.resize(w.size());
    hv_out = 0.0;
    for (size_t i=0; i<num_cpus; ++i) {
      hv_out += Hv[i];
    }
  }
};

/* ------------------------------------------------------------------------- */

void setWeightToFeatures(const valarray_t& w)
{
  for (size_t i=0; i<w.size(); ++i)
    features->setWeight(i, std::exp(w[i]));
}
      
static double eta = 1e-6;
void learn(valarray_t& w)
{
  std::unique_ptr<L1BallBase> solver;
  if (smd)
    solver.reset(new L1BallSMDLogreg);
  else {
    L1BallLogreg *l = new L1BallLogreg;
    if (fix_step_size)
      l->setFixStepSize(true);
    solver.reset(l);
  }
  solver->setMeanAbsWeight(lambda);
  solver->setTolerance(1e-4);
  solver->setEta0(eta);
  solver->setEtaIncrement(eta_increment);
  solver->setMaxIteration(num_iteration_internal);
  solver->solve(w);
  eta = L1Ball::l1norm(solver->lastEta())/solver->lastEta().size()/2.0;
  std::cerr << " finished " << solver->lastError() << " " << eta
	    << "(" << solver->lastEta().max() << ")" << "\n";
  setWeightToFeatures(w);
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

struct Validation
{
  int thread_id;
  Validation(int t) : thread_id(t)
  {
  }
  void operator()() const
  {
    const std::string kisen_filename = kisen_filenames[0];
    KisenFile kisen_file(kisen_filename.c_str());
    KisenIpxFile ipx(kisen_file.ipxFileName());

    skip[thread_id]=0;
    const int width = num_records/num_cpus;
    const int first = cross_start+width*thread_id;
    const size_t last = (thread_id+1 == (int)num_cpus) ? cross_start+num_records : first+width;
    for (size_t i=first; i<last; ++i) {
      if (ipx.rating(i, BLACK) < min_rating 
	  || ipx.rating(i, WHITE) < min_rating) {
	++skip[thread_id];
	continue;
      }

      NumEffectState state(kisen_file.initialState());
      std::vector<Move> moves = kisen_file.moves(i);
      rating::RatingEnv env;
      env.make(state);
      progress::Effect5x3 progress(state);
      for (size_t j=0; j+1<moves.size(); ++j) {
	if (state.inCheck(alt(state.turn())))
	  break;			// illegal
	const Move best_move = moves[j];

	RatedMoveVector moves;
	features->generateRating(state, env, 2000, moves);
	top_rated[progress.progress16().value()][thread_id].add(! moves.empty() && moves[0].move() == best_move);

	state.makeMove(best_move);
	env.update(state, best_move);
	progress.update(state, best_move);
      }
    }
  }
};


void analyze(const std::string& output_prefix, 
	     size_t kisen_start, size_t num_records,
	     const std::string& initial_file,
	     int cross_start, 
	     int max_progress,
	     bool high_rating_only)
{
  std::string log_filename = output_prefix + eval_type + "-moverating-log.txt";
  std::ofstream olog(log_filename.c_str());
  AggregateStream os(std::cerr, olog);

  const size_t dimension = features->featureSize();
  valarray_t w(dimension), old_w(dimension);
  os << "dimension " << dimension << " max-active " << features->groupSize() << "\n";
  if (!initial_file.empty()) {
    std::cerr << "loading " << initial_file << "\n";
    std::ifstream is(initial_file.c_str());
    for (size_t i=0; i<dimension; ++i)
      is >> w[i];
    if (! is)
      os << "error load failed from " << initial_file << "\n";
    old_w = w;
    setWeightToFeatures(w);
    double mean_abs_value = L1Ball::l1norm(w)/w.size();
    std::cerr << "initial w max " << w.max() << " min " << w.min() << " mean abs value " << mean_abs_value << "\n";
    if (mean_abs_value > lambda) {
      w *= lambda/mean_abs_value*127/128;
      os << "adjusted w max " << w.max() << " min " << w.min() << " mean abs value " << L1Ball::l1norm(w)/w.size() << "\n";
    }
  }

  for (size_t i=0; i<num_iteration+1; ++i) {
    os << "iteration " << i << "\n";
    time_t now = time(0);
    os << " cross validation " << ctime(&now);
    {
      boost::ptr_vector<std::thread> threads;
      for (size_t i=0; i<num_cpus; ++i) {
	for (int j=0; j<16; ++j)
	  top_rated[j][i].clear();
	threads.push_back(new std::thread(Validation(i)));
      }
      size_t total_skip = 0;
      for (size_t i=0; i<num_cpus; ++i) {
	threads[i].join();
	total_skip += skip[i];
	if (i)
	  for (int j=0; j<16; ++j)
	    top_rated[j][0].merge(top_rated[j][i]);
      }
      os << "  skip by rating " << total_skip << "\n";
      os << "progress " << std::setprecision(5);
      for (int j=0; j<16; ++j)
	os << j << " " << 100.0*top_rated[j][0].average() << "   ";
      os << "\n";
      for (int j=1; j<16; ++j)
	top_rated[0][0].merge(top_rated[j][0]);
      os << "all " << 100.0*top_rated[0][0].average() << "\n";
      os.flush();
    }
    if (i == num_iteration)
      break;
    now = time(0);
    os << " start search " << ctime(&now);
    time_point timer = clock::now();
    
    now = time(0);
    os << " start l1ball " << ctime(&now);
    double consumed = ::clock();
    learn(w);
    consumed = ::clock() - consumed;
    std::cerr << "  cpu time " << consumed / CLOCKS_PER_SEC << "\n";
    size_t total_skip = 0;
    {
      for (size_t i=0; i<num_cpus; ++i)
	total_skip += skip[i];
      os << "  skip by rating " << total_skip << "\n";
    }
    {
      std::ostringstream ss;
      ss << output_prefix << eval_type + "-moverating-" << i;
      std::string all_weights = ss.str()+".txt";
      std::ofstream os(all_weights.c_str());
      for (size_t i=0; i<w.size(); ++i)
	os << w[i] << "\n";
      for (size_t i=0; i<features->groupSize(); ++i) {
	features->save(ss.str(), i); 
	features->showGroup(std::cerr, i);
      }
    }
    old_w -= w;
    old_w = abs(old_w);
    os << "weight change: max " << old_w.max() << " ave. " << old_w.sum()/old_w.size()
       << "\n";

    const int num_non_zero = w.size() - std::count(&w[0], &w[0]+w.size(), 0);
    os << "#non-zero weights " << num_non_zero;
    os << "  mean abs weights " << L1Ball::l1norm(w)/dimension;
    os << "  mean abs weights (nonzero) " << L1Ball::l1norm(w)/num_non_zero << "\n";
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
