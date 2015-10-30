/* effect5x3.cc
 */
#include "osl/progress/effect5x3.h"
#include "osl/progress/effect5x3Table.h"

osl::progress::
Effect5x3::Effect5x3(const NumEffectState& state)
{
  area_progresses[BLACK]=makeProgressArea(WHITE, state, state.kingSquare(BLACK));
  area_progresses[WHITE]=makeProgressArea(BLACK, state, state.kingSquare(WHITE));
  stand_progresses[WHITE]=makeProgressStand(BLACK, state);
  stand_progresses[BLACK]=makeProgressStand(WHITE, state);
  progresses[BLACK]=area_progresses[BLACK]+stand_progresses[BLACK];
  progresses[WHITE]=area_progresses[WHITE]+stand_progresses[WHITE];
}

int osl::progress::
Effect5x3::makeProgressAll(Player defense, const NumEffectState& state,
			   Square king)
{  
  return makeProgressArea(alt(defense), state, king) 
    + makeProgressStand(alt(defense), state);
}

int osl::progress::
Effect5x3::makeProgressArea(Player attack, const NumEffectState& state,
			    Square king)
{  
  const Square center = Centering5x3::adjustCenter(king);

  const int min_x = center.x() - 2;
  const int min_y = center.y() - 1;

  // 利き
  int sum_effect = 0;

  for (int dx=0; dx<5; ++dx)
    {
      for (int dy=0; dy<3; ++dy)
	{
	  const Square target(min_x+dx,min_y+dy);
	  sum_effect += state.countEffect(attack, target) *
	    Effect5x3_Table.getAttackEffect(attack,king,dx,dy);
	}
    }
  return sum_effect / 2;
}

int osl::progress::
Effect5x3::makeProgressStand(Player attack, const NumEffectState& state)
{  
  // 持駒
  int sum_pieces = 0;
  sum_pieces += state.countPiecesOnStand<PAWN>(attack)*Effect5x3Table::StandPAWN;
  sum_pieces += state.countPiecesOnStand<LANCE>(attack)*Effect5x3Table::StandLANCE;
  sum_pieces += state.countPiecesOnStand<KNIGHT>(attack)*Effect5x3Table::StandKNIGHT;
  sum_pieces += state.countPiecesOnStand<SILVER>(attack)*Effect5x3Table::StandSILVER;
  sum_pieces += state.countPiecesOnStand<GOLD>(attack)*Effect5x3Table::StandGOLD;
  sum_pieces += state.countPiecesOnStand<BISHOP>(attack)*Effect5x3Table::StandBISHOP;
  sum_pieces += state.countPiecesOnStand<ROOK>(attack)*Effect5x3Table::StandROOK;
  return sum_pieces;
}

void osl::progress::
Effect5x3::updateStand(int& old_stand, Move last_move)
{
  if (last_move.isDrop()) {
    const Ptype ptype = last_move.ptype();
    old_stand -= Effect5x3_Table.piecesOnStand(ptype);
    return;
  }
  const Ptype ptype = last_move.capturePtype();
  if (ptype == PTYPE_EMPTY) {
    return;
  }
  old_stand += Effect5x3_Table.piecesOnStand(unpromote(ptype));
}

void osl::progress::
Effect5x3::update(const NumEffectState& new_state, Move last_move)
{
  const Player pl = last_move.player();
  updateStand(stand_progresses[alt(pl)], last_move);

  const Square kb = new_state.kingSquare<BLACK>(), kw = new_state.kingSquare<WHITE>();
  BoardMask mb = new_state.changedEffects(BLACK), mw = new_state.changedEffects(WHITE);
  bool king_move = last_move.ptype() == KING;
  if ((king_move && new_state.turn() == BLACK) || mb.anyInRange(Board_Mask_Table5x3_Center.mask(kw)))
    area_progresses[WHITE]=makeProgressArea(BLACK,new_state, kw);
  if ((king_move && new_state.turn() == WHITE) || mw.anyInRange(Board_Mask_Table5x3_Center.mask(kb)))
    area_progresses[BLACK]=makeProgressArea(WHITE,new_state, kb);

  progresses[BLACK]=area_progresses[BLACK]+stand_progresses[BLACK];
  progresses[WHITE]=area_progresses[WHITE]+stand_progresses[WHITE];
}

osl::progress::Effect5x3 osl::progress::
Effect5x3::expect(const NumEffectState&, Move move) const
{
  Effect5x3 new_progress = *this;
  if (move.capturePtype() != PTYPE_EMPTY) {
    const Player alt_pl = alt(move.player());
    int old = stand_progresses[alt_pl];
    updateStand(new_progress.stand_progresses[alt_pl], move);
    new_progress.progresses[alt_pl] += new_progress.stand_progresses[alt_pl] - old;
  }
  return new_progress;
}

/* ------------------------------------------------------------------------- */

osl::progress::
Effect5x3WithBonus::Effect5x3WithBonus(const NumEffectState& state) : Effect5x3(state)
{
  progress_bonuses[BLACK]=makeProgressAreaBonus<WHITE>(state, state.kingSquare<BLACK>());
  progress_bonuses[WHITE]=makeProgressAreaBonus<BLACK>(state, state.kingSquare<WHITE>());
  effect_mask[BLACK] = makeEffectMask<BLACK>(state);
  effect_mask[WHITE] = makeEffectMask<WHITE>(state);
  updateProgressBonuses(state);
}

template <osl::Player Defense>
osl::PieceMask osl::progress::
Effect5x3WithBonus::makeEffectMask(const NumEffectState &state)
{
  const Square king =
    state.kingSquare<Defense>();
  const Square center =
    Centering5x3::adjustCenter(king);

  const int min_x = center.x() - 2;
  const int min_y = center.y() - 1;

  PieceMask mask;
  for (int dx = 0; dx < 5; ++dx)
  {
    for (int dy = 0; dy < 3; ++dy)
    {
      const Square target(min_x+dx, min_y+dy);
      mask = mask | state.effectSetAt(target);
    }
  }
  return mask;
}

inline
int osl::progress::
Effect5x3WithBonus::attackEffect3(const NumEffectState& state, Player attack, Square target)
{
  const int a = state.countEffect(attack, target);
  if (a <= 1)
    return a;
  if (state.countEffect(alt(attack), target) > 0
      || state.pieceAt(target).isOnBoardByOwner(attack))
    return 1;
  return 2;
}

namespace osl
{
  namespace
  {
    template <Player P>
    int countPawnLanceKnight(const NumEffectState& state, Square target) 
    {
      // effect is max 2, pawn and lance cannot have the effect
      // to a position at the same time so this is OK
      const Square d = target+DirectionPlayerTraits<D,P>::offset();
      const Piece pd = state.pieceAt(d);

      // pawn and lance
      int count = 0;
      if (pd.ptypeO() == newPtypeO(P,PAWN))
	++count;
      else if (pd.ptypeO() == newPtypeO(P,LANCE))
	++count;
      else if (pd.isEmpty()) 
      {
	if (state.hasLongEffectAt<LANCE>(P, target))
	  ++count;
      }
      else if (pd.isEdge())
	return 0;
      
      // knight
      const Piece pdl = state.pieceAt(d+DirectionPlayerTraits<DL,P>::offset());
      if (pdl.ptypeO() == newPtypeO(P,KNIGHT))
	return count+1;
      const Piece pdr = state.pieceAt(d+DirectionPlayerTraits<DR,P>::offset());
      if (pdr.ptypeO() == newPtypeO(P,KNIGHT))
	return count+1;      
      return count;
    }
  }
}

template <osl::Player Attack, bool AlwaysPromotable, bool AlwaysNotPromotable>
int osl::progress::
Effect5x3WithBonus::makeProgressAreaBonus(const NumEffectState& state, Square king,
				 Square center)
{  
  const int min_x = center.x() - 2;
  const int min_y = center.y() - 1;

  // 利き
  int sum_effect = 0;

  for (int dy = 0; dy < 3; ++dy)
  {
    const Square target(king.x(), min_y + dy);
    int effect = attackEffect3(state, Attack, target) * 2;
    if (effect > 0)
    {
      if (! AlwaysPromotable
	  && (AlwaysNotPromotable || !target.canPromote<Attack>()) )
      {
	effect -= countPawnLanceKnight<Attack>(state, target);
	assert(effect >= 0);
      }
      sum_effect += effect *
	Effect5x3_Table.getAttackEffect(Attack, king, target.x() - min_x, dy) / 2;

    }
  }
  for (int x = king.x() - 1; x >= min_x; --x)
  {
    int y_count = 0;
    int sum = 0;
    for (int dy = 0; dy < 3; ++dy)
    {
      const Square target(x, min_y + dy);
      int effect = attackEffect3(state, Attack, target) * 2;
      if (effect > 0)
      {
	if (! AlwaysPromotable
	    && (AlwaysNotPromotable || !target.canPromote<Attack>()) )
	{
	  if (king.x() - x > 1)
	    effect = 0;
	  else
	    effect -= countPawnLanceKnight<Attack>(state, target);
	  assert(effect >= 0);
	}
	sum += effect *
	  Effect5x3_Table.getAttackEffect(Attack, king, x - min_x, dy) / 2;
	y_count++;
      }
    }
    sum_effect += sum;
    if (y_count == 3)
    {
      sum_effect += sum;
      break;
    }
  }
  for (int x = king.x() + 1; x < min_x + 5; ++x)
  {
    int y_count = 0;
    int sum = 0;
    for (int dy = 0; dy < 3; ++dy)
    {
      const Square target(x, min_y + dy);
      int effect = attackEffect3(state, Attack, target) * 2;
      if (effect > 0)
      {
	if (! AlwaysPromotable
	    && (AlwaysNotPromotable || !target.canPromote<Attack>()) )
	{
	  if (x - king.x() > 1)
	    effect = 0;
	  else
	    effect -= countPawnLanceKnight<Attack>(state, target);
	  assert(effect >= 0);
	}
	sum += effect *
	  Effect5x3_Table.getAttackEffect(Attack, king, x - min_x, dy) / 2;
	y_count++;
      }
    }
    sum_effect += sum;
    if (y_count == 3)
    {
      sum_effect += sum;
      break;
    }
  }
  return sum_effect / 2;
}

template <osl::Player Attack>
int osl::progress::
Effect5x3WithBonus::makeProgressAreaBonus(const NumEffectState& state,
					  Square king)
{  
  const Square center = Centering5x3::adjustCenter(king);

  const bool always_promotable = center.squareForBlack<Attack>().y() <= 2;
  if (always_promotable)
    return makeProgressAreaBonus<Attack,true,false>(state, king, center);
  const bool always_notpromotable = center.squareForBlack<Attack>().y() >= 5;
  if (always_notpromotable)
    return makeProgressAreaBonus<Attack,false,true>(state, king, center);
  return makeProgressAreaBonus<Attack,false,false>(state, king, center);
}

void osl::progress::
Effect5x3WithBonus::update(const NumEffectState& new_state, Move last_move)
{
  Effect5x3::update(new_state, last_move);

  const Square kb = new_state.kingSquare<BLACK>(), kw = new_state.kingSquare<WHITE>();
  BoardMask mask = new_state.changedEffects();
  mask.set(last_move.to()); mask.set(last_move.from());

  const bool update_black = mask.anyInRange(Board_Mask_Table5x3_Center.mask(kb));
  const bool update_white = mask.anyInRange(Board_Mask_Table5x3_Center.mask(kw));
  
  if (update_black) 
  {
    progress_bonuses[BLACK]=makeProgressAreaBonus<WHITE>(new_state,kb);
    effect_mask[BLACK] = makeEffectMask<BLACK>(new_state);
  }
  if (update_white) 
  {
    progress_bonuses[WHITE]=makeProgressAreaBonus<BLACK>(new_state,kw);
    effect_mask[WHITE] = makeEffectMask<WHITE>(new_state);
  }
  updateProgressBonuses(new_state, update_black, update_white);
}

osl::progress::Effect5x3WithBonus osl::progress::
Effect5x3WithBonus::expect(const NumEffectState&, Move move) const
{
  Effect5x3WithBonus new_progress = *this;
  if (move.capturePtype() != PTYPE_EMPTY) {
    const Player alt_pl = alt(move.player());
    int old = stand_progresses[playerToIndex(alt_pl)];
    new_progress.updateStand(alt_pl, move);
    new_progress.progresses[playerToIndex(alt_pl)] += new_progress.stand_progresses[playerToIndex(alt_pl)] - old;
  }
  return new_progress;
}

void osl::progress::
Effect5x3WithBonus::updateProgressBonuses(const NumEffectState& state, bool update_black, bool update_white)
{
  if (update_black && progress_bonuses[BLACK] != 0)
  {
    const int pieces = countEffectPieces(state, WHITE);
    progress_bonuses[BLACK] =
      std::min(pieces * pieces * 4,
	       progress_bonuses[BLACK]);
  }
  if (update_white && progress_bonuses[WHITE] != 0)
  {
    const int pieces = countEffectPieces(state, BLACK);
    progress_bonuses[WHITE] =
      std::min(pieces * pieces * 4,
	       progress_bonuses[WHITE]);
  }
}

int osl::progress::
Effect5x3WithBonus::countEffectPieces(const NumEffectState &state, Player attack) const
{
  PieceMask mask = effect5x3Mask(alt(attack));
  mask = mask & state.piecesOnBoard(attack);
  return mask.countBit();
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
