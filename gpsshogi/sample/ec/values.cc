#include "osl/apply_move/applyMove.h"
#include "osl/state/numEffectState.h"
#include "osl/eval/progressEval.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/record/kisen.h"
#include "osl/centering5x3.h"
#include "osl/mobility/rookMobility.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/simpleHashTable.h"
#include "osl/pieceStand.h"
#include "osl/checkmate/king8Info.h"
#include "eval/eval.h"
#include "eval/evalFactory.h"

#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>

#include <iostream>
#include <fstream>

struct Base
{
public:
  virtual void print(const osl::state::NumEffectState &state) = 0;
  void piecesOnStand(const osl::state::NumEffectState &state,
		     const osl::Player player)
  {
    for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
    {
      std::cout << state.countPiecesOnStand(player, osl::PieceStand::order[i])
		<< " ";
    }
  }
};

struct King8Pieces : public Base
{
  static osl::container::PieceMask king8EffectPieces(
    const osl::state::NumEffectState &state,
    const osl::Player attack)
  {
    osl::Piece king = state.kingPiece(osl::alt(attack));
    osl::container::PieceMask effect;
    for (int x = std::max(1, king.square().x() - 1);
	 x <= std::min(9, king.square().x() + 1); ++x)
    {
      for (int y = std::max(1, king.square().y() - 1);
	   y <= std::min(9, king.square().y() + 1); ++y)
      {
	osl::effect::NumBitmapEffect e = state.effectSetAt(osl::Square(x, y));
	effect = (effect|static_cast<osl::container::PieceMask>(e));
      }
    }
    // Discard PAWN, LANCE, KNIGHT
    effect &= state.piecesOnBoard(attack);
    osl::container::PieceMask small_mask;
    small_mask.orMask(osl::PtypeFuns<osl::PAWN>::indexNum,
		      osl::mask_t::makeDirect(osl::PtypeFuns<osl::PAWN>::indexMask));
    small_mask.orMask(osl::PtypeFuns<osl::LANCE>::indexNum,
		      osl::mask_t::makeDirect(osl::PtypeFuns<osl::LANCE>::indexMask));
    small_mask.orMask(osl::PtypeFuns<osl::KNIGHT>::indexNum,
		      osl::mask_t::makeDirect(osl::PtypeFuns<osl::KNIGHT>::indexMask));
    effect &= ~small_mask;
    return effect;
    //return effect.countBit();
  }
  template <osl::Ptype T>
  static int countPtype(osl::container::PieceMask mask)
  {
    return (mask.getMask(osl::PtypeFuns<T>::indexNum) &
	    osl::mask_t::makeDirect(osl::PtypeFuns<T>::indexMask)).countBit();
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    osl::container::PieceMask black_mask = king8EffectPieces(state, osl::BLACK);
    osl::container::PieceMask white_mask = king8EffectPieces(state, osl::WHITE);
    std::cout << black_mask.countBit() << " "
	      << white_mask.countBit() << " "
	      << countPtype<osl::PAWN>(black_mask) << " "
	      << countPtype<osl::LANCE>(black_mask) << " "
	      << countPtype<osl::KNIGHT>(black_mask) << " "
	      << countPtype<osl::SILVER>(black_mask) << " "
	      << countPtype<osl::GOLD>(black_mask) << " "
	      << countPtype<osl::BISHOP>(black_mask) << " "
	      << countPtype<osl::ROOK>(black_mask) << " "
	      << countPtype<osl::PBISHOP>(black_mask) << " "
	      << countPtype<osl::PROOK>(black_mask) << " "
	      << countPtype<osl::PAWN>(white_mask) << " "
	      << countPtype<osl::LANCE>(white_mask) << " "
	      << countPtype<osl::KNIGHT>(white_mask) << " "
	      << countPtype<osl::SILVER>(white_mask) << " "
	      << countPtype<osl::GOLD>(white_mask) << " "
	      << countPtype<osl::BISHOP>(white_mask) << " "
	      << countPtype<osl::ROOK>(white_mask) << " "
	      << countPtype<osl::PBISHOP>(white_mask) << " "
	      << countPtype<osl::PROOK>(white_mask) << " "
      ;
  }
};

std::string rookVerticalMobility(osl::state::NumEffectState &state,
				 const osl::Piece rook)
{
  if (!rook.isOnBoard())
  {
    if (rook.owner() == osl::BLACK)
      return "+*";
    else
      return "-*";
  }
  const int count = osl::mobility::RookMobility::countVerticalAll(osl::BLACK, state, rook);
  char buf[20];
  sprintf(buf, "%c%c%d", (rook.owner() == osl::BLACK) ? '+' : '-',
	  (rook.ptype() == osl::ROOK) ? 'R' : 'P', count);
  return buf;
}

std::string rookHorizontalMobility(osl::state::NumEffectState &state,
				   const osl::Piece rook)
{
  if (!rook.isOnBoard())
  {
    if (rook.owner() == osl::BLACK)
      return "+*";
    else
      return "-*";
  }
  const int count = osl::mobility::RookMobility::countHorizontalAll(osl::BLACK, state, rook);
  char buf[20];
  sprintf(buf, "%c%c%d", (rook.owner() == osl::BLACK) ? '+' : '-',
	  (rook.ptype() == osl::ROOK) ? 'R' : 'P', count);
  return buf;
}

struct Rook : public Base
{
public:
  void print(const osl::state::NumEffectState &state)
  {
    int black_rook = 0;
    int black_rook_on_board_attacking = 0;
    for (int i = osl::PtypeTraits<osl::ROOK>::indexMin;
	 i < osl::PtypeTraits<osl::ROOK>::indexLimit;
	 ++i)
    {
      const osl::Piece rook = state.pieceOf(i);
      if (rook.owner() == osl::BLACK)
      {
	++black_rook;
	if (rook.square().isOnBoard() && rook.square().canPromote(osl::BLACK))
	{
	  ++black_rook_on_board_attacking;
	}
      }
    }
    std::cout << black_rook << " "
	      << black_rook_on_board_attacking << " ";
  }
};

struct Rook1_1 : public Base
{
public:
  void print(const osl::state::NumEffectState &state)
  {
    osl::misc::CArray<int, 8> rooks;
    rooks.fill(0);
    for (int i = osl::PtypeTraits<osl::ROOK>::indexMin;
	 i < osl::PtypeTraits<osl::ROOK>::indexLimit;
	 ++i)
    {
      int index;
      const osl::Piece rook = state.pieceOf(i);
      if (rook.square().isPieceStand())
	index = 0;
      else if (rook.square().canPromote(rook.owner()))
	index = 1;
      else if (rook.square().canPromote(osl::alt(rook.owner())))
	index = 2;
      else
	index = 3;
      if (rook.owner() == osl::WHITE)
	index += 4;
      rooks[index]++;
    }
    for (int i = 0; i < 8; ++i)
    {
      std::cout << rooks[i] << " ";
    }
  }
};

struct PuR : public Base
{
  int pawnUnderRook(const osl::state::NumEffectState &state,
		    const osl::Piece rook)
  {
    if (rook.isOnBoard() && !rook.isPromoted() &&
	!rook.square().canPromote(osl::BLACK) &&
	!rook.square().canPromote(osl::WHITE) &&
	state.isPawnMaskSet(rook.owner(), rook.square().x()))
    {
      for (osl::Square pos =
	     osl::Board_Table.nextSquare(rook.owner(),
					   rook.square(), osl::U);
	   pos.isOnBoard() && state.pieceAt(pos).isEmpty();
	   pos = osl::Board_Table.nextSquare(rook.owner(), pos, osl::U))
      {
	if (pos.canPromote(rook.owner()) &&
	    !state.hasEffectAt(osl::alt(rook.owner()), pos))
	  return 0;
      }
      for (osl::Square pos =
	     osl::Board_Table.nextSquare(rook.owner(),
					   rook.square(), osl::D);
	   pos.isOnBoard();
	   pos = osl::Board_Table.nextSquare(rook.owner(), pos, osl::D))
      {
	const osl::Piece target = state.pieceAt(pos);
	if (!target.isEmpty() && target.owner() == rook.owner() &&
	    target.ptype() == osl::PAWN)
	{
	  return 1;
	}
      }
    }
    return 0;
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    for (int i = osl::PtypeTraits<osl::ROOK>::indexMin;
	 i < osl::PtypeTraits<osl::ROOK>::indexLimit;
	 ++i)
    {
      const osl::Piece rook = state.pieceOf(i);
      std::cout << rook.owner() << " " << pawnUnderRook(state, rook) << " ";
    }
  }
};

struct Bishop1_1 : public Base
{
public:
  void print(const osl::state::NumEffectState &state)
  {
    osl::misc::CArray<int, 8> bishops;
    bishops.fill(0);
    for (int i = osl::PtypeTraits<osl::BISHOP>::indexMin;
	 i < osl::PtypeTraits<osl::BISHOP>::indexLimit;
	 ++i)
    {
      int index;
      const osl::Piece bishop = state.pieceOf(i);
      if (bishop.square().isPieceStand())
	index = 0;
      else if (bishop.square().canPromote(bishop.owner()))
	index = 1;
      else if (bishop.square().canPromote(osl::alt(bishop.owner())))
	index = 2;
      else
	index = 3;
      if (bishop.owner() == osl::WHITE)
	index += 4;
      bishops[index]++;
    }
    for (int i = 0; i < 8; ++i)
    {
      std::cout << bishops[i] << " ";
    }
  }
};

struct Bishop2 : public Base
{
public:
  void print(const osl::state::NumEffectState &state)
  {
    const osl::Piece bishop1 =
      state.pieceOf(osl::PtypeTraits<osl::BISHOP>::indexMin);
    const osl::Piece bishop2 =
      state.pieceOf(osl::PtypeTraits<osl::BISHOP>::indexMin + 1);
    if (bishop1.owner() == bishop2.owner())
    {
      std::cout << bishop1.owner() << " ";
    }
    else
    {
      std::cout << "X" << " ";
    }
    if (bishop1.isOnBoard() && bishop2.isOnBoard() &&
	osl::isBasic(bishop1.ptype()) && osl::isBasic(bishop2.ptype()))
    {
      std::cout << "R ";
    }
    else
    {
      std::cout << "P ";
    }
  }
};

struct Knights : public Base
{
public:
  void print(const osl::state::NumEffectState &state)
  {
    int black_knights_on_board = 0;
    int white_knights_on_board = 0;
    for (int i = osl::PtypeTraits<osl::KNIGHT>::indexMin;
	 i < osl::PtypeTraits<osl::KNIGHT>::indexLimit;
	 ++i)
    {
      const osl::Piece knight = state.pieceOf(i);
      if (knight.square().isOnBoard())
      {
	if (knight.owner() == osl::BLACK)
	  ++black_knights_on_board;
	else
	  ++white_knights_on_board;
      }
    }
    std::cout << black_knights_on_board << " "
	      << white_knights_on_board << " "
	      << state.countPiecesOnStand(osl::BLACK, osl::KNIGHT) << " "
	      << state.countPiecesOnStand(osl::WHITE, osl::KNIGHT) << " ";
  }
};

struct Lances : public Base
{
public:
  void print(const osl::state::NumEffectState &state)
  {
    int black_lances_on_board = 0;
    int white_lances_on_board = 0;
    for (int i = osl::PtypeTraits<osl::LANCE>::indexMin;
	 i < osl::PtypeTraits<osl::LANCE>::indexLimit;
	 ++i)
    {
      const osl::Piece lance = state.pieceOf(i);
      if (lance.square().isOnBoard())
      {
	if (lance.owner() == osl::BLACK)
	  ++black_lances_on_board;
	else
	  ++white_lances_on_board;
      }
    }
    std::cout << black_lances_on_board << " "
	      << white_lances_on_board << " "
	      << state.countPiecesOnStand(osl::BLACK, osl::LANCE) << " "
	      << state.countPiecesOnStand(osl::WHITE, osl::LANCE) << " ";
  }
};

struct King8Defense : public Base
{
public:
  void print(const osl::state::NumEffectState &state)
  {
    const osl::Piece king = state.kingPiece(osl::BLACK);
    for (int y = king.square().y() - 1 ; y <= king.square().y() + 1; ++y)
    {
      for (int x = king.square().x() - 1; x <= king.square().x() + 1; ++x)
      {
	osl::Square pos(x, y);
	if (!pos.isValid())
	{
	  std::cout << static_cast<int>(osl::PTYPE_EDGE) << " ";
	  continue;
	}
	const osl::Piece p = state.pieceAt(pos);
	if (p.isEmpty())
	{
	  std::cout << static_cast<int>(osl::PTYPE_EMPTY) << " ";
	}
	else if (p.owner() == osl::BLACK)
	{
	  std::cout << static_cast<int>(p.ptype()) << " " ;
	}
	else
	{
	  std::cout << static_cast<int>(p.ptype() + osl::PTYPE_MAX + 1) << " ";
	}
      }
    }
  }
};

struct King24UnSupportedGS : public Base
{
  int countUnsupportedGS(const osl::state::NumEffectState &state,
			 osl::Player player)
  {
    const osl::Piece king = state.kingPiece(player);
    const int x = king.square().x();
    const int y = king.square().y();
    int result = 0;

    for (int i = osl::PtypeTraits<osl::GOLD>::indexMin;
	 i < osl::PtypeTraits<osl::GOLD>::indexLimit;
	 ++i)
    {
      const osl::Piece gold = state.pieceOf(i);
      if (gold.isOnBoardByOwner(player) &&
	  !state.hasEffectAt(player, gold.square()) &&
	  std::abs(x - gold.square().x()) <= 2 &&
	  std::abs(y - gold.square().y()) <= 2)
	++result;
    }
    for (int i = osl::PtypeTraits<osl::SILVER>::indexMin;
	 i < osl::PtypeTraits<osl::SILVER>::indexLimit;
	 ++i)
    {
      const osl::Piece silver = state.pieceOf(i);
      if (silver.isOnBoardByOwner(player) &&
	  !state.hasEffectAt(player, silver.square()) &&
	  std::abs(x - silver.square().x()) <= 2 &&
	  std::abs(y - silver.square().y()) <= 2)
	++result;
    }
    return result;
  }

  int countUnsupportedGold(const osl::state::NumEffectState &state,
			   osl::Player player)
  {
    const osl::Piece king = state.kingPiece(player);
    const int x = king.square().x();
    const int y = king.square().y();
    int result = 0;

    for (int i = osl::PtypeTraits<osl::GOLD>::indexMin;
	 i < osl::PtypeTraits<osl::GOLD>::indexLimit;
	 ++i)
    {
      const osl::Piece gold = state.pieceOf(i);
      if (gold.isOnBoardByOwner(player) &&
	  !state.hasEffectAt(player, gold.square()) &&
	  std::abs(x - gold.square().x()) <= 2 &&
	  std::abs(y - gold.square().y()) <= 2)
	++result;
    }
    return result;
  }
  int countUnsupportedSilver(const osl::state::NumEffectState &state,
			   osl::Player player)
  {
    const osl::Piece king = state.kingPiece(player);
    const int x = king.square().x();
    const int y = king.square().y();
    int result = 0;

    for (int i = osl::PtypeTraits<osl::SILVER>::indexMin;
	 i < osl::PtypeTraits<osl::SILVER>::indexLimit;
	 ++i)
    {
      const osl::Piece silver = state.pieceOf(i);
      if (silver.isOnBoardByOwner(player) &&
	  !state.hasEffectAt(player, silver.square()) &&
	  std::abs(x - silver.square().x()) <= 2 &&
	  std::abs(y - silver.square().y()) <= 2)
	++result;
    }
    return result;
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << countUnsupportedGS(state, osl::BLACK) << " "
	      << countUnsupportedGS(state, osl::WHITE) << " "
	      << countUnsupportedGold(state, osl::BLACK) << " "
	      << countUnsupportedGold(state, osl::WHITE) << " "
	      << countUnsupportedSilver(state, osl::BLACK) << " "
	      << countUnsupportedSilver(state, osl::WHITE) << " ";
  }
};

struct BishopMob : public Base {
  std::string bishopMobility(const osl::state::NumEffectState &state,
			     const osl::Piece bishop)
  {
    if (!bishop.isOnBoard())
    {
      if (bishop.owner() == osl::BLACK)
	return "+*";
      else
	return "-*";
    }
    const int count = osl::mobility::BishopMobility::countAll(osl::BLACK, state, bishop);
    char buf[20];
    sprintf(buf, "%c%c%d", (bishop.owner() == osl::BLACK) ? '+' : '-',
	    (bishop.ptype() == osl::BISHOP) ? 'R' : 'P', count);
    return buf;
  }

public:
  void print(const osl::state::NumEffectState &state)
  {
    std::cout
      << bishopMobility(state,
			state.pieceOf(osl::PtypeTraits<osl::BISHOP>::indexMin)) << " "
      << bishopMobility(state,
			state.pieceOf(osl::PtypeTraits<osl::BISHOP>::indexMin)) << " ";
  }
};

struct King8DefenseEffect : public Base
{
  int countKing8NoEffect(const osl::state::NumEffectState &state,
			 osl::Player player)
  {
    const osl::Piece king = state.kingPiece(player);
    int result = 0;
    for (int y = king.square().y() - 1; y <= king.square().y() + 1; ++y)
    {
      for (int x = king.square().x() - 1; x <= king.square().x() + 1; ++x)
      {
	osl::Square pos(x, y);
	if (pos.isValid() && state.countEffect(player, pos) == 1 &&
	    (state.pieceAt(pos).isEmpty() ||
	     state.pieceAt(pos).owner() != player) &&
	    state.hasEffectAt(osl::alt(player), pos))
	{
	  result++;
	}
      }
    }
    return result;
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << countKing8NoEffect(state, osl::BLACK) << " "
	      << countKing8NoEffect(state, osl::WHITE) << " ";
  }
};

struct KingY7 : public Base
{
  int countBottomEffect(const osl::state::NumEffectState &state,
			osl::Player player)
  {
    int y = 9;
    if (player == osl::BLACK && state.kingPiece(player).square().y() <= 7)
    {
      y = state.kingPiece(player).square().y() + 2;
    }
    else if (player == osl::WHITE && state.kingPiece(player).square().y() >= 3)
    {
      y = state.kingPiece(player).square().y() - 2;
    }
    else
    {
      return 0;
    }
    const osl::Square center =
      osl::Centering5x3::adjustCenter(state.kingPiece(player).square());
    int sum = 0;
    for (int x = center.x() - 2; x <= center.x() + 2; ++x)
    {
      sum += state.countEffect(osl::alt(player), osl::Square(x, y));
    }
    return sum;
  }
  int countMajorPieceEffect(const osl::state::NumEffectState &state,
			    osl::Player player)
  {
    int y = 9;
    if (player == osl::BLACK && state.kingPiece(player).square().y() <= 7)
    {
      y = state.kingPiece(player).square().y() + 2;
    }
    else if (player == osl::WHITE && state.kingPiece(player).square().y() >= 3)
    {
      y = state.kingPiece(player).square().y() - 2;
    }
    else
    {
      return 0;
    }
    const osl::Square center =
      osl::Centering5x3::adjustCenter(state.kingPiece(player).square());
    int sum = 0;
    for (int i = osl::PtypeTraits<osl::BISHOP>::indexMin;
	 i < osl::PtypeTraits<osl::ROOK>::indexLimit; ++i) {
      osl::Piece piece = state.pieceOf(i);
      if (!piece.isOnBoardByOwner(osl::alt(player)))
	continue;
      for (int x = center.x() - 2; x <= center.x() + 2; ++x)
      {
	osl::Square p(x, y);
	if (state.hasEffectAt(osl::alt(player), p, piece))
	  sum++;
      }
    }
    return sum;
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << countBottomEffect(state, osl::BLACK) << " "
	      << countBottomEffect(state, osl::WHITE) << " "
	      << countMajorPieceEffect(state, osl::BLACK) << " "
	      << countMajorPieceEffect(state, osl::WHITE) << " ";
  }
};

struct KingU2 : public Base
{
  int countU2Effect(const osl::state::NumEffectState &state,
			osl::Player player)
  {
    int y = 1;
    if (player == osl::BLACK && state.kingPiece(player).square().y() >= 3)
    {
      y = state.kingPiece(player).square().y() - 2;
    }
    else if (player == osl::WHITE && state.kingPiece(player).square().y() <= 7)
    {
      y = state.kingPiece(player).square().y() + 2;
    }
    else
    {
      return 0;
    }
    const osl::Square center =
      osl::Centering5x3::adjustCenter(state.kingPiece(player).square());
    int sum = 0;
    for (int x = center.x() - 2; x <= center.x() + 2; ++x)
    {
      sum += state.countEffect(osl::alt(player), osl::Square(x, y));
    }
    return sum;
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << countU2Effect(state, osl::BLACK) << " "
	      << countU2Effect(state, osl::WHITE) << " ";
  }
};

struct King24GSCount : public Base
{
public:
  void print(const osl::state::NumEffectState &state)
  {
    osl::misc::CArray<int,2> gs;
    gs.fill(0);
    for (int i = osl::PtypeTraits<osl::GOLD>::indexMin;
	 i < osl::PtypeTraits<osl::GOLD>::indexLimit;
	 ++i)
    {
      const osl::Piece gold = state.pieceOf(i);
      if (gold.isOnBoard() &&
	  std::abs(state.kingPiece(gold.owner()).square().x()
		   - gold.square().x()) <= 2 &&
	  std::abs(state.kingPiece(gold.owner()).square().y()
		   - gold.square().y()) <= 2)
	++gs[osl::playerToIndex(gold.owner())];
    }
    for (int i = osl::PtypeTraits<osl::SILVER>::indexMin;
	 i < osl::PtypeTraits<osl::SILVER>::indexLimit;
	 ++i)
    {
      const osl::Piece silver = state.pieceOf(i);
      if (silver.isOnBoard() &&
	  std::abs(state.kingPiece(silver.owner()).square().x()
		   - silver.square().x()) <= 2 &&
	  std::abs(state.kingPiece(silver.owner()).square().y()
		   - silver.square().y()) <= 2)
	++gs[osl::playerToIndex(silver.owner())];
    }
    std::cout << gs[osl::playerToIndex(osl::BLACK)] << " "
	      << gs[osl::playerToIndex(osl::WHITE)] << " ";
  }
};

struct Turn : public Base
{
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << state.turn() << " ";
  }
};

struct King8Effect : public Base
{
  void king8EffectPlus(const osl::state::NumEffectState &state,
		       const osl::Player player)
  {
    const osl::Square king = state.kingSquare(player);
    for (int y = king.y() - 1; y <= king.y() + 1; ++y)
    {
      for (int x = king.x() - 1; x <= king.x() + 1; ++x)
      {
	const osl::Square target(x, y);
	if (target.isValid() && state.pieceAt(target).isEmpty())
	{
	  std::cout << (state.countEffect(osl::alt(player), target) - state.countEffect(player, target))
		    << " ";
	}
	else
	{
	  std::cout << -11 << " ";
	}
      }
    }
  }
protected:
  void piecesOnStand(const osl::state::NumEffectState &state,
		     const osl::Player player)
  {
    for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
    {
      std::cout << state.countPiecesOnStand(player, osl::PieceStand::order[i])
		<< " ";
    }
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    king8EffectPlus(state, osl::BLACK);
    king8EffectPlus(state, osl::WHITE);
    piecesOnStand(state, osl::BLACK);
    piecesOnStand(state, osl::WHITE);
  }
};

struct King8Escape : public Base
{
  void king8Escape(const osl::state::NumEffectState &state,
		   const osl::Player player)
  {
    const osl::Square king = state.kingSquare(player);
    for (int y = king.y() - 1; y <= king.y() + 1; ++y)
    {
      for (int x = king.x() - 1; x <= king.x() + 1; ++x)
      {
	const osl::Square target(x, y);
	if (target.isValid() && state.pieceAt(target).isEmpty() &&
	    !state.hasEffectAt(osl::alt(player), target))
	{
	  std::cout << "1 ";
	}
	else
	{
	  std::cout << "0 ";
	}
      }
    }
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    king8Escape(state, osl::BLACK);
    king8Escape(state, osl::WHITE);
  }
};

struct KingEscapeRoute : public Base
{
  osl::misc::CArray<bool, 64> escape_map;
  KingEscapeRoute() {
#if 0
    escape_map.fill(false);
    addEscapeMap(escape_map, -1, -1);
    addEscapeMap(escape_map, -1, 0);
    addEscapeMap(escape_map, 0, -1);
    addEscapeMap(escape_map, 0, 0);
    addEscapeMap(escape_map, 0, 1);
    addEscapeMap(escape_map, 1, 0);
    addEscapeMap(escape_map, 1, 1);
#endif
  }

  // *  y1  y2
  //+KI *  *  
  // *  *  * 
  static void addEscapeMap(osl::misc::CArray<bool, 64> &escape_map,
			   int relative_y1, int relative_y2)
  {
    ++relative_y1;
    ++relative_y2;
#if 0
    for (size_t i = 0; i < escape_map.size(); ++i)
    {
      escape_map[i | (1 << relative_y1) | (1 << (relative_y2 + 3))] = true;
    }
#endif
  }
  int canKingEscape(const osl::state::NumEffectState &state,
		    const osl::Player player)
  {
    const osl::Square king = state.kingSquare(player);
    if (king.x() <= 7)
    {
      int escape = 0;
      for (int x = king.x() + 1; x <= king.x() + 2; ++x)
      {
	for (int y = king.y() - 1; y <= king.y() + 1; ++y)
	{
	  const osl::Square pos(x, y);
	  if (!pos.isValid())
	    continue;
	  if (state.pieceAt(pos).isEmpty() &&
	      !state.hasEffectAt(osl::alt(player), pos))
	  {
	    escape |= (1 << (y - king.y() + 1 + 3 * (x - king.x() - 1)));
	  }
	  if (state.hasEffectAt(osl::alt(player), pos) ||
	      !(state.hasEffectAt(player, pos) ||
		state.pieceAt(pos).isOnBoardByOwner(player)))
	  {
	    goto done;
	  }
	}
      }
      //if (escape_map[escape])
      return 1;
    }
  done:
    if (king.x() >= 3)
    {
      int escape = 0;
      for (int x = king.x() - 1; x <= king.x() - 2; --x)
      {
	for (int y = king.y() - 1; y <= king.y() + 1; ++y)
	{
	  const osl::Square pos(x, y);
	  if (!pos.isValid())
	    continue;
	  if (state.pieceAt(pos).isEmpty() &&
	      !state.hasEffectAt(osl::alt(player), pos))
	  {
	    escape |= (1 << (y - king.y() + 1 + 3 * (king.x() - x - 1)));
	    //std::cout << "2 " << escape << std::endl;
	  }
	  if (state.hasEffectAt(osl::alt(player), pos) ||
	      !(state.hasEffectAt(player, pos) ||
		state.pieceAt(pos).isOnBoardByOwner(player)))
	  {
	    return 0;
	  }
	}
      }
      //if (escape_map[escape])
      return 1;
    }
    return 0;
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << canKingEscape(state, osl::BLACK) << " "
	      << canKingEscape(state, osl::WHITE) << " ";
  }
};

struct KingNoEffect : public Base
{
  void kingNoEffect(const osl::state::NumEffectState &state,
		    const osl::Player player)
  {
    const osl::Square king = state.kingSquare(player);
    for (int x = king.x() - 1; x <= king.x() + 1; ++x)
    {
      for (int y = king.y() - 1; y <= king.y() + 1; ++y)
      {
	const osl::Square pos(x, y);
	if (!pos.isValid())
	{
	  std::cout << "0 ";
	  continue;
	}
	if ((state.pieceAt(pos).isEmpty() &&
	     state.countEffect(player, pos) == 1) ||
	    (pos == king && state.countEffect(player, pos) == 0))
	  std::cout << "1 ";
	else
	  std::cout << "0 ";
      }
    }
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    kingNoEffect(state, osl::BLACK);
    kingNoEffect(state, osl::WHITE);
  }
};

struct KnightCheck : public King8Effect
{
  int knightCheck(const osl::state::NumEffectState &state,
		  const osl::Player player)
  {
    const osl::Square king = state.kingSquare(player);
    const osl::Square up = osl::Board_Table.nextSquare(player, king, osl::U);
    if  (up.isValid())
    {
      const osl::Square ul = osl::Board_Table.nextSquare(player, up, osl::UL);
      const osl::Square ur = osl::Board_Table.nextSquare(player, up, osl::UR);
      if (ur.isValid() && !state.hasEffectAt(player, ur) &&
	  (state.countPiecesOnStand<osl::KNIGHT>(osl::alt(player)) > 0 ||
	   state.hasEffectByPtype<osl::KNIGHT>(osl::alt(player), ur)) &&
	  !state.hasEffectAt(player, king))
	return 1;

      if (ul.isValid() && !state.hasEffectAt(player, ul) &&
	  (state.countPiecesOnStand<osl::KNIGHT>(osl::alt(player)) > 0 ||
	  state.hasEffectByPtype<osl::KNIGHT>(osl::alt(player), ul)) &&
	  !state.hasEffectAt(player, king))
	return 1;
    }
    return 0;
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << knightCheck(state, osl::BLACK) << " "
	      << knightCheck(state, osl::WHITE) << " ";
    piecesOnStand(state, osl::BLACK);
    piecesOnStand(state, osl::WHITE);
  }
};

struct PawnOnBoard : public Base
{
  void printPawn(const osl::state::NumEffectState &state,
		 osl::Player player)
  {
    for (int x = 1; x <= 9; ++x)
    {
      std::cout << state.isPawnMaskSet(player, x) << " ";
    }
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    printPawn(state, osl::BLACK);
    printPawn(state, osl::WHITE);
  }
};

struct PawnAttackBase : public Base
{
#if 0
  void pawnAttackBase(const osl::state::NumEffectState &state,
		      osl::Player player)
  {
    // TODO: specialize per position, specialize per pieces on stand?
    osl::Square king = state.kingSquare(osl::alt(player));
    for (int i = osl::PtypeTraits<osl::PAWN>::indexMin;
	 i < osl::PtypeTraits<osl::PAWN>::indexLimit;
	 ++i)
    {
      osl::Piece pawn = state.pieceOf(i);
      if (pawn.isOnBoardByOwner(player))
      {
	if (std::abs(pawn.square().x() - king.x()) <= 2 &&
	    std::abs(pawn.square().y() - king.y()) <= 3)
	{
	  if (player == osl::BLACK)
	  {
	    if (pawn.square().y() > king.y() + 1 &&
		pawn.square().y() >= 3 &&
		pawn.square().y() == king.y() + 3 &&
		state.pieceAt(
		  osl::Square(pawn.square().x(),
				pawn.square().y() - 1)).isEmpty() &&
		!state.pieceAt(
		  osl::Square(pawn.square().x(),
				pawn.square().y() - 2)).isEmpty() &&
		state.pieceAt(
		  osl::Square(pawn.square().x(),
				pawn.square().y() - 2)).ptype() != osl::PAWN)
	    {
	      std::cout << "1 ";
	      return;
	    }
	  }
	  else
	  {
	    if (pawn.square().y() < king.y() - 1 &&
		pawn.square().y() <= 7 &&
		pawn.square().y() == king.y() - 3 &&
		state.pieceAt(
		  osl::Square(pawn.square().x(),
				pawn.square().y() + 1)).isEmpty() &&
		!state.pieceAt(
		  osl::Square(pawn.square().x(),
				pawn.square().y() + 2)).isEmpty() &&
		state.pieceAt(
		  osl::Square(pawn.square().x(),
				pawn.square().y() + 2)).ptype() != osl::PAWN)
	    {
	      std::cout << "1 ";
	      return;
	    }
	  }
	}
      }
    }
    std::cout << "0 ";
  }
#else
  template <osl::Player P>
  int pawnBaseBonus(const osl::state::NumEffectState& state,
		    const osl::Square pos)
  {
    if (state.pieceAt(pos).isEmpty() &&
	state.hasEffectByPtype<osl::PAWN>(P, pos))
    {
      const osl::Piece piece =
	state.pieceAt(pos + osl::Board_Table.getOffset<P>(osl::U));
      if (piece.isOnBoardByOwner(osl::PlayerTraits<P>::opponent) &&
	  piece.ptype() != osl::PAWN)
	return 1;
    }
    return 0;
  }
  // P is attack
  template <osl::Player P>
  void
  pawnAttackBase(const osl::state::NumEffectState& state)
  {
    const osl::Square king =
      state.kingSquare<osl::PlayerTraits<P>::opponent>();
    const int min_x = king.x() - 2;
    const int max_x = king.x() + 2;
    const int y1 = king.y() + osl::PlayerTraits<P>::offsetMul;
    const int y2 = king.y() + osl::PlayerTraits<P>::offsetMul * 2;
    for (int x = min_x; x <= max_x; ++x)
    {
      if (x < 1 || x > 9 || y1 < 1 || y1 > 9)
	std::cout << "0 ";
      else
      {
	osl::Square pos(x, y1);
	std::cout << (king.canPromote<osl::PlayerTraits<P>::opponent>() ? 0 :
		      pawnBaseBonus<P>(state, pos)) << " ";
      }
    }
    for (int x = min_x; x <= max_x; ++x)
    {
      if (x < 1 || x > 9 || y2 < 1 || y2 > 9)
	std::cout << "0 ";
      else
      {
	osl::Square pos(x, y2);
	std::cout << (king.canPromote<osl::PlayerTraits<P>::opponent>() ? 0 :
		      pawnBaseBonus<P>(state, pos)) << " ";
      }
    }
  }
#endif
public:
  void print(const osl::state::NumEffectState &state)
  {
    pawnAttackBase<osl::BLACK>(state);
    pawnAttackBase<osl::WHITE>(state);
    piecesOnStand(state, osl::BLACK);
    piecesOnStand(state, osl::WHITE);
  }
};

struct BishopMobilityCount : public Base
{
  void print(const osl::state::NumEffectState &state)
  {
    for (int i = osl::PtypeTraits<osl::BISHOP>::indexMin;
	 i < osl::PtypeTraits<osl::BISHOP>::indexLimit;
	 ++i)
    {
      const osl::Piece bishop = state.pieceOf(i);
      std::cout << bishop.owner() << " "
		<< (bishop.ptype() == osl::PBISHOP ? 1 : 0) << " ";
      if (bishop.isOnBoard())
      {
	std::cout << osl::mobility::BishopMobility::countAll(bishop.owner(), state, bishop)
		  << " "
		  << bishop.square().x() << " "
		  << bishop.square().y() << " ";
      }
      else
      {
	std::cout << "-1 -1 -1 ";
      }
    }
  }
};

struct MajorPieces : public Base
{
  void print(const osl::state::NumEffectState &state)
  {
    int bishop_count = 0;
    int rook_count = 0;
    for (int i = osl::PtypeTraits<osl::BISHOP>::indexMin;
	 i < osl::PtypeTraits<osl::BISHOP>::indexLimit;
	 ++i)
    {
      const osl::Piece bishop = state.pieceOf(i);
      if (bishop.owner() == osl::BLACK)
	++bishop_count;
    }
    for (int i = osl::PtypeTraits<osl::ROOK>::indexMin;
	 i < osl::PtypeTraits<osl::ROOK>::indexLimit;
	 ++i)
    {
      const osl::Piece rook = state.pieceOf(i);
      if (rook.owner() == osl::BLACK)
	++rook_count;
    }
    std::cout << rook_count << " " << bishop_count << " ";
  }
};

struct NiseAnaguma : public Base
{
  void openAnaguma(const osl::state::NumEffectState &state,
		   osl::Player player)
  {
    const osl::Square pos = state.kingSquare(player);
    if ((player == osl::BLACK && pos.y() == 9 &&
	 ((pos.x() == 1 && state.pieceAt(osl::Square(2, 8)).isEmpty()) ||
	  (pos.x() == 9 && state.pieceAt(osl::Square(8, 8)).isEmpty()))) ||
      (player == osl::WHITE && pos.y() == 1 &&
	 ((pos.x() == 1 && state.pieceAt(osl::Square(2, 2)).isEmpty()) ||
	  (pos.x() == 9 && state.pieceAt(osl::Square(8, 2)).isEmpty()))))
    {
      std::cout << "1 ";
    }
    else
    {
      std::cout << " ";
    }
  }
  
public:
  void print(const osl::state::NumEffectState &state)
  {
    openAnaguma(state, osl::BLACK);
    openAnaguma(state, osl::WHITE);
  }
};

struct KingCheck : public Base
{
  int canCheck(const osl::state::NumEffectState &state,
	       osl::Player player)
  {
    const osl::Player defense = osl::alt(player);
    const osl::Square king = state.kingSquare(defense);
    // Already in check
    if (osl::EffectUtil::isKingInCheck(defense, state))
      return 1;
    
    // Check by knight
    const osl::Square up = osl::Board_Table.nextSquare(defense, king, osl::U);
    if  (up.isValid())
    {
      const osl::Square ul = osl::Board_Table.nextSquare(defense, up, osl::UL);
      const osl::Square ur = osl::Board_Table.nextSquare(defense, up, osl::UR);
      if (ur.isValid() &&
	  !state.pieceAt(ur).isOnBoardByOwner(player) &&
	  (state.pieceAt(ur).isEmpty() &&
	   state.hasPieceOnStand(player, osl::KNIGHT)) ||
	  (state.hasEffectByPtype<osl::KNIGHT>(player, ur)))
	return 1;

      if (ul.isValid() &&
	  !state.pieceAt(ul).isOnBoardByOwner(player) &&
	  (state.pieceAt(ul).isEmpty() &&
	   state.hasPieceOnStand(player, osl::KNIGHT)) ||
	  (state.hasEffectByPtype<osl::KNIGHT>(player, ul)))
	return 1;
    }
    for (int y = king.y() - 1; y <= king.y() + 1; ++y)
    {
      for (int x = king.x() - 1; x <= king.x() + 1; ++x)
      {
	const osl::Square pos(x, y);
	if (!pos.isValid() || pos == king)
	  continue;
	if (!state.pieceAt(pos).isOnBoardByOwner(defense) ||
	    state.hasEffectAt(player, pos))
	  return 1;
      }
    }
    return 0;
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << canCheck(state, osl::BLACK) << " "
	      << canCheck(state, osl::WHITE) << " ";
  }
};

struct PawnAndLance : public Base
{
  void pawnAndLance(const osl::state::NumEffectState &state)
  {
    for (int i = osl::PtypeTraits<osl::LANCE>::indexMin;
	 i < osl::PtypeTraits<osl::LANCE>::indexLimit;
	 ++i)
    {
      const osl::Piece lance = state.pieceOf(i);
      std::cout << lance.owner() << " ";
      if (lance.square().isOnBoard() && !lance.isPromoted())
      {
	if (!state.isPawnMaskSet(lance.owner(), lance.square().x()))
	  std::cout << "2 ";
	else
	{
	  for (int y = 1; y <= 9; ++y)
	  {
	    const osl::Piece piece = state.pieceAt(osl::Square(lance.square().x(), y));
	    if (piece.isOnBoardByOwner(lance.owner()) &&
		piece.ptype() == osl::PAWN)
	    {
	      if ((y < lance.square().y() && lance.owner() == osl::BLACK) ||
		  (y > lance.square().y() && lance.owner() == osl::WHITE))
		std::cout << "0 ";
	      else
		std::cout <<  "1 ";
	      break;
	    }
	  }
	}
      }
      else
	std::cout << "-1 ";
    }
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    pawnAndLance(state);
  }
};

struct PawnAndBishop : public Base
{
  void pawnAndBishop(const osl::state::NumEffectState &state)
  {
    for (int i = osl::PtypeTraits<osl::BISHOP>::indexMin;
	 i < osl::PtypeTraits<osl::BISHOP>::indexLimit;
	 ++i)
    {
      const osl::Piece bishop = state.pieceOf(i);
      std::cout << bishop.owner() << " ";
      int val = 0;
      if (bishop.square().isOnBoard())
      {
	for (int j = osl::PtypeTraits<osl::PAWN>::indexMin;
	     j < osl::PtypeTraits<osl::PAWN>::indexLimit;
	     ++j)
	{
	  const osl::Piece pawn = state.pieceOf(j);
	  if (pawn.isOnBoardByOwner(bishop.owner()) &&
	      !pawn.isPromoted() &&
	      std::abs(pawn.square().x() - pawn.square().y()) % 2 ==
	      std::abs(bishop.square().x() - bishop.square().y()) % 2)
	    ++val;
	}
	std::cout << val << " ";
      }
    else
      std::cout << "-1 ";
    }
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    pawnAndBishop(state);
  }
};

struct KnightAdvance : public Base
{
  bool canAdvance(const osl::state::NumEffectState &state,
		  const osl::Player player, const osl::Square pos)
  {
    return (pos.isValid() && !state.pieceAt(pos).isOnBoardByOwner(player) &&
	    !state.hasEffectAt(osl::alt(player), pos));
  }

  void knightAdvance(const osl::state::NumEffectState &state)
  {
    for (int i = osl::PtypeTraits<osl::KNIGHT>::indexMin;
	 i < osl::PtypeTraits<osl::KNIGHT>::indexLimit;
	 ++i)
    {
      const osl::Piece knight = state.pieceOf(i);
      std::cout << knight.owner() << " ";
      if (knight.square().isOnBoard() && !knight.isPromoted())
      {
	const osl::Square ul =
	  osl::Board_Table.nextSquare(knight.owner(), knight.square(),
					osl::UUL);
	const osl::Square ur =
	  osl::Board_Table.nextSquare(knight.owner(), knight.square(),
					osl::UUR);
	std::cout << (canAdvance(state, knight.owner(), ul) ||
		      canAdvance(state, knight.owner(), ur)) << " ";
      }
    else
      std::cout << "-1 ";
    }
  }
public:
  void print(const osl::state::NumEffectState &state)
  {
    knightAdvance(state);
  }
};

struct Threat : public Base
{
private:
  osl::search::SimpleHashTable table;
  osl::search::SearchState2Core::checkmate_t checkmate_searcher;
  osl::search::SearchState2 state2;
public:
  Threat() : state2(osl::state::NumEffectState(osl::state::SimpleState(osl::HIRATE)), checkmate_searcher)
  {
  }
  void print(const osl::state::NumEffectState &state)
  {
    state2.setState(state);
    osl::eval::ProgressEval eval(state);
    osl::search::QuiescenceSearch2<osl::eval::ProgressEval> qsearch(state2,
								    table);
    osl::search::QuiescenceThreat threat1, threat2;
    if (!osl::EffectUtil::isKingInCheck(osl::alt(state.turn()), state)
	&& !osl::EffectUtil::isKingInCheck(state.turn(), state))
    {
      int val;
      if (state.turn() == osl::BLACK)
	val = qsearch.staticValueWithThreat<osl::BLACK>(
	  eval,
	  osl::search::FixedEval::winThreshold(osl::WHITE),
	  threat1, threat2);
      else
	val = qsearch.staticValueWithThreat<osl::WHITE>(
	  eval,
	  osl::search::FixedEval::winThreshold(osl::BLACK),
	  threat1, threat2);
      std::cout << val << " " << threat1.value << " " << threat2.value << " ";
    }
    else
    {
      std::cout << "X -1 -1 ";
    }
  }
};

struct LanceEffectPiece : public Base
{
  void print(const osl::state::NumEffectState &state)
  {
    for (int i = osl::PtypeTraits<osl::LANCE>::indexMin;
	 i < osl::PtypeTraits<osl::LANCE>::indexLimit;
	 ++i)
    {
      const osl::Piece lance = state.pieceOf(i);
      std::cout << lance.owner() << " ";
      if (!lance.isOnBoard())
      {
	std::cout << "X -1 ";
	continue;
      }
      osl::Square pos =
	osl::Board_Table.nextSquare(lance.owner(),
				      lance.square(), osl::U);
      while (pos.isOnBoard() && state.pieceAt(pos).isEmpty())
      {
	pos = osl::Board_Table.nextSquare(lance.owner(), pos, osl::U);
      }
      if (!pos.isOnBoard())
	std::cout << lance.owner() << "1 ";
      else
      {
	const osl::Piece piece = state.pieceAt(pos);
	std::cout << piece.owner() << " "
		  << static_cast<int>(piece.ptype()) << " ";
      }
    }
  }
};

struct RookEffectPiece : public Base
{
  osl::Piece getPieceOnDir(const osl::state::NumEffectState &state,
			   const osl::Square rook,
			   const osl::Direction dir)
  {
    osl::Square pos =
      osl::Board_Table.nextSquare(osl::BLACK,
				    rook, dir);
    while (pos.isOnBoard() && state.pieceAt(pos).isEmpty())
    {
      pos = osl::Board_Table.nextSquare(osl::BLACK, pos, dir);
    }
    if (!pos.isOnBoard())
      return osl::Piece::EDGE();
    else
      return state.pieceAt(pos);
  }
  void print(const osl::state::NumEffectState &state)
  {
    for (int i = osl::PtypeTraits<osl::ROOK>::indexMin;
	 i < osl::PtypeTraits<osl::ROOK>::indexLimit;
	 ++i)
    {
      const osl::Piece rook = state.pieceOf(i);
      std::cout << rook.owner() << " ";
      // print out rook position, pieces on each directions.
      if (!rook.isOnBoard())
      {
	std::cout << "-1 -1 X -1 X -1 X -1 X -1 ";
	continue;
      }
      std::cout << rook.square().x() << " "
		<< rook.square().y() << " ";
      osl::misc::CArray<osl::Direction, 4> dirs
	= {{osl::U, osl::D, osl::L, osl::R}};
      for (size_t j = 0; j < dirs.size(); ++j)
      {
	const osl::Piece target = getPieceOnDir(state, rook.square(),
						dirs[j]);
	const osl::Player p = target.isPiece() ? target.owner() : rook.owner();
	std::cout << p << " "
		  << static_cast<int>(target.ptype()) << " ";
      }
    }
  }
};

struct NoPawnDefense : public Base
{
  int noDefense(const osl::state::NumEffectState &state, osl::Player player)
  {
    int count = 0;
    const osl::Square king = state.kingSquare(player);
    for (int x = std::max(1, king.x() - 1);
	 x <= std::min(9, king.x() + 1); ++x)
    {
      for (osl::Square pos =
	     osl::Board_Table.nextSquare(
	       player, osl::Square(x, king.y()), osl::U);
	   pos.isValid();
	   pos = osl::Board_Table.nextSquare(player, pos, osl::U))
      {
	const osl::Piece piece = state.pieceAt(pos);
	if (piece.isOnBoardByOwner(player) && piece.ptype() == osl::PAWN)
	  break;
	if (piece.isEmpty() && !state.hasEffectAt(player, pos) &&
	    (state.isPawnMaskSet(player, x) ||
	     !state.hasPieceOnStand(player, osl::PAWN)))
	{
	  ++count;
	  break;
	}
      }
    }
    return count;
  }
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << noDefense(state, osl::BLACK) << " "
	      << noDefense(state, osl::WHITE) << " ";
  }
};

struct KingEscapeRoute2 : public Base
{
  // left effect, right effect, gold, silver, bishop pieces on stand,
  // rook attack

  // P is attack
  template <osl::Player P, bool LEFT>
  int countEffect(const osl::state::NumEffectState &state)
  {
    const osl::Square king =
      state.kingSquare<osl::PlayerTraits<P>::opponent>();
    const osl::Square center =
      osl::Centering5x3::adjustCenter(king);
    if ((((P == osl::BLACK && LEFT) || (P == osl::WHITE && !LEFT)) &&
	 king.x() >= 8) ||
	(((P == osl::WHITE &&LEFT) || (P == osl::BLACK && !LEFT)) &&
	 king.x() <= 2))
      return -1;
    int min_x = ((P == osl::BLACK && LEFT) || (P == osl::WHITE && !LEFT)) ?
      king.x() + 1 : king.x() - 2;
    int max_x = ((P == osl::BLACK && LEFT) || (P == osl::WHITE && !LEFT)) ?
      king.x() + 2 : king.x() - 1;
    int count = 0;
    for (int x = min_x; x <= max_x; ++x)
    {
      for (int y = center.y() - 1; y <= center.y() + 1; ++y)
      {
	const osl::Square pos(x, y);
	count += state.countEffect(P, pos);
//	if (state.hasEffectAt<P>(pos))
//	  ++count;
      }
    }
    return count;
  }
  template <osl::Player P>
  bool hasAttackPieceOnStand(const osl::state::NumEffectState &state)
  {
    return state.hasPieceOnStand<P, osl::BISHOP>() ||
      state.hasPieceOnStand<P, osl::GOLD>() ||
      state.hasPieceOnStand<P, osl::SILVER>();
  }
  template <osl::Player P>
  bool hasRookToAttack(const osl::state::NumEffectState &state)
  {
    for (int i = osl::PtypeTraits<osl::ROOK>::indexMin;
	 i < osl::PtypeTraits<osl::ROOK>::indexLimit;
	 ++i)
    {
      const osl::Piece rook = state.pieceOf(i);
      if (rook.owner() == P &&
	  (!rook.isOnBoard() || rook.square().canPromote<P>()))
	return true;
    }
    return false;
  }
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << countEffect<osl::BLACK, true>(state) << " "
	      << countEffect<osl::BLACK, false>(state) << " "
	      << hasAttackPieceOnStand<osl::BLACK>(state) << " "
	      << hasRookToAttack<osl::BLACK>(state) << " "
	      << countEffect<osl::WHITE, true>(state) << " "
	      << countEffect<osl::WHITE, false>(state) << " "
	      << hasAttackPieceOnStand<osl::WHITE>(state) << " "
	      << hasRookToAttack<osl::WHITE>(state) << " ";
  }
};

struct KingToEnter : public Base
{
  template <osl::Player P>
  bool toEnter(const osl::state::NumEffectState &state)
  {
    const osl::Square king = state.kingSquare<P>();
    if (!king.canPromote<osl::PlayerTraits<P>::opponent >())
    {
      int adjusted_x = king.x();
      if (adjusted_x == 9)
	--adjusted_x;
      else if (adjusted_x == 1)
	++adjusted_x;
      int min_y;
      int max_y;
      if (P == osl::BLACK)
      {
	min_y = std::max(1, king.y() - 2);
	max_y = king.y();
      }
      else
      {
	min_y = king.y();
	max_y = std::min(9, king.y() + 2);
      }
      for (int x = adjusted_x - 1; x <= adjusted_x + 1; ++x)
      {
	for (int y = min_y; y <= max_y; ++y)
	{
	  const osl::Square pos(x, y);
	  if (state.hasEffectAt<osl::PlayerTraits<P>::opponent>(pos))
	    return false;
	}
      }
      return true;
    }
    return false;
  }

  void print(const osl::state::NumEffectState &state)
  {
    std::cout << toEnter<osl::BLACK>(state) << " "
	      << toEnter<osl::WHITE>(state) << " ";
  }
};

struct ControlX : public Base
{
  template <osl::Player P>
  int controlSquare(const osl::state::NumEffectState &state)
  {
    const osl::Piece king =
      state.kingPiece<osl::PlayerTraits<P>::opponent>();
    const osl::Square center =
      osl::Centering5x3::adjustCenter(king.square());
    int min_x = 10;
    for (int x = 1; x <= 9; ++x)
    {
      for (int y = center.y() - 1; y <= center.y() + 1; ++y)
      {
	osl::Square pos(x, y);
	int defense_effect =
	  state.countEffect(osl::PlayerTraits<P>::opponent, pos);
	if (state.hasEffectByPiece(king, pos))
	  --defense_effect;
	if (state.countEffect(P, pos) > defense_effect)
	{
	  min_x = std::min(min_x, std::abs(x - king.square().x()));
	}
      }
    }
    return min_x;
  }

  void print(const osl::state::NumEffectState &state)
  {
    std::cout << controlSquare<osl::BLACK>(state) << " "
	      << controlSquare<osl::WHITE>(state) << " ";
  }
};

struct RookSafeMobility : public Base
{
  void print(const osl::state::NumEffectState &state)
  {
    for (int i = osl::PtypeTraits<osl::ROOK>::indexMin;
	 i < osl::PtypeTraits<osl::ROOK>::indexLimit;
	 ++i)
    {
      const osl::Piece piece = state.pieceOf(i);
      std::cout << piece.owner() << " "
		<< (osl::isPromoted(piece.ptype()) ? 1 : 0) << " ";
      if (piece.isOnBoard())
      {
	std::cout << (osl::mobility::RookMobility::countVerticalSafe(piece.owner(), state, piece) +
		      osl::mobility::RookMobility::countHorizontalSafe(piece.owner(), state, piece))
		  << " "
		  << piece.square().x() << " "
		  << piece.square().y() << " ";
      }
      else
      {
	std::cout << "-1 -1 -1 ";
      }
    }
  }
};

struct BishopSafeMobility : public Base
{
  void print(const osl::state::NumEffectState &state)
  {
    for (int i = osl::PtypeTraits<osl::BISHOP>::indexMin;
	 i < osl::PtypeTraits<osl::BISHOP>::indexLimit;
	 ++i)
    {
      const osl::Piece piece = state.pieceOf(i);
      std::cout << piece.owner() << " "
		<< (osl::isPromoted(piece.ptype()) ? 1 : 0) << " ";
      if (piece.isOnBoard())
      {
	std::cout << osl::mobility::BishopMobility::countSafe(piece.owner(), state, piece)
		  << " "
		  << piece.square().x() << " "
		  << piece.square().y() << " ";
      }
      else
      {
	std::cout << "-1 -1 -1 ";
      }
    }
  }
};

struct Rank7Effect : public Base
{
  template <osl::Player P>
  int countEffect(const osl::state::NumEffectState &state)
  {
    const osl::Square king = state.kingSquare<P>();
    if ((P == osl::BLACK && king.y() != 9) ||
	(P == osl::WHITE && king.y() != 1))
      return -1;

    const osl::Square center = osl::Centering5x3::adjustCenter(king);
    int count = 0;
    const int y = P == osl::BLACK ? 7 : 3;
    for (int x = center.x() - 2; x <= center.x() + 2; ++x)
    {
      const osl::Square target(x, y);
      count += state.countEffect(osl::PlayerTraits<P>::opponent, target);
    }
    return count;
  }
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << countEffect<osl::BLACK>(state) << " "
	      << countEffect<osl::WHITE>(state) << " ";
  }
};

struct Rank9Effect : public Base
{
  template <osl::Player P>
  int countEffect(const osl::state::NumEffectState &state)
  {
    const osl::Square king = state.kingSquare<P>();
    if ((P == osl::BLACK && king.y() != 7) ||
	(P == osl::WHITE && king.y() != 3))
      return -1;

    const osl::Square center = osl::Centering5x3::adjustCenter(king);
    int count = 0;
    const int y = P == osl::BLACK ? 9 : 1;
    for (int x = center.x() - 2; x <= center.x() + 2; ++x)
    {
      const osl::Square target(x, y);
      count += state.countEffect(osl::PlayerTraits<P>::opponent, target);
    }
    return count;
  }
  void print(const osl::state::NumEffectState &state)
  {
    std::cout << countEffect<osl::BLACK>(state) << " "
	      << countEffect<osl::WHITE>(state) << " ";
  }
};

struct IrlsEval : public Base
{
  std::unique_ptr<gpsshogi::Eval> eval;
  IrlsEval(const std::string &name)
  {
    eval.reset(gpsshogi::EvalFactory::newEval(name));
  }
  void print(const osl::state::NumEffectState &state)
  {
    double dummy;
    osl::vector<std::pair<int, double> > features;
    eval->features(state, dummy, features, 0);
    size_t index = 0;
    for (size_t i = 0; i < eval->dimension(); ++i)
    {
      if (index < features.size() &&
	  features[index].first == static_cast<int>(i))
      {
	std::cout << features[index].second << " ";
	++index;
      }
      else
      {
	std::cout << "0 ";
      }
    }
  }
};

Base *newBase(const std::string &output)
{
  if (output == "unsupported_gs")
  {
    return new King24UnSupportedGS;
  }
  else if (output == "king24gs")
  {
    return new King24GSCount;
  }
  else if (output == "bishop")
  {
    return new Bishop1_1;
  }
  else if (output == "bishop2")
  {
    return new Bishop2;
  }
  else if (output == "turn")
  {
    return new Turn;
  }
  else if (output == "king8effect")
  {
    return new King8Effect;
  }
  else if (output == "kingescape")
  {
    return new KingEscapeRoute;
  }
  else if (output == "kingescape2")
  {
    return new KingEscapeRoute2;
  }
  else if (output == "king8escape")
  {
    return new King8Escape;
  }
  else if (output == "knightcheck")
  {
    return new KnightCheck;
  }
  else if (output == "pawn")
  {
    return new PawnOnBoard;
  }
  else if (output == "pawnbase")
  {
    return new PawnAttackBase;
  }
  else if (output == "bishopmobility")
  {
    return new BishopMobilityCount;
  }
  else if (output == "majorpieces")
  {
    return new MajorPieces;
  }
  else if (output == "niseanaguma")
  {
    return new NiseAnaguma;
  }
  else if (output == "kingnoeffect")
  {
    return new KingNoEffect;
  }
  else if (output == "anacheck")
  {
    return new KingCheck;
  }
  else if (output == "pawnlance")
  {
    return new PawnAndLance;
  }
  else if (output == "pawnbishop")
  {
    return new PawnAndBishop;
  }
  else if (output == "knightadvance")
  {
    return new KnightAdvance;
  }
  else if (output == "threat")
  {
    return new Threat;
  }
  else if (output == "pawnunderrook")
  {
    return new PuR;
  }
  else if (output == "lanceeffectpiece")
  {
    return new LanceEffectPiece;
  }
  else if (output == "rookeffectpiece")
  {
    return new RookEffectPiece;
  }
  else if (output == "nopawndefense")
  {
    return new NoPawnDefense;
  }
  else if (output == "king8defense")
  {
    return new King8Defense;
  }
  else if (output == "kingtoenter")
  {
    return new KingToEnter;
  }
  else if (output == "control-x")
  {
    return new ControlX;
  }
  else if (output == "saferook")
  {
    return new RookSafeMobility;
  }
  else if (output == "safebishop")
  {
    return new BishopSafeMobility;
  }
  else if (output == "rank7")
  {
    return new Rank7Effect;
  }
  else if (output == "rank9")
  {
    return new Rank9Effect;
  }
  else if (output == "piece")
  {
    return new IrlsEval(output);
  }
  return NULL;
}

int main(int argc, char **argv)
{
  size_t start, end;
  bool include_all;
  std::string output;
  bool do_qsearch, ignore_checkmate_state;
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
    ("do-qsearch",
     boost::program_options::value<bool>(&do_qsearch)->default_value(false),
     "Run qsearch to determine static evaluation")
    ("ignore-checkmate",
     boost::program_options::value<bool>(&ignore_checkmate_state)->default_value(true),
     "When true, exclude states where player with turn has checkmate moves")
    ("output",
     boost::program_options::value<std::string>(&output)->default_value(""),
     "Decide the output style to use")
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

  std::unique_ptr<Base> out(newBase(output));
 
  if (!out)
  {
    std::cerr << "Unknown output class " << output << std::endl;
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

  osl::search::SimpleHashTable table;
  osl::search::SearchState2Core::checkmate_t checkmate_searcher;
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
    osl::progress::Effect5x3WithBonus progress(state);
    osl::progress::Effect5x3d defense(state);
#if 0
    osl::search::SearchState2 core(state, checkmate_searcher);
#endif

    std::string result = results[i - start];
    size_t end_index = moves.size();
    if (result.find(" ") != std::string::npos)
    {
      if (ignore_checkmate_state)
      {
	end_index =
	  std::max(0, atoi(result.substr(result.find(" ")).c_str()) - 1);
	assert(end_index <= moves.size());
      }
      result = result.substr(0, 1);
    }
    for (size_t j = 0; j <= end_index; j++)
    {
      const osl::Square black_king = state.kingSquare(osl::BLACK);
      const osl::Square white_king = state.kingSquare(osl::WHITE);
#if 0
      core.setState(state);
      osl::search::QuiescenceSearch2<osl::eval::ProgressEval> qsearch(core,
								      table);
#endif
      std::cout << result << " ";
      if (!do_qsearch)
	std::cout << eval.value() << " ";
#if 0
      else
      {
	std::cout << qsearch.search(
	  state.turn(), eval,
	  j == 0 ? osl::Move::PASS(osl::WHITE) : moves[j-1], 0) << " ";
      }
#endif
      std::cout << progress.progress16bonus(osl::BLACK).value() << " "
		<< progress.progress16bonus(osl::WHITE).value() << " "
		<< defense.progress16(osl::BLACK).value() << " "
		<< defense.progress16(osl::WHITE).value() << " "
		<< eval.progress16().value() << " "
		<< black_king.x() << " "
		<< black_king.y() << " "
		<< white_king.x() << " "
		<< white_king.y() << " "
	;
      out->print(state);
      std::cout << std::endl;
      if (j != moves.size())
      {
	osl::apply_move::ApplyMoveOfTurn::doMove(state, moves[j]);
	eval.update(state, moves[j]);
	progress.update(state, moves[j]);
	defense.update(state, moves[j]);
      }
    }
  }

  return 0;
}
