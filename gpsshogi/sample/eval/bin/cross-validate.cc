#include "analyzer.h"
#include "pvGenerator.h"
#include "eval/evalFactory.h"
#include "eval/eval.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/program_options.hpp>
#include <thread>
#include <memory>

#include <iostream>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;

const int MaxThreads = 64;
static int cross_validation_randomness = 1;
int bonanza_compatible;

static void validate(double search_window_for_validation,
		     int max_progress, int min_rating,
		     size_t num_records,
		     size_t split, int cross_start,
		     const std::vector<std::string> &kisen_filenames,
		     gpsshogi::Eval *eval, std::ostream &os)
{
  double sigmoid_alpha = 3.0/eval->pawnValue();
  if (bonanza_compatible) {
    search_window_for_validation = 256*bonanza_compatible/eval->pawnValue();
    sigmoid_alpha = 1.0/(search_window_for_validation/7.0);
  }
  const KisenAnalyzer::OtherConfig config =
    {
      search_window_for_validation, sigmoid_alpha,
      max_progress, cross_validation_randomness,
      (size_t)min_rating, eval
    };
  boost::ptr_vector<std::thread> threads;
      
  std::vector<std::string> csa_files;
  KisenAnalyzer::RecordConfig cross_configs[MaxThreads];
  PVGenerator::Result results[MaxThreads];
  KisenAnalyzer::distributeJob(split, &cross_configs[0], cross_start,
			       num_records,
			       kisen_filenames, min_rating,
			       csa_files);
  for (size_t j=0; j<split; ++j) {
    cross_configs[j].allow_skip_in_cross_validation = (bonanza_compatible == 0);
    threads.push_back(new std::thread(Validator(cross_configs[j], config, results+j)));
  }
  stat::Average werror, order_lb, order_ub, toprated, toprated_strict;
  size_t total_node_count = 0, siblings = 0;
  int record_processed = 0;
  for (size_t j=0; j<split; ++j) {
    threads[j].join();
    werror.merge(results[j].werrors);
    order_lb.merge(results[j].order_lb);
    order_ub.merge(results[j].order_ub);
    toprated.merge(results[j].toprated);
    toprated_strict.merge(results[j].toprated_strict);
    record_processed += results[j].record_processed;
    total_node_count += results[j].node_count;
    siblings += results[j].siblings;
  }
  os << "\n  mean errors in search " << werror.average() << "\n";
  os << "  average order (lb) " << order_lb.average() << "\n";
  os << "  average order (ub) " << order_ub.average() << "\n";
  os << "  ratio that best move is recorded move " << toprated.average() << "\n";
  os << "  ratio that best move is recorded move (strict) " << toprated_strict.average() << "\n";
  os << "  (#records) " << record_processed << "\n";
  os << "positions " << werror.numElements()
     << " siblings " << siblings
     << " node count " << total_node_count << "\n";
}

int main(int argc, char **argv)
{
  double search_window_for_validation;
  size_t num_records, num_cpus;
  std::string eval_type, initial_value;
  int max_progress;
  bool high_rating_only;
  int cross_validation_start;

  po::options_description options("all_options");

  options.add_options()
    ("num-records,n",
     po::value<size_t>(&num_records)->default_value(0),
     "number of records to be analyzed (all if 0)")
    ("num-cpus,N",
     po::value<size_t>(&num_cpus)->default_value(1),
     "number cpus to be used")
    ("kisen-file,k", po::value<std::vector<std::string> >(),
     "filename for records to be analyzed")
    ("eval,e",
     po::value<std::string>(&eval_type)->default_value(std::string("piece")),
     "evaluation function (piece, rich0, rich1)")
    ("vwindow",
     po::value<double>(&search_window_for_validation)->default_value(8),
     "search window for validation, relative to pawn value")
    ("initial-value-file,f",
     po::value<std::string>(&initial_value)->default_value(""),
     "File with initial eval values")
    ("max-progress",
     po::value<int>(&max_progress)->default_value(16),
     "When non-negative, only use states where progress is less than this "
      "value.")
    ("high-rating-only",
     po::value<bool>(&high_rating_only)->default_value(0),
      "When true only consider plays where both player have at least "
      "1500 rating value")
    ("cross-validation-start",
     po::value<int>(&cross_validation_start)->default_value(200000),
     "Start ID of record in kisen file to do cross validation")
    ("bonanza-compatible",
     po::value<int>(&bonanza_compatible)->default_value(0),
     "enable bonanza compatible mode if > 0, 1 for piece, 16 for progresseval")
    ("help", "produce help message")
    ;

  std::vector<std::string> kisen_filenames;
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

  OslConfig::setUp();

  std::unique_ptr<gpsshogi::Eval>
    eval(gpsshogi::EvalFactory::newEval(eval_type));
  if (eval == NULL)
  {
    std::cerr << "unknown eval type " << eval_type << "\n";
    throw std::runtime_error("unknown eval type");
  }
  if (! eval->load(initial_value.c_str()))
    std::cerr << "load failed " << initial_value << "\n";

  if (!osl::eval::ml::OpenMidEndingEval::setUp()) {
    std::cerr << "OpenMidEndingEval set up failed";
    // fall through as this might not be fatal depending on eval type used
  }
  if (!osl::progress::ml::NewProgress::setUp()) {
    std::cerr << "NewProgress set up failed";
    // fall through as this might not be fatal depending on eval type used
  }

  size_t min_rating = high_rating_only ? 1500 : 0;

  if (bonanza_compatible) {
    PVGenerator::setNormalDepth(1);
    PVGenerator::setQuiesceDepth(-1);
  }
  validate(search_window_for_validation, max_progress,
	   min_rating, num_records, num_cpus,
	   cross_validation_start, kisen_filenames, eval.get(), std::cout);

  return 0;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
