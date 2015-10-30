/* checkMoveGenerator.tcc
 */
#ifndef _CHECKMOVEGENERATOR_TCC
#define _CHECKMOVEGENERATOR_TCC

#include "checkMoveGenerator.h"
#include "checkMoveList.h"
#include "checkMoveListProvider.h"
#include "checkAssert.h"
#include "safeFilter.h"
#include "checkFilter.h"
#include "osl/checkmate/pawnCheckmateMoves.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/move_action/store.h"
#include "osl/move_classifier/check_.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/pawnDropCheckmate.h"
#include "osl/container/moveVector.h"
#include <cassert>

// TODO: 専用の move_action を作って高速化する

template <osl::Player P>
void osl::checkmate::CheckMoveGenerator<P>::
generateAttack(const NumEffectState& state, CheckMoveListProvider& src,
	       CheckMoveList& out, bool& has_pawn_checkmate)
{
  has_pawn_checkmate = false;
  using namespace move_classifier;
  check_assert(out.empty());
  const Square targetKing
    = state.template kingSquare<PlayerTraits<P>::opponent>();
  const bool counter_check = state.inCheck(P);
  MoveVector moves;
  {
    if (counter_check)
    {
#if 0
      // 速いと思われるがコードサイズは1割増える
      {
	move_action::Store store(moves);
	typedef move_action::CheckFilter<P,move_action::Store> check_filter_t;
	check_filter_t checkFilter(state,store);
	move_generator::Escape<P,check_filter_t>::
	  generateKingEscape(state, last_move, checkFilter);
      }
#else
      MoveVector escape;
      move_generator::GenerateEscape<P>::
	generateKingEscape(state, escape);
      for (size_t i=0; i<escape.size(); ++i)
      {
	if (MoveAdaptor<Check<P> >::isMember(state, escape[i]))
	  moves.push_back(escape[i]);
      }
#endif
    }
    else
    {
      move_action::Store store(moves);
      move_generator::AddEffectWithEffect<move_action::Store>
	::generate<P,true>
	(state,targetKing,store,has_pawn_checkmate);
    }
  }
  
  out.setSize(moves.size(), src);
  for (size_t i=0; i<moves.size(); ++i)
  {
    CheckMove new_move(moves[i]);
    const Square from = moves[i].from();
    if (! from.isPieceStand())
    {
      const Ptype ptype = moves[i].ptype();
      const Square to = moves[i].to();
      // 自殺手はfilter済み
#ifdef PAWN_CHECKMATE_SENSITIVE
      if (PawnCheckmateMoves::effectiveOnlyIfPawnCheckmate<P>(ptype, from, to))
      {
	new_move.flags = MoveFlags::NoPromote;
      }
#endif
    }
#ifdef DELAY_SACRIFICE
    const Square to = moves[i].to();
    const Ptype capturePtype = moves[i].capturePtype();
    if (capturePtype == PTYPE_EMPTY)
    { 
      const int defense = state.countEffect(alt(P),to);
      int offense = state.countEffect(P,to);
      if (from.isPieceStand())
	--offense;
      if (defense > offense)
      {
	new_move.flags.set(MoveFlags::SacrificeAttack);
      }
    }
#endif
    check_assert(std::find(moves.begin(), moves.begin()+=i, moves[i])
		 == (moves.begin()+=i)); // unique moves
    out[i] = new_move;
  }
}

namespace osl
{
  namespace checkmate
  {
    Ptype getCheapestDrop(Player turn, const NumEffectState& state)
    {
      if (state.hasPieceOnStand<PAWN>(turn))
	return PAWN;
      if (state.hasPieceOnStand<LANCE>(turn))
	return LANCE;
      if (state.hasPieceOnStand<KNIGHT>(turn))
	return KNIGHT;
      if (state.hasPieceOnStand<SILVER>(turn))
	return SILVER;
      if (state.hasPieceOnStand<GOLD>(turn))
	return GOLD;
      if (state.hasPieceOnStand<BISHOP>(turn))
	return BISHOP;
      if (state.hasPieceOnStand<ROOK>(turn))
	return ROOK;
      return PTYPE_EMPTY;
    }
    Ptype getSecondCheapestDrop(Player turn, const NumEffectState& state,
				Ptype cheapest)
    {
      if (cheapest == PAWN)
      {
	if (state.hasPieceOnStand<LANCE>(turn))
	  return LANCE;
	if (state.hasPieceOnStand<KNIGHT>(turn))
	  return KNIGHT;
	if (state.hasPieceOnStand<SILVER>(turn))
	  return SILVER;
	if (state.hasPieceOnStand<GOLD>(turn))
	  return GOLD;
	if (state.hasPieceOnStand<BISHOP>(turn))
	  return BISHOP;
	if (state.hasPieceOnStand<ROOK>(turn))
	  return ROOK;
      }
      return PTYPE_EMPTY;
    }
  }
}

template <osl::Player P>
unsigned int osl::checkmate::CheckMoveGenerator<P>::
generateEscape(const NumEffectState& state, CheckMoveListProvider& src,
	       CheckMoveList& out)
{
  using namespace move_classifier;
  const Player Defense = PlayerTraits<P>::opponent;

  unsigned int simple_king_moves = 0;

  MoveVector moves;
  move_generator::GenerateEscape<Defense>::
    generateKingEscape(state, moves);
  assert(out.empty());
  out.setSize(moves.size(), src);
#ifdef DELAY_INTERPOSE
  const Ptype cheapest = getCheapestDrop(alt(P), state);
  const Ptype secondCheapest = getSecondCheapestDrop(alt(P), state, cheapest);
#endif
  size_t num_output = 0;
  for (size_t i=0; i<moves.size(); ++i)
  {
    const Move m = moves[i];
    CheckMove cm(m);
    const Square to = m.to();
    const Square from = m.from();
    const Ptype ptype = m.ptype();
    if (from.isPieceStand())
    {
      if (PawnDropCheckmate<Defense>::isMember(state, ptype, from, to))
	continue;		// 受け方の打歩詰め王手
#ifdef DELAY_INTERPOSE
      if (((ptype != cheapest)
	   && (ptype != secondCheapest))
	  || ((! state.hasEffectAt(alt(P), to))	// 自分の利きがない
	      && (! state.hasMultipleEffectAt(P, to))))	// 焦点でもない
	cm.flags = MoveFlags::BlockingBySacrifice;
#endif
    }
    else 
    {  
      assert(move_classifier::SafeMove<PlayerTraits<P>::opponent>
	     ::isMember(state, ptype, from, to));
#ifdef DELAY_INTERPOSE
      if ((ptype != KING)
	  && (m.capturePtype() == PTYPE_EMPTY)
	  && (! state.hasMultipleEffectAt(alt(P), to)))
      {
	// 駒を動かす中合
	cm.flags = MoveFlags::BlockingBySacrifice;
      }
#endif
      if (ptype == KING && m.capturePtype() == PTYPE_EMPTY)
	++simple_king_moves;
    }
    out[num_output++] = cm;
  }
  out.shrinkSize(num_output);
  return simple_king_moves;
}

#endif /* _CHECKMOVEGENERATOR_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
