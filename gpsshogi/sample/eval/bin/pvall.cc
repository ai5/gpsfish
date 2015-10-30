/* pvall.cc
 */
#include "quiesce.h"
#include "pvVector.h"
#include "pvGenerator.h"
#include "eval/eval.h"
#include "eval/evalFactory.h"
#include "osl/record/kisen.h"
#include "osl/numEffectState.h"
#include "osl/move_probability/featureSet.h"
#include "osl/record/csaRecord.h"
#include "osl/misc/perfmon.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"
#include "osl/rating/featureSet.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <iomanip>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;
std::unique_ptr<gpsshogi::Eval> my_eval;
std::string eval_type, kisen_filename, csa_filename, eval_data;
size_t record_id, rand_seed, full_width_depth, quiesce_depth, quiesce_window;
int position_id, quiet;

void run(NumEffectState& state, const std::vector<Move>& moves);

unsigned long long total_cycles=0, total_node_count=0, total_rank=0, total_agree_count=0, total_positions=0;

int main(int argc, char **argv)
{
  po::options_description options("all_options");

  options.add_options()
    ("record,r",
     po::value<size_t>(&record_id)->default_value(0),
     "record-id")
    ("position,p",
     po::value<int>(&position_id)->default_value(0),
     "position-id, -1 for all")
    ("randomness",
     po::value<size_t>(&rand_seed)->default_value(1),
     "move-id")
    ("kisen-file,k",
     po::value<std::string>(&kisen_filename)->default_value("titles.kif"),
     "filename for records to be analyzed")
    ("csa-file,f",
     po::value<std::string>(&csa_filename)->default_value(""),
     "csa filename to be analyzed")
    ("eval,e",
     po::value<std::string>(&eval_type)->default_value(std::string("stableopenmidending")),
     "evaluation function (king or piece)")
    ("eval-data",
     po::value<std::string>(&eval_data)->default_value("../../../data/eval.txt"))
    ("depth",
     po::value<size_t>(&full_width_depth)->default_value(2))
    ("quiesce-depth",
     po::value<size_t>(&quiesce_depth)->default_value(4))
    ("quiesce-window",
     po::value<size_t>(&quiesce_window)->default_value(8))
    ("quiet-threshold,q",
     po::value<int>(&quiet)->default_value(0),
     "specifies threshold to print position and pv if positive, quiet if negative")
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
  std::cerr << "pawn value " << my_eval->pawnValue()
	    << " scaled to 100" << "\n";
  if (!eval_data.empty()) {
    if (! my_eval->load(eval_data.c_str()))
      std::cerr << "load failed " << eval_data << "\n";
  }

  rating::StandardFeatureSet::instance();
  move_probability::StandardFeatureSet::instance();
  PVGenerator::setNormalDepth(full_width_depth);
  PVGenerator::setQuiesceDepth(quiesce_depth);
  // if (loss_function == "hinge")
    PVGenerator::setWindowAsymmetry(4);

  if (csa_filename != "") {
    CsaFile file(csa_filename);
    const std::vector<osl::Move> moves = file.moves();
    NumEffectState state(file.initialState());
    run(state, moves);
    return 0;
  }

  KisenFile kisen_file(kisen_filename.c_str());
  NumEffectState state(kisen_file.initialState());
  const std::vector<Move> moves=kisen_file.moves(record_id);
  run(state, moves);
  std::cerr << "cycles " << total_cycles << " / " << total_node_count
	    << " = " << 1.0*total_cycles/total_node_count << "\n";
  std::cerr << "rank " << total_rank << " / " << total_positions
	    << " = " << 1.0*total_rank/total_positions << "\n";
  std::cerr << "agree " << total_agree_count << " / " << total_positions
	    << " = " << 1.0*total_agree_count/total_positions << "\n";
}

void run(const NumEffectState& state, const std::vector<Move>& moves, int position)
{
  const Move best_move = moves[position];
  const double scaling = 100.0/my_eval->pawnValue();
  const osl::progress::ml::NewProgress progress(state);
  const int width = quiesce_window * my_eval->pawnValue();
  int best_value;
  PVVector best_pv;
  PVGenerator::values_t values;
  size_t rank_search, rank_probability;
  Quiesce quiesce(my_eval.get(), full_width_depth, quiesce_depth); 
  for (size_t i=position; i+1<moves.size(); ++i)
    quiesce.addBigram(moves[i], moves[i+1]);
  misc::PerfMon clock;
  int outrange_better_move, outrange_other_move;
  const PVGenerator::SearchResult result = PVGenerator::searchSiblings
    (quiesce, width, state, progress.progress16(),
     best_move, record_id+position_id+rand_seed, 64, 4096,
     best_value, best_pv, values,
     rank_search, rank_probability, outrange_better_move, outrange_other_move);
  const size_t consumed = clock.stop();
  total_cycles += consumed;
  total_node_count += quiesce.nodeCount();
  ++total_positions;
  total_rank += rank_search;
  if (rank_search == 0)
    ++total_agree_count;
  std::sort(values.begin(), values.end()); 
  std::reverse(values.begin(), values.end());
  const int pv_piece = best_pv.pieceValueOfTurn();
  const int best_piece = values.empty() ? 0 : values[0].second.pieceValueOfTurn();
  const int piece_threshold = 256;
  if (quiet >=0 
      && (result != PVGenerator::SearchOK || (int)rank_search >= quiet || best_piece - pv_piece >= piece_threshold)) {
    std::cerr << state << "position = " << position << "\n";
    std::cout << std::setw(9) << "recorded" << "  ";
    for (size_t i=position; i<std::min((size_t)position+8, moves.size()); ++i)
      std::cout << csa::show(moves[i]);
    std::cout << "\n";
    if (! best_pv.empty()) {	
      std::cout << "***" << std::setw(6) << (int)(best_value*scaling) << "  ";
      for (Move move: best_pv)
	std::cout << csa::show(move);
      std::cout << " (" << pv_piece << ")\n";
    }
    std::cerr << "cycles/node " << (double)consumed/quiesce.nodeCount() << "\n";
    std::cerr << "node count " << quiesce.nodeCount() << "\n";  
  }
  if (result != PVGenerator::SearchOK) {
    if (quiet >=0) {
      std::cerr << "this position is not appropriate for learning, type "
		<< result << "\n";
    }
    return;
  }
  if (quiet>=0
      && ((int)rank_search >= quiet
	  || best_piece - pv_piece >= piece_threshold)) {
    std::cout << "rank probability " << rank_probability
	      << " search " << rank_search << "\n";
    for (size_t i=0; i<values.size(); ++i) {
      std::cout << std::setw(3) << i << std::setw(6)
		<< (int)(values[i].first*scaling) << "  ";
      for (Move move: values[i].second) {
	std::cout << csa::show(move);
      }
      std::cout << " (" << values[i].second.pieceValueOfTurn() << ")\n";
    }  
  }
}

void run(NumEffectState& state, const std::vector<Move>& moves)
{
  if (position_id >= 0 && (size_t)position_id >= moves.size()) {
    std::cerr << "position_id too large " << position_id << " " << moves.size() << "\n";
    return;
  }
  for (size_t j=0; j<((position_id >= 0) ? position_id : moves.size()); j++) {
    const Square opponent_king 
      = state.kingSquare(alt(state.turn()));
#ifndef NDEBUG
    const Player turn = state.turn();
#endif
    if (position_id < 0) 
      run(state, moves, j);
    assert(! state.hasEffectAt(turn, opponent_king));
    state.makeMove(moves[j]);
  }
  if (position_id >= 0)
    run(state, moves, position_id);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
