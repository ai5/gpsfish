/* ntesukiMoveGenerator.cc
 */
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "osl/state/numEffectState.h"
#include "osl/effect_util/neighboring8Direct.h"
#include "osl/effect_util/neighboring25Direct.h"
#include "osl/move_classifier/canAttackInNMoves.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/safeMove.h"
#include "osl/move_classifier/check.h"
#include "osl/move_generator/escape.h"
#include "osl/move_generator/legalMoves.h"
#include "osl/move_generator/addEffect.h"
#include "osl/move_generator/addEffect.tcc"
#include "osl/move_generator/addEffect8.h"
#include "osl/move_generator/kingWalk.h"
#include "osl/move_generator/drop.h"
#include "osl/move_generator/dropAroundKing8.h"
#include "osl/move_generator/openKingRoad.h"
#include "osl/move_generator/capture.h"
#include "osl/move_generator/capture.tcc"
#include "osl/move_generator/captureEffectToAroundKing8.h"
#include "osl/move_action/store.h"
#include "osl/move_action/safeFilter.h"
#include <iostream>

/*
 * n 手すき探索で用いる move generator.
 */

namespace osl
{
  namespace ntesuki
  {
    /* ----------------------------------------------------------------------
     * Utility
     * ----------------------------------------------------------------------
     */
    static
    bool
    hasEffectByBigPieces (const NumEffectState& state,
			  const Player player,
			  const Square pos)
    {
#if OSL_WORDSIZE == 64
      const PieceMask bigPieceMask (container::PieceMaskBase(
							     PieceMask::numToMask (PtypeTraits<ROOK>::indexMin)
							     | PieceMask::numToMask (PtypeTraits<ROOK>::indexMin + 1)
							     | PieceMask::numToMask (PtypeTraits<BISHOP>::indexMin)
							     | PieceMask::numToMask (PtypeTraits<BISHOP>::indexMin + 1)));
      
      const PieceMask pieceMask = (state.piecesOnBoard (player)
				   & state.effectAt (pos)
				   & bigPieceMask);
      return pieceMask.any();
#elif OSL_WORDSIZE == 32
      // TODO: 多分このコードで64bit 環境と共通にできると思うんだけど
      // 締切まではそのままに
      PieceMask bigPieceMask;
      bigPieceMask.set(PtypeTraits<ROOK>::indexMin);
      bigPieceMask.set(PtypeTraits<ROOK>::indexMin + 1);
      bigPieceMask.set(PtypeTraits<BISHOP>::indexMin);
      bigPieceMask.set(PtypeTraits<BISHOP>::indexMin + 1);
      const PieceMask pieceMask = (state.piecesOnBoard (player)
				   & state.effectAt (pos)
				   & bigPieceMask);
      return pieceMask.any();
#endif
    }

    template <Player P>
    static
    void
    getCheckMoves (const NumEffectState& state, MoveVector& moves)
    {
      using namespace move_classifier;

      const Square targetKing
	= state.kingSquare<PlayerTraits<P>::opponent>();

      move_action::Store store(moves);
      move_action::SafeFilter<P, state::NumEffectState, move_action::Store> store_safe(state, store);
      move_generator::AddEffect<P, true>::generate(state, targetKing, store_safe);
    }

    template <Player P>
    struct CaptureHelper
    {
      CaptureHelper(const NumEffectState& state,
                    move_action::Store& action)
        : state(state), action(action)
      {
      }

      void operator()(Piece p)
      {
        move_generator::GenerateCapture::generate(P,state, p.square(), action);
      }
    private:
      const NumEffectState& state;
      move_action::Store& action;
    };

    template <Player P, Ptype T>
    static
    void
    capture(const NumEffectState& state, move_action::Store action)
    {
      CaptureHelper<P> captureHelper(state, action);
      state.forEachOnBoard<PlayerTraits<P>::opponent, T, CaptureHelper<P> >(captureHelper);
    }

    /* ----------------------------------------------------------------------
     * ATTACK
     * ----------------------------------------------------------------------
     */
    /* GetAllAttackMoves
     */
    GetAllAttackMoves::GetAllAttackMoves(bool verbose)
      : NtesukiAttackMoveGenerator(verbose) {}
    GetAllAttackMoves::~GetAllAttackMoves() {}
    template <Player P>
    void GetAllAttackMoves::
    generate(const NumEffectState& state,
	     NtesukiMoveList& moves)
    {
      MoveVector move_candidates;
      LegalMoves::generate(state, move_candidates);
      moves = NtesukiMoveList(state, move_candidates);
    }
    template void GetAllAttackMoves::generate<BLACK>(const NumEffectState& state,
						     NtesukiMoveList& moves);
    template void GetAllAttackMoves::generate<WHITE>(const NumEffectState& state,
						     NtesukiMoveList& moves);
    
    /* GetAttackMoves
     */
    GetAttackMoves::GetAttackMoves(bool verbose)
      : NtesukiAttackMoveGenerator(verbose) {}
    GetAttackMoves::~GetAttackMoves() {}
    template <Player P>
    void GetAttackMoves::
    generate(const NumEffectState& state,
	     NtesukiMoveList& moves)
    {
#if 0
      const Square pos = state.template kingSquare<P>();
      const bool check = state.hasEffectAt(PlayerTraits<P>::opponent, pos);
      
      if (check)
      {
	MoveVector move_candidates;
	GenerateEscapeKing::generate(state, move_candidates);
	moves = NtesukiMoveList(state, move_candidates);
	return;
      }

      MoveVector check_candidates;
      getCheckMoves<P>(state, check_candidates);

      MoveVector move_candidates;
      move_action::Store store(move_candidates);

      move_generator::AddEffect8<P>::generate(state, store);
      capture<P, ROOK>(state, store);
      capture<P, BISHOP>(state, store);
      capture<P, GOLD>(state, store);
      capture<P, SILVER>(state, store);
      capture<P, KNIGHT>(state, store);
      capture<P, LANCE>(state, store);
      //shold we generate pawn?

      size_t deleted = 0;
      for (size_t i = 0; i < move_candidates.size(); ++i)
      {
	const Move m = move_candidates[i];
	if (check_candidates.isMember(move_candidates[i])
	    || (m.from() != Square::STAND() &&
		!move_classifier::SafeMove<P>::isMember(state,
							m.ptype(),
							m.from(),
							m.to())))
	{
	  ++deleted;
	  move_candidates[i] = Move::INVALID();
	}
      }
      
      for (size_t i = 0; i < move_candidates.size(); ++i)
      {
	if (move_candidates[i] == Move::INVALID()) continue;

	{
	  ntesuki_assert(!move_classifier::
			 PlayerMoveAdaptor<move_classifier::Check>::
			 isMember(state, move_candidates[i])
			 || (std::cerr << std::endl
			     << state
			     << check_candidates << std::endl
			     << move_candidates << std::endl,
			     0));
	}

	moves.push_front(NtesukiMove(move_candidates[i], NtesukiMove::NONE));
      }
      for (size_t i = 0; i < check_candidates.size(); ++i)
      {
	moves.push_front(NtesukiMove(check_candidates[i], NtesukiMove::CHECK_FLAG));
      }
      
#else
      MoveVector all_moves;
      MoveVector move_candidates;
      LegalMoves::generate(state, all_moves);
      
      const Square opKingSquare =
	state.kingSquare (alt(state.turn ()));

      for (unsigned int i=0; i<all_moves.size(); ++i)
      {
	const Move m = all_moves[i];
	
	/*
	 * 次の手を生成する
	 *  - 王に迫る可能性のある手(25近傍に利きをつける手)
	 *  - 角道・飛車道を空ける手 (CHECK)
	 *  - 取る手
	 */
	const Square from = m.from();

	if (Neighboring25Direct::hasEffect(state, m.ptypeO(), m.to(),
					   opKingSquare))
	{
	  move_candidates.push_back(m);
	}
	else if (hasEffectByBigPieces (state, state.turn (), from))
	{
	  move_candidates.push_back(m);
	}
	else
	{
	  const Square to = m.to();
	  const Piece atTo = state.pieceOnBoard (to);
	    
	  if ((atTo.isPiece())
	      && (atTo.owner() == alt (state.turn ())))
	  {
	    move_candidates.push_back(m);
	  }
	}
      }//for each move in all_moves
      moves = NtesukiMoveList(state, move_candidates);
#endif
    }//generate
    template void GetAttackMoves::generate<BLACK>(const NumEffectState& state,
						  NtesukiMoveList& moves);
    template void GetAttackMoves::generate<WHITE>(const NumEffectState& state,
						  NtesukiMoveList& moves);

    /* GetMultipleAttackMoves
     */
    GetMultipleAttackMoves::GetMultipleAttackMoves(bool verbose)
      : NtesukiAttackMoveGenerator(verbose) {}
    GetMultipleAttackMoves::~GetMultipleAttackMoves() {}
    template <Player P>
    void GetMultipleAttackMoves::
    generate(const NumEffectState& state,
	     NtesukiMoveList& moves)
    {
      assert (state.turn() == P);
      MoveVector all_moves;
      MoveVector move_candidates;
      LegalMoves::generate(state, all_moves);

      const Square opKingSquare =
	state.kingSquare (alt(state.turn ()));

      for (unsigned int i=0; i<all_moves.size(); ++i)
      {
	const Move m = all_moves[i];
	const Square from = m.from();
      
	if (move_classifier::canAttackInThreeMoves (m.player(),
						    m.ptype(),
						    m.to(),
						    opKingSquare))
	{
	  move_candidates.push_back(m);
	}
	else if (hasEffectByBigPieces (state, state.turn (), from))
	{
	  move_candidates.push_back(m);
	}
	else
	{
	  const Square to = m.to();
	  const Piece atTo = state.pieceOnBoard (to);
	  if (atTo.isPiece()
	      && (atTo.owner() == alt (state.turn ())))
	  {
	    move_candidates.push_back(m);
	  }
	}
      }//for each move in all_moves
      moves = NtesukiMoveList(state, move_candidates);
    }//generate
    template void GetMultipleAttackMoves::generate<BLACK>(const NumEffectState& state,
							  NtesukiMoveList& moves);
    template void GetMultipleAttackMoves::generate<WHITE>(const NumEffectState& state,
							  NtesukiMoveList& moves);

  }//ntesuki
}//osl


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
