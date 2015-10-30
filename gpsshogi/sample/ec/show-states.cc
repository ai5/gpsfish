#include "osl/apply_move/applyMove.h"
#include "osl/state/numEffectState.h"
#include "osl/effect/numBitmapEffect.h"
#include "osl/eval/progressEval.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/record/kisen.h"
#include "osl/centering5x3.h"
#include "osl/mobility/rookMobility.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/simpleHashTable.h"
#include "osl/progress/effect5x3.h"
#include "osl/pieceStand.h"
#include "osl/checkmate/king8Info.h"
#include "osl/record/csa.h"
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <fstream>

bool noMajorPieces(const osl::state::NumEffectState &state,
		   osl::Player player)
{
  for (int i = osl::PtypeTraits<osl::ROOK>::indexMin;
       i < osl::PtypeTraits<osl::ROOK>::indexLimit;
       ++i)
  {
    const osl::Piece rook = state.pieceOf(i);
    if (rook.owner() == player)
      return false;
  }
  for (int i = osl::PtypeTraits<osl::BISHOP>::indexMin;
       i < osl::PtypeTraits<osl::BISHOP>::indexLimit;
       ++i)
  {
    const osl::Piece rook = state.pieceOf(i);
    if (rook.owner() == player)
      return false;
  }
  return true;
}

bool knightDiagonal(const osl::state::NumEffectState &state)
{
  const osl::Square king = state.kingSquare<osl::BLACK>();
  if (king.x() <= 7 && king.y() >= 3)
  {
    const osl::Piece target_piece =
      state.pieceAt(osl::Square(king.x() + 2, king.y() - 2));
    if (target_piece.isOnBoardByOwner<osl::WHITE>() &&
	target_piece.ptype() == osl::KNIGHT)
      return true;
  }
  if (king.x() >= 3 && king.y() >= 3)
  {
    const osl::Piece target_piece =
      state.pieceAt(osl::Square(king.x() - 2, king.y() - 2));
    if (target_piece.isOnBoardByOwner<osl::WHITE>() &&
	target_piece.ptype() == osl::KNIGHT)
      return true;
  }
  return false;
}

int countRank9Effect(const osl::state::NumEffectState &state)
{
  int result = 0;
  const osl::Square king = state.kingSquare<osl::BLACK>();
  const osl::Square center = osl::Centering5x3::adjustCenter(king);

  for (int x = center.x() - 2; x <= center.x() + 2; ++x)
  {
    result += state.countEffect(osl::WHITE, osl::Square(x, 9));
  }

  return result;
}

bool canGoUp(const osl::state::NumEffectState &state)
{
  const osl::Square king = state.kingSquare<osl::BLACK>();
  if (king.y() == 1)
    return true;
  for (int x = std::max(1, king.x() - 1);
       x <= std::min(king.x() + 1, 9); ++x)
  {
    osl::Square target(x, king.y() - 1);
    if (!state.pieceAt(target).isOnBoardByOwner(osl::BLACK) &&
	!state.hasEffectAt<osl::WHITE>(target))
      return true;
  }
  return false;
}

bool canGoLeft(const osl::state::NumEffectState &state)
{
  const osl::Square king = state.kingSquare<osl::BLACK>();
  if (king.x() == 9)
    return false;
  for (int y = std::max(1, king.y() - 1);
       y <= std::min(king.y() + 1, 9); ++y)
  {
    osl::Square target(king.x() + 1, y);
    if (!state.pieceAt(target).isOnBoardByOwner(osl::BLACK) &&
	!state.hasEffectAt<osl::WHITE>(target))
      return true;
  }
  return false;
}

static bool middleKnight(const osl::state::NumEffectState& state)
{
  for (int i = osl::PtypeTraits<osl::KNIGHT>::indexMin;
       i < osl::PtypeTraits<osl::KNIGHT>::indexLimit; ++i)
  {
    osl::Piece knight = state.pieceOf(i);
    if (knight.isOnBoard() && !knight.isPromoted() &&
	knight.owner() == osl::WHITE && knight.square().y() > 3)
    {
      const osl::Square up =
	osl::Board_Table.nextSquare(knight.owner(),
				      knight.square(),
				      osl::U);
      if (state.pieceAt(up).isEmpty() &&
	  !state.hasEffectAt(knight.owner(), up))
      {
	return true;
      }
    }
  }
  return false;
}

static bool ownRookRank(const osl::state::NumEffectState& state)
{
  for (int i = osl::PtypeTraits<osl::ROOK>::indexMin;
       i < osl::PtypeTraits<osl::ROOK>::indexLimit; ++i)
  {
    const osl::Piece rook = state.pieceOf(i);
    if (rook.isOnBoardByOwner(osl::BLACK) &&
	!rook.square().canPromote(osl::BLACK) &&
	state.hasEffectByPtype<osl::SILVER>(
	  osl::BLACK,
	  osl::Square(rook.square().x(), 4)) &&
	!state.pieceAt(osl::Square(rook.square().x(), 5)).isOnBoardByOwner(osl::WHITE) &&
	!state.pieceAt(osl::Square(rook.square().x(), 3)).isOnBoardByOwner(osl::BLACK) &&
	state.countEffect(osl::WHITE, 
			  osl::Square(rook.square().x(), 3)) <= 1)
      return true;
  }
  return false;
}

int main(int argc, char **argv)
{
  size_t start, end;
  bool include_all;
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("start",
     boost::program_options::value<size_t>(&start)->default_value(0),
     "Start index of kisen file")
    ("end",
     boost::program_options::value<size_t>(&end)->default_value(60000),
     "End index of kisen file")
    ("all",
     boost::program_options::value<bool>(&include_all)->default_value(false),
     "Whether to include all plays.  When false, only plays with players"
     "more than or equal to rating 1500 are included")
    ("input-file", boost::program_options::value<std::vector<std::string> >(),
     "input files in kisen format")
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
      std::cerr << "Usage: " << argv[0] << " [options] kisen-file"
		<< std::endl;
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0]
	      << " [options] result-file kisen-file" << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  const std::vector<std::string> files =
    vm["input-file"].as< std::vector<std::string> >();

  if (files.size() != 2)
  {
    std::cerr << "Need two files" << std::endl;
    return 1;
  }

  std::ifstream fin(files[0].c_str());
  std::vector<std::string> results;
  results.reserve(end - start);
  std::string result;
  while (std::getline(fin, result))
  {
    results.push_back(result);
  }

  osl::record::KisenFile kisen(files[1]);
  std::string ipx(files[1]);
  ipx.replace(ipx.rfind("."), 4, ".ipx");
  osl::record::KisenIpxFile kisen_ipx(ipx);
  osl::eval::ProgressEval::setUp();
  const int min = osl::eval::PtypeEvalTraits<osl::PAWN>::val * 16 / 2 * 0;
  const int max = osl::eval::PtypeEvalTraits<osl::PAWN>::val * 16 / 2 * 100;
  int count = 0;

  for (size_t i = start; i < kisen.size() && i < end; i++)
  {
    if (!include_all &&
	kisen_ipx.getRating(i, osl::BLACK) < 1500 &&
	kisen_ipx.getRating(i, osl::WHITE) < 1500)
    {
      continue;
    }

    const osl::vector<osl::Move> moves = kisen.getMoves(i);
    osl::state::SimpleState simple_state = kisen.getInitialState();
    osl::state::NumEffectState state(simple_state);
    osl::eval::ProgressEval eval(state);
    osl::progress::Effect5x3 progress(state);

    std::string result = results[i - start];
    size_t end_index = moves.size();
    if (result.find(" ") != std::string::npos)
    {
      end_index =
	std::max(0, atoi(result.substr(result.find(" ")).c_str()) - 1);
      if (end_index > moves.size())
	std::cerr << end_index << " " << moves.size();
      assert(end_index <= moves.size());
      result = result.substr(0, 1);
    }

    // eval.openingValue() < 0 && result == "-"
    // noMajorPieces(state, osl::BLACK) && result == "+" &&
    // knightDiagonal(state) && result == "-" &&
    // progress.progress16().value() <= 3 &&
    for (size_t j = 0; j <= end_index; j++)
    {
      if (!(j < moves.size() - 1 &&
	    moves[j+1].to() != moves[j].to()) &&
	  (min <= eval.value() && eval.value() <= max) &&
	  result == "-" &&
	  eval.openingValue() < -512 * 2 &&
	  state.turn() == osl::BLACK &&
	  eval.attackDefenseBonus() > 0 &&
	  !osl::EffectUtil::isKingInCheck(state.turn(), state))
      {
	std::string filename =
	  (boost::format("%1$05d%2%.csa") % count % result).str();
	std::ofstream out(filename.c_str());
	out << state
	    << "' " << result <<std::endl;
	for (size_t k = j; k < moves.size(); k++)
	{
	  out << osl::record::csa::show(moves[k]) << std::endl;
	}
	++count;
      }
      if (j != moves.size())
      {
	osl::apply_move::ApplyMoveOfTurn::doMove(state, moves[j]);
	eval.update(state, moves[j]);
	progress.update(state, moves[j]);
      }
    }
  }

  return 0;
}
