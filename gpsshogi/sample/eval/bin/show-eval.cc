/* show-eval.cc
 */
#include "eval/eval.h"
#include "eval/evalFactory.h"
#include "eval/progress.h"
#include "osl/record/csaRecord.h"
#include "osl/eval/progressEval.h"
#include "osl/progress.h"
#include "osl/oslConfig.h"
#include <boost/program_options.hpp>
#include <memory>
#include <iostream>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;

std::unique_ptr<gpsshogi::Eval> my_eval;
std::string eval_type, filename, csa_filename;

int main(int argc, char *argv[])
{
  po::options_description options("all_options");

  options.add_options()
    ("eval,e",
     po::value<std::string>(&eval_type)->default_value(std::string("piece")),
     "evaluation function (king or piece)")
    ("filename,f",
     po::value<std::string>(&filename)->default_value("irls.txt"),
     "filename for weights of evaluation function")
    ("csa-filename,c",
     po::value<std::string>(&csa_filename)->default_value(""),
     "filename of a position to evaluate")
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

  OslConfig::setUp();
  osl::progress::ml::NewProgress::setUp();
  my_eval.reset(gpsshogi::EvalFactory::newEval(eval_type));
  if (my_eval == NULL) {
    std::cerr << "unknown eval type " << eval_type << "\n";
    throw std::runtime_error("unknown eval type");
  }
  if (! my_eval->load(filename.c_str())) {
    std::cerr << "load failed\n";
    return 1;
  }

  if (csa_filename != "") 
  {
    osl::eval::ProgressEval::setUp();

    CsaFileMinimal file(csa_filename);
    NumEffectState state(file.initialState());
    auto r = file.load();
    const auto moves=r.moves;
    for (size_t i=0; i<moves.size(); ++i) {
      std::cerr << state;
      osl::eval::ProgressEval peval(state);
      std::cerr << "progress eval " << peval.value()/16 << " ("
		<< peval.openingValue() << " "
		<< peval.endgameValue() << ")\n";
      std::cerr << "value " << my_eval->eval(state) << "\n";
      my_eval->showEvalSummary(state);
      state.makeMove(moves[i]);
    }
    std::cerr << state;
    std::cerr << "value " << my_eval->eval(state) << "\n";
    my_eval->showEvalSummary(state);
  }
  else 
  {
    my_eval->showAll(std::cout);
  }  
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
