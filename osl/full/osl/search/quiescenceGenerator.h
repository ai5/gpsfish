/* quiescenceGenerator.h
 */
#ifndef OSL_QUIESCENCEGENERATOR_H
#define OSL_QUIESCENCEGENERATOR_H

#include "osl/move_classifier/shouldPromoteCut.h"
#include "osl/search/historyTable.h"
#include "osl/move_generator/capture_.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_generator/promote_.h"
#include "osl/move_generator/safeDropMajorPiece.h"
#include "osl/move_generator/addEffect8.h"
#include "osl/move_generator/additionalLance.h"
#include "osl/eval/pieceEval.h"
#include "osl/numEffectState.h"
#include "osl/container/square8.h"

namespace osl
{
  namespace search
  {
    /**
     * QuiescenceSearch で使う指手生成
     */
    template <Player P>
    struct QuiescenceGenerator
    {
      /**
       * P が敵の PTYPE を取る手を生成する．
       * @param dont_capture これを取る手は除く
       */
      template <Ptype PTYPE, bool has_dont_capture>
      static void capture(const NumEffectState&,
			  MoveVector& moves, Piece dont_capture);
      /**
       * P が敵の target を取る手を生成する．
       */
      static void capture(const NumEffectState&,
			  Square target, MoveVector& moves);
      /**
       * P が敵の target を取る手を最大1手生成する．
       */
      static void capture1(const NumEffectState& state,
			   Square target, MoveVector& moves)
      {
	move_generator::GenerateCapture::generate1(P, state, target, moves);
      }
      static void promote(const NumEffectState&, PieceMask pins,
			  MoveVector& moves);
      template <Ptype PTYPE>
      static void promote(const NumEffectState&, MoveVector& moves);
      template <Ptype PTYPE, size_t N>
      static void promoteN(const NumEffectState&, MoveVector& moves,
			   const HistoryTable& table);
      static void check(const NumEffectState&, PieceMask pins,
			MoveVector& moves, bool no_liberty=false);
      static void check(const NumEffectState&, PieceMask pins, bool no_liberty,
			const Square8& sendoffs, MoveVector& moves);
      static void escapeKing(const NumEffectState& state, MoveVector& moves); 
      /**
       * @return existance of safe (in takeback) move
       */
      static bool escapeKingInTakeBack(const NumEffectState& state, MoveVector& moves, bool check_by_lance); 
      static void dropMajorPiece(const NumEffectState& state, MoveVector& moves); 
      static void dropMajorPiece3(const NumEffectState& state, MoveVector& moves,
				  const HistoryTable& table); 

      static void attackMajorPiece(const NumEffectState& state, PieceMask pins,
				   MoveVector& moves); 
      static void escapeAll(const NumEffectState& state, MoveVector& moves); 
      /**
       * @param escape KING以外の駒
       */
      static void escapeNormalPiece(const NumEffectState& state, 
				    Piece escape, MoveVector& moves,
				    bool add_support_only=false); 
      /**
       * 直前に指手から逃げる
       */
      template <class EvalT>
      static void escapeFromLastMove(const NumEffectState& state, 
				     Move last_move, MoveVector& moves); 
      template <class EvalT>
      static void escapeFromLastMoveOtherThanPawn(const NumEffectState& state, 
				     Move last_move, MoveVector& moves); 
      /**
       * @return 利きのない場所に移動可能
       */
      static bool escapeByMoveOnly(const NumEffectState& state, 
				   Piece piece, MoveVector& moves); 
      static void attackGoldWithPawn(const NumEffectState& state,
				     PieceMask pins, MoveVector& moves); 
      static void attackWithKnight(const NumEffectState& state,
				       PieceMask pins, Square attack_from, 
				       bool has_knight, MoveVector& moves); 
      static void attackSilverWithPawn(const NumEffectState& state,
				       PieceMask pins, MoveVector& moves); 
      static void attackKnightWithPawn(const NumEffectState& state,
				       PieceMask pins, MoveVector& moves); 
      /**
       * 角が前に進む. 覗いて成を受けにくい読み抜けを防ぐ
       */
      static void advanceBishop(const NumEffectState& state, 
				MoveVector& moves); 
      template <Direction DIR>
      static void advanceBishop(const NumEffectState& state, 
				const Square from, MoveVector& moves); 
      static void attackKing8(const NumEffectState& state, PieceMask pins,
			      MoveVector& moves); 
      static void attackToPinned(const NumEffectState& state, PieceMask pins,
				 MoveVector& moves); 

      static void utilizePromoted(const NumEffectState& state, 
				  Piece target,
				  MoveVector& moves); 

      static void breakThreatmate(const NumEffectState& state, 
				  Move threatmate, PieceMask pins,
				  MoveVector& moves);
      static void kingWalk(const NumEffectState& state, 
			   MoveVector& moves);
    private:
      static void attackMajorPieceSecondSelection(bool target_has_support,
						  const MoveVector& src,
						  MoveVector& out);
      static void attackMajorPieceFirstSelection(const NumEffectState& state,
						 PieceMask pins,
						 const MoveVector& all_moves,
						 MoveVector& moves,
						 MoveVector& expensive_drops);
      static void attackMajorPieceZerothSelection(const NumEffectState& state,
						  const MoveVector& src,
						  Square target,
						  MoveVector& open_out,
						  MoveVector& out);
    };
  } // namespace search
} // namespace osl




template <osl::Player P>
inline
void osl::search::QuiescenceGenerator<P>::
capture(const NumEffectState& state, Square target, MoveVector& moves)
{
  move_generator::GenerateCapture::generate(P,state, target, moves);
#ifndef NDEBUG
  for (Move m: moves)
    assert(! ShouldPromoteCut::canIgnoreMove<P>(m));
#endif
}

template <osl::Player P> 
template <osl::Ptype PTYPE> 
inline
void osl::search::QuiescenceGenerator<P>::
promote(const NumEffectState& state, MoveVector& moves)
{
  move_generator::Promote<P>::template generatePtype<PTYPE>(state, moves);
}

template <osl::Player P> 
template <osl::Ptype PTYPE, size_t N>
inline
void osl::search::QuiescenceGenerator<P>::
promoteN(const NumEffectState& state, MoveVector& moves, const HistoryTable& table)
{
  MoveVector all;
  move_generator::Promote<P>::template generatePtype<PTYPE>(state, all);
  FixedCapacityVector<std::pair<int,Move>, 16*2> selected;
  for (Move m: all) {
    if (state.hasEffectAt(alt(P), m.to()))
      continue;
    selected.push_back(std::make_pair(table.value(m), m));
  }
  std::sort(selected.begin(), selected.end());
  for (size_t i=0; i<std::min(N, selected.size()); ++i)
    moves.push_back(selected[selected.size()-1-i].second);
}

#endif /* OSL_QUIESCENCEGENERATOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
