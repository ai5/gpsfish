#include "osl/record/csaRecord.h"
#include "osl/record/checkDuplicate.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/functional/hash.hpp>
#include <boost/format.hpp>
#include <boost/multi_array.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <vector>

/**< max players */
static const unsigned int MAX_PLAYERS = 20; 

/**< player and his/her id */
typedef std::unordered_map<std::string, unsigned int> players_t;
static players_t players;

/**< player_a, player_b, a's black 0 or white 1, [wins, losses, others] */
typedef boost::multi_array<unsigned int, 4> array_t;
array_t winloss(boost::extents[MAX_PLAYERS][MAX_PLAYERS][2][3]);

enum GameResult
{
  BLACK_WIN = 0,
  WHITE_WIN,
  OTHERS
};

const std::string& getPlayerName(const unsigned int id)
{
  players_t::const_iterator each_player = players.begin();
  for (; each_player != players.end(); ++each_player)
  {
    if (each_player->second == id)
      break;
  }
  assert(each_player != players.end());
  return each_player->first;
}

unsigned int setPlayer(const std::string& player)
{
  players_t::const_iterator hit = players.find(player);
  if (hit == players.end())
  {
    // new player
    if (players.size() >= MAX_PLAYERS)
    {
      std::cerr << "No longer accomodate a new player.\n";
      exit(-1);
    }
    const unsigned int new_id = players.size();
    players.insert(std::make_pair(player, new_id));
    return new_id;
  }
  else
  {
    return hit->second;
  }
}

void increment(unsigned int black,
               unsigned int white,
               GameResult gr)
{
  // player_a is black
  winloss[black][white][0][static_cast<unsigned int>(gr)] += 1;
  // player_a is white
  if (gr == BLACK_WIN)
  {
    winloss[white][black][1][1] += 1;  // player_a lost
  }
  else if (gr == WHITE_WIN)
  {
    winloss[white][black][1][0] += 1;  // player_a won
  }
  else
  {
    assert(gr == OTHERS);
    winloss[white][black][1][2] += 1;  // others
  }
}

GameResult getGameResult(const std::string& csa_file,
                         const std::vector<osl::Move>& moves)
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


void readFile(const std::string& csa_file,
              osl::record::CheckDuplicate& duplicates)
{
  const osl::CsaFile csa(csa_file);
  const auto& record = csa.load();
  const auto moves = record.moves();
 
  if (duplicates.regist(moves))
    return;

  const std::string& black = record.player[osl::BLACK];
  const std::string& white = record.player[osl::WHITE];

  const unsigned int black_id = setPlayer(black);
  const unsigned int white_id = setPlayer(white);
  
  const GameResult gr = getGameResult(csa_file, moves);
  increment(black_id, white_id, gr);
}

void printTotal(std::ostream& out)
{
  out << "=== Total [ #wins / #losses / #others ] ===\n";

  for (unsigned int player_a = 0; player_a < players.size(); ++player_a)
  {

    for (unsigned int player_b = 0; player_b < players.size(); ++player_b)
    {
      if (player_a == player_b)
        continue;

      out << boost::format("%- 17s ") % getPlayerName(player_a);

      unsigned int wins = 0, losses = 0, others = 0;
      wins += winloss[player_a][player_b][0][0];   // a is black
      wins += winloss[player_a][player_b][1][0];   // a is white
      losses += winloss[player_a][player_b][0][1]; // a is black
      losses += winloss[player_a][player_b][1][1];
      others += winloss[player_a][player_b][0][2]; // a is black
      others += winloss[player_a][player_b][1][2];

      out << boost::format("%5d/%5d/%5d ") 
        % wins % losses % others;

      out << boost::format("%- 17s ") % getPlayerName(player_b);
    }
    out << "\n";
  }
  out << "\n";
}

void printResult(std::ostream& out)
{
  out << "=== Left players are BLACK [ #wins / #losses / #others ] ===\n";
  out << boost::format("%= 17s ") % "";
  for (unsigned int player_a = 0; player_a < players.size(); ++player_a)
  {
    out << boost::format("%= 17s ") % getPlayerName(player_a);
  }
  out << "\n";

  for (unsigned int player_a = 0; player_a < players.size(); ++player_a)
  {
    out << boost::format("%= 17s ") % getPlayerName(player_a);

    for (unsigned int player_b = 0; player_b < players.size(); ++player_b)
    {
      if (player_a == player_b)
      {
        out << boost::format("%= 17s ") % "-";
        continue;
      }

      out << boost::format("%5d/%5d/%5d ") 
        % winloss[player_a][player_b][0][0]
        % winloss[player_a][player_b][0][1]
        % winloss[player_a][player_b][0][2];
    }
    out << "\n";
  }
  out << "\n";
}


int main(int argc, char **argv)
{
  std::string kisen_filename;

  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("input-file", boost::program_options::value<std::vector<std::string> >(),
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
    readFile(file, check_duplicate);
  }

  std::locale::global(std::locale(""));
  printResult(std::cout);
  printTotal(std::cout);
  check_duplicate.print(std::cout);

  return 0;
}

/* vim: set ts=2 sw=2 ft=cpp : */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
