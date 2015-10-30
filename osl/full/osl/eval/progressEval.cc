/* progressEval.cc
 */
#include "osl/eval/progressEval.h"
#include "osl/container/pieceValues.h"
#include "osl/numEffectState.tcc"
#include "osl/effect_util/effectUtil.tcc"
#include "osl/oslConfig.h"
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <cstdio>

osl::eval::PtypeEvalTable osl::eval::ProgressEval::Piece_Value;
static osl::SetUpRegister _initializer([](){ 
    osl::eval::ProgressEval::Piece_Value.init(); 
});

static_assert((osl::eval::ProgressEval::ROUND_UP 
	       & (osl::eval::ProgressEval::ROUND_UP-1))
	      == 0, "");
#ifndef MINIMAL
template <class Opening>
osl::CArray<int, osl::PTYPEO_SIZE> osl::eval::ProgressEvalGeneral<Opening>::capture_values;

template <class Opening>
int osl::eval::
ProgressEvalGeneral<Opening>::expect(const NumEffectState& state, Move move) const 
{
  if (move.isPass())
    return value();
  progress_t new_progress = current_progress.expect(state, move);
  return composeValue(opening_eval.expect(state, move),
		      endgame_eval.expect(state, move), 
		      progress16(), new_progress.progress16(BLACK),
		      new_progress.progress16(WHITE),
		      defense_effect.progress16(BLACK),
		      defense_effect.progress16(WHITE),
		      minor_piece_bonus.value(progress16(),
					      progress16bonus(BLACK),
					      progress16bonus(WHITE)),
		      progress_independent_bonus,
		      progress_dependent_bonus);
}

template <class Opening>
void osl::eval::
ProgressEvalGeneral<Opening>::setUpInternal(const char *filename_given)
{
  // read weights if exists
  std::string filename;
  if (filename_given)
    filename = filename_given;
  else {
    filename = OslConfig::home();
    filename += "/data/progresseval.txt";
  }
  if (boost::filesystem::exists(filename.c_str())) {
    if (OslConfig::verbose())
      std::cerr << "loading " << filename << "\n";
    CArray<int, AdjustableDimension> w;
    FILE *fp = fopen(filename.c_str(), "r");
    for (size_t i=0; i<w.size(); ++i) {
      if (fscanf(fp, "%d", &w[i]) != 1) {
	std::cerr << filename << " read failed " << i << "\n";
	break;
      }
    }
    fclose(fp);
    resetWeights(&w[0]);
  }

  // others
  for (int i=0; i<PTYPEO_SIZE; ++i) {
    // 序盤を使用
    const PtypeO ptypeo = static_cast<PtypeO>(i+PTYPEO_MIN);
    capture_values[i] = composeValue(opening_eval_t::captureValue(ptypeo), 0, 
				     Progress16(0), Progress16(0), Progress16(0),
				     Progress16(0), Progress16(0), 0, 0, 0);
  }
}

template <class Opening>
void osl::eval::
ProgressEvalGeneral<Opening>::resetWeights(const int *w)
{
  opening_eval_t::resetWeights(w);
  endgame_eval_t::resetWeights(w+PTYPE_SIZE);
}

template <class Opening>
osl::eval::
ProgressEvalGeneral<Opening>::ProgressEvalGeneral(const NumEffectState& state) 
  : opening_eval(state), endgame_eval(state),
    current_progress(state), defense_effect(state),
    minor_piece_bonus(state), major_pieces(0),
    cache(INVALID)
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit; ++i)
  {
    if (state.pieceOf(i).owner() == osl::BLACK)
      ++major_pieces;
  }
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit; ++i)
  {
    if (state.pieceOf(i).owner() == osl::BLACK)
      ++major_pieces;
  }
  can_check_pieces.fill(0);
  // knight and pawn are intentionally omitted
  initializeCheckPiece<BLACK, ROOK>(state);
  initializeCheckPiece<BLACK, BISHOP>(state);
  initializeCheckPiece<BLACK, GOLD>(state);
  initializeCheckPiece<BLACK, SILVER>(state);
  initializeCheckPiece<BLACK, LANCE>(state);
  initializeCheckPiece<WHITE, ROOK>(state);
  initializeCheckPiece<WHITE, BISHOP>(state);
  initializeCheckPiece<WHITE, GOLD>(state);
  initializeCheckPiece<WHITE, SILVER>(state);
  initializeCheckPiece<WHITE, LANCE>(state);
  rook_mobility = calculateMobilityBonusRook(state);
  bishop_mobility = calculateMobilityBonusBishop(state);
  lance_mobility = calculateMobilityBonusLance(state);
  progress_independent_bonus = calculateMobilityBonus();
  progress_independent_bonus += calculateAttackRooks(state);
  progress_independent_bonus += calculateSilverPenalty(state);
  progress_independent_bonus += calculateGoldPenalty(state);
  attack_bonus[BLACK] = calculateAttackBonusEach<WHITE>(state);
  attack_bonus[WHITE] = calculateAttackBonusEach<BLACK>(state);
  progress_dependent_bonus  = attackBonusScale(attack_bonus[BLACK], WHITE);
  progress_dependent_bonus += attackBonusScale(attack_bonus[WHITE], BLACK);
  progress_dependent_bonus += calculatePinBonus(state);
  progress_independent_bonus += calculateKnightCheck(state);
  progress_independent_bonus += calculateRookRankBonus(state);
  progress_independent_bonus += calculateEnterKingBonus<BLACK>(state);
  progress_independent_bonus += calculateEnterKingBonus<WHITE>(state);
  progress_independent_bonus += calculateMiddleKingBonus<BLACK>(state);
  progress_independent_bonus += calculateMiddleKingBonus<WHITE>(state);
  assert(initialized());
}

template <class Opening>
template<osl::Player P, osl::Ptype PTYPE>
void osl::eval::
ProgressEvalGeneral<Opening>::initializeCheckPiece(
  const NumEffectState &state)
{
  if (state.hasPieceOnStand<PTYPE>(P))
  {
    int count = state.countPiecesOnStand(P, PTYPE);
    initializeCheckPieceDir<P, PTYPE, UL, LONG_UL>(state, count);
    initializeCheckPieceDir<P, PTYPE, U, LONG_U>(state, count);
    initializeCheckPieceDir<P, PTYPE, UR, LONG_UR>(state, count);
    initializeCheckPieceDir<P, PTYPE, L, LONG_L>(state, count);
    initializeCheckPieceDir<P, PTYPE, R, LONG_R>(state, count);
    initializeCheckPieceDir<P, PTYPE, DL, LONG_DL>(state, count);
    initializeCheckPieceDir<P, PTYPE, D, LONG_D>(state, count);
    initializeCheckPieceDir<P, PTYPE, DR, LONG_DR>(state, count);
  }
}

template <class Opening>
template<osl::Player P, osl::Ptype PTYPE, osl::Direction Dir, osl::Direction LongDir>
void osl::eval::
ProgressEvalGeneral<Opening>::initializeCheckPieceDir(
  const NumEffectState &,
  int count)
{
  if (PtypeTraits<PTYPE>::moveMask & (DirectionTraits<Dir>::mask
				      | DirectionTraits<LongDir>::mask))
    can_check_pieces[P][Dir] = count;
}

template <class Opening>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateMobilityBonusRook(const NumEffectState& state)
{
  using namespace osl::mobility;
  int val=0;
  for(int i=PtypeTraits<ROOK>::indexMin;
      i<PtypeTraits<ROOK>::indexLimit;++i){
    Piece p=state.pieceOf(i);
    if(p.isOnBoardByOwner<BLACK>()){
      int vc= RookMobility::countVerticalAll(BLACK,state,p);
      int hc= RookMobility::countHorizontalAll(BLACK,state,p);
      if(p.isPromoted()){
       val+=MobilityTable::prookVertical[vc];
       val+=MobilityTable::prookHorizontal[hc];
      }
      else{
       val+=MobilityTable::rookVertical[vc];
       val+=MobilityTable::rookHorizontal[hc];
      }
    }
    else if(p.isOnBoardByOwner<WHITE>()){
      int vc= RookMobility::countVerticalAll(WHITE,state,p);
      int hc= RookMobility::countHorizontalAll(WHITE,state,p);
      if(p.isPromoted()){
       val-=MobilityTable::prookVertical[vc];
       val-=MobilityTable::prookHorizontal[hc];
      }
      else{
       val-=MobilityTable::rookVertical[vc];
       val-=MobilityTable::rookHorizontal[hc];
      }
    }
  }
  return val;
}

template <class Opening>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateMobilityBonusBishop(const NumEffectState& state)
{
  using namespace osl::mobility;
  int val=0;
  for(int i=PtypeTraits<BISHOP>::indexMin;
      i<PtypeTraits<BISHOP>::indexLimit;++i){
    Piece p=state.pieceOf(i);
    if(p.isOnBoardByOwner<BLACK>()){
      int c= BishopMobility::countAll(BLACK,state,p);
      if(p.isPromoted())
       val+=MobilityTable::pbishop[c];
      else
       val+=MobilityTable::bishop[c];
    }
    else if(p.isOnBoardByOwner<WHITE>()){
      int c= BishopMobility::countAll(WHITE,state,p);
      if(p.isPromoted())
       val-=MobilityTable::pbishop[c];
      else
       val-=MobilityTable::bishop[c];
    }
  }
  return val;
}

template <class Opening>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateMobilityBonusLance(const NumEffectState& state)
{
  using namespace osl::mobility;
  int val=0;
  for(int i=PtypeTraits<LANCE>::indexMin;
      i<PtypeTraits<LANCE>::indexLimit;++i){
    Piece p=state.pieceOf(i);
    if(p.isOnBoardByOwner<BLACK>() && !p.isPromoted()){
      int c= LanceMobility::countAll(BLACK,state,p);
      val+=MobilityTable::lance[c];
    }
    else if(p.isOnBoardByOwner<WHITE>()  && !p.isPromoted()){
      int c= LanceMobility::countAll(WHITE,state,p);
      val-=MobilityTable::lance[c];
    }
  }
  return val;
}

template <class Opening> inline
int osl::eval::
ProgressEvalGeneral<Opening>::calculateMobilityBonus() const
{
  using namespace osl::mobility;
  int val=rook_mobility + bishop_mobility + lance_mobility;
  return val*128/100 * 12;
}

template <class Opening>
void osl::eval::
ProgressEvalGeneral<Opening>::update(
  const NumEffectState& new_state, Move last_move)
{
  if (last_move.isPass())
    return;
  const Ptype captured = last_move.capturePtype();
  if (last_move.isDrop())
  {
    const Ptype ptype = last_move.ptype();
    if (ptype == ROOK)
    {
      --can_check_pieces[playerToIndex(last_move.player())][U];
      --can_check_pieces[playerToIndex(last_move.player())][D];
      --can_check_pieces[playerToIndex(last_move.player())][L];
      --can_check_pieces[playerToIndex(last_move.player())][R];
    }
    else if (ptype == BISHOP)
    {
      --can_check_pieces[playerToIndex(last_move.player())][UL];
      --can_check_pieces[playerToIndex(last_move.player())][DL];
      --can_check_pieces[playerToIndex(last_move.player())][UR];
      --can_check_pieces[playerToIndex(last_move.player())][DR];
    }
    if (ptype == GOLD)
    {
      --can_check_pieces[playerToIndex(last_move.player())][U];
      --can_check_pieces[playerToIndex(last_move.player())][D];
      --can_check_pieces[playerToIndex(last_move.player())][L];
      --can_check_pieces[playerToIndex(last_move.player())][R];
      --can_check_pieces[playerToIndex(last_move.player())][UL];
      --can_check_pieces[playerToIndex(last_move.player())][UR];
    }
    else if (ptype == SILVER)
    {
      --can_check_pieces[playerToIndex(last_move.player())][U];
      --can_check_pieces[playerToIndex(last_move.player())][UL];
      --can_check_pieces[playerToIndex(last_move.player())][DL];
      --can_check_pieces[playerToIndex(last_move.player())][UR];
      --can_check_pieces[playerToIndex(last_move.player())][DR];
    }
    if (ptype == LANCE)
    {
      --can_check_pieces[playerToIndex(last_move.player())][U];
    }
  }

  if (captured != PTYPE_EMPTY)
  {
    const Ptype captured_base = unpromote(captured);
    if (isMajor(captured_base))
    {
      if (last_move.player() == BLACK)
	++major_pieces;
      else
	--major_pieces;
    }
    if (captured_base == ROOK)
    {
      ++can_check_pieces[playerToIndex(last_move.player())][U];
      ++can_check_pieces[playerToIndex(last_move.player())][D];
      ++can_check_pieces[playerToIndex(last_move.player())][L];
      ++can_check_pieces[playerToIndex(last_move.player())][R];
    }
    else if (captured_base == BISHOP)
    {
      ++can_check_pieces[playerToIndex(last_move.player())][UL];
      ++can_check_pieces[playerToIndex(last_move.player())][DL];
      ++can_check_pieces[playerToIndex(last_move.player())][UR];
      ++can_check_pieces[playerToIndex(last_move.player())][DR];
    }
    if (captured_base == GOLD)
    {
      ++can_check_pieces[playerToIndex(last_move.player())][U];
      ++can_check_pieces[playerToIndex(last_move.player())][D];
      ++can_check_pieces[playerToIndex(last_move.player())][L];
      ++can_check_pieces[playerToIndex(last_move.player())][R];
      ++can_check_pieces[playerToIndex(last_move.player())][UL];
      ++can_check_pieces[playerToIndex(last_move.player())][UR];
    }
    else if (captured_base == SILVER)
    {
      ++can_check_pieces[playerToIndex(last_move.player())][U];
      ++can_check_pieces[playerToIndex(last_move.player())][UL];
      ++can_check_pieces[playerToIndex(last_move.player())][DL];
      ++can_check_pieces[playerToIndex(last_move.player())][UR];
      ++can_check_pieces[playerToIndex(last_move.player())][DR];
    }
    if (captured_base == LANCE)
    {
      ++can_check_pieces[playerToIndex(last_move.player())][U];
    }
  }
  opening_eval.update(new_state, last_move);
  endgame_eval.update(new_state, last_move);
  current_progress.update(new_state, last_move);
  defense_effect.update(new_state, last_move);
  minor_piece_bonus.update(new_state, last_move);

  if (new_state.longEffectChanged<ROOK>())
    rook_mobility = calculateMobilityBonusRook(new_state);
  if (new_state.longEffectChanged<BISHOP>())
    bishop_mobility = calculateMobilityBonusBishop(new_state);
  if (new_state.longEffectChanged<LANCE>())
    lance_mobility = calculateMobilityBonusLance(new_state);

  progress_independent_bonus = calculateMobilityBonus();
  progress_independent_bonus += calculateAttackRooks(new_state);
  progress_independent_bonus += calculateSilverPenalty(new_state);
  progress_independent_bonus += calculateGoldPenalty(new_state);
  
  {
    bool capture_or_drop = last_move.isDrop() || last_move.capturePtype() != PTYPE_EMPTY;
    const Square kb = new_state.kingSquare<BLACK>(), kw = new_state.kingSquare<WHITE>();
    BoardMask mask = new_state.changedEffects();
    mask.set(last_move.from()); mask.set(last_move.to());
    if ((capture_or_drop && new_state.turn() == BLACK)
	|| mask.anyInRange(Board_Mask_Table3x3.mask(kb)))
      attack_bonus[BLACK] = calculateAttackBonusEach<WHITE>(new_state);
    if ((capture_or_drop && new_state.turn() == WHITE)
	|| mask.anyInRange(Board_Mask_Table3x3.mask(kw)))
      attack_bonus[WHITE] = calculateAttackBonusEach<BLACK>(new_state);
  }
  progress_dependent_bonus  = attackBonusScale(attack_bonus[BLACK], WHITE);
  progress_dependent_bonus += attackBonusScale(attack_bonus[WHITE], BLACK);
  progress_dependent_bonus += calculatePinBonus(new_state);
  progress_independent_bonus += calculateKnightCheck(new_state);
  progress_independent_bonus += calculateRookRankBonus(new_state);
  progress_independent_bonus += calculateEnterKingBonus<BLACK>(new_state);
  progress_independent_bonus += calculateEnterKingBonus<WHITE>(new_state);
  progress_independent_bonus += calculateMiddleKingBonus<BLACK>(new_state);
  progress_independent_bonus += calculateMiddleKingBonus<WHITE>(new_state);
  invalidateCache();
}

template <class Opening>
int osl::eval::
ProgressEvalGeneral<Opening>::calculatePinBonus(
  const NumEffectState& state) const
{
  const Piece black_king = state.kingPiece<BLACK>();
  const Piece white_king = state.kingPiece<WHITE>();
  int bonus = 0;
  PieceMask white_mask = pin_mask[WHITE] = state.pin(WHITE);
  PieceMask black_mask = pin_mask[BLACK] = state.pin(BLACK);
  while (white_mask.any())
  {
    const Piece piece = state.pieceOf(white_mask.takeOneBit());
    bonus -= endgame_eval.valueOf(
      black_king, white_king,
      piece) / 4;
  }

  while (black_mask.any())
  {
    const Piece piece = state.pieceOf(black_mask.takeOneBit());
    bonus -= endgame_eval.valueOf(
      black_king, white_king,
      piece) / 4;
  }
	
  return bonus * progress16().value() / 16;
}

template <class Opening>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateAttackRooks(
  const NumEffectState& state) const
{
  int rooks = 0;
  for(int i = PtypeTraits<ROOK>::indexMin;
      i < PtypeTraits<ROOK>::indexLimit; ++i)
  {
    const Piece rook = state.pieceOf(i);
    if (rook.isOnBoard() && rook.square().canPromote(rook.owner()) &&
	state.kingPiece(alt(rook.owner())).square().canPromote(rook.owner()))
    {
      if (rook.owner() == BLACK)
	++rooks;
      else
	--rooks;
    }
  }
  if (rooks == 2)
    return (PtypeEvalTraits<KNIGHT>::val + PtypeEvalTraits<PAWN>::val) * 16;
  else if (rooks == -2)
    return -(PtypeEvalTraits<KNIGHT>::val + PtypeEvalTraits<PAWN>::val) * 16;

  return 0;
}

template <class Opening>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateAttackBonus(
  const NumEffectState& state) const
{
  return attackBonusScale(calculateAttackBonusEach<BLACK>(state), BLACK) +
    attackBonusScale(calculateAttackBonusEach<WHITE>(state), WHITE);
}

template <class Opening>
template <osl::Player Attack, osl::Direction Dir>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateAttackBonusOne(
  const NumEffectState& state) const
{
  constexpr Player defense = alt(Attack);
  const Square king = state.kingSquare<defense>();

  const Square target = king + DirectionPlayerTraits<Dir, defense>::offset();
  int result = 0;

  const Piece p = state.pieceAt(target);
  if (! p.isEdge() && (Dir != UUR || Attack != BLACK || p.isOnBoard()))
  {
    int effect_diff = (state.countEffect(Attack, target) -
		       state.countEffect(alt(Attack), target));
    if ((effect_diff >= 0 && p.isEmpty()) ||
	(effect_diff >= 1 && !p.isEmpty() &&
	 p.owner() ==alt(Attack)))
    {
      if (Dir == UL || Dir == U || Dir == UR)
	result = PtypeEvalTraits<PAWN>::val * 3 * 16;
      else if (Dir == L || Dir == R)
	result = (PtypeEvalTraits<PAWN>::val * 1 +
		   PtypeEvalTraits<PAWN>::val / 2) * 16;
      else
	result = PtypeEvalTraits<PAWN>::val * 1 * 16;

      if ((effect_diff > 0 &&
	   (target.canPromote<Attack>() ||
	    state.hasEffectByPtype<GOLD>(Attack,target) ||
	    state.hasEffectByPtype<SILVER>(Attack,target) ||
	    state.hasEffectByPtype<ROOK>(Attack,target) ||
	    state.hasEffectByPtype<BISHOP>(Attack,target))) ||
	  (p.isEmpty() &&
	   can_check_pieces[Attack][Dir] > 0))
	result += PtypeEvalTraits<PAWN>::val * 16;
    }
  }

  if (Attack == BLACK)
    return result;
  else
    return -result;
}

// P is attacking player
template <class Opening>
template <osl::Player P>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateAttackBonusEach(
  const NumEffectState& state) const
{
  int result = 0;
  result += calculateAttackBonusOne<P, UL>(state);
  result += calculateAttackBonusOne<P, U>(state);
  result += calculateAttackBonusOne<P, UR>(state);
  result += calculateAttackBonusOne<P, L>(state);
  result += calculateAttackBonusOne<P, R>(state);
  result += calculateAttackBonusOne<P, DL>(state);
  result += calculateAttackBonusOne<P, D>(state);
  result += calculateAttackBonusOne<P, DR>(state);
  return result;
}

template <class Opening>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateSilverPenalty(
  const NumEffectState &state) const
{
  int result = 0;
  const int bonus = PtypeEvalTraits<PAWN>::val / 4 * 16;
  for (int i = PtypeTraits<SILVER>::indexMin;
       i < PtypeTraits<SILVER>::indexLimit; ++i)
  {
    const Piece silver = state.pieceOf(i);
    if (!silver.isOnBoard() || silver.isPromoted())
      continue;

    if (silver.square().y() >= 4 && silver.square().y() <= 6)
    {
      Square dl = Board_Table.nextSquare(silver.owner(),
					     silver.square(), DL);
      Square dr = Board_Table.nextSquare(silver.owner(),
					     silver.square(), DR);
      if ((!dl.isOnBoard() ||
	   state.pieceAt(dl).isOnBoardByOwner(silver.owner()) ||
	   state.hasEffectAt(alt(silver.owner()), dl)) &&
	  (!dr.isOnBoard() ||
	   state.pieceAt(dr).isOnBoardByOwner(silver.owner()) ||
	   state.hasEffectAt(alt(silver.owner()), dr)))
      {
	if (silver.owner() == BLACK)
	  result -= bonus;
	else
	  result += bonus;
      }
    }
	    
  }
  return result;
}

template <class Opening>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateGoldPenalty(
  const NumEffectState &state) const
{
  int result = 0;
  const int bonus = PtypeEvalTraits<PAWN>::val / 4 * 16;
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit; ++i)
  {
    const Piece gold = state.pieceOf(i);
    if (!gold.isOnBoard())
      continue;

    if (gold.square().y() >= 4 && gold.square().y() <= 6)
    {
      Square d = Board_Table.nextSquare(gold.owner(),
					    gold.square(), D);
      if ((state.pieceAt(d).isOnBoardByOwner(gold.owner()) ||
	   state.hasEffectAt(alt(gold.owner()), d)))
      {
	if (gold.owner() == BLACK)
	  result -= bonus;
	else
	  result += bonus;
      }
    }
	    
  }
  return result;
}

template <class Opening>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateKnightCheck(
  const NumEffectState& state) const
{
  return calculateKnightCheckEach<BLACK>(state) +
    calculateKnightCheckEach<WHITE>(state);
}

// P is attacking player
template <class Opening>
template <osl::Player P>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateKnightCheckEach(
  const NumEffectState& state) const
{
  const int bonus = (P == BLACK ? 1 : -1) *
    (PtypeEvalTraits<PAWN>::val * 3 + PtypeEvalTraits<PAWN>::val / 2) * 16;
  const Square king = state.kingSquare<alt(P)>();
  const Square up = king +
    DirectionPlayerTraits<U,alt(P)>::offset();
  if (!state.hasEffectAt<alt(P)>(king)
      && ! state.pieceAt(up).isEdge())
  {
    const Square ur =
      up + DirectionPlayerTraits<UR,alt(P)>::offset();
    if (! state.pieceAt(ur).isEdge() && 
	state.pieceAt(ur).isEmpty() &&
	!state.hasEffectAt<alt(P)>(ur) &&
	(state.hasPieceOnStand<KNIGHT>(P) ||
	 state.hasEffectByPtype<KNIGHT>(P, ur)))
    {
      if (state.hasPieceOnStand<GOLD>(P))
	return bonus;
      else
	return bonus / 2;
    }

    const Square ul =
      up + DirectionPlayerTraits<UL,alt(P)>::offset();
    if (! state.pieceAt(ul).isEdge() &&
	state.pieceAt(ul).isEmpty() &&
	!state.hasEffectAt<alt(P)>(ul) &&
	(state.hasPieceOnStand<KNIGHT>(P) ||
	 state.hasEffectByPtype<KNIGHT>(P, ul)))
    {
      if (state.hasPieceOnStand<GOLD>(P))
	return bonus;
      else
	return bonus / 2;
    }
  }
  return 0;
}

// P is defense player
template <class Opening>
template <osl::Player P>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateEnterKingBonus(
  const NumEffectState& state) const
{
  const Square king = state.kingSquare<P>();

  if ((P == BLACK && king.y() > 4) ||
      (P == WHITE && king.y() < 6))
  {
    return 0;
  }

  // If not the last rank, check one rank above
  if ((P == BLACK && king.y() >= 2) ||
      (P == WHITE && king.y() <= 8))
  {
    const int y = P == BLACK ? king.y() - 1 : king.y() + 1;
    const int min_x = std::max(1, king.x() - 1);
    const int max_x = std::min(9, king.x() + 1);
    bool found_opening = false;
    for (int x = min_x; x <= max_x; ++x)
    {
      Square pos(x, y);
      Piece piece = state.pieceAt(pos);
      if (piece.isEmpty())
      {
	if (!state.hasEffectAt<alt(P)>(pos))
	  found_opening = true;
	else if (state.countEffect(P, pos) <=
		 state.countEffect(alt(P), pos))
	  return 0;
      }
      else if (piece.owner() == alt(P))
      {
	return 0;
      }
      else if (piece.owner() == P)
      {
	if (state.countEffect(P, pos) <
	    state.countEffect(alt(P), pos))
	  return 0;
      }
      else
	abort();
    }
    if (!found_opening)
      return 0;
  }

  return PtypeEvalTraits<PAWN>::val * 16 * sign(P) * 4;
}

// P is defense player
template <class Opening>
template <osl::Player P>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateMiddleKingBonus(
  const NumEffectState& state) const
{
  const Square king = state.kingSquare<P>();

  if ((P == BLACK && king.y() >= 6 && major_pieces == 4) ||
      (P == WHITE && king.y() <= 4 && major_pieces == 0))
  {
    return PtypeEvalTraits<PAWN>::val * 2 * 16 * sign(P);
  }
  return 0;
}

template<class Opening>
int osl::eval::
ProgressEvalGeneral<Opening>::calculateRookRankBonus(
  const NumEffectState& state) const
{
  int bonus = 0;
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit; ++i)
  {
    const Piece rook = state.pieceOf(i);
    const Player owner = rook.owner();
    const int target_y = owner == BLACK ? 3 : 7;
    const int inbetween_y = owner == BLACK ? 4 : 6;
    if (rook.isOnBoard() && !rook.square().canPromote(owner))
    {
      const Piece rank5 = state.pieceAt(Square(rook.square().x(), 5));
      const Piece rank4 = state.pieceAt(Square(rook.square().x(),
						    inbetween_y));
      const Square rank3_pos(rook.square().x(), target_y);
      if (state.hasEffectByPtype<SILVER>(
	    owner,
	    Square(rook.square().x(),
	    inbetween_y)) &&
	  !rank5.isOnBoardByOwner(alt(owner)) &&
	  !state.pieceAt(rank3_pos).isOnBoardByOwner(owner) &&
	  state.countEffect(alt(owner), 
			    Square(rook.square().x(), target_y)) <= 1 &&
	  state.countEffect(owner,
			    Square(rook.square().x(), inbetween_y)) >=
	  state.countEffect(alt(owner),
			    Square(rook.square().x(), inbetween_y)))
      {
	if (rook.owner() == BLACK)
	  bonus += PtypeEvalTraits<PAWN>::val * 2 * 16;
	else
	  bonus -= PtypeEvalTraits<PAWN>::val * 2 * 16;
      }
      else if (((rank5.isOnBoardByOwner(owner) &&
		 rank5.ptype() == PAWN &&
		 state.hasEffectByPiece(rook, rank5.square())) ||
		(rank4.isOnBoardByOwner(owner) &&
		 rank4.ptype() == PAWN &&
		 state.hasEffectByPiece(rook, rank4.square()))) &&
	       !state.hasEffectAt(alt(owner),
				      rank3_pos) &&
		  state.countEffect(alt(owner),
				    Square(rook.square().x(),
						  inbetween_y)) <= 1)
      {
	if (rook.owner() == BLACK)
	  bonus += PtypeEvalTraits<PAWN>::val * 2 * 16;
	else
	  bonus -= PtypeEvalTraits<PAWN>::val * 2 * 16;
      }
      else if (state.hasEffectByPiece(rook, rank3_pos) &&
	       !state.hasEffectAt(alt(owner), rank3_pos) &&
	       !state.isPawnMaskSet(owner, rook.square().x()))
      {
	if (rook.owner() == BLACK)
	  bonus += PtypeEvalTraits<PAWN>::val * 16;
	else
	  bonus -= PtypeEvalTraits<PAWN>::val * 16;
      }
    }
  }
  return bonus;
}

template <class Opening>
void osl::eval::
ProgressEvalGeneral<Opening>::setValues(const SimpleState& state,
					Progress16 progress16,
					PieceValues& out)
{
  PieceValues opening, endgame;
  const NumEffectState nstate(state);
  const progress_t progress(nstate);
  const defense_t defense_effect(nstate);
  const MinorPieceBonus minor_piece_bonus(state);
  opening_eval_t::setValues(state, opening);
  endgame_eval_t::setValues(state, endgame);
  for (int i=0; i<Piece::SIZE; ++i)
  {
    out[i] = composeValue(opening[i] & (~1), endgame[i] & (~1), progress16,
			  progress.progress16(BLACK),
			  progress.progress16(WHITE),
			  defense_effect.progress16(BLACK),
			  defense_effect.progress16(WHITE),
			  minor_piece_bonus.value(progress16,
						  progress.progress16bonus(BLACK),
						  progress.progress16bonus(WHITE)), 0, 0);
  }
}

template <class Opening>
void osl::eval::
ProgressEvalGeneral<Opening>::setValues(const SimpleState& state, PieceValues& out)
{
  const NumEffectState nstate(state);
  const progress_t progress(nstate);
  setValues(state, progress.progress16(), out);
}

template <class Opening>
osl::eval::ProgressDebugInfo osl::eval::
ProgressEvalGeneral<Opening>::debugInfo(const NumEffectState& state) const
{
  ProgressDebugInfo debug_info;
  debug_info.eval = value();
  debug_info.opening = openingValue();
  debug_info.endgame = endgameValue();
  debug_info.progress = current_progress.progress16().value();
  debug_info.progress_bonus = attackDefenseBonus();
  debug_info.progress_independent_bonus = progress_independent_bonus;
  debug_info.progress_dependent_bonus = progress_dependent_bonus;
  debug_info.minor_piece_bonus = minorPieceValue();

  debug_info.black_danger = current_progress.progress16bonus(BLACK).value();
  debug_info.white_danger = current_progress.progress16bonus(WHITE).value();
  debug_info.black_defense = defense_effect.progress16(BLACK).value();
  debug_info.white_defense = defense_effect.progress16(WHITE).value();

  debug_info.mobility_bonus = calculateMobilityBonus();
  debug_info.two_rook_bonus = calculateAttackRooks(state);
  debug_info.knight_check_bonus = calculateKnightCheck(state);
  debug_info.rook_rank_bonus = calculateRookRankBonus(state);
  debug_info.enter_king_bonus = calculateEnterKingBonus<BLACK>(state) +
    calculateEnterKingBonus<WHITE>(state);
  debug_info.middle_king_bonus = calculateMiddleKingBonus<BLACK>(state) +
    calculateMiddleKingBonus<WHITE>(state);
  debug_info.silver_penalty = calculateSilverPenalty(state);
  debug_info.gold_penalty = calculateGoldPenalty(state);

  debug_info.king8_attack_bonus = calculateAttackBonus(state);
  debug_info.pin_bonus = calculatePinBonus(state);

  debug_info.minor_piece_bonus_info =
    minor_piece_bonus.debugInfo(progress16(),
				progress16bonus(BLACK),
				progress16bonus(WHITE));

  return debug_info;
}

namespace osl
{
  namespace eval
  {
    template class ProgressEvalGeneral<progress_eval_opening_t>;
  }

#ifndef DFPNSTATONE
  template void
  EffectUtil::findThreat<eval::ProgressEval>(const NumEffectState& state,
					     Square position,
					     PtypeO ptypeo,
					     PieceVector& out);
#endif

}

#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
