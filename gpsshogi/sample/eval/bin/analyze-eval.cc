/* analyze-eval.cc
 */
#include "eval/eval.h"
#include "eval/evalFactory.h"
#include "eval/progress.h"
#include "osl/progress.h"
#include "osl/oslConfig.h"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;

std::unique_ptr<gpsshogi::Eval> my_eval;
std::string eval_type, filename, csa_filename, feature_name;
int feature_index;

int main(int argc, char *argv[])
{
  po::options_description options("all_options");

  options.add_options()
    ("eval,e",
     po::value<std::string>(&eval_type)->default_value(std::string("stableopenmidending")),
     "evaluation function (king or piece)")
    ("filename,f",
     po::value<std::string>(&filename)->default_value("../stable-eval.txt"),
     "filename for weights of evaluation function")
    ("feature,F",
     po::value<std::string>(&feature_name)->default_value("PieceEvalComponent"),
     "feature name to show")
    ("index,i",
     po::value<int>(&feature_index)->default_value(-1),
     "local index inside feature")
    ("help", "produce help message")
    ;

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
  if (vm.count("help")) {
    std::cerr << options << std::endl;
    return 0;
  }

  osl::OslConfig::setUp();
  my_eval.reset(gpsshogi::EvalFactory::newEval(eval_type));
  if (! my_eval) {
    std::cerr << "unknown eval type " << eval_type << "\n";
    throw std::runtime_error("unknown eval type");
  }
  if (! my_eval->load(filename.c_str())) {
    std::cerr << "load failed\n";
    return 1;
  }

  std::cerr << feature_name << ' ' << feature_index << "\n";
  if (feature_index >= 0)
    std::cout << my_eval->describe(feature_name, feature_index) << "\n";
  else
    std::cout << my_eval->describeAll(feature_name) << "\n";
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
