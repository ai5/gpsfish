// compare-book.cc
#include "osl/book/openingBook.h"
#include "osl/csa.h"
#include "osl/record/kanjiPrint.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceSearch2.tcc"
#include "osl/search/simpleHashTable.h"
#include "osl/eval/pieceEval.h"
//#include "osl/misc/math.h"
#include "osl/search/fixedEval.h"
#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include <boost/format.hpp>
#include <unordered_map>
#include <iostream>
#include <deque>

typedef std::vector<osl::book::WMove> WMoveContainer;

osl::Player the_player = osl::BLACK;
std::string dump_mode = "none";
int is_determinate = 0;	   // test only top n moves.  0 for all
int max_depth, non_determinate_depth;
double ratio;		   // use moves[n+1] when the weight[n+1] >= ratio*weight[n]

size_t state_count = 0;

void printUsage(std::ostream& out, 
                char **argv,
                const boost::program_options::options_description& command_line_options)
{
  out << "Usage: " << argv[0] << " [options] <book-a.dat> <book-b.dat>\n"
      << command_line_options 
      << std::endl;
}

typedef std::unordered_map<osl::HashKey,int,std::hash<osl::HashKey>> table_t;
void store(osl::book::WeightedBook& book, table_t& table, std::vector<int>& parents)
{
  WMoveContainer moves = book.moves(book.startState());
  parents.resize(book.totalState());
  std::fill(parents.begin(), parents.end(), -1);
  boost::progress_display progress(book.totalState());

  typedef std::pair<int, int> state_depth_t;
  std::deque<state_depth_t> stateToVisit;
  stateToVisit.push_back(state_depth_t(book.startState(), 1)); // root is 1
  // depth-1手目からdepth手目のstate。depth手目はまだ指されていない（これか
  // らdepth手目）

  typedef std::pair<int, int> eval_depth_t;
  long leaves = 0;
  int depth_found = 0;
  while (!stateToVisit.empty())
  {
    const state_depth_t state_depth = stateToVisit.front();
    const int stateIndex = state_depth.first;
    const int depth      = state_depth.second;
    stateToVisit.pop_front();
    ++progress;
    assert(parents[stateIndex] >= 0 || stateIndex == book.startState());

    depth_found = std::max(depth_found, depth);
    WMoveContainer moves = book.moves(stateIndex);
    
    // 自分（the_player）の手番では、良い手のみ指す
    // 相手はどんな手を指してくるか分からない
    std::sort(moves.begin(), moves.end(), osl::book::WMoveWeightMoveSort());
    if ( !moves.empty() &&
          ((the_player == osl::BLACK && depth % 2 == 1) ||
           (the_player == osl::WHITE && depth % 2 == 0)) )
    {
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
      ++leaves;
      continue;
    }

    if (moves[0].weight) {
      // not leaf
      const osl::NumEffectState state(book.board(stateIndex));
      const osl::HashKey key(state);
      table[key] = stateIndex;
    }


    // recursively search the tree
    for (std::vector<osl::book::WMove>::const_iterator each = moves.begin();
         each != moves.end(); ++each)
    {
      const int nextIndex = each->stateIndex();
      if (parents[nextIndex] < 0) {
	parents[nextIndex] = stateIndex;
	stateToVisit.push_back(state_depth_t(nextIndex, depth+1));
      }
    } // each wmove
  } // while loop

  // Show the result
  std::cout << std::endl;
  std::cout << boost::format("Player: %s\n") % the_player;
  std::cout << 
    boost::format("#leaves: %d, max_depth %d\n") 
                  % leaves 
                  % depth_found;
}

void show_moves(const char *name, osl::book::WeightedBook& book, int node)
{
  WMoveContainer moves = book.moves(node);  
  std::sort(moves.begin(), moves.end(), osl::book::WMoveWeightMoveSort());

  if (! moves.empty() && moves[0].weight) {
    std::cout << name;
    for (size_t i=0; i<moves.size(); ++i) {
      if (moves[i].weight == 0)
	break;
      const int next_state_index = moves[i].stateIndex();
      const int black_win        = book.blackWinCount(next_state_index);
      const int white_win        = book.whiteWinCount(next_state_index);
      std::cout << "  " << osl::csa::show(moves[i].move)
		<< "(" << moves[i].weight << "," << black_win << "," << white_win << ")";
    }
    std::cout << "\n";
  }
}

void show_history(const osl::MoveVector& history)
{
  std::cout << "[" << history.size() << "]";
  for (size_t i=0; i<history.size(); ++i)
    std::cout << " " << osl::csa::show(history[i]);
  std::cout << std::endl;
}

osl::MoveVector make_history(osl::book::WeightedBook& book, const std::vector<int>& parents, int node)
{
  std::vector<int> history;
  history.push_back(node);
  while (parents[node] >= 0) {
    node = parents[node];
    history.push_back(node);
  }
  std::reverse(history.begin(), history.end());
  assert(book.startState() == history[0]);

  osl::MoveVector result;  
  for (size_t i=0; i<history.size()-1; ++i) {
    const WMoveContainer& moves = book.moves(history[i]);  
    for (WMoveContainer::const_iterator p=moves.begin(); p!=moves.end(); ++p) {
      if (p->stateIndex() != history[i+1])
	continue;
      result.push_back(p->move);
      break;
    }
  }
  return result;
}

void dump(osl::book::WeightedBook& book_a, const std::vector<int>& parents_a, int node_a, 
	  osl::book::WeightedBook& book_b, const std::vector<int>& parents_b, int node_b)
{
  const osl::NumEffectState state(book_a.board(node_a));
  osl::record::KanjiPrint printer(std::cout, 
				  std::shared_ptr<osl::record::Characters>(
				    new osl::record::KIFCharacters())
    );
  printer.print(state);
  const osl::MoveVector history_a = make_history(book_a, parents_a, node_a);
  const osl::MoveVector history_b = make_history(book_b, parents_b, node_b);
  show_history(history_a);
  if (! (history_a == history_b))
    show_history(history_b);
  show_moves("a", book_a, node_a);
  show_moves("b", book_b, node_b);
}

void dump(const char *name, osl::book::WeightedBook& book, const std::vector<int>& parents, int node)
{
  const osl::NumEffectState state(book.board(node));
  osl::record::KanjiPrint printer(std::cout, 
				  std::shared_ptr<osl::record::Characters>(
				    new osl::record::KIFCharacters())
    );
  printer.print(state);
  show_history(make_history(book, parents, node));
  show_moves(name, book, node);
}

bool is_same_node(osl::book::WeightedBook& book_a, int node_a, 
		  osl::book::WeightedBook& book_b, int node_b)
{
  WMoveContainer moves_a = book_a.moves(node_a);
  WMoveContainer moves_b = book_b.moves(node_b);
  
  std::sort(moves_a.begin(), moves_a.end(), osl::book::WMoveWeightMoveSort());
  std::sort(moves_b.begin(), moves_b.end(), osl::book::WMoveWeightMoveSort());

  size_t i=0;
  for (; i<std::min(moves_a.size(), moves_b.size()); ++i) {
    if (moves_a[i].weight == 0)
      return moves_b[i].weight == 0;
    if (moves_b[i].weight == 0)
      return false;
    if (moves_a[i].move != moves_b[i].move)
      return false;
  }
  if (i == moves_a.size())
    return i == moves_b.size() || moves_b[i].weight == 0;
  return moves_a[i].weight == 0;
}

void compare(osl::book::WeightedBook& book_a, const table_t& table_a, const std::vector<int>& parents_a,
	     osl::book::WeightedBook& book_b, const table_t& table_b, const std::vector<int>& parents_b)
{
  long only_a = 0, only_b = 0, same = 0, diff = 0;
  for (table_t::const_iterator p=table_a.begin(); p!=table_a.end(); ++p) {
    table_t::const_iterator q=table_b.find(p->first);
    if (q == table_b.end()) {
      ++only_a;
      if (dump_mode == "a")
	dump("a", book_a, parents_a, p->second);	
      continue;
    }
    if (is_same_node(book_a, p->second, book_b, q->second))
      ++same;
    else {
      ++diff;
      if (dump_mode == "common")
	dump(book_a, parents_a, p->second, 
	     book_b, parents_b, q->second);	
    }
  }
  for (table_t::const_iterator p=table_b.begin(); p!=table_b.end(); ++p) {
    table_t::const_iterator q=table_a.find(p->first);
    if (q == table_a.end()) {
      ++only_b;
      if (dump_mode == "b")
	dump("b", book_b, parents_b, p->second);	
      continue;
    }
  }
  std::cout << "same " << same << " diff " << diff
	    << " only-in-a " << only_a << " only-in-b " << only_b << std::endl;
}

int main(int argc, char **argv)
{
  std::string player_str;

  namespace bp = boost::program_options;
  bp::variables_map vm;
  bp::options_description command_line_options;
  command_line_options.add_options()
    ("player,p", bp::value<std::string>(&player_str)->default_value("black"),
     "specify a player, black or white, in whose point of view the book is validated. "
     "default black.")
    ("input-file,f", bp::value<std::vector<std::string> >(),
     "a joseki file to validate.")
    ("dump", bp::value<std::string>(&dump_mode)->default_value(dump_mode),
     "common: dump positions where two books have different moves\n"
     "(a|b): dump positions registered to only book_[ab]\n")
    ("determinate", bp::value<int>(&is_determinate)->default_value(0),
     "only search the top n moves.  (0 for all,  1 for determinate).")
    ("non-determinate-depth", bp::value<int>(&non_determinate_depth)->default_value(100),
     "use the best move where the depth is greater than this value")
    ("max-depth", bp::value<int>(&max_depth)->default_value(100),
     "do not go beyond this depth from the root")
    ("ratio", bp::value<double>(&ratio)->default_value(0.0),
     "skip move[i] (i >= n), if weight[n] < weight[n-1]*ratio")
    ("help,h", "show this help message.");
  bp::positional_options_description p;
  p.add("input-file", -1);

  std::vector<std::string> filenames;
  try
  {
    bp::store(
      bp::command_line_parser(
	argc, argv).options(command_line_options).positional(p).run(), vm);
    bp::notify(vm);
    filenames = vm["input-file"].as<std::vector<std::string> >();
    if (vm.count("help") || filenames.size() != 2 
	|| (dump_mode != "none" && dump_mode != "a" && dump_mode != "b" && dump_mode != "common"))
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

  osl::book::WeightedBook book_a(filenames[0].c_str()), book_b(filenames[1].c_str());
  osl::CArray<std::vector<int>,2> parents;
  osl::CArray<table_t,2> tables;
  std::cout << boost::format("Book: %s\n") % filenames[0];
  store(book_a, tables[0], parents[0]);
  std::cout << boost::format("Book: %s\n") % filenames[1];
  store(book_b, tables[1], parents[1]);

  compare(book_a, tables[0], parents[0], book_b, tables[1], parents[1]);
  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
