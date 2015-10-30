/* pv.cc
 */
#include "quiesce.h"
#include "pvVector.h"
#include "eval/eval.h"
#include "eval/evalFactory.h"
#include "osl/record/kisen.h"
#include "osl/numEffectState.h"
#include "osl/record/csaRecord.h"
#include "osl/misc/perfmon.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"
#include "osl/rating/featureSet.h"
#include <boost/program_options.hpp>
#include <memory>
#include <iostream>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;
std::unique_ptr<gpsshogi::Eval> my_eval;
std::string eval_type, kisen_filename, csa_filename, eval_data;
size_t record_id, position_id, move_id, full_width_depth, quiesce_depth, quiesce_window;
bool all_move, quiet;

void run(const NumEffectState& state, Move best_move);

int main(int argc, char **argv)
{
  po::options_description options("all_options");

  options.add_options()
    ("record,r",
     po::value<size_t>(&record_id)->default_value(0),
     "record-id")
    ("position,p",
     po::value<size_t>(&position_id)->default_value(0),
     "position-id")
    ("move,m",
     po::value<size_t>(&move_id)->default_value(0),
     "move-id")
    ("kisen-file,k",
     po::value<std::string>(&kisen_filename)->default_value("../../../data/kisen/01.kif"),
     "filename for records to be analyzed")
    ("csa-file,f",
     po::value<std::string>(&csa_filename)->default_value(""),
     "csa filename to be analyzed")
    ("eval,e",
     po::value<std::string>(&eval_type)->default_value(std::string("stableopenmidending")),
     "evaluation function (king, piece, ...)")
    ("eval-data",
     po::value<std::string>(&eval_data)->default_value("../../../data/eval.txt"))
    ("depth",
     po::value<size_t>(&full_width_depth)->default_value(2))
    ("quiesce-depth",
     po::value<size_t>(&quiesce_depth)->default_value(4))
    ("quiesce-window",
     po::value<size_t>(&quiesce_window)->default_value(16))
    ("all-move,a",
     po::value<bool>(&all_move)->default_value(true))
    ("quiet,q",
     po::value<bool>(&quiet)->default_value(false),
     "show only cycles elapsed")
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
  if (!osl::eval::ml::OpenMidEndingEval::setUp()) {
    std::cerr << "OpenMidEndingEval set up failed";
    // fall through as this might not be fatal depending on eval type used
  }
  if (!osl::progress::ml::NewProgress::setUp()) {
    std::cerr << "NewProgress set up failed";
    // fall through as this might not be fatal depending on eval type used
  }
  my_eval.reset(gpsshogi::EvalFactory::newEval(eval_type));
  if (my_eval == NULL) {
    std::cerr << "unknown eval type " << eval_type << "\n";
    throw std::runtime_error("unknown eval type");
  }
  std::cerr << "pawn value " << my_eval->pawnValue() << "\n";
  if (!eval_data.empty()) {
    if (! my_eval->load(eval_data.c_str()))
      std::cerr << "load failed " << eval_data << "\n";
  }

  rating::StandardFeatureSet::instance();

  if (csa_filename != "") {
    CsaFile file(csa_filename);
    const auto moves = file.moves();
    NumEffectState state(file.initialState());
    PVVector pv;
    std::cerr << state << "\n";
    int value;
    if (moves.empty()) 
    {
      Quiesce quiesce(my_eval.get(), full_width_depth, quiesce_depth); 
      {
	quiesce.quiesce(state, value, pv);
	std::cerr << pv << value << "\n";
      }
    }
    else
    {
      run(state, moves[0]);
    }
    return 0;
  }

  KisenFile kisen_file(kisen_filename.c_str());
  NumEffectState state(kisen_file.initialState());
  const std::vector<Move> moves=kisen_file.moves(record_id);
  if (position_id >= moves.size()) {
    std::cerr << "position_id too large " << position_id << " " << moves.size() << "\n";
    return 1;
  }
  for (size_t j=0; j<position_id; j++) {
    const Square opponent_king 
      = state.kingSquare(alt(state.turn()));
#ifndef NDEBUG
    const Player turn = state.turn();
#endif
    assert(! state.hasEffectAt(turn, opponent_king));
    state.makeMove(moves[j]);
  }
  Move next_move = moves[position_id];
  run(state, next_move);
}

void run(const NumEffectState& state, Move best_move)
{
  Quiesce quiesce(my_eval.get(), full_width_depth, quiesce_depth); 
  size_t consumed = 0;
  PVVector pv;
  MoveVector moves;
  state.generateLegal(moves);

  std::cerr << state << best_move << "\n";
  int best_value;
  {
    NumEffectState new_state = state;
    new_state.makeMove(best_move);
    misc::PerfMon clock;
    quiesce.quiesce(new_state, best_value, pv);
    consumed += clock.stop();
  }
  if (! quiet) {
    std::cerr << "best value " << best_value;
    for (Move move: pv)
      std::cerr << " " << csa::show(move);
    std::cerr << "\n";
  }

  if (move_id >= moves.size()) {
    std::cerr << "move_id too large " << move_id << " " << moves.size() << "\n";
    if (moves.empty()) return;
    move_id = moves.size()-1;
  }
  for (size_t i=(all_move ? 0 : move_id); i<=move_id; ++i) {
    if (! quiet)
      std::cerr << "root " << i << " " << moves[i] << "\n";
    NumEffectState new_state = state;
    new_state.makeMove(moves[i]);
    PVVector pv;
    pv.push_back(moves[i]);
    misc::PerfMon clock;
    const int width = my_eval->pawnValue()*quiesce_window;
    int value;
    quiesce.quiesce(new_state, value, pv, best_value-width, best_value+width);
    consumed += clock.stop();
    if (! quiet) {
      std::cerr << value;
      for (Move move: pv)
	std::cerr << " " << csa::show(move);
      std::cerr << "\n";
    }
  }
  std::cerr << "cycles/node " << (double)consumed/quiesce.nodeCount() << "\n";
  std::cerr << "node count " << quiesce.nodeCount() << "\n";  
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
