/* numEffectState.tcc
 */
#ifndef OSL_NUM_EFFECT_STATE_TCC
#define OSL_NUM_EFFECT_STATE_TCC

#include "osl/numEffectState.h"
#include "osl/move_classifier/kingOpenMove.h"
#include "osl/bits/numSimpleEffect.tcc"

template <osl::Player P>
bool osl::NumEffectState::
hasEffectByWithRemove(Square target,Square removed) const
{
  const Piece piece = pieceAt(removed);
  if (! piece.isPiece()) 
    return hasEffectAt<P>(target);
  if (piece.owner() == P) 
  {
    if (hasEffectNotBy(P, piece, target))
      return true;
  }
  else 
  {
    if (hasEffectAt(P, target))
      return true;
  }
  if (! longEffectAt(removed, P).any())
    return false;
  const Direction d = Board_Table.getLongDirection<BLACK>(Offset32(target,removed));
  if (!isLong(d))
    return false;
  const int num=longEffectNumTable()[piece.number()][longToShort(d)];
  return (! Piece::isEmptyNum(num)
	  && pieceOf(num).owner()==P);
}

namespace osl
{
  template <osl::Player P, bool InterestEmpty, Direction Dir>
  struct TestEffectOfMove
  {
    template <class State, class Function>
    static void testShort(const State& s, int mask, Square from, 
			  Function& f) {
      static_assert(! DirectionTraits<Dir>::isLong, "Dir");
      if (! (mask & DirectionTraits<Dir>::mask))
	return;

      const Offset offset = DirectionPlayerTraits<Dir,P>::offset();
      const Square target = from+offset;
      const Piece piece = s.pieceAt(target);
      if (piece.isEdge())
	return;
      if (InterestEmpty || (! piece.isEmpty()))
	f(target);
    }
    template <class State, class Function>
    static void testLong(const State& s, int mask, Square from, 
			 Function& f) {
      static_assert(DirectionTraits<Dir>::isLong, "Dir");
      if (! (mask & DirectionTraits<Dir>::mask))
	return;

      const Offset offset = DirectionPlayerTraits<Dir,P>::offset();

      Square target = from+offset;
      Piece piece = s.pieceAt(target);
      while (piece.isEmpty()) {
	if (InterestEmpty)
	  f(target);		
	target = target+offset;
	piece = s.pieceAt(target);
      }
      if (piece.isPiece()) {
	f(target);
      }
    }
  };
  struct SafeCapture
  {
  public:
    const NumEffectState& state;
    Piece safe_one;
    SafeCapture(const NumEffectState& s) : state(s), safe_one(Piece::EMPTY()) {
    }
    template <Player P>
    void doAction(Piece effect_piece, Square target) {
      if (move_classifier::KingOpenMove<P>::isMember
	  (state, effect_piece.ptype(), effect_piece.square(), target))
	return;
      safe_one = effect_piece;
    }
  };
} // namespace osl

template <osl::Player P, class Function, bool InterestEmpty>
void osl::NumEffectState::
forEachEffectOfPtypeO(Square from, Ptype ptype, Function& f) const
{
  const int mask = Ptype_Table.getMoveMask(ptype);
  TestEffectOfMove<P,InterestEmpty,UL>::testShort(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,U>::testShort(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,UR>::testShort(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,L>::testShort(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,R>::testShort(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,DL>::testShort(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,D>::testShort(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,DR>::testShort(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,UUL>::testShort(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,UUR>::testShort(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,LONG_UL>::testLong(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,LONG_U>::testLong(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,LONG_UR>::testLong(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,LONG_L>::testLong(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,LONG_R>::testLong(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,LONG_DL>::testLong(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,LONG_D>::testLong(*this, mask, from, f);
  TestEffectOfMove<P,InterestEmpty,LONG_DR>::testLong(*this, mask, from, f);
}

template <class Function, bool InterestEmpty>
void osl::NumEffectState::
forEachEffectOfPtypeO(Square from, PtypeO ptypeo, Function& f) const
{
  const Player P = getOwner(ptypeo);
  if (P == BLACK)
    this->template forEachEffectOfPtypeO<BLACK,Function,InterestEmpty>
      (from, getPtype(ptypeo), f);
  else
    this->template forEachEffectOfPtypeO<WHITE,Function,InterestEmpty>
      (from, getPtype(ptypeo), f);
}


template <osl::Player P>
osl::Piece
osl::NumEffectState::safeCaptureNotByKing(Square target, Piece king) const
{
  assert(king.owner() == P);
  assert(king.ptype() == KING);
  PieceMask ignore = pin(P);
  ignore.set(king.number());
  const Piece piece = findAttackNotBy(P, target, ignore);
  if (piece.isPiece())
    return piece;
  SafeCapture safe_captures(*this);
  this->template forEachEffectNotBy<P>(target, king, safe_captures);
  
  return safe_captures.safe_one;
}

#endif /* OSL_NUM_EFFECT_STATE_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
