#include "osl/book/openingBook.h"
#include "osl/eval/pieceEval.h"
#include "osl/hashKey.h"
#include "osl/misc/math.h"
#include "osl/record/csaRecord.h"
#include "osl/record/kanjiPrint.h"
#include "osl/search/fixedEval.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceSearch2.tcc"
#include "osl/search/simpleHashTable.h"
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include <deque>
#include <iostream>
#include <vector>


namespace bp = boost::program_options;
bp::variables_map vm;

typedef std::vector<osl::book::WMove> WMoveContainer;

osl::Player the_player = osl::BLACK;
bool is_dump = false;
int error_threshold = 500;
int is_determinate = 0;	   // test only top n moves.  0 for all
int max_depth, non_determinate_depth;
double ratio;		   // use moves[n+1] when the weight[n+1] >= ratio*weight[n]
bool is_quick = false;

std::shared_ptr<osl::NumEffectState> state_to_compare;
size_t state_count = 0;

/**
 * qsearch
 *
 * @param s state
 * @param lastMove
 * @return evaluation value
 */
int qsearch(const osl::SimpleState &s, 
            const osl::Move& lastMove)
{
  if (is_quick) return 0;

  typedef osl::search::QuiescenceSearch2<osl::eval::PieceEval> qsearch_t;
  osl::NumEffectState state(s);
  osl::search::SimpleHashTable table(100000, -1, false);
  osl::search::SearchState2Core::checkmate_t checkmate_searcher;
  osl::search::SearchState2Core core(state, checkmate_searcher);
  qsearch_t qs(core, table);
  osl::eval::PieceEval ev(state);
  return qs.search(state.turn(), ev, lastMove, 4);
}

void showStatistics(const std::deque<int>& src)
{
  double sum, mean, var, dev, skew, kurt;
  osl::misc::computeStats(src.begin(), src.end(), sum, mean, var, dev, skew, kurt);

  std::cout << boost::format(" total: %g\n")  % src.size()
            << boost::format(" mean:  %g\n") % mean
            << boost::format(" dev:   %g\n")  % dev;
}

void printUsage(std::ostream& out, 
                char **argv,
                const boost::program_options::options_description& command_line_options)
{
  out <<
    "Usage: " << argv[0] << " [options] <a_joseki_file.dat>\n"
      << command_line_options 
      << std::endl;
}

void showInfoOfState(osl::book::WeightedBook& book, const int state_index)
{
  osl::record::KanjiPrint printer(std::cerr, 
                                  std::shared_ptr<osl::record::Characters>(
                                            new osl::record::KIFCharacters())
                                  );

  std::cout << boost::format("state_index: %g\n") % state_index
            << boost::format("black win:   %g\n") % book.blackWinCount(state_index)
            << boost::format("white win:   %g\n") % book.whiteWinCount(state_index);

  std::cout << "\nTarget state:\n";
  printer.print(book.board(state_index));
  std::cout << "\n";

  WMoveContainer moves = book.moves(state_index);
  std::cout << boost::format("found %g moves\n") % moves.size();
  std::sort(moves.begin(), moves.end(), osl::book::WMoveSort());
  for (WMoveContainer::const_iterator each = moves.begin();
       each != moves.end(); ++each)
  {
    std::cout << boost::format("[%g] %g") % each->weight % each->move;
    const int next_index = each->stateIndex();
    std::cout << "\n";
    printer.print(book.board(next_index));
  }
}


void doMain(const std::string& file_name)
{
  osl::record::KanjiPrint printer(std::cerr, 
                                  std::shared_ptr<osl::record::Characters>(
                                            new osl::record::KIFCharacters())
                                  );
  if (vm.count("verbose"))
    std::cout << boost::format("Opening... %s\n ") % file_name;
  osl::book::WeightedBook book(file_name.c_str());

  if (vm.count("verbose"))
    std::cout << boost::format("Total states: %d\n") % book.totalState();
  bool states[book.totalState()]; // mark states that have been visited.
  memset(states, 0, sizeof(bool) * book.totalState());
  boost::progress_display progress(book.totalState());

  int state_index_to_compare = -1;
  if (state_to_compare)
    state_index_to_compare = book.stateIndex(*state_to_compare);

  typedef std::pair<int, int> state_depth_t;
  std::vector<state_depth_t> stateToVisit;

  if (vm.count("verbose"))
    std::cout << boost::format("Start index: %d\n)") % book.startState();
  stateToVisit.push_back(state_depth_t(book.startState(), 1)); // root is 1
  // depth-1手目からdepth手目のstate。depth手目はまだ指されていない（これか
  // らdepth手目）

  typedef std::pair<int, int> eval_depth_t;
  std::deque<eval_depth_t> evals;
  long finishing_games = 0;

  while (!stateToVisit.empty())
  {
    const state_depth_t state_depth = stateToVisit.back();
    if (vm.count("verbose"))
      std::cout << boost::format("Visiting... %d\n") % state_depth.first;
    const int stateIndex = state_depth.first;
    const int depth      = state_depth.second;
    stateToVisit.pop_back();
    states[stateIndex] = true;
    ++progress;


    // see if the state presents in the book
    if (state_to_compare && 
        state_index_to_compare == stateIndex)
      ++state_count;

    WMoveContainer moves = book.moves(stateIndex);
    if (vm.count("verbose"))
      std::cout << boost::format("  #moves... %d\n") % moves.size();
    
    // 自分（the_player）の手番では、良い手のみ指す
    // 相手はどんな手を指してくるか分からない
    if ( !moves.empty() &&
          ((the_player == osl::BLACK && depth % 2 == 1) ||
           (the_player == osl::WHITE && depth % 2 == 0)) )
    {
      std::sort(moves.begin(), moves.end(), osl::book::WMoveSort());
      int min = 1;
      if (is_determinate) 
      {
	min = moves.at(0).weight;
	if (depth <= non_determinate_depth) 
	{
	  for (int i=1; i<=std::min(is_determinate, (int)moves.size()-1); ++i) 
	  {
	    const int weight = moves.at(i).weight;
	    if ((double)weight < (double)moves.at(i-1).weight*ratio)
	      break;
	    min = weight;
	  }
	}
      }
      // Do not play 0-weighted moves.
      if (min == 0) min = 1;

      WMoveContainer::iterator each = moves.begin();
      for (; each != moves.end(); ++each)
      {
        if (each->weight < min)
          break;
      }
      moves.erase(each, moves.end());
    }

    if (moves.empty() || depth > max_depth) // found leaves
    {
      const osl::NumEffectState state(book.board(stateIndex));
      const int value = qsearch(state, osl::Move::PASS(alt(state.turn())));

      if ( (the_player == osl::BLACK && value < -1 * error_threshold) ||
           (the_player == osl::WHITE && value > error_threshold) )
      {
        ++finishing_games;
        if (is_dump)
        {
          std::cerr << std::endl;
          std::cerr << "eval: " << value << std::endl;
          printer.print(state);
	  std::cerr << "piece value:" << osl::PieceEval(state).value() << "\n" << state;
        }
      }
      else
      {
        evals.push_back(eval_depth_t(value, depth));
      }
      continue;
    }

    // 結果の再現性を高めるため、visitの順番を決める
    std::sort(moves.begin(), moves.end(), osl::book::WMoveMoveSort());

    // recursively search the tree
    for (std::vector<osl::book::WMove>::const_iterator each = moves.begin();
         each != moves.end(); ++each)
    {
      // consistancy check
      const osl::SimpleState state(book.board(stateIndex));
      const osl::hash::HashKey hash(state);
      const int nextIndex = each->stateIndex();
      const osl::SimpleState next_state(book.board(nextIndex));
      const osl::hash::HashKey next_hash(next_state);
      const osl::hash::HashKey moved_hash = hash.newMakeMove(each->move);
      if (moved_hash != next_hash)
        throw std::string("Illegal move found.");

      if (! states[nextIndex])
	stateToVisit.push_back(state_depth_t(nextIndex, depth+1));
    } // each wmove
  } // while loop

  // Show the result
  std::cout << std::endl;
  std::cout << boost::format("Book: %s\n") % file_name;
  std::cout << boost::format("Player: %s\n") % the_player;
  std::cout << "FU=128 points\n";
  std::cout << 
    boost::format("#states: %d (+ %d finishing games over %d points; max %d)\n") 
                  % evals.size() 
                  % finishing_games 
                  % error_threshold 
                  % max_depth;
  {
    std::cout << "Eval\n";
    std::deque<int> tmp;
    for (std::deque<eval_depth_t>::const_iterator each = evals.begin();
         each != evals.end(); ++each)
      tmp.push_back(each->first);
    showStatistics(tmp);
  }
  {
    std::cout << "Depth\n";
    std::deque<int> tmp;
    for (std::deque<eval_depth_t>::const_iterator each = evals.begin();
         each != evals.end(); ++each)
      tmp.push_back(each->second);
    showStatistics(tmp);
  }
  if (state_to_compare)
  {
    std::cout << "\nthe state hits: " << state_count << std::endl;
    printer.print(*state_to_compare);

    const int stateIndex  = book.stateIndex(*state_to_compare);
    const std::vector<int> parents = book.parents(stateIndex);
    if (parents.empty())
    {
      std::cout << "\nNo parent\n";
    }
    else
    {
      int i = 0;
      for (std::vector<int>::const_iterator each = parents.begin();
           each != parents.end(); ++each, ++i)
      {
        std::cout << boost::format("\n--- Parent: %g ---\n ") % i;
        showInfoOfState(book, *each);
      }
    }

  }
}


int main(int argc, char **argv)
{
  std::string player_str;
  std::string file_name;
  size_t csa_move_index;
  std::string csa_file_name;

  bp::options_description command_line_options;
  command_line_options.add_options()
    ("player,p", bp::value<std::string>(&player_str)->default_value("black"),
     "specify a player, black or white, in whose point of view the book is validated. "
     "default black.")
    ("input-file,f", bp::value<std::string>(&file_name)->default_value("./joseki.dat"),
     "a joseki file to validate.")
    ("dump", bp::value<bool>(&is_dump)->default_value(false),
     "dump finishing games' states")
    ("threshold", bp::value<int>(&error_threshold)->default_value(500),
     "threshold of evaluatoin value to recognize a finishing game.")
    ("determinate", bp::value<int>(&is_determinate)->default_value(0),
     "only search the top n moves.  (0 for all,  1 for determinate).")
    ("non-determinate-depth", bp::value<int>(&non_determinate_depth)->default_value(100),
     "use the best move where the depth is greater than this value")
    ("max-depth", bp::value<int>(&max_depth)->default_value(100),
     "do not go beyond this depth from the root")
    ("ratio", bp::value<double>(&ratio)->default_value(0.0),
     "skip move[i] (i >= n), if weight[n] < weight[n-1]*ratio")
    ("csa-move", bp::value<size_t>(&csa_move_index)->default_value(1),
     "n-th-move state in the csa file")
    ("csa", bp::value<std::string>(&csa_file_name)->default_value(""),
     "a csa file name. See if a state in the game exists in the book or not.")
    ("quick", bp::value<bool>(&is_quick)->default_value(false),
     "skip quiescence search.")
    ("verbose,v", "output verbose messages.")
    ("help,h", "show this help message.");
  bp::positional_options_description p;
  p.add("input-file", 1);

  try
  {
    bp::store(
      bp::command_line_parser(
	argc, argv).options(command_line_options).positional(p).run(), vm);
    bp::notify(vm);
    if (vm.count("help"))
    {
      printUsage(std::cout, argv, command_line_options);
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options\n"
	      << e.what() << std::endl;
    printUsage(std::cerr, argv, command_line_options);
    return 1;
  }

  if (player_str == "black")
    the_player = osl::BLACK;
  else if (player_str == "white")
    the_player = osl::WHITE;
  else
  {
    printUsage(std::cerr, argv, command_line_options);
    return 1;
  }

  if (!csa_file_name.empty())
  {
    is_quick = true;
    const osl::CsaFile csa(csa_file_name);
    const osl::Record record = csa.load();
    const auto moves = record.moves();
    const auto initial_state = record.initialState();
    state_to_compare.reset(new osl::NumEffectState(initial_state));

    if (csa_move_index < 1) csa_move_index = 1;
    if (csa_move_index > moves.size()) csa_move_index = moves.size();
    if ( (the_player == osl::BLACK && csa_move_index%2 == 0) ||
         (the_player == osl::WHITE && csa_move_index%2 == 1) )
    {
      std::cout << "Invalid csa move index: " << csa_move_index << std::endl;
      return -1;
    }
    for (size_t i=0; i < csa_move_index; i++)
    {
       const osl::Move& move = moves[i];
       state_to_compare->makeMove(move);
    }
  }

  doMain(file_name);

  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
