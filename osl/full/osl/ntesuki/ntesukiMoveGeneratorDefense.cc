/* ntesukiMoveGenerator.cc
 */
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "osl/state/numEffectState.h"
#include "osl/effect_util/neighboring8Direct.h"
#include "osl/move_generator/escape.h"
#include "osl/move_classifier/canAttackInNMoves.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/safeMove.h"
#include "osl/move_classifier/check.h"
#include "osl/move_generator/legalMoves.h"
#include "osl/move_generator/addEffect.h"
#include "osl/move_generator/drop.h"
#include "osl/move_generator/addEffect8.h"
#include "osl/move_generator/kingWalk.h"
#include "osl/move_generator/openKingRoad.h"
#include "osl/move_generator/dropAroundKing8.h"
#include "osl/move_generator/captureEffectToAroundKing8.h"
#include "osl/move_generator/addEffect8Defense.h"
#include "osl/move_action/store.h"
#include <iostream>

/*
 * n 手すき探索で用いる move generator.
 */

namespace osl
{
  namespace ntesuki
  {
    /* ----------------------------------------------------------------------
     * DEFENSE
     * ----------------------------------------------------------------------
     */
    /* GetAllDefenseMoves
     */
    GetAllDefenseMoves::GetAllDefenseMoves(bool verbose)
      : NtesukiDefenseMoveGenerator(verbose) {}
    GetAllDefenseMoves::~GetAllDefenseMoves() {}
    template <Player P>
    void GetAllDefenseMoves::
    generate(const NumEffectState& state,
	     NtesukiMoveList& moves,
	     const Square& last_to)
    {
      assert (state.turn() == P);
      MoveVector move_candidates;
      LegalMoves::generate(state, move_candidates);
      moves = NtesukiMoveList(state, move_candidates);
    }
    template void GetAllDefenseMoves::generate<BLACK>(const NumEffectState& state,
						      NtesukiMoveList& moves,
						      const Square& last_to);
    template void GetAllDefenseMoves::generate<WHITE>(const NumEffectState& state,
						      NtesukiMoveList& moves,
						      const Square& last_to);

    /* GetDefenseMoves
     * - 実装済み
     * -- 玉を動かす
     * -- 玉のまわり 8近傍に駒を打つ
     * -- 玉の逃げ道を空ける     
     * -- 玉のまわり8近傍に利きをつけている駒(を取る
     * -- 玉のまわり 8近傍に(守りの)利きをつけるによる defense
     * -- 直前に動いた駒を取る手
     * - 未実装
     * -- 玉のまわり 8近傍の駒を取る
     * -- 最後に動かした大駒から八近傍へ利きがある場合、
     *    その利きの途中のマスで、自分が利きがあるところに駒を打つ
     * - 相手からの反撃のうち
     * -- 駒を取りつつ王手(特に銀がないときの銀取りのような手)
     */
    GetDefenseMoves::GetDefenseMoves(bool verbose)
      : NtesukiDefenseMoveGenerator(verbose) {}
    GetDefenseMoves::~GetDefenseMoves(){}
    template <Player P>
    void GetDefenseMoves::
    generate(const NumEffectState& state,
	     NtesukiMoveList& moves,
	     const Square& last_to)
    {
      MoveVector move_candidates;

      const Square pos = state.template kingSquare<P>();
      const bool check = state.hasEffectAt(PlayerTraits<P>::opponent, pos);

      if (check)
      {
	GenerateEscapeKing::generate(state, move_candidates);
	moves = NtesukiMoveList(state, move_candidates);
	return;
      }

      typedef move_action::Store action_t;
      assert (state.turn() == P);

      move_action::Store store_candidates(move_candidates);

      move_generator::KingWalk<P>::
	generate(static_cast<const SimpleState &>(state),
		 store_candidates);
      move_generator::OpenKingRoad<P>::
	generate(static_cast<const SimpleState &>(state),
		 store_candidates);
      move_generator::CaptureEffectToAroundKing8<P>::
	generate(state, store_candidates);
      move_generator::GenerateCapture::
	generate(state.turn(), state, last_to, store_candidates);
      //these are non drop moves that could be 'non safe moves'
      MoveVector move_safe;
      using namespace osl::move_classifier;
      for (unsigned int i = 0; i < move_candidates.size(); ++i)
      {
	const Move m = move_candidates[i];

	if (PlayerMoveAdaptor<SafeMove>::isMember(state, m))
	{
	  move_safe.push_back(m);
	}
      }

      move_action::Store store_safe(move_safe);
      move_generator::AddEffect8Defense<P>::
	generate(state, store_safe);
      //drop moves that are always safe
      move_generator::DropAroundKing8<P>::
	generate(static_cast<const SimpleState &>(state),
		 store_safe);
      move_safe.unique();
      moves = NtesukiMoveList(state, move_safe);
    }
    template void GetDefenseMoves::generate<BLACK>(const NumEffectState& state,
						   NtesukiMoveList& moves,
						   const Square& last_to);
    template void GetDefenseMoves::generate<WHITE>(const NumEffectState& state,
						   NtesukiMoveList& moves,
						   const Square& last_to);
  }//ntesuki
}//osl


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
