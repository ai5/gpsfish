/* select-record.cc
 */
#include "eval/eval.h"
#include "eval/evalFactory.h"
#include "eval/progress.h"
#include "pvVector.h"
#include "l1Ball.h"
#include "analyzer.h"
#include "pvFile.h"
#include "pvGenerator.h"
#include "osl/record/kisen.h"
#include "osl/numEffectState.h"
#include "osl/misc/milliSeconds.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/program_options.hpp>
#include <thread>
#include <iostream>
#include <algorithm>
#include <utility>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;
typedef std::valarray<double> valarray_t;

const int MaxThreads = 64;
std::string output_file;
std::string kisen_filename;
std::unique_ptr<gpsshogi::Eval> my_eval;
std::vector<int> feature_count[MaxThreads];
std::vector<std::pair<double,std::vector<double> > > record_rate;
size_t num_cpus;
int max_record;
int Q;

class PVRun
{
protected:
  int thread_id;
public:
  explicit PVRun(int tid) : thread_id(tid)
  {
  }
  virtual ~PVRun() {}
  virtual void finishRecord(int record)=0;
  virtual void addFeatures(const MoveData&)=0;
};

class PVCount : public PVRun
{
  std::vector<char> found;
public:
  PVCount(int tid) : PVRun(tid), found(my_eval->dimension())
  {
    std::fill(found.begin(), found.end(), 0);
  }
  void finishRecord(int record)
  {
    int count = 0;
    for (size_t i=0; i<found.size(); ++i) {
      feature_count[thread_id][i] += found[i];
      count += found[i];
      found[i] = 0;
    }
    assert(count);
  }
  void addFeatures(const MoveData& features)
  {
    for (size_t i=0; i<features.diffs.size(); ++i)
      found.at(features.diffs[i].first) |= 1;
  }
};

class PVRate : public PVRun
{
  std::vector<int> found;
public:
  PVRate(int tid) : PVRun(tid), found(my_eval->dimension())
  {
    std::fill(found.begin(), found.end(), 0);
  }
  static double maxRate(std::vector<int>& found)
  {
    double rate = 0.0;
    for (size_t i=0; i<found.size(); ++i) {
      if (found[i] == 0) 
	continue;
      rate = std::max(rate, 1.0/feature_count[0][i]);
    }
    return rate;
  }
  static std::pair<double,std::vector<double> > quantile(std::vector<int>& found)
  {
    int count = 0;
    for (size_t i=0; i<found.size(); ++i)
      if (found[i]) {
	found[i] = feature_count[0][i];
	++count;
	assert(found[i]);
      }
    assert(count);
    std::sort(found.begin(), found.end(), std::greater<int>());
    auto p = std::lower_bound(found.begin(), found.end(), 0, std::greater<int>());
    const size_t s = p - found.begin();
    std::vector<double> array(Q);
    if (s == 0)
      return std::make_pair(0.0, array);
    for (int i=0; i<std::min(Q/2,8); ++i)
      array[i] = 1.0/found[s-s/Q*i-1];
    return std::make_pair(1.0/found[s-s/Q-1], array);
  }
  void finishRecord(int record)
  {
    // record_rate[record] = maxRate(found);
    record_rate[record] = quantile(found);
    std::fill(found.begin(), found.end(), 0);
  }
  void addFeatures(const MoveData& features)
  {
    for (size_t i=0; i<features.diffs.size(); ++i) {
      found.at(features.diffs[i].first) |= 1;
    }
  }
};

void run(const char *filename, PVRun *run) 
{
  PVFileReader pr(filename);
  int record, position;
  PVVector pv_best, pv;

  int cur_record=-1, cur_position=-1;
  int num_records = 0;

  KisenFile kisen_file(kisen_filename);
  NumEffectState state(kisen_file.initialState());
  std::vector<Move> moves;

  while (pr.newPosition(record, position)) {
    if (record != cur_record) {
      if (cur_record >= 0)
	run->finishRecord(cur_record);
      cur_record = record;
      moves = kisen_file.moves(cur_record);
      max_record = std::max(max_record, record);
      if ((++num_records) % 50 == 0)
	std::cerr << '.';
      state = NumEffectState(kisen_file.initialState());
    }
    if (position == 0) {
      cur_position = 0;
      state = NumEffectState(kisen_file.initialState());
    } else {
      assert(position > cur_position);
      while (position > cur_position) {
	state.makeMove(moves[cur_position]);
	++cur_position;
      }
    }
    bool has_best_move = pr.readPv(pv);
    assert(has_best_move);
    if (!has_best_move)
      continue;
    int moves = 0;
    MoveData features;
    Analyzer::analyzeLeaf(state, pv, *my_eval, features);
    run->addFeatures(features);
    while (pr.readPv(pv)) {
      ++moves;
      Analyzer::analyzeLeaf(state, pv, *my_eval, features);
      run->addFeatures(features);
    }
  }
}

struct CountRun
{
  int thread_id;
  const char *filename;
  CountRun(int tid, const char *file) : thread_id(tid), filename(file)
  {
  }
  void operator()() const
  {
    if (feature_count[thread_id].size() == 0)
      feature_count[thread_id].resize(my_eval->dimension());
    assert(feature_count[thread_id].size() == my_eval->dimension());
    PVCount counter(thread_id);
    run(filename, &counter);
  }
};

struct RateRun
{
  int thread_id;
  const char *filename;
  RateRun(int tid, const char *file) : thread_id(tid), filename(file)
  {
  }
  void operator()() const
  {
    PVRate rate(thread_id);
    run(filename, &rate);
  }
};

int main(int argc, char **argv)
{
  nice(20);
  
  std::string eval_type;
  
  po::options_description options("all_options");
  options.add_options()
    ("kisen-file,k", po::value<std::string>(&kisen_filename)->default_value("../../../data/kisen/01.kif"),
     "filename for records to be analyzed")
    ("num-cpus,N",
     po::value<size_t>(&num_cpus)->default_value(1),
     "number cpus to be used")
    ("output,o",
     po::value<std::string>(&output_file)->default_value("selected.kif"),
     "output filename of reordered kisen file")
    ("eval,e",
     po::value<std::string>(&eval_type)->default_value(std::string("piece")),
     "evaluation function (piece, rich0, rich1)")
    ("quantile,Q",
     po::value<int>(&Q)->default_value(8),
     "quantile point to sort")
    ;
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("pv-file", po::value<std::vector<std::string> >());
  po::options_description command_line_options;
  command_line_options.add(options).add(hidden);
  po::positional_options_description p;
  p.add("pv-file", -1);

  po::variables_map vm;
  std::vector<std::string> filenames;
  try
  {
    po::store(po::command_line_parser(argc, argv).
	      options(command_line_options).positional(p).run(), vm);
    po::notify(vm);
    filenames = vm["pv-file"].as<std::vector<std::string> >();
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

  OslConfig::setUp();
  if (!osl::eval::ml::OpenMidEndingEval::setUp()) {
    std::cerr << "OpenMidEndingEval set up failed";
    // fall through as this might not be fatal depending on eval type used
  }
  if (!osl::progress::ml::NewProgress::setUp()) {
    std::cerr << "NewProgress set up failed";
    // fall through as this might not be fatal depending on eval type used
  }
  std::cerr << "Q " << Q << " eval " << eval_type << "\n";
  my_eval.reset(gpsshogi::EvalFactory::newEval(eval_type));

  std::cerr << "count\n";
  {
    size_t counted = 0;
    while (counted < filenames.size()) {
      boost::ptr_vector<std::thread> threads;
      const size_t split = std::min(num_cpus, filenames.size()-counted);
      for (size_t i=0; i<split; ++i)
	threads.push_back(new std::thread(CountRun(i, filenames[counted+i].c_str())));
      counted += split;
      for (size_t i=0; i<split; ++i)
	threads[i].join();
    }
    for (size_t i=1; i<std::min(num_cpus, filenames.size()); ++i)
      for (size_t j=0; j<feature_count[i].size(); ++j)
	feature_count[0][j] += feature_count[i][j];
  }
  record_rate.resize(max_record);

  std::cerr << "\nrate\n";
  {
    size_t rated = 0;
    while (rated < filenames.size()) {
      boost::ptr_vector<std::thread> threads;
      const size_t split = std::min(num_cpus, filenames.size()-rated);
      for (size_t i=0; i<split; ++i)
	threads.push_back(new std::thread(RateRun(i, filenames[rated+i].c_str())));
      rated += split;
      for (size_t i=0; i<split; ++i)
	threads[i].join();
    }
  }
  std::cerr << "\nrecord\n";
  typedef std::pair<std::pair<double,std::vector<double> >, int> rate_id_t;
  std::vector<rate_id_t> rate_id(record_rate.size());
  for (size_t i=0; i<record_rate.size(); ++i)
    rate_id[i] = std::make_pair(record_rate[i], i);
  std::sort(rate_id.begin(), rate_id.end(), std::greater<rate_id_t>());
  for (int i=0; i<20; ++i) {
    const int index = i*max_record/20;
    std::cerr << index << " " << rate_id[index].first.first << " " << 1.0/rate_id[index].first.first << " ";
    for (int j=0; j<std::min(Q/2,8); ++j)
      std::cerr << " " << 1.0/rate_id[index].first.second[j];
    std::cerr << "\n";
  }
  std::cerr << "feature\n";
  std::sort(feature_count[0].begin(), feature_count[0].end());
  for (int i=0; i<20; ++i) {
    const int index = i*feature_count[0].size()/20;
    std::cerr << index << " " << feature_count[0][index] << "\n";
  }

  std::ofstream os(output_file.c_str());
  KisenWriter okisen(os);

  KisenFile kisen_file(kisen_filename);
  NumEffectState state(kisen_file.initialState());
  for (size_t i=0; i<rate_id.size(); ++i) {
    const int index = rate_id[i].second;
    std::vector<Move> moves = kisen_file.moves(index);
    okisen.save(RecordMinimal{state, moves});
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
