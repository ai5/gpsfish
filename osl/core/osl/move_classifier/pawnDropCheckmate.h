/* pawnDropCheckmate.h
 */
#ifndef OSL_MOVE_CLASSIFIER_PAWNDROPCHECKMATE_H
#define OSL_MOVE_CLASSIFIER_PAWNDROPCHECKMATE_H

#include "osl/numEffectState.h"
#include "osl/bits/king8Info.h"

namespace osl
{
  namespace move_classifier
  {
    /**
     * 打歩詰の判定.
     * @param P 指手(攻撃)側
     */
    template <Player P>
    struct PawnDropCheckmate
    {
      /**
       * kingSquare に居る alt(P)の玉が dir 方向に逃げられるか.
       */
      static bool canEscape(const NumEffectState& state, Square kingSquare, 
			    Direction dir, Square dropAt);
      /** 王が前以外に移動可能か */
      static bool escape7(const NumEffectState& state, 
			  Square kingSquare, Square to);
      static bool isMember(const NumEffectState& state, 
			   Ptype ptype,Square from,Square to)
      {
	// 打歩
	if (! from.isPieceStand())
	  return false;
	if (ptype != PAWN)
	  return false;
	const Player Opponent = alt(P);
	const Piece king = state.template kingPiece<Opponent>();
	const Square king_position = king.square();
	// DirectionPlayerTraits?
	// 玉頭
        if (king_position != (to + DirectionPlayerTraits<U,P>::offset()))
	  return false;
	// 玉で取れない
	if (! state.hasEffectAt(P, to))
	  return false;
	if (King8Info(state.Iking8Info(Opponent)).liberty() != 0)
	  return false;
	// 玉以外の駒で取れない
	if (state.safeCaptureNotByKing<Opponent>(to, king)
	    != Piece::EMPTY())
	  return false;
	// どこにも逃げられない
	return escape7(state, king_position, to);
      }
    };
  } // namespace move_classifier
} // namespace osl

template <osl::Player P>
bool
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
 osl::move_classifier::PawnDropCheckmate<P>::
canEscape(const NumEffectState& state, Square kingSquare, 
	  Direction dir, Square dropAt) 
{
  const Player Opponent = alt(P);
  const Square target 
    = kingSquare + Board_Table.getOffset(Opponent, dir);
  const Piece p = state.pieceAt(target);
  if (p.isOnBoardByOwner<Opponent>())
    return false;		// 自分の駒がいたら移動不能
  if (target.isEdge())
    return false;
  Piece attacker;
  if (! state.template hasEffectAt<P>(target, attacker))
    return true;		// 利きがない
  if (attacker == Piece::EMPTY())
    return false;		// 攻撃側に複数の利き
  assert(attacker.owner() == P);
  // drop によりふさがれた利きなら逃げられる
  //    -OU
  // XXX+FU+HI
  // の場合のXXXなど．
  const Offset shortOffset
    = Board_Table.getShortOffsetNotKnight(Offset32(target,dropAt));
  if (shortOffset.zero())
    return false;
  const Square attackFrom = attacker.square();
  return shortOffset
    == Board_Table.getShortOffsetNotKnight(Offset32(dropAt,attackFrom));
}

template <osl::Player P>
bool
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
 osl::move_classifier::PawnDropCheckmate<P>::
escape7(const NumEffectState& state, Square king_position, Square to)
{
  // U は歩
  if (canEscape(state, king_position, UL, to))
    return false;
  if (canEscape(state, king_position, UR, to))
    return false;
  if (canEscape(state, king_position, L, to))
    return false;
  if (canEscape(state, king_position, R, to))
    return false;
  if (canEscape(state, king_position, DL, to))
    return false;
  if (canEscape(state, king_position, D, to))
    return false;
  if (canEscape(state, king_position, DR, to))
    return false;
  return true;
}
      

#endif /* OSL_MOVE_CLASSIFIER_PAWNDROPCHECKMATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
