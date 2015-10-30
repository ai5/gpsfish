/* feature.cc
 */
#include "osl/move_probability/feature.h"
#include "osl/checkmate/checkmateIfCapture.h"
#include <iostream>

osl::move_probability::
Feature::~Feature()
{
}


void osl::move_probability::PatternCommon::
updateCache(StateInfo& info)
{
  for (int x=1; x<=9; ++x) {
    for (int y=1; y<=9; ++y) {
      const Square position(x,y);
      updateCacheOne(position, info);
    }
  }
}

void osl::move_probability::PatternCommon::
updateCacheOne(Square position, StateInfo& info)
{
  const NumEffectState& state = *info.state;
  StateInfo::pattern_square_t& cache
    = info.pattern_cache[position.index()];
  cache.fill(-1);
  const CArray<Square,2> kings = {{
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  const Player turn = state.turn();
  int cur = 0;

  Piece target = state.pieceAt(position);
  PtypeO ptypeo = target.ptypeO();
  if (turn == WHITE)
    ptypeo = altIfPiece(ptypeo);
  size_t basic = ptypeOIndex(ptypeo)*SquareDim;
  std::pair<Ptype,Ptype> pair;
  ToEffect::supportAttack(state, position,
			  info.pin[turn], info.pin[alt(turn)],
			  turn, pair);
  int effect9 = classifyEffect9(state, turn, position);
  cache[cur++] = basic + pair.first;
  cache[cur++] = basic + AttackBase + pair.second;
  cache[cur++] = basic + EffectBase + effect9;
  assert(pair.first != PTYPE_EDGE);
  assert(pair.second != PTYPE_EDGE);
  if (isPiece(ptypeo)) {
    if (info.attack_shadow[target.number()][turn])
      cache[cur++] = basic + PTYPE_EDGE;
    if (info.attack_shadow[target.number()][alt(turn)])
      cache[cur++] = basic + AttackBase + PTYPE_EDGE;
  }
  assert(basic + EffectBase + effect9 < PatternCacheSize);
  int op_king_distance = abs(kings[alt(turn)].x()-position.x())
    + abs(kings[alt(turn)].y()-position.y());
  if (op_king_distance == 0) // KING
    op_king_distance = state.king8Info(alt(turn)).libertyCount();
  else
    --op_king_distance;
  if (op_king_distance < OpKingSize)
    cache[cur++] = basic + OpKingBase + op_king_distance;
  int my_king_distance = abs(kings[turn].x()-position.x())
    + abs(kings[turn].y()-position.y());
  if (my_king_distance == 0) // KING
    my_king_distance = state.king8Info(turn).libertyCount();
  else
    --my_king_distance;
  if (my_king_distance < MyKingSize)
    cache[cur++] = basic + MyKingBase + my_king_distance;
  // promotion
  if (position.canPromote(turn))
    cache[cur++] = basic + PromotionBase;
  else if (position.canPromote(alt(turn)))
    cache[cur++] = basic + PromotionBase + 1;
  if (target.isPiece()) {
    // pin or open
    if (state.pinOrOpen(turn).test(target.number()))
      cache[cur++] = basic + PinOpenBase + (target.owner() == turn);
    if (state.pinOrOpen(alt(turn)).test(target.number()))
      cache[cur++] = basic + PinOpenBase + 2 + (target.owner() == alt(turn));
    // last to
    if (info.history->hasLastMove()) {
      if (info.history->lastMove().to() == position)
	cache[cur++] = basic + LastToBase;
      for (int i=1; i<4; ++i) {
	if (! info.history->hasLastMove(i+1))
	  break;
	if (info.history->lastMove(i+1).to() == position)
	  cache[cur++] = basic + LastToBase + i;
      }
    }
  }
  // last effect changed
  if (info.history->hasLastMove()) {
    if (info.changed_effects.test(position))
      cache[cur++] = basic + LastEffectChangedBase;
    if (target.isPiece() && info.last_add_effect.test(target.number())) {
      int ptype_index = info.last_move_ptype5 - PTYPE_BASIC_MIN + 1;
      cache[cur++] = basic + LastEffectChangedBase + ptype_index;
    }
  }
}



void osl::move_probability::
BlockLong::updateCache(StateInfo& info)
{
  const NumEffectState& state = *info.state;
  for (int i=PtypeTraits<LANCE>::indexMin;
       i<PtypeTraits<LANCE>::indexLimit; ++i) {
    const Piece p = state.pieceOf(i);
    if (! p.isOnBoard() || p.isPromoted())
      continue;
    const Direction d = p.owner() == BLACK ? U : D;
    makeLongAttackOne(info, p, d);
  }
  for (int i=PtypeTraits<BISHOP>::indexMin;
       i<PtypeTraits<BISHOP>::indexLimit; ++i) {
    const Piece p = state.pieceOf(i);
    if (! p.isOnBoard())
      continue;
    makeLongAttackOne(info, p, UL);
    makeLongAttackOne(info, p, UR);
    makeLongAttackOne(info, p, DL);
    makeLongAttackOne(info, p, DR);
  }
  for (int i=PtypeTraits<ROOK>::indexMin;
       i<PtypeTraits<ROOK>::indexLimit; ++i) {
    const Piece p = state.pieceOf(i);
    if (! p.isOnBoard())
      continue;
    makeLongAttackOne(info, p, L);
    makeLongAttackOne(info, p, R);
    makeLongAttackOne(info, p, U);
    makeLongAttackOne(info, p, D);
  }
}

void osl::move_probability::
BlockLong::makeLongAttackOne(StateInfo& info,
			     Piece piece, Direction d)
{
  const NumEffectState& state = *info.state;
  StateInfo::long_attack_t& out
    = info.long_attack_cache[piece.number()][d];
  const Player turn = state.turn();
  const PtypeO attacking = (turn == BLACK)
    ? piece.ptypeO() : alt(piece.ptypeO());
  Square attack_to = state.mobilityOf(d, piece.number());
  Square attack_to2 = attack_to;
  assert(! attack_to.isPieceStand());
  if (attack_to.isEdge())
    attack_to -= Board_Table.getOffsetForBlack(d);
  else {
    const Offset o = Board_Table.getOffsetForBlack(d);
    attack_to2 += o;	// allow edge if neighboring
    if (state.pieceAt(attack_to2).isEmpty()) {
      do { 
	attack_to2 += o;
      } while (state.pieceAt(attack_to2).isEmpty());
      if (state.pieceAt(attack_to2).isEdge())
	attack_to2 -= o;
    }
  }
  PtypeO attacked = state.pieceOnBoard(attack_to).ptypeO();
  if (isPiece(attacked) && turn == WHITE)
    attacked = alt(attacked);
  int index = (longAttackIndex(attacking)*PTYPEO_SIZE
	       + ptypeOIndex(attacked))*OptionSize;
  out.push_back(index);
  if (attack_to.isNeighboring8(state.kingSquare(turn)))
    out.push_back(index + 1); // 1,King8
  if (! state.hasEffectAt(turn, attack_to))
    out.push_back(index + 2); // 2,HasSupport;
  if (attack_to.canPromote(alt(turn)))
    out.push_back(index + 3); // 3,Promotable;
  Piece attacked2 = state.pieceAt(attack_to2);
  if (attacked2.isOnBoardByOwner(turn)) { 
    out.push_back(index + 4); // 4,Shadowing;
    if (! state.hasEffectAt(turn, attack_to2))
      out.push_back(index + 5); // 5,
    if (attack_to2.canPromote(alt(turn)))
      out.push_back(index + 6); // 6,
    info.attack_shadow[attacked2.number()][piece.owner()] = true;
  }
  if (info.threatmate_move.isNormal()) {
    Square threat_at = info.threatmate_move.to();
    if (threat_at == attack_to
	|| (! Board_Table.getShortOffsetNotKnight(Offset32(threat_at, attack_to)).zero()
	    && Board_Table.isBetween(threat_at,
				     piece.square(), attack_to)))
      out.push_back(index + 7); // 7, threatmate block?
  }
}


bool osl::move_probability::
CheckmateIfCapture::hasSafeCapture(NumEffectState& state, Move move)
{
  return ! checkmate::CheckmateIfCapture::effectiveAttack(state, move, 0);
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
