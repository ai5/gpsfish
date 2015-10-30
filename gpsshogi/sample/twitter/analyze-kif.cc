/* analyze-kif.cc
 */
#include "annotate.h"
#include "gpsshogi/revision.h"
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/search/alphaBeta2.h"
#include "osl/search/searchMonitor.h"
#include "osl/search/simpleHashTable.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/record/kakinoki.h"
#include "osl/record/record.h"
#include "osl/record/ki2.h"
#include "osl/record/csaRecord.h"
#include "osl/misc/iconvConvert.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

namespace po = boost::program_options;
using namespace osl;

void analyze(const NumEffectState& state, const std::vector<Move>& moves, size_t move_number);
void analyze_root(const NumEffectState& state, const std::vector<Move>& moves, size_t move_number);
size_t multi_pv, skip_first_moves;
int seconds, min_seconds;
bool root_info, analyze_all_moves;
int main(int argc, char **argv)
{
  size_t move_number;
  po::options_description options;
  std::string filename;
  options.add_options()
    ("filename,f", po::value<std::string>(&filename),
     "specify .kif, .ki2 or .csa file to be analyzed")
    ("moves,m", po::value<size_t>(&move_number),
     "specify move number")
    ("all", po::value<bool>(&analyze_all_moves)->default_value(false),
     "analyze all moves")
    ("skip-first",
     po::value<size_t>(&skip_first_moves)->default_value(20),
     "the number of opening moves to be ignored when analyze all moves")
    ("seconds,S",
     po::value<int>(&seconds)->default_value(300),
     "seconds for search")
    ("min-seconds",
     po::value<int>(&min_seconds)->default_value(45),
     "minimum seconds for search")
    ("root-info,i",
     po::value<bool>(&root_info)->default_value(false),
     "analyze characteristics of root position and the previous move (do not search best move)")
    ("multi-pv", po::value<size_t>(&multi_pv)->default_value(100),
     "specify move number")
    ("version", "Show revision number")
    ("help,h", "Show help message");
  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);
    if (vm.count("help")) {
      std::cerr << "Usage: " << argv[0] << " [options] files" << std::endl;
      std::cout << options << std::endl;
      return 1;
    }
    if (vm.count("version")) {
      std::cout << gpsshogi::gpsshogi_revision << std::endl;
      return 0;
    }
    min_seconds = std::min(min_seconds, seconds);
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }
  if (filename.empty())
    return 1;
  std::vector<Move> moves;
  NumEffectState state;
  try 
  {
    if (filename.find(".kif") == filename.size()-4) 
    {
      KakinokiFile file(filename);
      moves = file.moves();
      state = file.initialState();
    } 
    else if (filename.find(".ki2") == filename.size()-4) 
    {
      Ki2File file(filename);
      moves = file.moves();
      state = file.initialState();
    }
    else if (filename.find(".csa") == filename.size()-4) 
    {
      CsaFile file(filename);
      moves = file.moves();
      state = file.initialState();
    }
  }
  catch (KakinokiIOError&) 
  {
    return 1;
  }
  if (move_number > moves.size() && !analyze_all_moves)
    return 2;
  eval::ml::OpenMidEndingEval::setUp();
  progress::ml::NewProgress::setUp();

  for (size_t i=0; i<moves.size(); ++i) {
    if (! state.isValidMove(moves[i]))
      return 2;
    state.makeMove(moves[i]);
    if (i+1 == move_number || (analyze_all_moves && i>= skip_first_moves)) {
      if (root_info)
	analyze_root(state, moves, i+1);
      else
	analyze(state, moves, i+1);
    }
    if (i+1 == move_number && ! analyze_all_moves)
      break;
  }  
  return 0;
}

struct ForcedMoveMonitor : public osl::search::SearchMonitor
{
  bool forced_move;
  ForcedMoveMonitor() : forced_move(false)
  {
  }
  void rootForcedMove(Move the_move)
  {
    forced_move = true;
  }
};

bool is_threatmate(const NumEffectState& state,
		   const search::SimpleHashTable *table)
{
  const search::SimpleHashRecord *record = table 
    ? table->find(HashKey(state)) : 0;
  return record
    && record->threatmate().isThreatmate(state.turn());
}

void analyze(const NumEffectState& src, const std::vector<Move>& moves, size_t move_number)
{
  static double scale = eval::ml::OpenMidEndingEval::captureValue(newPtypeO(WHITE,PAWN))/200.0;
  std::cerr << src;
  const std::shared_ptr<ForcedMoveMonitor> monitor(new ForcedMoveMonitor);
  game_playing::AlphaBeta2OpenMidEndingEvalPlayer player;
  player.addMonitor(monitor);
  player.setNextIterationCoefficient(3.0);
  player.setVerbose(2);
  if (osl::OslConfig::isMemoryLimitEffective()) 
  {
    player.setTableLimit(std::numeric_limits<size_t>::max(), 200);
    player.setNodeLimit(std::numeric_limits<size_t>::max());
  }
  else 
  {
    player.setTableLimit(3000000, 200);
  }
  player.setDepthLimit(2000, 400, 200);

  game_playing::GameState state(src);
  const int sec = (moves.size() == move_number) ? seconds : min_seconds;
  search::TimeAssigned time(milliseconds(sec*1000));

  const time_point start_time = clock::now();
  search::AlphaBeta2SharedRoot root_info;
  player.enableMultiPV(multi_pv);
  MoveWithComment move = player.analyzeWithSeconds(state, time, root_info);
  const time_point finish_time = clock::now();
  const double consumed = toSeconds(finish_time - start_time);

  std::ostringstream ret, ret_en;
  ret << "[(" << move_number << ") ";
  ret_en << "[(" << move_number << ") ";
  NumEffectState s;
  if (move_number) 
  {
    for (size_t i=0; i<move_number-1; ++i)
      s.makeMove(moves[i]);
    ret << ki2::show(moves[move_number-1], s) << "]";
    ret_en << psn::showXP(moves[move_number-1]) << "]";
    s.makeMove(moves[move_number-1]);
  }
  if (monitor->forced_move) 
  {
    ret << " ?";
    ret_en << " ?";
  }
  else 
  {
    ret << " " << move.value;
    ret_en << " " << move.value;
  }
  const SimpleHashTable *table = player.table();
  const Move last_move = move_number ? moves[move_number-1] : Move();
  if (move.move.isNormal())
  {
    ret << ' ' << ki2::show(move.move, s, last_move);
    ret_en << ' ' << psn::showXP(move.move);
    s.makeMove(move.move);
    size_t max_pv_length = move.moves.size();
    if (multi_pv)
      max_pv_length = std::min((size_t)10, max_pv_length);
    for (size_t i=0; i<max_pv_length; ++i) {
      if (is_threatmate(s, table)) {
	ret << "(" K_TSUMERO ")";
	ret_en << " (threatmate) ";
      }
      ret << ki2::show(move.moves[i], s, i ? move.moves[i-1] : move.move);
      ret_en << ' ' << psn::showXP(move.moves[i]);
      s.makeMove(move.moves[i]);      
    }
  }
  if (monitor->forced_move) 
  {
    ret << " (" << forced_move_in_japanese << ")";
    ret_en << " (forced move)";
  }
  ret << " (" << (int)consumed << "s)";
  ret_en << " (" << (int)consumed << "s)";

  // other moves
  if (move.move.isNormal() && root_info.last_pv.size() > 1)
  {
    const int root_limit = root_info.last_pv.back().depth;
    const std::vector<search::RootPV>& last_pv = root_info.last_pv;

    // sort by evaluation values
    std::vector<std::pair<int,search::SearchState2::PVVector> > others;
    for (int i=last_pv.size()-1; 
	 i>=0 && last_pv[i].depth == root_limit && last_pv.size()-i <= 6;
	 --i) {
      if (last_pv[i].pv.empty() || last_pv[i].pv[0] == move.move)
	continue;
      // secondary moves will be reported many times when checkmate is found on pv
      bool found = false;
      for (size_t j=0; j<others.size(); ++j)
	if (others[j].second[0] == last_pv[i].pv[0])
	  found = true;
      if (found)
	break;			// moves after this move were the results of old iteration
      others.push_back(std::make_pair(last_pv[i].eval/scale*eval::delta(src.turn()),
				      last_pv[i].pv));
    }
    if (! others.empty()) {
      std::sort(others.begin(), others.end());
      for (int i=others.size()-1; i>=0; --i) {
	ret << " / " << static_cast<int>(others[i].first*eval::delta(src.turn()));
	ret_en << " / " << static_cast<int>(others[i].first*eval::delta(src.turn()));
	s = src;
	const Move move = others[i].second[0];
	ret << ' ' << ki2::show(move, s, last_move);
	ret_en << ' ' << psn::showXP(move);
	s.makeMove(move);      
	for (size_t j=1; j<std::min((size_t)4, others[i].second.size()); ++j) {
	  if (is_threatmate(s, table)) {
	    ret << "(" K_TSUMERO ")";
	    ret_en << " (threatmate) ";
	  }
	  const Move move = others[i].second[j];
	  ret << ki2::show(move, s, others[i].second[j-1]);
	  ret_en << ' ' << psn::showXP(move);
	  s.makeMove(move);      
	}
      }
    }
  }

  // japanese
  std::string utf8 = misc::IconvConvert::convert("EUC-JP", "UTF-8", ret.str());
  std::ostringstream outfile;
  outfile << "analyses" << std::setw(3) << std::setfill('0') << move_number << ".txt";
  std::ofstream os(outfile.str().c_str());
  os << utf8 << "\n";
  // english
  std::ostringstream outfile_en;
  outfile_en << "analyses" << std::setw(3) << std::setfill('0') << move_number << "-en.txt";
  std::ofstream os_en(outfile_en.str().c_str());
  os_en << ret_en.str() << "\n";
}


void analyze_root(const NumEffectState& src, const std::vector<Move>& moves, size_t move_number)
{
  std::ostringstream ret, ret_en;
  ret << "[(" << move_number << ") ";
  ret_en << "[(" << move_number << ") ";
  NumEffectState s;
  if (move_number) 
  {
    for (size_t i=0; i<move_number-1; ++i)
      s.makeMove(moves[i]);
    ret << ki2::show(moves[move_number-1], s);
    ret_en << psn::showXP(moves[move_number-1]);
    s.makeMove(moves[move_number-1]);
  }
  ret << "]\n* ";
  ret_en << "]\n* ";

  std::string japanase_euc, english;
  annotate::AnalysesResult shared;
  annotate::analyze(src, moves, move_number-1, shared);
  explain(src, shared, japanase_euc, english);
  if (! japanase_euc.empty())
  {
    // japanese
    ret << japanase_euc;
    std::string utf8 = misc::IconvConvert::convert("EUC-JP", "UTF-8", ret.str());
    std::ostringstream outfile;
    outfile << "info" << std::setw(3) << std::setfill('0') << move_number << ".txt";
    std::ofstream os(outfile.str().c_str());
    os << utf8;
  }
  if (! english.empty())
  {
    // english
    ret_en << english;
    std::ostringstream outfile_en;
    outfile_en << "info" << std::setw(3) << std::setfill('0') << move_number << "-en.txt";
    std::ofstream os_en(outfile_en.str().c_str());
    os_en << ret_en.str();
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
