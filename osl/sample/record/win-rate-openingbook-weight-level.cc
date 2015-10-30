#include "osl/oslConfig.h"
#include "osl/game_playing/weightTracer.h"
#include "osl/record/csaRecord.h"
#include "osl/record/checkDuplicate.h"
#include "osl/book/openingBook.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <boost/multi_array.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <vector>

using namespace osl;

enum GameResult
{
  BLACK_WIN = 0,
  WHITE_WIN,
  OTHERS
};

struct WinLoss
{
  unsigned int wins;
  unsigned int losses;

  WinLoss() 
    : wins(0), losses(0)
  {}

  double getRate() const
  {
    if (isEmpty())
      return 0.0;

    return 1.0*wins/(wins+losses);
  }

  bool isEmpty() const
  {
    return (wins + losses) == 0;
  }
};

std::ostream& operator<<(std::ostream& out, const WinLoss& wl) 
{
  out << boost::format("%5.3f (#wins=%6d, #losses=%6d)") 
         % wl.getRate() % wl.wins % wl.losses;
  return out;
}

struct Result 
{
  enum {MAX_LEVEL = 99,
        MAX_DEPTH = 999};
  /**< n-th move, n-th level, [wins, losses] */
  typedef boost::multi_array<unsigned int, 3> array_t;
  array_t winloss;
  unsigned int top_level;
  unsigned int top_depth;

  Result()
    : winloss(boost::extents[MAX_DEPTH][MAX_LEVEL][2]),
      top_level(0), top_depth(0)
    {}
  
  void add(const unsigned int depth,
           const unsigned int level,
           const bool win)
  {
    assert(depth < MAX_DEPTH);
    assert(level < MAX_LEVEL);
    if (win)
      winloss[depth][level][0] += 1;
    else
      winloss[depth][level][1] += 1;

    top_level = std::max(top_level, level);
    top_depth = std::max(top_depth, depth);
  }

  bool printAtDepth(std::ostream& out, const unsigned int depth) const;
  void printByLevel(std::ostream& out) const;
  void printByDepth(std::ostream& out) const;
  void showLevels(std::ostream& out,
                  std::vector<WinLoss>& vector) const;
};

void Result::showLevels(std::ostream& out,
                        std::vector<WinLoss>& vector) const
{
  // GC
  std::vector<WinLoss>::reverse_iterator empty = vector.rbegin();
  for (/*none*/; empty != vector.rend(); ++empty) {
    if (!empty->isEmpty())
      break;
  }
  vector.erase(empty.base(), vector.end());

  unsigned int level = 1;
  for (const WinLoss& wl: vector) {
    out << boost::format("%2d: %s\n") % level++ % wl;
  }
}

bool Result::printAtDepth(std::ostream& out,
                          const unsigned int depth) const
{
  std::vector<WinLoss> vector;
  for(unsigned int level=0; level < MAX_LEVEL; ++level) {
    WinLoss wl;
    wl.wins   += winloss[depth][level][0];
    wl.losses += winloss[depth][level][1];
    vector.push_back(wl);
  }

  showLevels(out, vector);
  return vector.empty();
}

void Result::printByDepth(std::ostream& out) const
{
  for (unsigned int depth=1; depth < top_depth; ++depth) {
    out << boost::format("\n>> Depth %2d\n") % depth;
    printAtDepth(out, depth);
  }
}

void Result::printByLevel(std::ostream& out) const
{
  std::vector<WinLoss> vector;
  for(unsigned int level=0; level < MAX_LEVEL; ++level) {
    WinLoss wl;
    for (unsigned int depth=0; depth < MAX_DEPTH; ++depth) {
      wl.wins   += winloss[depth][level][0];
      wl.losses += winloss[depth][level][1];
    }
    vector.push_back(wl);
  }

  showLevels(out, vector);
}

// ==============================================
// Global variables
// ==============================================
static Result result;
osl::book::WeightedBook book(osl::OslConfig::openingBook());


// ==============================================
// Functions
// ==============================================

GameResult getGameResult(const std::string& csa_file,
                         const std::vector<Move>& moves)
{
  std::ifstream in(csa_file.c_str());
  if (!in)
  {
    std::cerr << "File not found: " << csa_file << "\n";
    exit(-1);
  }
  
  bool hit = false;
  std::string line;
  while (std::getline(in, line))
  {
    if (line.find("%TORYO") != std::string::npos)
    {
      hit = true;
      break;
    }
  }

  if (hit)
  {
    return (moves.size() % 2 == 1 ? BLACK_WIN : WHITE_WIN);
  }
  else
  {
    return OTHERS;
  }
}


void increment(const std::vector<Move>& moves, 
               const Player player,
               const bool win)
{
  int stateIndex = ::book.startState();
  Player turn = BLACK;
  unsigned int depth = 1;

  for (const Move& move: moves) {
    osl::book::WeightedBook::WMoveContainer wmoves =
	::book.moves(stateIndex, (player == turn ? false : true));
    std::sort(wmoves.begin(), wmoves.end(), 
              osl::book::WMoveSort());

    /*
     * It ends if a move that is not included in the book is played.
     */
    osl::book::WeightedBook::WMoveContainer::iterator found =
      wmoves.begin();
    for (/*none*/; found != wmoves.end(); ++found) {
      if (found->move == move) 
        break;
    } 
    if (found == wmoves.end())
      return; // finish this record

    /*
     * Increment 
     */
    if (turn == player) {
      const unsigned int level = std::distance(wmoves.begin(), found); // 1, 2, ...
      result.add(depth, level, win);
    }

    /*
     * Prepare for the next iteration
     */
    turn = alt(turn);
    depth += 1;
    stateIndex = found->stateIndex();
  } // each move
}


void readFile(const std::string& player_name,
              const std::string& csa_file,
              osl::record::CheckDuplicate& duplicates)
{
  const osl::CsaFile csa(csa_file);
  const auto& record = csa.load();
  const auto moves = record.moves();
 
  const GameResult gr = getGameResult(csa_file, moves);
  if (gr == OTHERS)
    return;

  if (duplicates.regist(moves))
    return;

  const std::string& black = record.player[BLACK];
  const std::string& white = record.player[WHITE];

  Player player = BLACK;
  bool win = true;
  if (black.find(player_name) != std::string::npos) {
    player = BLACK;
    win = (gr == BLACK_WIN);
  }
  else if (white.find(player_name) != std::string::npos) {
    player = WHITE;
    win = (gr == WHITE_WIN);
  }
  else {
    std::cerr << "Ignore this play: " << csa_file << "\n";
    return;
  }

  increment(moves, player, win);
}


int main(int argc, char **argv)
{
  std::string player_name;

  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("input-file", boost::program_options::value<std::vector<std::string> >(),
     "input files in the CSA format")
    ("player", boost::program_options::value<std::string>(&player_name)->default_value("gps"),
     "input files in the CSA format")
    ("help", "Show help message");
  boost::program_options::variables_map vm;
  boost::program_options::positional_options_description p;
  p.add("input-file", -1);

  try
  {
    boost::program_options::store(
      boost::program_options::command_line_parser(
	argc, argv).options(command_line_options).positional(p).run(), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
      std::cerr << "Usage: " << argv[0] << " [options] csa-file [...]\n";
      std::cerr << "       " << argv[0] << " [options]\n";
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0] << " [options] csa-file [...]\n";
    std::cerr << "       " << argv[0] << " [options]\n";
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  std::vector<std::string> files;
  if (vm.count("input-file"))
  {
    const std::vector<std::string> temp = vm["input-file"].as<std::vector<std::string> >();
    files.insert(files.end(), temp.begin(), temp.end());
  }
  else
  {
    std::string line;
    while(std::getline(std::cin , line))
    {
      boost::algorithm::trim(line);
      files.push_back(line);
    }
  }

  osl::record::CheckDuplicate check_duplicate;

  for (const std::string& file: files)
  {
    readFile(player_name, file, check_duplicate);
  }

  result.printByLevel(std::cout);
  result.printByDepth(std::cout);

  std::locale::global(std::locale(""));
  check_duplicate.print(std::cout);

  return 0;
}

/* vim: set ts=2 sw=2 ft=cpp : */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
