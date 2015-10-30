/* king8Group.cc
 */
#include "osl/rating/group/king8Group.h"
#include <sstream>

osl::rating::AttackKing8Group::AttackKing8Group() 
  : Group("AttackKing8")
{
  for (int attack=0; attack<3; ++attack) {
    for (int defense=0; defense<3; ++defense) {
      for (int s=PTYPE_PIECE_MIN; s<= PTYPE_MAX; ++s) {
	for (int t=PTYPE_PIECE_MIN; t<= PTYPE_MAX; ++t) {
	  for (int p=0; p<8; ++p)	// progress8
	    push_back(new AttackKing8(static_cast<Ptype>(s), static_cast<Ptype>(t), true, attack, defense));
	  for (int p=0; p<8; ++p)	// progress8
	    push_back(new AttackKing8(static_cast<Ptype>(s), static_cast<Ptype>(t), false, attack, defense));
	}
	for (int p=0; p<8; ++p)	// progress8
	  push_back(new AttackKing8(static_cast<Ptype>(s), PTYPE_EMPTY, true, attack, defense));
      }
    }
  }
}

int osl::rating::AttackKing8Group::findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
{
  const Ptype self = move.ptype();
  const Square position
    = Neighboring8Direct::findNearest(state, move.ptypeO(), move.to(), 
				      state.kingSquare(alt(state.turn())));
  if (position.isPieceStand() || position == move.from())
    return -1;
  if (! move.isDrop() && state.hasEffectByPiece(state.pieceOnBoard(move.from()), position))
    return -1;
  const Piece p = state.pieceAt(position);
  const Ptype target = p.ptype();
  const int base = CountEffect2::index(state, position, env) 
    * (PTYPE_MAX+1 - PTYPE_PIECE_MIN) * ((PTYPE_MAX+1 - PTYPE_PIECE_MIN)*2 +1);

  const int self_index = self - PTYPE_PIECE_MIN;

  int index = base + self_index*((PTYPE_MAX+1 - PTYPE_PIECE_MIN)*2 +1);
  if (!isPiece(target)) {
    index += (PTYPE_MAX+1 - PTYPE_PIECE_MIN)*2;
  }
  else {
    index += (target - PTYPE_PIECE_MIN)*2 + (p.owner() != move.player());
  }

  const int progress8 = env.progress.value()/2;
  return index*8 + progress8;
}

osl::rating::DefenseKing8Group::DefenseKing8Group() 
  : Group("DefenseKing8")
{
  for (int danger=0; danger<=4; ++danger) {
    for (int s=PTYPE_PIECE_MIN; s<= PTYPE_MAX; ++s) {
      for (int p=0; p<8; ++p)	// progress8
	push_back(new DefenseKing8(static_cast<Ptype>(s), true,  danger));
      for (int p=0; p<8; ++p)	// progress8
	push_back(new DefenseKing8(static_cast<Ptype>(s), false, danger));
    }
  }
}

int osl::rating::DefenseKing8Group::findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
{
  assert(env.attack_count_for_turn == DefenseKing8::count(state));
  const int self_index = (move.ptype() - PTYPE_PIECE_MIN)*2;
  int index;
  if (DefenseKing8::matchDrop(state, move))
    index = env.attack_count_for_turn*(PTYPE_MAX+1-PTYPE_PIECE_MIN)*2 + self_index;
  else if (DefenseKing8::matchMove(state, move))
    index = env.attack_count_for_turn*(PTYPE_MAX+1-PTYPE_PIECE_MIN)*2 + self_index + 1;
  else
    return -1;
  const int progress8 = env.progress.value()/2;
  return index*8 + progress8;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
