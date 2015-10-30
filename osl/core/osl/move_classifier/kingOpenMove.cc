#include "osl/move_classifier/kingOpenMove.h"
#include "osl/numEffectState.h"

template <osl::Player P>
template <bool hasException>
bool osl::move_classifier::KingOpenMove<P>::
isMemberMain(const NumEffectState& state, Ptype, Square from, Square to,
	     Square exceptFor)
{
  assert(! from.isPieceStand());
  Square king_position=state.template kingSquare<P>();
  if (king_position.isPieceStand())
    return false;
  /**
   * 守っている玉が動く状況では呼ばない
   */
  assert(king_position != from);
  /**
   * openになってしまうかどうかのチェック
   */
  Offset offset=Board_Table.getShortOffsetNotKnight(Offset32(king_position,from));
  /**
   * 移動元が王の8方向でないか
   * openにならない
   */
  if(offset.zero() ||
     offset==Board_Table.getShortOffsetNotKnight(Offset32(king_position,to)))
    return false;
  if(!state.isEmptyBetween(from,king_position,offset,true)) return false;
  Square pos=from;
  Piece p;
  for(pos-=offset;;pos-=offset){
    // TODO: exceptFor を毎回チェックする必要があるのはoffset方向の時だけ
    if (! ((hasException && (pos == exceptFor)) 
	   || (p=state.pieceAt(pos), p.isEmpty())))
      break;
    assert(pos.isOnBoard());
  }
  /**
   * そのptypeoがそのoffsetを動きとして持つか
   * 注: 持つ => safe でない => false を返す
   */
  if (! p.isOnBoardByOwner<alt(P)>())
    return false;
  return Ptype_Table.getEffect(p.ptypeO(),pos,king_position).hasEffect();
}

namespace osl
{
  // explicit template instantiation
  template struct move_classifier::KingOpenMove<BLACK>;
  template struct move_classifier::KingOpenMove<WHITE>;

  template bool move_classifier::KingOpenMove<BLACK>::isMemberMain<true>(const NumEffectState&, Ptype,Square,Square,Square);
  template bool move_classifier::KingOpenMove<BLACK>::isMemberMain<false>(const NumEffectState&, Ptype,Square,Square,Square);
  template bool move_classifier::KingOpenMove<WHITE>::isMemberMain<true>(const NumEffectState&, Ptype,Square,Square,Square);
  template bool move_classifier::KingOpenMove<WHITE>::isMemberMain<false>(const NumEffectState&, Ptype,Square,Square,Square);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
