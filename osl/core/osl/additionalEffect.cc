#include "osl/additionalEffect.h"
#include "osl/bits/additionalOrShadow.h"

bool osl::effect_util::
AdditionalEffect::hasEffect(const NumEffectState& state, Square target, 
			       Player attack)
{
  PieceMask direct = state.effectSetAt(target) & state.piecesOnBoard(attack);
  PieceMask mask;
  mask.setAll();
  mask.clearBit<KNIGHT>();
  direct &= (state.promotedPieces() | mask);
  
  while (direct.any()) {
    const int num = direct.takeOneBit();
    const Square p = state.pieceOf(num).square();
    const Direction d=Board_Table.getShort8<BLACK>(p,target);
    const int num1=state.longEffectNumTable()[num][d];
    if(!Piece::isEmptyNum(num1) && state.pieceOf(num1).owner()==attack) return true;
  }
  return false;
}

template <int count_max>
int osl::effect_util::
AdditionalEffect::count(const NumEffectState& state, Square target, 
			Player attack)
{
  PieceVector direct_pieces;
  state.findEffect(attack, target, direct_pieces);
  return AdditionalOrShadow::count<count_max>
    (direct_pieces, state, target, attack);
}

bool osl::effect_util::
AdditionalEffect::hasEffectStable(const NumEffectState& state, Square target, 
				  Player attack)
{
  return count<1>(state, target, attack);
}

int osl::effect_util::
AdditionalEffect::count2(const NumEffectState& state, Square target, 
			    Player attack)
{
  return count<2>(state, target, attack);
}

void osl::effect_util::
AdditionalEffect::find(const NumEffectState& state, Square target, 
		       const PieceVector& direct_effects,
		       PieceVector& black, PieceVector& white)
{
  for (Piece p: direct_effects)
  {
    const Square from = p.square();
    const Offset32 diff32 = Offset32(from, target);
    const Offset step = Board_Table.getShortOffsetNotKnight(diff32);
    if (step.zero())
      continue;
    // 利きが8方向の場合
    Piece candidate=state.nextPiece(from, step);
    if (! candidate.isPiece())
      continue;
    const Offset32 diff_reverse = Offset32(target,candidate.square());
    for (; candidate.isPiece(); 
	 candidate=state.nextPiece(candidate.square(), step))
    {
      const EffectContent effect 
	= Ptype_Table.getEffect(candidate.ptypeO(), diff_reverse);
      if (! effect.hasEffect())
	break;
      if (candidate.owner() == BLACK)
	black.push_back(candidate);
      else
	white.push_back(candidate);
    }
  }
  
}

void osl::effect_util::
AdditionalEffect::find(const NumEffectState& state, Square target, 
		     PieceVector& black, PieceVector& white)
{
  PieceVector direct_pieces;
  state.findEffect(BLACK, target, direct_pieces);
  find(state, target, direct_pieces, black, white);
  
  direct_pieces.clear();
  state.findEffect(WHITE, target, direct_pieces);
  find(state, target, direct_pieces, black, white);
}

void osl::effect_util::
AdditionalEffect::count(const NumEffectState& state, Square target,
		      int& black, int& white)
{
  PieceVector black_pieces, white_pieces;
  find(state, target, black_pieces, white_pieces);
  black = black_pieces.size();
  white = white_pieces.size();
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
