#include "osl/progress.h"
#include "osl/eval/weights.h"
#include "osl/eval/midgame.h"
#include "osl/eval/minorPiece.h"
#include "osl/additionalEffect.h"
#include "osl/bits/centering5x3.h"
#include "osl/oslConfig.h"
#include <iostream>
#include <fstream>

bool osl::progress::ml::
operator==(const NewProgressData& l, const NewProgressData& r)
{
  return l.progresses == r.progresses
    && l.attack5x5_progresses == r.attack5x5_progresses
    && l.stand_progresses == r.stand_progresses
    && l.effect_progresses == r.effect_progresses
    && l.defenses == r.defenses
    && l.rook == r.rook && l.bishop == r.bishop && l.gold == r.gold
    && l.silver == r.silver && l.promoted == r.promoted
    && l.king_relative_attack == r.king_relative_attack
    && l.king_relative_defense == r.king_relative_defense
    && l.non_pawn_ptype_attacked_pair == r.non_pawn_ptype_attacked_pair
    && l.non_pawn_ptype_attacked_pair_eval == r.non_pawn_ptype_attacked_pair_eval;
}

osl::CArray<int, 81*15*10>
osl::progress::ml::NewProgress::attack_relative;
osl::CArray<int, 81*15*10>
osl::progress::ml::NewProgress::defense_relative;
osl::CArray<int, osl::Piece::SIZE>
osl::progress::ml::NewProgress::stand_weight;
osl::CArray<int, 1125>
osl::progress::ml::NewProgress::attack5x5_weight;
osl::CArray<int, 5625>
osl::progress::ml::NewProgress::attack5x5_x_weight;
osl::CArray<int, 10125>
osl::progress::ml::NewProgress::attack5x5_y_weight;
osl::CArray<int, 75>
osl::progress::ml::NewProgress::effectstate_weight;
osl::CArray<int, 4284>
osl::progress::ml::NewProgress::king_relative_weight;
osl::CArray<int, 262144> 
osl::progress::ml::NewProgress::attacked_ptype_pair_weight;
osl::CArray<int, 10> 
osl::progress::ml::NewProgress::pawn_facing_weight;
osl::CArray<int, 16> 
osl::progress::ml::NewProgress::promotion37_weight;
osl::CArray<int, 56> 
osl::progress::ml::NewProgress::piecestand7_weight;
int osl::progress::ml::NewProgress::max_progress;
bool osl::progress::ml::NewProgress::initialized_flag;

bool osl::progress::ml::NewProgress::setUp(const char *filename)
{
  if (initialized_flag)
    return true;

  static CArray<int, 25> effect_weight;
  static CArray<int, 225> effect_x_weight, effect_y_weight;
  static CArray<int, 25> effect_defense_weight;
  static CArray<int, 225> effect_per_effect;
  static CArray<int, 225> effect_per_effect_defense;
  static CArray<int, 2025> effect_per_effect_y, effect_per_effect_x;
  std::ifstream is(filename);
  int read_count = 0;

  osl::eval::ml::Weights weights(25);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    effect_weight[i] = val;
    ++read_count;
  }
  for (size_t i = 0; i < 225; ++i)
  {
    int val;
    is >> val;
    effect_x_weight[i] = val;
    ++read_count;
  }
  for (size_t i = 0; i < 225; ++i)
  {
    int val;
    is >> val;
    effect_y_weight[i] = val;
    ++read_count;
  }
  weights.resetDimension(25);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    effect_defense_weight[i] = val;
    ++read_count;
  }
  weights.resetDimension(225);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    effect_per_effect[i] = val;
    ++read_count;
  }
  weights.resetDimension(225);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    effect_per_effect_defense[i] = val;
    ++read_count;
  }

  weights.resetDimension(2025);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    effect_per_effect_y[i] = val;
    ++read_count;
  }
  weights.resetDimension(2025);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    effect_per_effect_x[i] = val;
    ++read_count;
  }
  weights.resetDimension(Piece::SIZE);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    stand_weight[i] = val;
    ++read_count;
  }
  weights.resetDimension(1125);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    attack5x5_weight[i] = val;
    ++read_count;
  }
  weights.resetDimension(75);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    effectstate_weight[i] = val;
    ++read_count;
  }
  weights.resetDimension(5625);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    attack5x5_x_weight[i] = val;
    ++read_count;
  }
  weights.resetDimension(10125);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    attack5x5_y_weight[i] = val;
    ++read_count;
  }
  weights.resetDimension(4284);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    king_relative_weight[i] = val;
    ++read_count;
  }
  weights.resetDimension(262144);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    attacked_ptype_pair_weight[i] = val;
    ++read_count;
  }
  weights.resetDimension(10);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    pawn_facing_weight[i] = val;
    ++read_count;
  }
  weights.resetDimension(16);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    promotion37_weight[i] = val;
    ++read_count;
  }
  weights.resetDimension(56);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    int val;
    is >> val;
    piecestand7_weight[i] = val;
    ++read_count;
  }
  {
    int val;
    is >> val;
    max_progress = val;
    ++read_count;
#ifdef EVAL_QUAD
    while (((max_progress/ProgressScale) % 3) && max_progress > 0)
      --max_progress;
#endif
  }
  for(int king_x=1;king_x<=9;king_x++){
    for(int king_y=1;king_y<=9;king_y++){
      Square king(king_x,king_y);
      int king_index=(king_x-1)*9+king_y-1;
      const Square center = Centering5x3::adjustCenter(king);
      const int min_x = center.x() - 2;
      const int min_y = center.y() - 1;
      int i=0;
      for (int dx=0; dx<5; ++dx)
      {
	for (int dy=0; dy<3; ++dy,++i)
	{
	  const Square target(min_x+dx,min_y+dy);
	  int index0=king_index*15+i;
	  int index_a=index0*10;
	  int index_d=index0*10;
	  attack_relative[index_a]=
	    effect_weight[index<BLACK>(king, target)] +
	    effect_x_weight[indexX<BLACK>(king, target)] +
	    effect_y_weight[indexY<BLACK>(king, target)];
	  defense_relative[index_d]=
	    effect_defense_weight[index<BLACK>(king, target)];
	  for(int count=0;count<=8;count++){
	    attack_relative[index_a+count+1]=
	      effect_per_effect[indexPerEffect<BLACK>(king, target, count)] +
	      effect_per_effect_y[indexPerEffectY<BLACK>(king, target, count)] +
	      effect_per_effect_x[indexPerEffectX<BLACK>(king, target, count)];
	    defense_relative[index_d+count+1]=
	      effect_per_effect_defense[indexPerEffect<BLACK>(king, target, count)];
	  }
	}
      }
    }
  }
  for(int king_x=1;king_x<=5;king_x++)
    for(int promoted=0;promoted<=4;promoted++)
      for(int silver=0;silver<=4;silver++)
	for(int gold=0;gold<=4;gold++)
	  for(int bishop=0;bishop<=2;bishop++)
	    for(int rook=0;rook<=2;rook++){
	      int index0=promoted + 5 * (silver + 5 * (gold + 5 * (bishop + 3 * rook)));
	      int index1=king_x - 1 + 5 * (promoted + 5 * (silver + 5 * (gold + 5 * (bishop + 3 * rook))));
	      attack5x5_x_weight[index1]+=attack5x5_weight[index0];
	    }
  for (int i=0; i<PTYPE_SIZE*2*PTYPE_SIZE; ++i)
    for (int j=i+1; j<PTYPE_SIZE*2*PTYPE_SIZE; ++j) {
      attacked_ptype_pair_weight[eval::ml::NonPawnAttackedPtypePair::index2(j,i)]
	= attacked_ptype_pair_weight[eval::ml::NonPawnAttackedPtypePair::index2(i,j)];
    }
  // set sum of [1..i] into [i], keep [0] as is.
  for (int i=2; i<10; ++i)
    pawn_facing_weight[i] += pawn_facing_weight[i-1];

  initialized_flag = static_cast<bool>(is);
  if (!initialized_flag)
  {
    std::cerr << "Failed to load NewProgress data " << read_count
	      << " from file " << filename << std::endl;
  }
  return initialized_flag;
}

bool osl::progress::ml::NewProgress::setUp()
{
  return setUp(defaultFilename().c_str());  
}

std::string osl::progress::ml::NewProgress::defaultFilename()
{
  std::string filename = OslConfig::home();
  filename += "/data/progress.txt";
  return filename;
}

template <osl::Player P>
void osl::progress::ml::NewProgress::progressOne(
  const NumEffectState &state, int &attack, int &defense)
{
  const Square king = state.kingSquare<P>();
  const Square center = Centering5x3::adjustCenter(king);
  const int min_x = center.x() - 2;
  const int min_y = center.y() - 1;

  attack = defense = 0;
  Square kingRel=king;
  if(P==WHITE){
    kingRel=kingRel.rotate180();
  }
  int index0=((kingRel.x()-1)*9+kingRel.y()-1)*15;
  int index_a=index0*10 + (P==WHITE ? 10*14 : 0);
  for (int dx=0; dx<5; ++dx)
  {
    for (int dy=0; dy<3; ++dy)
    {
      const Square target(min_x+dx,min_y+dy);
      const int attack_count =
	state.countEffect(alt(P), target);
      const int defense_count =
	state.countEffect(P, target);
      attack += attack_count *attack_relative[index_a]+
	attack_relative[index_a+std::min(attack_count,8)+1];
      defense +=
	defense_count * defense_relative[index_a]+
	defense_relative[index_a+std::min(defense_count,8)+1];
      if(P==BLACK){
	index_a+=10;
      }
      else{
	index_a-=10;
      }
    }
  }
}

template <osl::Player P>
void osl::progress::ml::NewProgress::updateAttack5x5PiecesAndState(
  const NumEffectState &state)
{
  const Square king = state.kingSquare<P>();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  effect_progresses[P] = 0;

  PieceMask mask;
  for (int y = min_y; y <= max_y; ++y)
  {
    for (int x = min_x; x <= max_x; ++x)
    {
      const Square target(x, y);
      const NumBitmapEffect effect = state.effectSetAt(target);
      const int effect_diff =
	effect.countEffect(alt(P)) - effect.countEffect(P);
      const int x_diff = std::abs(x - king.x());
      const int y_diff = (P == WHITE ? king.y() - y : y - king.y());
      int index = std::max(std::min(effect_diff, 2), -2) + 2 + 5 * x_diff +
	5 * 3 * (y_diff + 2);
      effect_progresses[P] += effectstate_weight[index];
      mask |= effect;
    }
  }
  updateAttack5x5Pieces<P>(mask, state);
}

template <osl::Player P>
void osl::progress::ml::NewProgress::updateAttack5x5Pieces(
  PieceMask mask, const NumEffectState& state)
{
  const Player attack = alt(P);
  mask &= state.piecesOnBoard(attack);

  rook[attack] = mask.selectBit<ROOK>().countBit();
  bishop[attack] = mask.selectBit<BISHOP>().countBit();
  gold[attack] = mask.selectBit<GOLD>().countBit();
  silver[attack] =
    (mask & ~state.promotedPieces()).selectBit<SILVER>().countBit();
  PieceMask promoted_pieces = mask & state.promotedPieces();
  promoted_pieces.clearBit<ROOK>();
  promoted_pieces.clearBit<BISHOP>();
  promoted[attack] =
    std::min(promoted_pieces.countBit(), 4);
}

template <osl::Player P>
int osl::progress::ml::NewProgress::attack5x5Value(
  const NumEffectState &state) const
{
  const Player attack = alt(P);
  int king_x = state.kingSquare<P>().x();
  if (king_x > 5)
    king_x = 10 - king_x;
  const int king_y = (P == BLACK ? state.kingSquare<P>().y() :
		      10 - state.kingSquare<P>().y());
  return (attack5x5_x_weight[index5x5x(
	      rook[attack] + state.countPiecesOnStand<ROOK>(attack),
	      bishop[attack] + state.countPiecesOnStand<BISHOP>(attack),
	      gold[attack] + state.countPiecesOnStand<GOLD>(attack),
	      silver[attack] + state.countPiecesOnStand<SILVER>(attack),
	      promoted[attack], king_x)] +
	  attack5x5_y_weight[index5x5y(
	      rook[attack] + state.countPiecesOnStand<ROOK>(attack),
	      bishop[attack] + state.countPiecesOnStand<BISHOP>(attack),
	      gold[attack] + state.countPiecesOnStand<GOLD>(attack),
	      silver[attack] + state.countPiecesOnStand<SILVER>(attack),
	      promoted[attack], king_y)]);
}

void
osl::progress::ml::NewProgress::updatePieceKingRelativeBonus(
  const NumEffectState &state)
{
  const CArray<Square,2> kings = {{ 
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  king_relative_attack.fill(0);
  king_relative_defense.fill(0);
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.ptype() == osl::KING || !piece.isOnBoard())
      continue;
    Player pl = piece.owner();
    const int index_attack = indexRelative(piece.owner(), kings[alt(pl)],
					   piece);
    const int index_defense = indexRelative(piece.owner(), kings[pl],
					    piece) + 2142;
    king_relative_attack[pl] += king_relative_weight[index_attack];
    king_relative_defense[pl] += king_relative_weight[index_defense];
  }
}

template <osl::Player Owner>
void osl::progress::ml::NewProgress::
updateNonPawnAttackedPtypePairOne(const NumEffectState& state)
{
  PieceMask attacked = state.effectedMask(alt(Owner)) & state.piecesOnBoard(Owner);
  attacked.reset(state.kingPiece<Owner>().number());
  mask_t ppawn = state.promotedPieces().getMask<PAWN>() & attacked.selectBit<PAWN>();
  attacked.clearBit<PAWN>();
  attacked.orMask(PtypeFuns<PAWN>::indexNum, ppawn);
  PieceVector pieces;
  while (attacked.any())
  {
    const Piece piece = state.pieceOf(attacked.takeOneBit());
    pieces.push_back(piece);
  }
  typedef eval::ml::NonPawnAttackedPtypePair feature_t;
  int result = 0;
  MultiInt result_eval;
  for (size_t i=0; i<pieces.size(); ++i) {
    const int i0 = feature_t::index1(state, pieces[i]);
    result += attacked_ptype_pair_weight[feature_t::index2(0,i0)];
    for (size_t j=i+1; j<pieces.size(); ++j) {
      const int i1 = feature_t::index1(state, pieces[j]);
      result += attacked_ptype_pair_weight[feature_t::index2(i0,i1)];
      if (Owner == BLACK)
	result_eval += feature_t::table[feature_t::index2(i0, i1)];
      else
	result_eval -= feature_t::table[feature_t::index2(i0, i1)];
    }
  }
  non_pawn_ptype_attacked_pair[Owner] = result;
  non_pawn_ptype_attacked_pair_eval[Owner] = result_eval;
}

void osl::progress::ml::NewProgress::
updateNonPawnAttackedPtypePair(const NumEffectState& state)
{
  updateNonPawnAttackedPtypePairOne<BLACK>(state);
  updateNonPawnAttackedPtypePairOne<WHITE>(state);
}

void osl::progress::ml::NewProgress::
updatePawnFacing(const NumEffectState& state)
{
  PieceMask attacked = state.effectedMask(WHITE) & state.piecesOnBoard(BLACK);
  mask_t pawn = attacked.selectBit<PAWN>()
    & ~(state.promotedPieces().getMask<PAWN>());
  int count = 0;
  while (pawn.any()) {
    const Piece p(state.pieceOf(pawn.takeOneBit()+PtypeFuns<PAWN>::indexNum*32));
    if (state.hasEffectByPtypeStrict<PAWN>(WHITE, p.square()))
      ++count;
  }
  pawn_facing = pawn_facing_weight[count];
}

template <osl::Player P>
void osl::progress::ml::
NewProgress::promotion37One(const NumEffectState& state, int rank)
{
  typedef eval::ml::Promotion37 feature_t;
  CArray<int,PTYPE_SIZE> count = {{ 0 }};
  for (int x=1; x<=9; ++x) {
    const Square target(x, rank);
    if (! state[target].isEmpty())
      continue;
    int a = state.countEffect(P, target);
    const int d = state.countEffect(alt(P), target);
    if (a > 0 && a == d)
      a += AdditionalEffect::hasEffect(state, target, P);
    if (a <= d)
      continue;
    const Ptype ptype = state.findCheapAttack(P, target).ptype();
    if (isPiece(ptype) && ! isPromoted(ptype))
      count[ptype]++;
  }
  for (int p=PTYPE_BASIC_MIN; p<=PTYPE_MAX; ++p) {
    if (count[p] > 0) {
      promotion37 += promotion37_weight[p];
      promotion37_eval += feature_t::table[p]*sign(P);
    }
    if (count[p] > 1) {
      promotion37 += promotion37_weight[p-8]*(count[p]-1);
      promotion37_eval += feature_t::table[p-8]*(sign(P)*(count[p]-1));
    }
  }
}

void osl::progress::ml::NewProgress::
updatePromotion37(const NumEffectState& state)
{
  promotion37 = 0;
  promotion37_eval = MultiInt();
  promotion37One<BLACK>(state, 3);
  promotion37One<WHITE>(state, 7);
}

void osl::progress::ml::NewProgress::
updatePieceStand7(const NumEffectState& state)
{
  piecestand7 = 0;
  for (int z=0; z<2; ++z) {
    CArray<int,7> stand = {{ 0 }};
    int filled = 0;
    for (Ptype ptype: PieceStand::order)
      if (state.hasPieceOnStand(indexToPlayer(z), ptype))
	stand[filled++] = ptype-PTYPE_BASIC_MIN;
    for (int i=0; i<std::min(7,filled+1); ++i)
      piecestand7 += piecestand7_weight[stand[i] + 8*i];
  }
}

osl::progress::ml::NewProgress::NewProgress(
  const NumEffectState &state)
{
  assert(initialized_flag);
  
  progressOne<BLACK>(state,
		     progresses[BLACK],
		     defenses[WHITE]);
  progressOne<WHITE>(state,
		     progresses[WHITE],
		     defenses[BLACK]);
  updateAttack5x5PiecesAndState<BLACK>(state);
  updateAttack5x5PiecesAndState<WHITE>(state);
  attack5x5_progresses[BLACK] =
    attack5x5Value<BLACK>(state);
  attack5x5_progresses[WHITE] =
    attack5x5Value<WHITE>(state);
  stand_progresses.fill(0);
  for (Ptype ptype: PieceStand::order)
  {
    const int black_count =
      state.countPiecesOnStand(BLACK, ptype);
    const int white_count =
      state.countPiecesOnStand(WHITE, ptype);
    for (int j = 0; j < black_count; ++j)
    {
      stand_progresses[WHITE] +=
	stand_weight[Ptype_Table.getIndexMin(ptype) + j];
    }
    for (int j = 0; j < white_count; ++j)
    {
      stand_progresses[BLACK] +=
	stand_weight[Ptype_Table.getIndexMin(ptype) + j];
    }
  }
  updatePieceKingRelativeBonus(state);
  updateNonPawnAttackedPtypePair(state);
  updatePawnFacing(state);
  updatePromotion37(state);
  updatePieceStand7(state);
}

template<osl::Player P>
inline
void osl::progress::ml::NewProgress::updateMain(
  const NumEffectState &new_state,
  Move last_move)
{
  const Player altP=alt(P);
  assert(new_state.turn()==altP);
  assert(last_move.player()==P);
  const Square kb = new_state.kingSquare<BLACK>(), kw = new_state.kingSquare<WHITE>();
  const BoardMask mb = new_state.changedEffects(BLACK), mw = new_state.changedEffects(WHITE);
  const bool king_move = last_move.ptype() == KING;
  if ((king_move && altP == BLACK) || mb.anyInRange(Board_Mask_Table5x3_Center.mask(kw)) || mw.anyInRange(Board_Mask_Table5x3_Center.mask(kw)))
  {
    progressOne<WHITE>(new_state,progresses[WHITE],defenses[BLACK]);
  }
  if ((king_move && altP == WHITE) || mw.anyInRange(Board_Mask_Table5x3_Center.mask(kb)) || mb.anyInRange(Board_Mask_Table5x3_Center.mask(kb)))
  {
    progressOne<BLACK>(new_state,progresses[BLACK],defenses[WHITE]);
  }

  const Ptype captured = last_move.capturePtype();

  if (last_move.isDrop())
  {
    const int count =
      new_state.countPiecesOnStand(P, last_move.ptype()) + 1;
    const int value =
      stand_weight[Ptype_Table.getIndexMin(last_move.ptype()) + count - 1];
    stand_progresses[altP] -= value;
  }
  else if (captured != PTYPE_EMPTY)
  {
    Ptype ptype = unpromote(captured);
    const int count = new_state.countPiecesOnStand(P, ptype);
    const int value =
      stand_weight[(Ptype_Table.getIndexMin(ptype) + count - 1)];
    stand_progresses[altP] += value;
  }
  
  if (king_move)
  {
    updatePieceKingRelativeBonus(new_state);
  }
  else
  {
    const CArray<Square,2> kings = {{ 
	new_state.kingSquare(BLACK),
	new_state.kingSquare(WHITE),
      }};
    if (!last_move.isDrop())
    {
      const int index_attack =
	indexRelative<P>(kings[altP],
			 last_move.oldPtype(), last_move.from());
      const int index_defense =
	indexRelative<P>(kings[P],
			 last_move.oldPtype(), last_move.from()) + 2142;
      king_relative_attack[P] -=
	king_relative_weight[index_attack];
      king_relative_defense[P] -=
	king_relative_weight[index_defense];
    }
    {
      const int index_attack =
	indexRelative<P>(kings[altP],
			 last_move.ptype(), last_move.to());
      const int index_defense =
	indexRelative<P>(kings[P],
			 last_move.ptype(), last_move.to()) + 2142;
      king_relative_attack[P] +=
	king_relative_weight[index_attack];
      king_relative_defense[P] +=
	king_relative_weight[index_defense];
    }
    if (captured != PTYPE_EMPTY)
    {
      const int index_attack =
	indexRelative<altP>(kings[P],
			    captured, last_move.to());
      const int index_defense =
	indexRelative<altP>(kings[altP],
			    captured, last_move.to()) + 2142;
      king_relative_attack[altP] -=
	king_relative_weight[index_attack];
      king_relative_defense[altP] -=
	king_relative_weight[index_defense];
    }
  }
  updateNonPawnAttackedPtypePair(new_state);
  updatePawnFacing(new_state);
  updatePromotion37(new_state);
  updatePieceStand7(new_state);
}

template<osl::Player P>
void osl::progress::ml::NewProgress::updateSub(
  const NumEffectState &new_state,
  Move last_move)
{
  const Player altP=alt(P);
  assert(new_state.turn()==altP);
  if (last_move.isPass())
    return;
  const Square kb = new_state.kingSquare<BLACK>(), kw = new_state.kingSquare<WHITE>();
  const BoardMask mb = new_state.changedEffects(BLACK), mw = new_state.changedEffects(WHITE);
  const bool king_move = last_move.ptype() == KING;
  const Ptype captured = last_move.capturePtype();

  if ((king_move && altP == BLACK) ||
      mb.anyInRange(Board_Mask_Table5x5.mask(kw)) ||
      mw.anyInRange(Board_Mask_Table5x5.mask(kw)))
  {
    updateAttack5x5PiecesAndState<WHITE>(new_state);
    attack5x5_progresses[WHITE] =
      attack5x5Value<WHITE>(new_state);
  }
  else if (altP == WHITE &&(last_move.isDrop() || captured != PTYPE_EMPTY))
  {
    attack5x5_progresses[WHITE] =
      attack5x5Value<WHITE>(new_state);
  }
  if ((king_move && altP == WHITE) ||
      mw.anyInRange(Board_Mask_Table5x5.mask(kb)) ||
      mb.anyInRange(Board_Mask_Table5x5.mask(kb)))
  {
    updateAttack5x5PiecesAndState<BLACK>(new_state);
    attack5x5_progresses[BLACK] =
      attack5x5Value<BLACK>(new_state);
  }
  else if (altP == BLACK && (last_move.isDrop() || captured != PTYPE_EMPTY))
  {
    attack5x5_progresses[BLACK] =
      attack5x5Value<BLACK>(new_state);
  }
  updateMain<P>(new_state, last_move);
}

osl::progress::ml::NewProgressDebugInfo osl::progress::ml::NewProgress::debugInfo() const
{
  NewProgressDebugInfo info;

  info.black_values[NewProgressDebugInfo::ATTACK_5X3] = progresses[0];
  info.black_values[NewProgressDebugInfo::DEFENSE_5X3] = defenses[0];
  info.black_values[NewProgressDebugInfo::ATTACK5X5] = attack5x5_progresses[0];
  info.black_values[NewProgressDebugInfo::STAND] = stand_progresses[0];
  info.black_values[NewProgressDebugInfo::EFFECT5X5] = effect_progresses[0];
  info.black_values[NewProgressDebugInfo::KING_RELATIVE_ATTACK] = king_relative_attack[0];
  info.black_values[NewProgressDebugInfo::KING_RELATIVE_DEFENSE] = king_relative_defense[0];
  info.black_values[NewProgressDebugInfo::NON_PAWN_ATTACKED_PAIR] = non_pawn_ptype_attacked_pair[0];

  info.white_values[NewProgressDebugInfo::ATTACK_5X3] = progresses[1];
  info.white_values[NewProgressDebugInfo::DEFENSE_5X3] = defenses[1];
  info.white_values[NewProgressDebugInfo::ATTACK5X5] = attack5x5_progresses[1];
  info.white_values[NewProgressDebugInfo::STAND] = stand_progresses[1];
  info.white_values[NewProgressDebugInfo::EFFECT5X5] = effect_progresses[1];
  info.white_values[NewProgressDebugInfo::KING_RELATIVE_ATTACK] = king_relative_attack[1];
  info.white_values[NewProgressDebugInfo::KING_RELATIVE_DEFENSE] = king_relative_defense[1];
  info.white_values[NewProgressDebugInfo::NON_PAWN_ATTACKED_PAIR] = non_pawn_ptype_attacked_pair[1];

  return info;
}

namespace osl
{
  namespace progress
  {
    namespace ml
    {
      template void osl::progress::ml::NewProgress::updateSub<osl::BLACK>(const NumEffectState &new_state,Move last_move);
      template void osl::progress::ml::NewProgress::updateSub<osl::WHITE>(const NumEffectState &new_state,Move last_move);
    }
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
