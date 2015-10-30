#include <boost/program_options.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

#include <osl/misc/carray.h>
#include <osl/misc/carray2d.h>
#include "osl/pieceStand.h"

struct WinLoose
{
public:
  int win, loose;
  WinLoose() : win(0), loose(0)
  {
  }
  void add(const std::string &result)
  {
    if (result == "+")
      ++win;
    else if (result == "-")
      ++loose;
    else if (result == "*")
      ++win, ++loose;
  }
};

enum VALUE
{
  RESULT = 0,
  EVAL,
  BLACK_PROGRESS,
  WHITE_PROGRESS,
  BLACK_DEFENSE,
  WHITE_DEFENSE,
  PROGRESS,
  BLACK_KING_X,
  BLACK_KING_Y,
  WHITE_KING_X,
  WHITE_KING_Y,
  VALUE_START,
};

class WinRatio
{
  WinLoose stat[600];

public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    add(result, standard_values[EVAL]);
  }
  void add(const std::string &result, int eval)
  {
    eval = eval / (16 * 128 / 2);
    stat[std::max(0, std::min(eval + 300, 599))].add(result);
  }
  void output(std::ostream &os)
  {
    for (int i = 0; i < 600; ++i)
    {
      if (stat[i].win + stat[i].loose >= 100)
      {
	os << i - 300 << " "
	   << (100.0 * stat[i].win / (stat[i].win + stat[i].loose))
	   << std::endl;
      }
    }
  }
  virtual void output(const std::string filename)
  {
    std::ofstream out(filename.c_str());
    output(out);
  }
};

class WinRatioForT : public WinRatio
{
public:
  virtual void output(const std::string filename)
  {
    WinRatio::output(filename.substr(0, filename.length() - 1) + ".txt");
  }
};

class WinRatioBase
{
public:
  virtual void add(const std::string& result,
		   const std::vector<int>& standard_values,
		   const std::vector<std::string>& values) = 0;
  virtual void output(const std::string &prefix = "") = 0;
  virtual ~WinRatioBase() {}
};

class AllWinRatio : public WinRatioBase
{
  WinRatio ratio;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values[EVAL]);
  }
  void output(const std::string &prefix = "")
  {
    std::ofstream out("all.txt");
    ratio.output(out);
  }
};

class ProgressWinRatio : public WinRatioBase
{
  osl::misc::CArray<WinRatio, 16> progresses;
  std::string prefix_;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    progresses[standard_values[PROGRESS]].add(result, standard_values[EVAL]);
  }
  void output(const std::string &prefix = "")
  {
    for (size_t i = 0; i < progresses.size(); ++i)
    {
      const std::string filename =
	(boost::format("%1%progress-%2%.txt") % prefix.c_str() % i).str();
      std::ofstream out(filename.c_str());
      progresses[i].output(out);
    }
  }
};

template <int INDEX, int VALUE_SIZE>
class PieceWinRatio : public WinRatioBase
{
  osl::misc::CArray2d<WinRatio, 2, VALUE_SIZE> pieces;
  std::string prefix_;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    int index = atoi(values[INDEX].c_str());
    if (index > 0)
      pieces[0][index].add(result, standard_values[EVAL]);
    else
      pieces[1][-index].add(result, standard_values[EVAL]);
  }
  void output(const std::string &prefix = "")
  {
    for (size_t i = 0; i < pieces.size1(); ++i)
    {
      for (size_t j = 0; j < pieces.size2(); ++j)
      {
	const std::string filename =
	  (boost::format("%1%%2%-%3%.txt") % prefix.c_str()
	   % (i == 0 ? "black" : "white")
	   % j).str();
	pieces[i][j].output(filename);
      }
    }
  }
};

template<int INDEX>
class PieceStandWinRatio : public WinRatioBase
{
  osl::misc::CArray2d<WinRatio, 14, 2> pieces;
  std::string prefix_;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    for (size_t i = 0; i < pieces.size1(); ++i)
    {
      int index = (atoi(values[INDEX + i].c_str()) > 0) ? 1 : 0;
      pieces[i][index].add(result, standard_values[EVAL]);
    }
  }
  void output(const std::string &prefix = "")
  {
    for (size_t i = 0; i < pieces.size1(); ++i)
    {
      for (size_t j = 0; j < pieces.size2(); ++j)
      {
	const std::string filename =
	  (boost::format("%1%%2%-%3%-%4%.txt") % prefix.c_str()
	   % osl::PieceStand::order[i % osl::PieceStand::order.size()]
	   % (i < osl::PieceStand::order.size() ? "black" : "white")
	   % j).str();
	pieces[i][j].output(filename);
      }
    }
  }
};

template<int INDEX, class Ratio>
class RookPieceStandWinRatio : public WinRatioBase
{
  osl::misc::CArray<Ratio, 5> pieces;
  std::string prefix_;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    int index = atoi(values[INDEX].c_str()) -
      atoi(values[INDEX + osl::PieceStand::order.size()].c_str()) + 2;
//    std::cout << "index " << index << " " << values[INDEX] << " " << values[INDEX + osl::PieceStand::order.size()] << std::endl;
    pieces[index].add(result, standard_values, values);
  }
  void output(const std::string &prefix = "")
  {
    for (size_t i = 0; i < pieces.size(); ++i)
    {
      const std::string new_prefix =
	(boost::format("%1%rook-%2%-") % prefix.c_str()
	 % i).str();
      pieces[i].output(new_prefix);
    }
  }
};

template<int INDEX, class Ratio>
class RookPieceStandNoGSWinRatio : public WinRatioBase
{
  osl::misc::CArray<Ratio, 2> pieces;
  std::string prefix_;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    if (values[INDEX] != "0" &&
	values[INDEX + 2] == "0" && values[INDEX + 3] == "0")
      pieces[0].add(result, standard_values, values);
    if (values[INDEX + osl::PieceStand::order.size()] != "0" &&
	values[INDEX + 2 + osl::PieceStand::order.size()] == "0" &&
	values[INDEX + 3 + osl::PieceStand::order.size()] == "0")
      pieces[1].add(result, standard_values, values);
  }
  void output(const std::string &prefix = "")
  {
    for (size_t i = 0; i < pieces.size(); ++i)
    {
      const std::string new_prefix =
	(boost::format("%1%rook-nogs-%2%-") % prefix.c_str()
	 % (i == 0 ? "black" : "white")).str();
      pieces[i].output(new_prefix);
    }
  }
};

class Bishop2WinRatio : public WinRatioBase
{
  AllWinRatio ratio;
  osl::misc::CArray<WinRatio, 2> bishop2;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values, values);
    // OWNER, RAW
    if (values[VALUE_START] == "+")
    {
      bishop2[0].add(result, standard_values[EVAL]);
    }
    else if(values[VALUE_START] == "-")
    {
      bishop2[1].add(result, standard_values[EVAL]);
    }
  }
  void output(const std::string &prefix = "")
  {
    ratio.output();
    const char *filename[] = { "b2-black.txt",
			       "b2-white.txt" };
    for (size_t i = 0; i < bishop2.size(); ++i)
    {
      std::ofstream f(filename[i]);
      bishop2[i].output(f);
    }
  }
};

class UnsupportedGSWinRatio : public WinRatioBase
{
  AllWinRatio ratio;
  osl::misc::CArray2d<WinRatio, 2, 8> gs;
  osl::misc::CArray2d<WinRatio, 2, 2> gold;
  osl::misc::CArray2d<WinRatio, 2, 2> silver;
  //osl::misc::CArray2d<ProgressWinRatio, 2, 8> gs;
  enum
  {
    BLACK_GS = 0,
    WHITE_GS,
    BLACK_GOLD,
    WHITE_GOLD,
    BLACK_SILVER,
    WHITE_SILVER
  };
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values, values);
    std::vector<int> target_values;
    for (int i = BLACK_GS; i <= WHITE_SILVER; ++i)
    {
      target_values.push_back(atoi(values[VALUE_START + i].c_str()));
    }
    // black_gs, white_gs 
    int black = target_values[BLACK_GS];
    int white = target_values[WHITE_GS];
    if (black >= 1 && white == 0)
      gs[0][black].add(result, standard_values, values);
    if (white >= 1 && black == 0)
      gs[1][white].add(result, standard_values, values);
    if (target_values[BLACK_GOLD] >= 1)
      gold[0][1].add(result, standard_values, values);
    else
      gold[0][0].add(result, standard_values, values);
    if (target_values[WHITE_GOLD] >= 1)
      gold[1][1].add(result, standard_values, values);
    else
      gold[1][0].add(result, standard_values, values);
    if (target_values[BLACK_SILVER] >= 1)
      silver[0][1].add(result, standard_values, values);
    else
      silver[0][0].add(result, standard_values, values);
    if (target_values[WHITE_SILVER] >= 1)
      silver[1][1].add(result, standard_values, values);
    else
      silver[1][0].add(result, standard_values, values);
  }
  void output(const std::string &prefix = "")
  {
    ratio.output();
    for (size_t j = 0; j < gs.size1(); ++j)
    {
      for (size_t i = 0; i < gs.size2(); ++i)
      {
#if 1
	const std::string filename =
	  (boost::format("%1%-gs-%2%.txt")
	   % (j == 0 ? "black" : "white") % i).str();
	std::ofstream out(filename.c_str());
	gs[j][i].output(out);
#else
	const std::string prefix =
	  (boost::format("%1%-gs-%2%-")
	   % (j == 0 ? "black" : "white") % i).str();
	gs[j][i].output(prefix);
#endif
      }
    }
    for (size_t i = 0; i < gold.size1(); ++i)
    {
      for (size_t j = 0; j < gold.size2(); ++j)
      {
	std::ofstream out(
	  (boost::format("%1%-gold-%2%.txt") % (i == 0 ? "black" : "white")
	   % j).str().c_str());
	gold[i][j].output(out);
      }
    }
    for (size_t i = 0; i < silver.size1(); ++i)
    {
      for (size_t j = 0; j < silver.size2(); ++j)
      {
	std::ofstream out(
	  (boost::format("%1%-silver-%2%.txt") % (i == 0 ? "black" : "white")
	   % j).str().c_str());
	silver[i][j].output(out);
      }
    }
  }
};

template <class Ratio>
class TurnWinRatio : public WinRatioBase
{
  AllWinRatio ratio;
  osl::misc::CArray<Ratio, 2> turn;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values, values);
    if (values[VALUE_START] == "+")
    {
      turn[0].add(result, standard_values, values);
    }
    else
    {
      turn[1].add(result, standard_values, values);
    }
  }
  void output(const std::string &prefix = "")
  {
    ratio.output();
    for (size_t i = 0; i < turn.size(); ++i)
    {
      std::string filename = (boost::format("%1%-")
			      % (i == 0 ? "black" : "white")).str();
      turn[i].output(filename);
    }
  }
};

template<class Ratio>
class King8WinRatio : public WinRatioBase
{
  enum
  {
    BLACK_UL = 0,
    BLACK_U,
    BLACK_UR,
    BLACK_L,
    BLACK_C,
    BLACK_R,
    BLACK_DL,
    BLACK_D,
    BLACK_DR,
    WHITE_DR,
    WHITE_D,
    WHITE_DL,
    WHITE_R,
    WHITE_C,
    WHITE_L,
    WHITE_UR,
    WHITE_U,
    WHITE_UL,
    PLACE_MAX
  };
  AllWinRatio ratio;
  osl::misc::CArray<Ratio, PLACE_MAX> effect_ratio;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values, values);
    for (size_t i = 0; i < PLACE_MAX; ++i)
    {
      if (atoi(values[VALUE_START + i].c_str()) >= 0)
	effect_ratio[i].add(result, standard_values, values);
    }
  }
  void output(const std::string &prefix = "")
  {
    ratio.output();
    const char *filename[] = 
      {
	"BLACK_UL",
	"BLACK_U",
	"BLACK_UR",
	"BLACK_L",
	"BLACK_C",
	"BLACK_R",
	"BLACK_DL",
	"BLACK_D",
	"BLACK_DR",
	"WHITE_DR",
	"WHITE_D",
	"WHITE_DL",
	"WHITE_R",
	"WHITE_C",
	"WHITE_L",
	"WHITE_UR",
	"WHITE_U",
	"WHITE_UL"
      };
    for (size_t i = 0; i < effect_ratio.size(); ++i)
    {
      std::string outname = (boost::format("%1%-")
			     % filename[i]).str();
      effect_ratio[i].output(outname);
    }
  }
};

class King8TestWinRatio : public WinRatioBase
{
  enum
  {
    BLACK_UL = 0,
    BLACK_U,
    BLACK_UR,
    BLACK_L,
    BLACK_C,
    BLACK_R,
    BLACK_DL,
    BLACK_D,
    BLACK_DR,
    WHITE_DR,
    WHITE_D,
    WHITE_DL,
    WHITE_R,
    WHITE_C,
    WHITE_L,
    WHITE_UR,
    WHITE_U,
    WHITE_UL,
    PLACE_MAX
  };
  ProgressWinRatio all_ratio;
  ProgressWinRatio ratio;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    all_ratio.add(result, standard_values, values);
    if (standard_values[BLACK_KING_X] == 8 &&
	standard_values[BLACK_KING_Y] == 8 &&
	values[VALUE_START + BLACK_L] == "9" /* GOLD */)
      ratio.add(result, standard_values, values);
  }
  void output(const std::string &prefix = "")
  {
    all_ratio.output();
    ratio.output("78KI-");
  }
};

template<int ValueMax>
class BlackWhiteWinRatio : public WinRatioBase
{
  AllWinRatio ratio;
  osl::misc::CArray2d<WinRatio, 2, ValueMax> target_ratio;
  std::string label;
public:
  BlackWhiteWinRatio(const std::string &name)
    : label(name) { }
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values, values);
    target_ratio[0][atoi(values[VALUE_START].c_str())].add(result, standard_values, values);
    target_ratio[1][atoi(values[VALUE_START+1].c_str())].add(result, standard_values, values);
  }
  void output(const std::string &prefix = "")
  {
    ratio.output();
    for (size_t i = 0; i < target_ratio.size1(); ++i)
    {
      for (size_t j = 0; j < target_ratio.size2(); ++j)
      {
	std::string filename =
	  (boost::format("%1%-%2%-%3%.txt") % (i == 0 ? "black" : "white")
	   % label.c_str() % j).str();
	target_ratio[i][j].output(filename);
      }
    }
  }
};

template<class Ratio, int ValueMax>
class BlackWhiteWinRatioT : public WinRatioBase
{
  AllWinRatio ratio;
  osl::misc::CArray2d<Ratio, 2, ValueMax> target_ratio;
  std::string label;
public:
  BlackWhiteWinRatioT(const std::string &name)
    : label(name) { }
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values, values);
    target_ratio[0][atoi(values[VALUE_START].c_str())].add(result, standard_values, values);
    target_ratio[1][atoi(values[VALUE_START+1].c_str())].add(result, standard_values, values);
  }
  void output(const std::string &prefix = "")
  {
    ratio.output();
    for (size_t i = 0; i < target_ratio.size1(); ++i)
    {
      for (size_t j = 0; j < target_ratio.size2(); ++j)
      {
	std::string filename =
	  (boost::format("%1%-%2%-%3%-") % (i == 0 ? "black" : "white")
	   % label.c_str() % j).str();
	target_ratio[i][j].output(filename);
      }
    }
  }
};

class K24GSCountWinRatio : public WinRatioBase
{
  AllWinRatio ratio;
  osl::misc::CArray2d<WinRatio, 2, 9> gs;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values, values);
    gs[0][atoi(values[VALUE_START].c_str())].add(result, standard_values, values);
    gs[1][atoi(values[VALUE_START+1].c_str())].add(result, standard_values, values);
  }
  void output(const std::string &prefix = "")
  {
    ratio.output();
    for (size_t i = 0; i < gs.size1(); ++i)
    {
      for (size_t j = 0; j < gs.size2(); ++j)
      {
	std::ofstream out(
	  (boost::format("%1%-gs-%2%.txt") % (i == 0 ? "black" : "white")
	   % j).str().c_str());
	gs[i][j].output(out);
      }
    }
  }
};

class KingWinRatio : public WinRatioBase
{
  AllWinRatio ratio;
  osl::misc::CArray2d<WinRatio, 3, 3> king;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    std::vector<int> new_values(standard_values);
    ratio.add(result, new_values, values);
    king[(new_values[BLACK_KING_Y] - 1) / 3][(new_values[WHITE_KING_Y] - 1) / 3].add(result, new_values, values);
  }
  void output(const std::string &prefix = "")
  {
    ratio.output();
    const char *black_name[] = { "black-enter",
				 "black-middle",
				 "black-normal" };
    const char *white_name[] = { "white-normal",
				 "white-middle",
				 "white-enter" };
    for (size_t i = 0; i < king.size1(); ++i)
    {
      for (size_t j = 0; j < king.size2(); ++j)
      {
	const std::string &filename = (boost::format("%1%-%2%.txt") %
				       black_name[i] % white_name[j]).str();
	std::ofstream f(filename.c_str());
	king[i][j].output(f);
      }
    }
  }
};

class KingYWinRatio : public WinRatioBase
{
  AllWinRatio all_ratio;
  osl::misc::CArray2d<WinRatio, 2, 10>  ratio;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    all_ratio.add(result, standard_values, values);
    ratio[osl::playerToIndex(osl::BLACK)][standard_values[BLACK_KING_Y]].add(result, standard_values, values);
    ratio[osl::playerToIndex(osl::WHITE)][standard_values[WHITE_KING_Y]].add(result, standard_values, values);
  }
  void output(const std::string &prefix = "")
  {
    all_ratio.output();
    for (size_t i = 0; i < ratio.size1(); ++i)
    {
      for (size_t j = 0; j < ratio.size2(); ++j)
      {
	std::string filename = (boost::format("%1%%2%-ky-%3%.txt")
				% prefix.c_str()
				% (i == 0 ? "black" : "white")
				% j).str();
	ratio[i][j].output(filename);
      }
    }
  }
};


class PawnWinRatio : public WinRatioBase
{
  AllWinRatio ratio;
  osl::misc::CArray2d<WinRatio, 2, 9> pawn;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values, values);
    // Attack
    // TODO: consider how far away it is from your king
    // See whether only one side can drop pawn.
    // See if it has pawn on stand
    for (int i = 1; i <= 9; ++i)
    {
      bool pawn_on_board = atoi(values[i + VALUE_START - 1].c_str()) > 0;
      int relative_x = std::abs(i - standard_values[WHITE_KING_X]);
      if (!pawn_on_board)
	pawn[0][relative_x].add(result, standard_values, values);
    }
    for (int i = 1; i <= 9; ++i)
    {
      bool pawn_on_board = atoi(values[i + VALUE_START - 1 + 9].c_str()) > 0;
      int relative_x = std::abs(i - standard_values[BLACK_KING_X]);
      if (!pawn_on_board)
	pawn[1][relative_x].add(result, standard_values, values);
    }
  }
  void output(const std::string &prefix = "")
  {
    ratio.output();
    for (size_t i = 0; i < pawn.size1(); ++i)
    {
      for (size_t j = 0; j < pawn.size2(); ++j)
      {
	std::string filename = (boost::format("%1%-no-pawn-%2%.txt") % (i == 0 ? "black" : "white")
	   % j).str();
	pawn[i][j].output(filename);
      }
    }
  }
};

class MobilityWinRatio : public WinRatioBase
{
  AllWinRatio ratio;
  osl::misc::CArray2d<WinRatio, 2, 17> bishop_mobility;
  osl::misc::CArray2d<WinRatio, 2, 17> pbishop_mobility;
  void add1(const std::string& result,
	    const std::vector<int>& standard_values,
	    const std::vector<std::string>& values,
	    int offset)
  {
    osl::Player player = values[VALUE_START + offset] == "+" ? osl::BLACK : osl::WHITE;
    bool promoted = values[VALUE_START + offset + 1] == "1";
    int mobility = atoi(values[VALUE_START + offset + 2].c_str());
    if (player == osl::BLACK && mobility >= 0)
    {
      if (promoted)
	pbishop_mobility[osl::playerToIndex(player)][mobility].add(result, standard_values, values);
      else
	bishop_mobility[osl::playerToIndex(player)][mobility].add(result, standard_values, values);
    }
  }
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values, values);
    add1(result, standard_values, values, 0);
    add1(result, standard_values, values, 5);
  }
  // bm- files
  void output(const std::string &prefix = "")
  {
    ratio.output();
    for (size_t i = 0; i < bishop_mobility.size1(); ++i)
    {
      for (size_t j = 0; j < bishop_mobility.size2(); ++j)
      {
	std::string filename =
	  (boost::format("%1%-bishop-%2%.txt") % (i == 0 ? "BLACK" : "WHITE")
	   % j).str();
	bishop_mobility[i][j].output(filename);
      }
    }
    for (size_t i = 0; i < pbishop_mobility.size1(); ++i)
    {
      for (size_t j = 0; j < pbishop_mobility.size2(); ++j)
      {
	std::string filename =
	  (boost::format("%1%-pbishop-%2%.txt") % (i == 0 ? "BLACK" : "WHITE")
	   % j).str();
	pbishop_mobility[i][j].output(filename);
      }
    }
  }
};

class MajorPieceWinRatio : public WinRatioBase
{
  AllWinRatio ratio;
  osl::misc::CArray2d<WinRatioForT, 10, 2> major_pieces;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values, values);
    int rook = atoi(values[VALUE_START].c_str());
    int bishop = atoi(values[VALUE_START+1].c_str());
    if (rook + bishop == 4)
      major_pieces[standard_values[BLACK_KING_X]][0].add(result, standard_values, values);
    if (rook + bishop == 0)
      major_pieces[standard_values[WHITE_KING_X]][1].add(result, standard_values, values);
  }
  // mp- files
  void output(const std::string &prefix = "")
  {
    ratio.output();
    for (size_t x = 0; x < major_pieces.size1(); ++x)
    {
      for (size_t i = 0; i < major_pieces.size2(); ++i)
      {
	std::string filename =
	  (boost::format("mp-%1%-%2%-") % x % (i == 0 ? "BLACK" : "WHITE")).str();
	major_pieces[x][i].output(filename);
      }
    }
  }
};

class PawnAttackBaseWinRatio : public WinRatioBase
{
  AllWinRatio ratio;
  osl::misc::CArray<WinRatio, 20> pab_ratio;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio.add(result, standard_values, values);
    for (size_t i = 0; i < 20; ++i)
    {
      if (atoi(values[VALUE_START + i].c_str()) > 0)
	pab_ratio[i].add(result, standard_values, values);
    }
  }
  void output(const std::string &prefix = "")
  {
    ratio.output();
#if 0
    const char *filename[] = 
      {
	"ULL",
	"UL",
	"U",
	"UR",
	"URR",
	"UULL",
	"UUL",
	"UU",
	"UUR",
	"UURR",
      };
#endif
    for (size_t i = 0; i < pab_ratio.size(); ++i)
    {
      std::string filename = (boost::format("%1%-%2%.txt")
			      % (i < 10 ? "black" : "white")
			      % i /*filename[i % 10]*/).str();
      pab_ratio[i].output(filename);
    }
  }
};

class ThreatWinRatio : public WinRatioBase
{
  AllWinRatio all_ratio;
  WinRatio threat_ratio;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    if (values[VALUE_START] == "X")
      return;

    all_ratio.add(result, standard_values, values);
    threat_ratio.add(result, atoi(values[VALUE_START].c_str()));
  }
  void output(const std::string &prefix = "")
  {
    all_ratio.output();
    threat_ratio.output("threat.txt");
  }
};

template<int piece_num, int value_max, class Ratio>
class PerPieceWinRatioWinRatio : public WinRatioBase
{
  AllWinRatio all_ratio;
  osl::misc::CArray2d<Ratio, 2, value_max> ratio;
  std::string label;
public:
  PerPieceWinRatioWinRatio(const std::string &name)
    : label(name) { }
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    all_ratio.add(result, standard_values, values);
    for (int i = 0; i < piece_num; ++i)
    {
      const osl::Player player =
	(values[VALUE_START + 2 * i] == "+" ? osl::BLACK : osl::WHITE);
      const int index = atoi(values[VALUE_START + 2 * i + 1].c_str());
      if (index == -1)
	continue;
      ratio[osl::playerToIndex(player)][index].add(result, standard_values, values);
    }
  }
  void output(const std::string &prefix = "")
  {
    all_ratio.output();
    for (size_t i = 0; i < ratio.size1(); ++i)
    {
      for (size_t j = 0; j < ratio.size2(); ++j)
      {
	std::string filename = (boost::format("%1%-%2%-%3%-")
				% label % (i == 0 ? "BLACK" : "WHITE")
				% j).str();
	ratio[i][j].output(filename);
      }
    }
  }
};

template<int piece_num, int value_max, class Ratio>
class PerPiecePieceWinRatioWinRatio : public WinRatioBase
{
  AllWinRatio all_ratio;
  osl::misc::CArray<osl::misc::CArray2d<Ratio, 2, value_max>, 2> ratio;
  std::string label;
public:
  PerPiecePieceWinRatioWinRatio(const std::string &name)
    : label(name) { }
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    all_ratio.add(result, standard_values, values);
    for (int i = 0; i < piece_num; ++i)
    {
      if (values[VALUE_START + 3 * i + 1] == "X")
	continue;
      const osl::Player lance_player =
	(values[VALUE_START + 3 * i] == "+" ? osl::BLACK : osl::WHITE);
      const osl::Player target_player =
	(values[VALUE_START + 3 * i + 1] == "+" ? osl::BLACK : osl::WHITE);
      const int index = atoi(values[VALUE_START + 3 * i + 2].c_str());
      if (index == -1)
	continue;
      ratio[osl::playerToIndex(lance_player)][osl::playerToIndex(target_player)][index].add(result, standard_values, values);
    }
  }
  void output(const std::string &prefix = "")
  {
    all_ratio.output();
    for (size_t x = 0; x < ratio.size(); ++x)
    {
      for (size_t i = 0; i < ratio[x].size1(); ++i)
      {
	for (size_t j = 0; j < ratio[x].size2(); ++j)
	{
	  std::string filename = (boost::format("%1%-%2%-%3%-%4%-")
				  % label % (x == 0 ? "BLACK" : "WHITE")
				  % (i == 0 ? "BLACK" : "WHITE")
				  % static_cast<osl::Ptype>(j)).str();
	  ratio[x][i][j].output(filename);
	}
      }
    }
  }
};

template<int piece_num, int value_max, class Ratio>
class PerPiecePiecePosWinRatioWinRatio : public WinRatioBase
{
  AllWinRatio all_ratio;
  osl::misc::CArray<osl::misc::CArray2d<Ratio, 2, value_max>, 2> ratio;
  std::string label;
public:
  PerPiecePiecePosWinRatioWinRatio(const std::string &name)
    : label(name) { }
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    all_ratio.add(result, standard_values, values);
    // turn, x, y, owner1, ptype, ... , owner4, ptype4
    int start = VALUE_START+1;
    for (int i = 0; i < piece_num; ++i)
    {
      const osl::Player player =
	(values[start++] == "+" ? osl::BLACK : osl::WHITE);
      start += 2; // skip position for now
      for (int j = 0; j < 4; ++j)
      {
	const osl::Player target_player =
	  (values[start++] == "+" ? osl::BLACK : osl::WHITE);
	const int index = atoi(values[start++].c_str());
	if (index == -1)
	  continue;
	ratio[osl::playerToIndex(player)][osl::playerToIndex(target_player)][index].add(result, standard_values, values);
      }
    }
  }
  void output(const std::string &prefix = "")
  {
    all_ratio.output();
    for (size_t x = 0; x < ratio.size(); ++x)
    {
      for (size_t i = 0; i < ratio[x].size1(); ++i)
      {
	for (size_t j = 0; j < ratio[x].size2(); ++j)
	{
	  std::string filename = (boost::format("%1%-%2%-%3%-%4%-")
				  % label % (x == 0 ? "BLACK" : "WHITE")
				  % (i == 0 ? "BLACK" : "WHITE")
				  % static_cast<osl::Ptype>(j)).str();
	  ratio[x][i][j].output(filename);
	}
      }
    }
  }
};

class KingEscape2WinRatio : public WinRatioBase
{
  // left effect, right effect, attack piece on stand, rook to attack, all, one_effect, both_effect
  AllWinRatio all_ratio;
  osl::misc::CArray2d<WinRatioForT, 2, 7> ratio;
  osl::misc::CArray<osl::misc::CArray2d<WinRatioForT, 16, 16>, 2> effect_ratio;
  void internalAdd(const std::string& result,
		   const std::vector<int>& standard_values,
		   const std::vector<std::string>& values,
		   const int index1, const int start)
  {
    bool all = true;
    osl::misc::CArray<bool, 4> bool_result;
    for (size_t i = 0; i < bool_result.size(); ++i)
    {
      const int value = atoi(values[start + i].c_str());
      const bool is_true = value != 0;
      all = all && is_true;
      if (is_true)
	ratio[index1][i].add(result, standard_values, values);
      bool_result[i] = is_true;
    }
    if (all)
      ratio[index1][4].add(result, standard_values, values);
    if ((bool_result[0] ^ bool_result[1]) && !bool_result[2] && !bool_result[3])
      ratio[index1][5].add(result, standard_values, values);
    if ((bool_result[0] && bool_result[1]))
    {
      ratio[index1][6].add(result, standard_values, values);
      int left = atoi(values[start].c_str());
      int right = atoi(values[start + 1].c_str());
      left = std::max(0, std::min(15, left));
      right = std::max(0, std::min(15, right));
      effect_ratio[index1][left][right].add(result, standard_values, values);
    }
  }
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    all_ratio.add(result, standard_values, values);
    internalAdd(result, standard_values, values, 0, VALUE_START);
    internalAdd(result, standard_values, values, 1,
		VALUE_START + 4);
  }
  void output(const std::string &prefix = "")
  {
    all_ratio.output();
    static const char *name[] =
    {
      "LEFT_EFFECT",
      "RIGHT_EFFECT",
      "ATTACK_ON_STAND",
      "ATTACK_ROOK",
      "ALL",
      "ONE_EFFECT",
      "BOTH_EFFECT",
    };
    for (size_t i = 0; i < ratio.size1(); ++i)
    {
      for (size_t j = 0; j < ratio.size2(); ++j)
      {
	std::string filename = (boost::format("ke-%1%-%2%-")
				% (i == 0 ? "BLACK" : "WHITE")
				% name[j]).str();
	ratio[i][j].output(filename);
      }
    }
    for (size_t i = 0; i < effect_ratio.size(); ++i)
    {
      for (size_t j = 0; j < effect_ratio[i].size1(); ++j)
      {
	for (size_t k = 0; k < effect_ratio[i].size2(); ++k)
	{
	  std::string filename = (boost::format("ke-both-%1%-%2%-%3%-")
				  % (i == 0 ? "BLACK" : "WHITE")
				  % j % k).str();
	  effect_ratio[i][j][k].output(filename);
	}
      }
    }
  }
};

template <class Ratio>
class KingXWinRatio : public WinRatioBase
{
  osl::misc::CArray2d<Ratio, 2, 10> ratio;
  std::string prefix_;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    ratio[0][standard_values[BLACK_KING_X]].add(result, standard_values, values);
    ratio[1][standard_values[WHITE_KING_X]].add(result, standard_values, values);
  }
  void output(const std::string &prefix = "")
  {
    for (size_t i = 0; i < ratio.size1(); ++i)
    {
      for (size_t j = 1; j < ratio.size2(); ++j)
      {
	const std::string filename =
	  (boost::format("%1%%2%-king-x-%3%-") % prefix.c_str()
	   % (i == 0 ? "BLACK" : "WHITE") % j).str();
	ratio[i][j].output(filename);
      }
    }
  }
};

template <class Ratio>
class ControlXWinRatio : public WinRatioBase
{
  AllWinRatio all_ratio;
  osl::misc::CArray2d<Ratio, 2, 10> ratio;
  osl::misc::CArray2d<Ratio, 2, 10> relative_ratio;
  std::string prefix_;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    all_ratio.add(result, standard_values, values);
    int black_x_distance = atoi(values[VALUE_START].c_str());
    int white_x_distance = atoi(values[VALUE_START + 1].c_str());
    if (black_x_distance != 10 && white_x_distance == 10)
      ratio[0][black_x_distance].add(result, standard_values, values);
    if (black_x_distance == 10 && white_x_distance != 10)
      ratio[1][white_x_distance].add(result, standard_values, values);
    if (black_x_distance != 10 && white_x_distance != 10)
    {
      if (black_x_distance <= white_x_distance)
	relative_ratio[0][white_x_distance - black_x_distance].add(result, standard_values, values);
      else
	relative_ratio[1][black_x_distance - white_x_distance].add(result, standard_values, values);
    }
 }
  void output(const std::string &prefix = "")
  {
    all_ratio.output();
    for (size_t i = 0; i < ratio.size1(); ++i)
    {
      for (size_t j = 0; j < ratio.size2(); ++j)
      {
	const std::string filename =
	  (boost::format("%1%%2%-control-x-%3%-") % prefix.c_str()
	   % (i == 0 ? "BLACK" : "WHITE") % j).str();
	ratio[i][j].output(filename);
      }
    }
    for (size_t i = 0; i < relative_ratio.size1(); ++i)
    {
      for (size_t j = 0; j < relative_ratio.size2(); ++j)
      {
	const std::string filename =
	  (boost::format("%1%%2%-control-x-relative-%3%-") % prefix.c_str()
	   % (i == 0 ? "BLACK" : "WHITE") % j).str();
	relative_ratio[i][j].output(filename);
      }
    }
  }
};

template <class Ratio>
class Rank7WinRatio : public WinRatioBase
{
  AllWinRatio all_ratio;
  osl::misc::CArray2d<Ratio, 2, 10> ratio;
  std::string prefix_;
public:
  void add(const std::string& result,
	   const std::vector<int>& standard_values,
	   const std::vector<std::string>& values)
  {
    all_ratio.add(result, standard_values, values);
    int black_effect = atoi(values[VALUE_START].c_str());
    int white_effect = atoi(values[VALUE_START + 1].c_str());
    if (black_effect != -1)
    {
      black_effect = std::min(static_cast<int>(ratio.size2()) - 1,
			      black_effect);
      ratio[0][black_effect].add(result, standard_values, values);
    }
    if (white_effect != -1)
    {
      white_effect = std::min(static_cast<int>(ratio.size2()) - 1,
			      white_effect);
      ratio[1][white_effect].add(result, standard_values, values);
    }
 }
  void output(const std::string &prefix = "")
  {
    all_ratio.output();
    for (size_t i = 0; i < ratio.size1(); ++i)
    {
      for (size_t j = 0; j < ratio.size2(); ++j)
      {
	const std::string filename =
	  (boost::format("%1%%2%-7effect-%3%-") % prefix.c_str()
	   % (i == 0 ? "BLACK" : "WHITE") % j).str();
	ratio[i][j].output(filename);
      }
    }
  }
};

void compute_ratio(const std::string &filename,
		   WinRatioBase &ratio)
{
  std::ifstream fin(filename.c_str());
  std::string line;
  while (std::getline(fin, line))
  {
    std::vector<std::string> values;
    boost::algorithm::split(values, line, boost::algorithm::is_any_of(" "));
    const std::string &result = values[0];
    if (result == "++" || result == "--")
      continue;
    std::vector<int> standard_values;
    standard_values.push_back(0); // dummy for win value
    for (int i = EVAL; i < VALUE_START; ++i)
    {
      standard_values.push_back(atoi(values[i].c_str()));
    }
    ratio.add(result, standard_values, values);
  }
}

int main(int argc, char **argv)
{
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("input-file", boost::program_options::value< std::vector<std::string> >(),
     "input files in CSA format")
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
      std::cerr << "Usage: " << argv[0] << " [options] result-files"
		<< std::endl;
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0] << " [options] result-files" << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  const std::vector<std::string> files =
    vm["input-file"].as< std::vector<std::string> >();

  //KingYWinRatio ratio;
  //KingWinRatio ratio;
  //UnsupportedGSWinRatio ratio;
  //K24GSCountWinRatio ratio;
  //BlackWhiteWinRatioT<PieceStandWinRatio<VALUE_START + 2>, 2> ratio("kc");
  //TurnWinRatio<WinRatioForT> ratio;
  //BlackWhiteWinRatioT<WinRatioForT, 2> ratio("kte");
  //BlackWhiteWinRatioT<ProgressWinRatio, 10> ratio("kne");
  //BlackWhiteWinRatioT<ProgressWinRatio, 2> ratio("na");
  //BlackWhiteWinRatioT<WinRatioForT, 2> ratio("ac");
  //BlackWhiteWinRatioT<ProgressWinRatio, 2> ratio("ac");
  //BlackWhiteWinRatioT<PieceStandWinRatio<VALUE_START+2>, 2> ratio("pb");
  //King8TestWinRatio ratio;
  //King8WinRatio<PieceStandWinRatio<VALUE_START+18> > ratio;
  //King8WinRatio<KingYWinRatio> ratio;
  //PieceStandWinRatio<VALUE_START+18> ratio;
  //RookPieceStandWinRatio<VALUE_START+18, ProgressWinRatio> ratio;
  //RookPieceStandNoGSWinRatio<VALUE_START+18, WinRatioForT> ratio;
  //King8WinRatio<ProgressWinRatio> ratio;
  //ProgressWinRatio ratio;
  //PawnWinRatio ratio;
  //MobilityWinRatio ratio;
  //MajorPieceWinRatio ratio;
  //KingEscape2WinRatio ratio;
  //KingXWinRatio<ProgressWinRatio> ratio;
  //PawnAttackBaseWinRatio ratio;
  //PerPieceWinRatioWinRatio<2, 2, ProgressWinRatio> ratio("pu");
  //PerPieceWinRatioWinRatio<4, 2, ProgressWinRatio> ratio("ka");
  //PerPiecePieceWinRatioWinRatio<4, osl::PTYPE_MAX + 1, WinRatioForT> ratio("lep");
  //PerPiecePiecePosWinRatioWinRatio<2, osl::PTYPE_MAX + 1, WinRatioForT> ratio("rep");
  //ThreatWinRatio ratio;
  //AllWinRatio ratio;
  //ControlXWinRatio<WinRatioForT> ratio;
  //Rank7WinRatio<WinRatioForT> ratio;
  PieceWinRatio<VALUE_START + static_cast<int>(osl::BISHOP), 3> ratio;
  for (size_t i = 0; i < files.size(); i++)
  {
    compute_ratio(files[i], ratio);
  }
  ratio.output();

  return 0;
}
