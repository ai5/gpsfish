/* ntesukiMoveGenerator.cc
 */
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "osl/ntesuki/ntesukiRecord.h"
#include "osl/state/numEffectState.h"
#include "osl/effect_util/neighboring8Effect.h"
#include "osl/effect_util/neighboring25Direct.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_generator/addEffect_.h"
#include "osl/move_classifier/canAttackInNMoves.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/safeMove.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_action/store.h"
#include "osl/move_action/safeFilter.h"
#include "osl/effect_util/effectUtil.h"
#include <iostream>
#ifdef NDEBUG
# include "osl/move_generator/escape_.tcc"
# include "osl/move_generator/capture_.tcc"
#endif

#include "osl/move_action/store.h"

namespace osl
{
  namespace ntesuki
  {
  }
}

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
    inline
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


    /* NtesukiMoveGenerator
     */
    NtesukiMoveGenerator::NtesukiMoveGenerator(bool verbose)
      :verbose(verbose) {}
    NtesukiMoveGenerator::~NtesukiMoveGenerator() {}

    template <osl::Player T>
    static void
    generate_all_moves(const NumEffectState& state,
		       MoveVector& moves)
    {
      if (state.inCheck(T))
      {
	// 王手がかかっている時は防ぐ手のみを生成
	GenerateEscapeKing::generate(state, moves);
      }
      else
      {
	MoveVector all_moves;
	// そうでなければ全ての手を生成
	GenerateAllMoves::generate(T, state, all_moves);
	// この指手は，玉の素抜きがあったり，打歩詰の可能性があるので
	// 確認が必要
	using namespace osl::move_classifier;
	for (unsigned int i=0; i<all_moves.size(); ++i)
	{
	  const Move m = all_moves[i];
	  if (m.isDrop()
	      || PlayerMoveAdaptor<SafeMove>::isMember(state, m))
	  {
	    moves.push_back(m);
	  }
	}
      }

    }
    template <Player T>
    void NtesukiMoveGenerator::
    generateWithRzone(const NumEffectState& state,
		      NtesukiRecord *record,
		      int pass_left,
		      NtesukiMoveList& moves)
    {
      /* 攻め方の手番で， rzone を使っての手生成. */
      ntesuki_assert(record->turn() == T);
      const Player O = PlayerTraits<T>::opponent;

      const Square targetKing
	= state.kingSquare<O>();
      if (state.inCheck(T))
      {
	/* 自玉に王手がかかっている場合は，全合法手を生成 */
	MoveVector move_candidates;
	GenerateEscapeKing::generate(state, move_candidates);
	moves = NtesukiMoveList(state, move_candidates);

	setOrder<T>(state, moves);
      }
      else if (pass_left == 0)
      {
	/* 詰み探索の場合には，王手のみ生成 */
	MoveVector move_candidates;
	{
	  move_action::Store store(move_candidates);
	  move_action::SafeFilter<T, move_action::Store>
	    store_safe(state, store);
	  move_generator::AddEffect<T, true>::generate(state,
						       targetKing,
						       store_safe);
	}
	moves = NtesukiMoveList(state, move_candidates);

	for (NtesukiMoveList::iterator move_it = moves.begin();
	     move_it != moves.end(); move_it++)
	{
	  /* 大駒のただ捨ては遅らせる */
	  const Move m = move_it->getMove();
	  const Square to = m.to();
	  const Piece atTo = state.pieceOnBoard(to);
	  if (m.isDrop() &&
	      isMajor(m.ptype()) &&
	      state.countEffect2(O, to) == 1)
	  {
	    move_it->setOrder(1);
	  }
	  else
	  {
	    move_it->setOrder(0);
	  }
	}

	moves.push_front(NtesukiMove(Move::PASS(T)));
	moves.front().setOrder(0);
      }
      else if (!record->rzone_move_generation)
      {
	return generate<T>(state, moves);
      }
      else
      {
	int rzone_order = pass_left - 1;
	Rzone rzone_cur = record->rzone<T>()[rzone_order];
	ntesuki_assert(rzone_cur.any());
	
	/* 取る手を生成 */
	MoveVector captures;
	{
	  move_action::Store store(captures);
	
	  capture<T, ROOK>(state, store);
	  capture<T, BISHOP>(state, store);
	  capture<T, GOLD>(state, store);
	  capture<T, SILVER>(state, store);
	  capture<T, KNIGHT>(state, store);
	  capture<T, LANCE>(state, store);
	}
	for (size_t i = 0; i < captures.size(); ++i)
	{
	  const Move m = captures[i];
	  if (move_classifier::SafeMove<T>::isMember(state,
						     m.ptype(),
						     m.from(),
						     m.to()))
	  {
	    moves.add(m);
	  }
	}
	
	/* 玉が逃げそうな位置に利きをつける手を生成 */
	for (int x = 1; x <=9; ++x)
	{
	  for (int y = 1; y <=9; ++y)
	  {
	    bool close_to_target = false;
	    const Square pos(x,y);
	    if (NtesukiRecord::use_9rzone == true)
	    {
	      for (int dx = -1; dx <=1; dx++)
	      {
		for (int dy = -1; dy <=1; dy++)
		{
		  int xx = x + dx;
		  int yy = y + dy;
		  if (xx > 0 && xx < 10 && yy > 0 && yy < 10)
		  {
		    ntesuki_assert(xx >= 1 && xx <= 9);
		    ntesuki_assert(yy >= 1 && yy <= 9);
		    const Square pos2(xx,yy);
		    close_to_target |= rzone_cur.test(pos2);
		  }
		}
	      }
	    }
	    else
	    {
	      close_to_target = rzone_cur.test(pos);
	    }
	    if (close_to_target)
	    {
	      MoveVector king_blocks;
	      {
		move_action::Store store2(king_blocks);
		move_action::SafeFilter<T, move_action::Store>
		  store_safe(state, store2);
		move_generator::AddEffect<T, false>::generate(state, pos, store_safe);
	      }
	      for (size_t i = 0; i < king_blocks.size(); ++i)
	      {
		const Move m = king_blocks[i];
		moves.add(m);
	      }
	    }
	  }
	}

	setOrder<T>(state, moves);
	moves.push_front(NtesukiMove(Move::PASS(T)));
	moves.front().setOrder(0);
      }
    }


    template <Player T>
    void NtesukiMoveGenerator::
    generate(const NumEffectState& state,
	     NtesukiMoveList& moves)
    {
      MoveVector all_moves;
      generate_all_moves<T>(state, all_moves);
      moves = NtesukiMoveList(state, all_moves);

      setOrder<T>(state, moves);

      if (!state.inCheck(T))
      {
	moves.push_front(NtesukiMove(Move::PASS(T)));
	moves.front().setOrder(0);
      }
    }//generate

    template <Player T>
    void NtesukiMoveGenerator::
    setOrder(const NumEffectState& state,
	     NtesukiMoveList& moves)
    {
      const Square opKingSquare =
	state.kingSquare (alt(state.turn ()));

      for (NtesukiMoveList::iterator move_it = moves.begin();
	   move_it != moves.end(); move_it++)
      {
	if (!opKingSquare.isOnBoard())
	{
	  move_it->setOrder(3);
	  continue;
	}
	const Move m = move_it->getMove();
	const Square from = m.from();
	const Square to = m.to();
	const Piece atTo = state.pieceOnBoard (to);

	/* order == 0 : 王手 */
	if (move_it->isCheck())
	{
	  move_it->setOrder(0);
	}
	/* order == 1 : 王の近傍への利き, 大駒の道, 小駒以外を取る */
	else if (Neighboring8Effect::hasEffect(state, m.ptypeO(), to,
					       opKingSquare))
	{
	  move_it->setOrder(1);
	}
	else if (hasEffectByBigPieces (state, state.turn (), from))
	{
	  move_it->setOrder(1);
	}
	else if ((atTo.isPiece()) &&
		 (atTo.owner() == alt (state.turn ()))&&
		 (atTo.ptype() != PAWN))
	{
	  move_it->setOrder(1);
	}
	/* order == 2 : 王の25近傍，小駒を取る */
	else if (Neighboring25Direct::hasEffect(state, m.ptypeO(), to,
						opKingSquare))
	{
	  move_it->setOrder(2);
	}
	else if ((atTo.isPiece()) &&
		 (atTo.owner() == alt (state.turn ())))
	{
	  move_it->setOrder(2);
	}
	/* order > 2 : それ以外の手 */
	else
	{
	  move_it->setOrder(3);
	}
      }
    }

    template void NtesukiMoveGenerator::generate<BLACK>(const NumEffectState& state,
							NtesukiMoveList& moves);
    template void NtesukiMoveGenerator::generate<WHITE>(const NumEffectState& state,
							NtesukiMoveList& moves);

    template void NtesukiMoveGenerator::generateWithRzone<BLACK>(const NumEffectState& state,
								 NtesukiRecord* record,
								 int pass_left,
								 NtesukiMoveList& moves);
    template void NtesukiMoveGenerator::generateWithRzone<WHITE>(const NumEffectState& state,
								 NtesukiRecord* record,
								 int pass_left,
								 NtesukiMoveList& moves);

  }//ntesuki
}//osl


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
