/* quiescenceGenerator.tcc
 */
#ifndef QUIESCENCEGENERATOR_TCC
#define QUIESCENCEGENERATOR_TCC

#include "osl/search/quiescenceGenerator.h"
#include "osl/search/breakThreatmate.h"
#include "osl/additionalEffect.h"
#include "osl/effect_util/sendOffSquare.h"
#include "osl/effect_util/shadowEffect.h"
#include "osl/effect_util/neighboring8Direct.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/effect_util/pin.h"
#include "osl/move_generator/attackToPinned.h"
#include "osl/move_generator/addEffectWithEffect.h"

template <osl::Player P> 
template <osl::Ptype PTYPE, bool has_dont_capture> 
void osl::search::QuiescenceGenerator<P>::
capture(const NumEffectState& state, MoveVector& moves, Piece dont_capture)
{
  mask_t pieces = state.effectedMask(P).template selectBit<PTYPE>()
    & state.piecesOnBoard(alt(P)).getMask(PtypeFuns<PTYPE>::indexNum);
  
  if (has_dont_capture && dont_capture.isPiece() && (dont_capture.ptype() == PTYPE))
  {
    while (pieces.any()) {
      const Piece target = state.pieceOf(pieces.takeOneBit()+PtypeFuns<PTYPE>::indexNum*32);
      if (target != dont_capture)
	capture(state, target.square(), moves);
    }
  }
  else
  {
    while (pieces.any()) {
      const Piece target = state.pieceOf(pieces.takeOneBit()+PtypeFuns<PTYPE>::indexNum*32);
      capture(state, target.square(), moves);
    }
  }
  if (((PTYPE == KNIGHT) || (PTYPE == BISHOP))
      && state.hasPieceOnStand<LANCE>(P))
  {
    // 歩で駒を取る手がある場合は，追加利きをつける

    // NOTE: MoveVector はpush_backしてもiteratorをinvalidate
    // しないという仕様に依存
    MoveVector::const_iterator original_end = moves.end();
    for (MoveVector::const_iterator p=moves.begin(); p!=original_end; ++p)
    {
      const Square from = p->from();
      if ((p->oldPtype() == PAWN)
	  && (state.hasEffectAt<alt(P)>(p->to())))
      {
	move_generator::AdditionalLance<P>::generate(state, from, moves);
      }
    }
  }
}

template <osl::Player P> inline
void osl::search::QuiescenceGenerator<P>::
attackMajorPieceFirstSelection(const NumEffectState& state,
			       PieceMask pins, const MoveVector& all_moves,
			       MoveVector& moves,
			       MoveVector& expensive_drops)
{
  for (Move m: all_moves)
  {
    const Square to = m.to();
    {
      const int defense = state.countEffect(alt(P),to, pins);
      int offense = state.countEffect(P,to) + (m.isDrop() ? 1 : 0);
      if (defense >= offense)
	offense += AdditionalEffect::count2(state, to, P);
      if (m.ptype() != PAWN)
      {
	if (defense > offense)
	  continue;
      }
      else
      {
	if (defense && (offense==1))
	{
	  if (! (state.hasEffectByPtype<ROOK>(alt(P),to)
		 && state.hasEffectByPtype<BISHOP>(alt(P),to)))
	    continue;
	}
      }
      const Ptype ptype = m.ptype();
      if ((defense >= offense)
	  && (unpromote(ptype) != PAWN))
	continue;
      if (ptype != PAWN)
      {
	const Square front_position
	  = to + DirectionPlayerTraits<U,P>::offset();
	const Piece front_piece = state.pieceAt(front_position);
	if (front_piece.ptypeO() == newPtypeO(alt(P),PAWN))
	  continue;
      }
      if (m.isDrop() && (m.ptype() == PAWN)
	  && to.canPromote<P>())
      {
	// 垂れ歩
	const Square back_position
	  = to + DirectionPlayerTraits<D,P>::offset();
	if (state.pieceOnBoard(back_position).isEmpty())
	  moves.push_back(Move(back_position, PAWN, P));
      }
      const int ptype_value = eval::Ptype_Eval_Table.value(ptype);
      const bool is_large_piece
	= (ptype_value > eval::PtypeEvalTraits<BISHOP>::val);
      if ((m.isDrop() 
	   && (ptype_value > eval::PtypeEvalTraits<KNIGHT>::val))
	  || (is_large_piece && defense))
	expensive_drops.push_back(m);
      else
	moves.push_back(m);
    }
  }
}

template <osl::Player P> inline
void osl::search::QuiescenceGenerator<P>::
attackMajorPieceSecondSelection(bool target_has_support,
				const MoveVector& src,
				MoveVector& out)
{
  for (Move m: src)
  {
    const Ptype ptype = m.ptype();
    if (target_has_support
	&& (eval::Ptype_Eval_Table.value(ptype) 
	    >= eval::PtypeEvalTraits<GOLD>::val))
      continue;

    out.push_back(m);
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
attackMajorPieceZerothSelection(const NumEffectState& state,
				const MoveVector& src,
				Square target,
				MoveVector& open_out,
				MoveVector& out)
{
  for (Move m: src)
  {
    assert(!ShouldPromoteCut::canIgnoreAndNotDrop<P>(m));
    const bool is_open
      = (! m.isDrop())
      && state.hasEffectAt<P>(m.from())
      && (state.hasEffectByPtype<LANCE>(P, m.from())
	  || state.hasEffectByPtype<BISHOP>(P, m.from())
	  || state.hasEffectByPtype<ROOK>(P, m.from()))
      && ! state.hasEffectIf(m.ptypeO(), m.to(), target);
    // 取る手は重複
    const bool may_overlap = m.isCaptureOrPromotion();
    if (is_open) 
    {
      if (! may_overlap		// 取り返される場合はたぶん重複しない
	  || state.hasEffectAt<alt(P)>(m.to()))
	open_out.push_back(m);
    }
    else 
    {
      if (! may_overlap)
	out.push_back(m);
    }
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
attackMajorPiece(const NumEffectState& state, PieceMask pins,
		 MoveVector& moves) 
{
  using namespace move_action;
  MoveVector work;
  MoveVector unsupported, supported;
  assert(PtypeTraits<BISHOP>::indexLimit == PtypeTraits<ROOK>::indexMin);
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit; i++)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoardByOwner<alt(P)>())
    {
      const Square target = p.square();
      assert(isMajor(p.ptype()));
      const bool unstable_rook = (p.ptype() == ROOK)
	&& (state.pieceAt(target + DirectionPlayerTraits<U, alt(P)>::offset())
	    != Piece::EMPTY());	// 前に進めない
      work.clear();
      if (state.hasEffectAt(alt(P), target) && (! unstable_rook))
      {
	move_generator::GenerateAddEffectWithEffect::generate<false>
	  (P, state, target, work);
	attackMajorPieceZerothSelection(state, work, target, moves, supported);	
      }
      else if (unstable_rook || (! state.hasEffectAt(P, target)))
      {
	// ただで取れる時には利きをつけない
	move_generator::GenerateAddEffectWithEffect::generate<false>
	  (P, state, target, work);
	attackMajorPieceZerothSelection(state, work, target, moves, unsupported);
      }
    }
  }

  // first pass
  MoveVector drops_supported, drops_unsupported;
  attackMajorPieceFirstSelection(state, pins, unsupported, moves, 
				 drops_unsupported);
  attackMajorPieceFirstSelection(state, pins, supported, moves, 
				 drops_supported);
  // second pass
  if (moves.size() > 5)
    return;
  attackMajorPieceSecondSelection(false, drops_unsupported, moves);
  if (moves.size() > 5)
    return;
  attackMajorPieceSecondSelection(true, drops_supported, moves);
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
escapeNormalPiece(const NumEffectState& state, 
		  Piece escape, MoveVector& moves, bool add_support_only)
{
  assert(escape.ptype() != KING);
  using namespace move_action;
  MoveVector all_moves;
  const Piece attack_piece
    = state.findCheapAttack(alt(P), escape.square());
  const int attack_ptype_value 
    = eval::Ptype_Eval_Table.value(unpromote(attack_piece.ptype()));
  const Square escape_from = escape.square();
  if (! add_support_only)
  {
    GenerateEscape<P>::generateCheap(state, escape, all_moves);
    for (Move m: all_moves)
    {
      const Square to = m.to();
      if (m.isDrop())
      {
	if (! state.hasEffectAt<P>(to))
	  continue;	// 中合
#ifdef QSEARCH_EXPENSIVE_BLOCK
	if (eval::Ptype_Eval_Table.value(m.ptype()) > attack_ptype_value)
	  continue;
#else
	if (m.ptype() != PAWN)
	  continue;
#endif
	moves.push_back(m);
      }
      else
      {
	if (m.from() != escape_from)
	{
	  // 移動合
	  if (! state.hasMultipleEffectAt(P, to))
	    continue;
	  if (eval::Ptype_Eval_Table.value(m.ptype()) > attack_ptype_value)
	    continue;
	}
	// 取る手は重複
	// TODO: KnightCaptureDepth
	assert(! ShouldPromoteCut::canIgnoreMove<P>(m));
	if (! m.isCaptureOrPromotion())
	{
	  moves.push_back(m);
	}
      }
    }
  }
  // 紐をつける
  if (unpromote(attack_piece.ptype()) == PAWN)
    return;
  if ((eval::Ptype_Eval_Table.value(escape.ptype()) - attack_ptype_value)
      >= (eval::PtypeEvalTraits<BISHOP>::val - eval::PtypeEvalTraits<KNIGHT>::val))
    return;
  if (state.hasEffectAt<P>(escape_from)
      || (state.countEffect(alt(P), escape_from) != 1))
    return;
  all_moves.clear();
  move_generator::GenerateAddEffectWithEffect::generate<false>
    (P, state, escape_from, all_moves);
  const size_t escape_moves = moves.size();
  MoveVector not_drop;
  for (Move m: all_moves)
  {
    const Square to = m.to();
    if (m.isDrop())
    {
      if (state.hasEffectAt<alt(P)>(to)
	  && (! state.hasEffectAt<P>(to)))
	continue;
      // 打つ手は歩か香車だけ
      if ((m.ptype() != PAWN) && (m.ptype() != LANCE))
	continue;
      moves.push_back(m);
    }
    else
    {
      // 移動
      const int defense = state.countEffect(P,to);
      const int offense = state.countEffect(alt(P),to);
      if (offense >= defense)
	continue;
      // 取る手は重複
      // TODO: KnightCaptureDepth
      assert(! ShouldPromoteCut::canIgnoreMove<P>(m));
      if (! m.isCaptureOrPromotion())
      {
	moves.push_back(m);
      }
    }
  }
  // 合わせ角
  if (state.hasEffectByPtype<BISHOP>(alt(P), escape_from)
      && state.hasPieceOnStand<BISHOP>(P))
  {
    const Piece bishop = state.findAttackAt<BISHOP>(alt(P), escape_from);
    assert(unpromote(bishop.ptype()) == BISHOP);
    {
      Square p = bishop.square();
      const Offset offset = Board_Table.getShortOffset(Offset32(p,escape_from));
      p += offset;
      while (state.pieceAt(p).isEmpty())
      {
	if (state.hasEffectAt<P>(p))
	{
	  moves.push_back(Move(p, BISHOP, P));
	  break;
	}
	p += offset;
      }
    }
  }
  
  if (escape_moves == moves.size()) // drop not found
    moves.push_back(not_drop.begin(), not_drop.end());
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
escapeAll(const NumEffectState& state, MoveVector& moves) 
{
  // 逃げるのは大駒か価値の一番高い駒のみ
  const PieceMask pieces = state.effectedMask(alt(P)) & state.piecesOnBoard(P);
  bool found_attacked_piece = false;
  mask_t m = pieces.selectBit<ROOK>();
  while (m.any())
  {
    const Piece p = state.pieceOf(m.takeOneBit()+PtypeFuns<ROOK>::indexNum*32);
    assert(p.isOnBoardByOwner<P>()
	   && state.hasEffectAt<alt(P)>(p.square()));
    found_attacked_piece = true;
    escapeNormalPiece(state, p, moves);
  }
  m = pieces.selectBit<BISHOP>();
  while (m.any())
  {
    const Piece p = state.pieceOf(m.takeOneBit()+PtypeFuns<BISHOP>::indexNum*32);
    assert(p.isOnBoardByOwner<P>()
	   && state.hasEffectAt<alt(P)>(p.square()));
    found_attacked_piece = true;
    escapeNormalPiece(state, p, moves);
  }

  if (found_attacked_piece)
    goto finish;
  m = pieces.selectBit<GOLD>();
  while (m.any())
  {
    const Piece p = state.pieceOf(m.takeOneBit()+PtypeFuns<GOLD>::indexNum*32);
    assert(p.isOnBoardByOwner<P>()
	   && state.hasEffectAt<alt(P)>(p.square()));
    escapeNormalPiece(state, p, moves);
    goto finish;
  }
  m = pieces.selectBit<SILVER>();
  while (m.any())
  {
    const Piece p = state.pieceOf(m.takeOneBit()+PtypeFuns<SILVER>::indexNum*32);
    assert(p.isOnBoardByOwner<P>()
	   && state.hasEffectAt<alt(P)>(p.square()));
    escapeNormalPiece(state, p, moves);
    goto finish;
  }
  m = pieces.selectBit<KNIGHT>();
  while (m.any())
  {
    // 成桂は逃げない
    const Piece p = state.pieceOf(m.takeOneBit()+PtypeFuns<KNIGHT>::indexNum*32);
    assert(p.isOnBoardByOwner<P>()
	   && state.hasEffectAt<alt(P)>(p.square()));
    escapeNormalPiece(state, p, moves, p.ptype()==PKNIGHT);
    goto finish;
  }
  m = pieces.selectBit<LANCE>();
  while (m.any())
  {
    // 香，成香は逃げない
    const Piece p = state.pieceOf(m.takeOneBit()+PtypeFuns<LANCE>::indexNum*32);
    assert(p.isOnBoardByOwner<P>()
	   && state.hasEffectAt<alt(P)>(p.square()));
    escapeNormalPiece(state, p, moves, true);
    goto finish;
  }
  m = pieces.selectBit<PAWN>();
  while (m.any())
  {
    // 歩，と金は逃げない
    const Piece p = state.pieceOf(m.takeOneBit()+PtypeFuns<PAWN>::indexNum*32);
    assert(p.isOnBoardByOwner<P>()
	   && state.hasEffectAt<alt(P)>(p.square()));
    escapeNormalPiece(state, p, moves, true);
    goto finish;
  }
finish:
  return;
}

template <osl::Player P>
template <class EvalT>
void osl::search::QuiescenceGenerator<P>::
escapeFromLastMoveOtherThanPawn(const NumEffectState& state, Move last_move, 
				MoveVector& moves)
{
  if (! last_move.isNormal())
    return;
  assert(last_move.ptype() != PAWN);

  PieceVector targets;
  const Square from = last_move.to();
  EffectUtil::findThreat<EvalT>(state, from, last_move.ptypeO(), targets);
  if (targets.empty())
    return;
  assert(targets[0].ptype() != KING);

  escapeNormalPiece(state, targets[0], moves, 
		    (last_move.ptype() != PAWN)
		    && (eval::Ptype_Eval_Table.value(targets[0].ptype())
			<= eval::Ptype_Eval_Table.value(KNIGHT)));
  if (targets.size() > 1)
    escapeNormalPiece(state, targets[1], moves, 
		      (last_move.ptype() != PAWN)
		      && (eval::Ptype_Eval_Table.value(targets[1].ptype())
			  <= eval::Ptype_Eval_Table.value(KNIGHT)));
}

template <osl::Player P>
bool osl::search::QuiescenceGenerator<P>::
escapeByMoveOnly(const NumEffectState& state, Piece piece, MoveVector& moves)
{
  assert(piece.isOnBoardByOwner<P>());

  MoveVector all_moves;
  GeneratePieceOnBoard::generate(state.turn(),state,piece,all_moves);
   
  const Player Opponent = alt(P);
  const Square from = piece.square();
  
  const bool consider_shadowing
    = state.hasEffectByPtype<LANCE>(Opponent, from)
    || state.hasEffectByPtype<ROOK>(Opponent, from)
    || state.hasEffectByPtype<BISHOP>(Opponent, from);
  const bool is_major = isMajor(piece.ptype());
  const bool chase_danger
    = (! state.hasEffectAt(P, from)
       && (state.hasMultipleEffectAt(alt(P), from)
	   || AdditionalEffect::hasEffect(state, from, alt(P))));
  for (Move m: all_moves)
  {
    assert(m.from() == piece.square());
    if (m.isCapture())
      continue;
    const Square to = m.to();
    const bool safe_position
      = (consider_shadowing
	 ? (! state.hasEffectByWithRemove<Opponent>(to, from))
	 : (! state.hasEffectAt<Opponent>(to)));
    if (safe_position)
    {
      if (! is_major || ! chase_danger)
	return true;
      if (! to.canPromote<Opponent>())
	return true; // 自陣以外も対象にする場合は QuiescenceSearch も調整
      if ((to - from) != Board_Table.getShortOffset(Offset32(to, from)))
	return true;		// 二歩以上逃げられれば良し
    }
    moves.push_back(m);
  }
  return false;
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
check(const NumEffectState& state, PieceMask pins, MoveVector& moves,
      bool no_liberty)
{
  Square8 sendoffs;
  const Square king_position = state.kingSquare(alt(P));
  effect_util::SendOffSquare::find<P>(state, king_position, sendoffs);
  check(state, pins, no_liberty, sendoffs, moves);
}


template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
check(const NumEffectState& state, PieceMask pins, bool no_liberty, 
      const Square8& sendoffs, MoveVector& moves)
{
  using namespace move_action;
  MoveVector all_moves;
  const Square king_position = state.kingSquare(alt(P));
  move_generator::GenerateAddEffectWithEffect::generate<true>
    (P, state, king_position, all_moves);

  MoveVector merginal_moves;
  for (Move m: all_moves)
  {
    // 取る手，成る手は重複
    assert(! ShouldPromoteCut::canIgnoreAndNotDrop<P>(m));
    if (m.isPromotion())
      continue;
    const Square to = m.to();
    const Ptype captured = m.capturePtype();
    
    if ((captured != PTYPE_EMPTY)
	&& (eval::Ptype_Eval_Table.value(captured)
	    >= eval::PtypeEvalTraits<KNIGHT>::val))
      continue;
    const bool is_open
      = (! m.isDrop())
      && state.hasEffectAt<P>(m.from())
      && (state.hasEffectByPtype<LANCE>(P, m.from())
	  || state.hasEffectByPtype<BISHOP>(P, m.from())
	  || state.hasEffectByPtype<ROOK>(P, m.from()))
      && ! state.hasEffectIf(m.ptypeO(), m.to(), king_position);
    if (! is_open)
    {
      if ((m.ptype() != PAWN) && (m.ptype() != KNIGHT))
      {
	// 歩の前は読まない
	const Square front_position
	  = to + DirectionPlayerTraits<U,P>::offset();
	const Piece front_piece = state.pieceAt(front_position);
	if (front_piece.ptypeO() == newPtypeO(alt(P),PAWN))
	  continue;
      }
      if (! sendoffs.isMember(to))
      {
	const int defense = state.countEffect(alt(P),to, pins);
	int offense = state.countEffect(P,to) + no_liberty;
	const bool may_effective = offense && (! sendoffs.empty());
	if (m.isDrop())
	  ++offense;
	if (defense >= offense)
	  offense += AdditionalEffect::count2(state, to, P);
	if (defense >= offense)
	  offense += ShadowEffect::count2(state, to, P);
	if (m.ptype() == KNIGHT)
	{
	  const Square front_position
	    = to + DirectionPlayerTraits<U,P>::offset();
	  if (state.hasEffectAt<P>(front_position))
	    offense+=1;		// pin もしくは空いた場所に何か打てるかも
	}
	if (defense > offense)
	{
	  const bool side_attack
	    = (king_position.x() == 1 || king_position.x() == 9)
	    && m.ptype() == PAWN;
	  if (! side_attack)
	    continue;		// 乱暴な王手は指さない
	}
	else if (defense == offense)
	{
	  if ((unpromote(m.ptype()) == PAWN)
	      || state.hasEffectByPtype<PAWN>(P, to)
	      || state.hasEffectByPtype<LANCE>(P, to))
	    moves.push_back(m);
	  else if (may_effective)
	    merginal_moves.push_back(m);
	  continue;
	}
      }
    }
    if (m.isDrop() && (m.ptype() == PAWN)
	&& to.canPromote<P>())
    {
      // 垂れ歩
      const Square back_position
	= to + DirectionPlayerTraits<D,P>::offset();
      if (state.pieceOnBoard(back_position).isEmpty())
	moves.push_back(Move(back_position, PAWN, P));
    }
    moves.push_back(m);
  }
  if (moves.size() < 3)
    moves.push_back(merginal_moves.begin(), merginal_moves.end());
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
promote(const NumEffectState& state, PieceMask pins, MoveVector& moves)
{
  using namespace move_action;
  MoveVector all_moves;
  move_generator::Promote<P>::generate(state, all_moves);

  for (Move m: all_moves)
  {
    const Square to = m.to();
    const int defense = state.countEffect(alt(P),to, pins);
    int offense = state.countEffect(P,to);
    if (defense >= offense)
      offense += AdditionalEffect::count2(state, to, P);
    if (m.ptype() == PPAWN
	&& defense && offense + 1 >= defense)
    {
      // 歩が成れる時は追加利きをつけてみる?
      if (m.ptype() == PPAWN)
      {
	// 飛車が横に動く
	for (int i = PtypeTraits<ROOK>::indexMin;
	     i < PtypeTraits<ROOK>::indexLimit; ++i)
	{
	  const Piece rook = state.pieceOf(i);
	  if (! rook.isOnBoardByOwner<P>() || rook.ptype() == PROOK
	      || rook.square().template canPromote<P>())
	    continue;
	  const Square mid(m.from().x(), rook.square().y());
	  if (state.pieceOnBoard(mid).isEmpty()
	      && state.isEmptyBetween(m.from(), mid)
	      && state.isEmptyBetween(rook.square(), mid))
	  {
	    const Move m(rook.square(), mid, ROOK, PTYPE_EMPTY, false, P);
	    moves.push_back(m);
	  }
	}
	if (state.hasPieceOnStand<LANCE>(P))
	  move_generator::AdditionalLance<P>::generate(state, m.from(), moves);
	// あとは桂馬くらい?
      }
    }
    if (defense > offense)
    {
      continue;     // 成りではあまり乱暴な手は指さない
    }
    if ((defense == offense)
	&& (m.ptype() != PPAWN))
      continue;

    moves.push_back(m);
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
attackKing8(const NumEffectState& state, PieceMask pins, MoveVector& moves) 
{
  using namespace move_action;
  MoveVector all_moves;

  MoveVector not8;
  MoveVector not_drop;
  MoveVector major_drop;
  move_generator::AddEffect8<P>::generate(state, all_moves);

  const Square king_position = state.kingSquare(alt(P));
  const int king_x = king_position.x();
  for (Move m: all_moves)
  {
    // 取る手，成る手は重複
    assert(! ShouldPromoteCut::canIgnoreAndNotDrop<P>(m));
    assert(! m.isPromotion());
    const Square to = m.to();
#ifndef NDEBUG
    const Ptype captured = m.capturePtype();
    assert(captured==PTYPE_EMPTY);
#endif    

    const Ptype ptype = m.ptype();

    const int defense = state.countEffect(alt(P),to,pins);
    int offense = state.countEffect(P,to) + (m.isDrop() ? 1 : 0);
    if (defense >= offense)
      offense += AdditionalEffect::count2(state, to, P);
    if (ptype == PAWN)
    {
      const Square head = to + DirectionPlayerTraits<U,P>::offset();
      if (state.hasEffectAt<P>(head))
	++offense;
    }
    if (defense > offense)
      continue;
    if (defense == offense)
    {
      if (ptype != PAWN)
	continue;
    }
    if (isMajor(ptype))
    {
      if (defense == 0)
      {
	// 大駒は利きのあるところにはいかない TODO: kingのみ許可?
	if (isPromoted(ptype) && to.x() == king_x 
	    && abs(to.y() - king_position.y()) <= 2)
	  moves.push_back(m);
	else if (m.isDrop())
	  major_drop.push_back(m);
	else
	  not_drop.push_back(m);
      }
      continue;
    } 
    if (Ptype_Table.getMoveMask(m.ptype()) == PtypeTraits<GOLD>::moveMask)
    {
      const int y = to.y();
      if (((P == BLACK) && (y == 1))
	  || ((P == WHITE) && (y == 9)))
	continue;		// 1段目の金類
    }
    // 歩の前は読まない TODO: 桂馬だけ許す?
    if (ptype != PAWN)
    {
      if (state.hasEffectByPtype<PAWN>(alt(P), to))
	continue;
      if (state.hasEffectByPtype<LANCE>(alt(P), to))
      {
	not_drop.push_back(m);
	continue;
      }
    }
    if (! king_position.isNeighboring8(to))
      not8.push_back(m);
    else if (! m.isDrop())
      not_drop.push_back(m);
    else
      moves.push_back(m);
  }
  const size_t minimum_moves
    = (king_position.squareForBlack<P>().y() == 1) ? 3 : 2;
  for (MoveVector::const_iterator p=not8.begin();
       (p!=not8.end()) && (moves.size() <= minimum_moves); ++p)
  {
    moves.push_back(*p);
  }
  for (MoveVector::const_iterator p=not_drop.begin();
       (p!=not_drop.end()) && (moves.size() <= minimum_moves); ++p)
  {
    moves.push_back(*p);
  }
  for (MoveVector::const_iterator p=major_drop.begin();
       (p!=major_drop.end()) && (moves.size() <= minimum_moves); ++p)
  {
    moves.push_back(*p);
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
attackToPinned(const NumEffectState& state, PieceMask pins, MoveVector& moves)
{
  using namespace move_action;
  MoveVector all_moves;
  move_generator::AttackToPinned<P>::generate(state, all_moves);
  
  for (Move m: all_moves)
  {
    // 取る手，成る手は重複
    assert(!ShouldPromoteCut::canIgnoreAndNotDrop<P>(m));
    if (m.isPromotion())
      continue;
    const Square to = m.to();
    const Ptype captured = m.capturePtype();
    
    if (captured != PTYPE_EMPTY)
      continue;

    const Ptype ptype = m.ptype();

    const int defense = state.countEffect(alt(P),to,pins);
    int offense = state.countEffect(P,to) + (m.isDrop() ? 1 : 0);
    if (defense >= offense)
      offense += AdditionalEffect::count2(state, to, P);
    if (ptype == PAWN)
    {
      const Square head = to + DirectionPlayerTraits<U,P>::offset();
      if (state.hasEffectAt<P>(head))
	++offense;
    }
    if (defense > offense)
      continue;
    if (defense == offense)
    {
      if (ptype != PAWN)
	continue;
    }
    // 歩の前は読まない TODO: 桂馬だけ許す?
    if (ptype != PAWN)
    {
      const Square front_position
	= to + DirectionPlayerTraits<U,P>::offset();
      const Piece front_piece = state.pieceAt(front_position);
      if (front_piece.ptypeO() == newPtypeO(alt(P),PAWN))
	continue;
    }
    moves.push_back(m);
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
utilizePromoted(const NumEffectState& state, Piece target,
		MoveVector& moves)
{
  if (! target.isPromoted())
    return;
  if (/*isMajor*/
      target.ptype() == ROOK
      || target.ptype() == BISHOP)
    return;

  MoveVector all_moves;
  move_generator::GeneratePieceOnBoard::generate(P,state, target, all_moves);

  MoveVector others;
  const Player Opponent = alt(P);
  bool has_good_capture = false;
  bool may_have_additional = false; // 飛車の前とか
  for (Move m: all_moves)
  {
    assert(m.from() == target.square());
    if (m.isCapture())
    {
      if (m.capturePtype() != PAWN)
	has_good_capture =true;
      continue;
    }
    
    const Square to = m.to();
    int offense = state.countEffect(P, to);
    const int defense = state.countEffect(Opponent, to);
    const int additional = AdditionalEffect::count2(state, to, P);
    if (defense >= offense)
    {
      offense += additional;
      may_have_additional |= (additional > 0);
    }
    if (defense >= offense)
    {
      others.push_back(m);
      continue;
    }
    moves.push_back(m);
  }
  if ((! has_good_capture) && may_have_additional)
    moves.push_back(others.begin(), others.end());
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
breakThreatmate(const NumEffectState& state, 
		Move threatmate, PieceMask pins, MoveVector& moves)
{
  MoveVector all_moves, major_piece, major_sacrifice;
  assert(threatmate.isNormal());
  const Square target = threatmate.to();

  // defense by add effect
  move_generator::GenerateAddEffectWithEffect::generate<false>
    (P, state, target, all_moves);

  for (Move m: all_moves)
  {
    const Ptype ptype = m.ptype();
    if (ptype == KING)
      continue;			// KING_WALK will generate
    if (! m.isDrop())
    {
      assert(!ShouldPromoteCut::canIgnoreAndNotDrop<P>(m));
      if (m.isPromotion()) 
	continue;
      if (isMajor(ptype)
	  && state.hasEffectByPiece(state.pieceOnBoard(m.from()), target))
	continue;			// already have effect
    }
    const Square to = m.to();
    const int me = state.countEffect(P, to) + (m.isDrop() ? 1 : 0);
    const int op = state.countEffect(alt(P), to, pins);
    if ((me >= 2) || (op == 0))
    {
      if (isMajor(ptype))
      {
	if (op)
	  major_sacrifice.push_back(m);
	else 
	  major_piece.push_back(m);
      }
      else			// ! major
      {
	if (m.isDrop() && (ptype == GOLD || ptype == SILVER)
	    && (to.x() == 1 || to.x() == 9))
	  major_piece.push_back(m);
	else
	  moves.push_back(m);
      }
    }
  }
  all_moves.clear();
  
  if (threatmate.isDrop())
  {
    // 場所を指定して、Drop を作る方法は?
    if (state.hasPieceOnStand<PAWN>(P)
	&& ! state.isPawnMaskSet(P, target.x())
	&& PtypePlayerTraits<PAWN,P>::canDropTo(target))
      moves.push_back(Move(target, PAWN, P));
    else if (state.hasPieceOnStand<LANCE>(P)
	     && PtypePlayerTraits<LANCE,P>::canDropTo(target))
      moves.push_back(Move(target, LANCE, P));
    else if (state.hasPieceOnStand<KNIGHT>(P)
	     && PtypePlayerTraits<KNIGHT,P>::canDropTo(target))
      moves.push_back(Move(target, KNIGHT, P));
    else if (state.hasPieceOnStand<SILVER>(P))
      moves.push_back(Move(target, SILVER, P));
    else if (state.hasPieceOnStand<GOLD>(P))
      moves.push_back(Move(target, GOLD, P));
  }
  // 長い利きのblock他
  BreakThreatmate::findBlockLong(state, threatmate, all_moves);
  if (! all_moves.empty()) 
  {
    Ptype cheapest = PTYPE_EMPTY;
    if (state.hasPieceOnStand<PAWN>(P)) cheapest = PAWN;
    else if (state.hasPieceOnStand<LANCE>(P)) cheapest = LANCE;
    else if (state.hasPieceOnStand<KNIGHT>(P)) cheapest = KNIGHT;
    for (Move m: all_moves) {
      const int d = state.countEffect(P, m.to()) + m.isDrop();
      const int a = state.countEffect(alt(P), m.to());
      if (a >= d && ! (d >= 2 && m.ptype() == PAWN))
	continue;
      if (m.ptype() != cheapest && cheapest != PTYPE_EMPTY && Ptype_Table.canDropTo(P, cheapest, m.to()))
	continue;
      moves.push_back(m);
      break;
    }
    all_moves.clear();
  }
  
  
  const Square king_position = state.kingSquare(P);
  // make a space for king
  {
    for (int i=SHORT8_DIRECTION_MIN; i<=SHORT8_DIRECTION_MAX; ++i)
    {
      const Piece p = state.pieceAt(Board_Table.nextSquare
				       (P,king_position,Direction(i)));
      if (! (p.isPiece() && p.owner() == P))
	continue;
      if (state.hasEffectAt(alt(P), p.square()))
	continue;
      move_generator::GeneratePieceOnBoard::generate(P,state, p, all_moves);
    }
    for (Move m: all_moves)
    {
      assert(! m.isDrop());
      assert(!ShouldPromoteCut::canIgnoreAndNotDrop<P>(m));
      if (m.isPromotion())
	continue;
      const Ptype captured = m.capturePtype();
      if ((captured != PTYPE_EMPTY)
	  && (eval::Ptype_Eval_Table.value(captured)
	      >= eval::PtypeEvalTraits<KNIGHT>::val))
	continue;
      moves.push_back(m);
    }
  }
  // defence by add effect: unusual moves
  const size_t minimum_moves = 8;
  for (MoveVector::const_iterator p=major_piece.begin(); 
       p!=major_piece.end() && moves.size() < minimum_moves; ++p)
  {
    moves.push_back(*p);
  }
  for (MoveVector::const_iterator p=major_sacrifice.begin(); 
       p!=major_sacrifice.end() && moves.size() < minimum_moves; ++p)
  {
    moves.push_back(*p);
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
attackGoldWithPawn(const NumEffectState& state, PieceMask pins, 
		   MoveVector& moves) 
{
  using namespace move_action;

  const bool has_pawn   = state.hasPieceOnStand<PAWN>(P);
  const bool has_lance  = state.hasPieceOnStand<LANCE>(P);
  const bool has_knight = state.hasPieceOnStand<KNIGHT>(P);
  const bool has_silver = state.hasPieceOnStand<SILVER>(P);
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (! p.isOnBoardByOwner<alt(P)>())
      continue;
    const Square head = p.square() + DirectionPlayerTraits<D,P>::offset();
    if (state.pieceAt(head).isEmpty())
    {
      // TODO: ただの歩は利きがはずれる時に許す
      const int defense = state.countEffect(alt(P), head, pins);
      int attack = state.countEffect(P, head)
	+ state.hasEffectByPtype<BISHOP>(P, p.square());
      if (defense >= attack)
	attack += AdditionalEffect::count2(state, head, P);

      // move
      if (! head.canPromote<P>()) // 成りはすでに生成されているはず
      {
	const Square origin = head + DirectionPlayerTraits<D,P>::offset();
	if (defense <= attack
	    || state.hasEffectByPtype<BISHOP>(P, origin)) // open attack
	{
	  const Piece candidate = state.pieceAt(origin);
	  if (candidate.ptype() == PAWN && candidate.owner() == P)
	  {
	    const Move move(origin, head, PAWN, PTYPE_EMPTY, false, P);
	    moves.push_back(move);
	    ++attack;		// 歩の利きがある
	  }
	}
      }      
      // drop
      if (defense <= attack)
      {
	if (has_pawn && !state.template isPawnMaskSet<P>(head.x()))
	{
	  const Move move(head, PAWN, P);
	  moves.push_back(move);
	}
	if (has_lance)
	{
	  const Move move(head, LANCE, P);
	  moves.push_back(move);
	}
	if (has_silver)
	{
	  const Move move(head, SILVER, P);
	  moves.push_back(move);
	}
      }
      const bool generate_long_lance = has_lance 
	&& (! state.hasPieceOnStand<PAWN>(alt(P))
	    || state.template isPawnMaskSet<alt(P)>(head.x()));
      if (generate_long_lance) 
      {
	for (Square to=head + DirectionPlayerTraits<D,P>::offset(); 
	     state.pieceAt(to).isEmpty(); 
	     to+=DirectionPlayerTraits<D,P>::offset())
	{
	  const int defense = state.countEffect(alt(P), to, pins);
	  const int attack = state.countEffect(P, to);
	  if (defense > attack)
	    continue;
	  const Move move(to, LANCE, P);
	  moves.push_back(move);
	}
      }
    }
    // knight
    if (! state.pieceAt(head).isEdge())
    {
      const Square knight_l = head+DirectionPlayerTraits<DL,P>::offset();
      attackWithKnight(state, pins, knight_l, has_knight, moves);
      const Square knight_r = head+DirectionPlayerTraits<DR,P>::offset();
      attackWithKnight(state, pins, knight_r, has_knight, moves);
    }
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
attackWithKnight(const NumEffectState& state, PieceMask pins, 
		     Square attack_from, 
		     bool has_knight, MoveVector& moves) 
{
  if (state.pieceAt(attack_from) != Piece::EMPTY())
    return;
  
  const int defense = state.countEffect(alt(P), attack_from, pins);
  int attack = state.countEffect(P, attack_from);

  if (defense == 1 
      && (attack == 1 || (has_knight && attack == 0)))
  {
    const Piece gold = state.findAttackAt<GOLD>(alt(P), attack_from);
    if (gold.ptype() == GOLD)
    {
      // 取ると敵の金の守りが外れる
      const Square guarded
	= gold.square()+DirectionPlayerTraits<U,P>::offset();
      if (state.pieceAt(guarded).isOnBoardByOwner(alt(P))
	  && (state.countEffect(alt(P), guarded) 
	      <= state.countEffect(P, guarded)))
	++attack;
    }
    // 取ると角道が開く
    const Square head = attack_from + DirectionPlayerTraits<U,P>::offset();
    const Piece head_piece = state.pieceOnBoard(head);
    if (head_piece.isOnBoardByOwner<alt(P)>()) {
      if (state.hasEffectByPiece(head_piece, attack_from)
	  && state.hasEffectByPtype<BISHOP>(P, head))
	++attack;
    }
  }
  
  if (defense > attack)
    return;
  
  if (has_knight)
  {
    const Move drop(attack_from, KNIGHT, P);
    moves.push_back(drop);
  }

  if (defense < attack)
  {
    mask_t mask=state.effectSetAt(attack_from).getMask(PtypeFuns<KNIGHT>::indexNum);
    mask &= mask_t::makeDirect(PtypeFuns<KNIGHT>::indexMask);
    mask &= state.piecesOnBoard(P).getMask(PtypeFuns<KNIGHT>::indexNum);
    static_assert(PtypeTraits<KNIGHT>::indexLimit < 32, "");
    while (mask.any())
    {
      const int n = mask.takeOneBit();
      const Piece knight = state.pieceOf(n);
      if (knight.isPromoted())
	continue;
      
      const Move move(knight.square(), attack_from, KNIGHT, PTYPE_EMPTY, false, P);
      assert(state.isAlmostValidMove<false>(move));
      moves.push_back(move);
    }
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
attackKnightWithPawn(const NumEffectState& state, PieceMask pins,
		     MoveVector& moves) 
{
  using namespace move_action;

  const bool has_pawn = state.hasPieceOnStand<PAWN>(P);
  for (int i = PtypeTraits<KNIGHT>::indexMin;
       i < PtypeTraits<KNIGHT>::indexLimit; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard() && p.ptype() == KNIGHT && p.owner() == alt(P))
    {
      const Square head = p.square()+DirectionPlayerTraits<D,P>::offset();
      // ただの歩を許すかどうか...
      if (state.pieceAt(head).isEmpty())
      {
	const int defense = state.countEffect(alt(P), head, pins);
	const int offense = state.countEffect(P, head);
	if ((defense <= offense)
	    && (! head.canPromote<P>())) // 成りはすでに生成されているはず
	{
	  const Square origin = head+DirectionPlayerTraits<D,P>::offset();
	  const Piece candidate = state.pieceAt(origin);
	  if (candidate.ptype() == PAWN && candidate.owner() == P)
	  {
	    const Move move(origin, head, PAWN, PTYPE_EMPTY, false, P);
	    moves.push_back(move);
	  }
	}
	if (has_pawn && !state.template isPawnMaskSet<P>(head.x())
	    && (defense <= offense+1))
	{
	  const Move move(head, PAWN, P);
	  moves.push_back(move);
	}
      }
    }
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
attackSilverWithPawn(const NumEffectState& state, PieceMask pins,
		     MoveVector& moves) 
{
  using namespace move_action;

  const bool has_pawn   = state.hasPieceOnStand<PAWN>(P);
  const bool has_lance  = state.hasPieceOnStand<LANCE>(P);
  const bool has_knight = state.hasPieceOnStand<KNIGHT>(P);
  const bool has_silver = state.hasPieceOnStand<SILVER>(P);
  const Square opponent_king = state.kingSquare(alt(P));
  for (int i = PtypeTraits<SILVER>::indexMin;
       i < PtypeTraits<SILVER>::indexLimit; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (! p.isOnBoardByOwner<alt(P)>())
      continue;

    const Square head = p.square()+DirectionPlayerTraits<D,P>::offset();
    if (state.pieceAt(head).isEmpty())
    {
      const int defense = state.countEffect(alt(P), head, pins);
      int attack = state.countEffect(P, head) 
	+ state.hasEffectByPtype<BISHOP>(P, p.square());
      if (defense >= attack)
	attack += AdditionalEffect::count2(state, head, P);
      const bool near_king 
	= Neighboring8Direct::hasEffect(state, p.ptypeO(), p.square(),
					opponent_king);
      if (defense > attack)
      {
	if (! near_king)
	  continue;
	// TODO: ただの歩は利きがはずれる時に許す
      }
      if (! head.canPromote<P>()) // 成りはすでに生成されているはず
      {
	const Square origin = head+DirectionPlayerTraits<D,P>::offset();
	const Piece candidate = state.pieceAt(origin);
	if (candidate.ptype() == PAWN && candidate.owner() == P)
	{
	  const Move move(origin, head, PAWN, PTYPE_EMPTY, false, P);
	  moves.push_back(move);
	}
      }
      if (has_pawn && !state.template isPawnMaskSet<P>(head.x()))
      {
	const Move move(head, PAWN, P);
	moves.push_back(move);
      }
      if (! near_king)
	continue;
      if (defense <= attack)
      {
	if (has_lance)
	{
	  const Move move(head, LANCE, P);
	  moves.push_back(move);
	}
	if (has_silver)
	{
	  const Move move(head, SILVER, P);
	  moves.push_back(move);
	}
      }
    }
    // knight
    if (! state.pieceAt(head).isEdge())
    {
      const Square knight_l = head+DirectionPlayerTraits<DL,P>::offset();
      attackWithKnight(state, pins, knight_l, has_knight, moves);
      const Square knight_r = head+DirectionPlayerTraits<DR,P>::offset();
      attackWithKnight(state, pins, knight_r, has_knight, moves);
    }
    // ppawn
    const CArray<Square,2> side = {{
      p.square() + DirectionTraits<L>::blackOffset(),
      p.square() + DirectionTraits<R>::blackOffset(),
    }};
    for (Square s: side) {
      if (! state.pieceAt(s).isEmpty())
	continue;
      if (state.countEffect(P, s) 
	  < state.countEffect(alt(P), s))
	continue;
      MoveVector candidate;
      move_generator::GenerateCapture::generate(P,state, s, candidate);

      for (Move m: candidate) {
	if (m.isPromotion() 
	    || state.hasEffectByPiece(state.pieceOnBoard(m.from()),
				      p.square()))
	  continue;
	assert(! ShouldPromoteCut::canIgnoreMove<P>(m));
	moves.push_back(m);
      }
    }
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
kingWalk(const NumEffectState& state, MoveVector& moves)
{
  const Piece my_king = state.kingPiece<P>();
  MoveVector all_moves;
  move_generator::GeneratePieceOnBoard::generate(P,state, my_king, all_moves);

  for (Move m: all_moves)
  {
    if (state.hasEffectAt(alt(P), m.to()))
      continue;
    moves.push_back(m);
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
escapeKing(const NumEffectState& state, MoveVector& moves) 
{
  MoveVector all_moves;
  GenerateEscape<P>::generateCheap(state, state.template kingPiece<P>(), all_moves);

  for (Move m: all_moves)
  {
    const Square to = m.to();
    if (m.isDrop())
    {
      // 焦点以外の中合は読まない
      if (! state.hasEffectAt<P>(to))
      {
	if (! ((m.ptype() == PAWN)
	       && state.hasMultipleEffectAt(alt(P), to)))
	  continue;
      }
    }
    else if (m.ptype() != KING)
    {
      // 焦点以外の中合は読まない
      if (! m.isCapture())
      {
	if (! state.hasMultipleEffectAt(P, to))
	{
	  if (! ((m.ptype() == PAWN)
		 && state.hasMultipleEffectAt(alt(P), to)))
	    continue;
	}
      }
    }
    moves.push_back(m);
  }
}

template <osl::Player P>
bool osl::search::QuiescenceGenerator<P>::
escapeKingInTakeBack(const NumEffectState& state, MoveVector& moves,
		     bool check_by_lance) 
{
  bool has_safe_move = false;
  assert(moves.empty());
  MoveVector all_moves;
  GenerateEscape<P>::generate(state, state.template kingPiece<P>(), all_moves);

  Square last_drop_to = Square::STAND();
  MoveVector drops;
  for (Move m: all_moves)
  {
    const Square to = m.to();
    if (m.ptype() == KING)
    {
      // TODO: 何か犠牲があるはず
      has_safe_move = true; // KFEND 流: 逃げる場所があれば良しとする
      continue;
    }
    if (m.isCapture())
    {
      moves.push_back(m);
      continue;
    }
    // 無駄合は読まない
    const int attack_count = state.countEffect(alt(P),to);
    const int defense_count 
      = state.countEffect(P,to) + (m.isDrop() ? 1 : 0);
    if (defense_count <= attack_count)
      continue;
    if ((attack_count == 1) && (! check_by_lance))
    {
      // 大駒の攻撃に対して自分の利きが充分なら良いとする
      has_safe_move = true;
      continue;
    }

    // 攻方がこれ以上drop の王手をかけないので合駒は1手で詰の判定には充分
    // TODO: 一番安い駒にする
    if (to != last_drop_to)
    {
      last_drop_to = to;
      drops.push_back(m);
    }
  }
  if (! has_safe_move)
    moves.push_back(drops.begin(), drops.end());
  return has_safe_move;
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
advanceBishop(const NumEffectState& state, MoveVector& moves) 
{
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit; ++i)
  {
    const Piece bishop = state.pieceOf(i);
    if (! bishop.isOnBoardByOwner<P>())
      continue;
    if (bishop.ptype() != BISHOP)
      continue;
    const Square from = bishop.square();
    if (state.hasEffectAt<alt(P)>(from))
      continue;			// escape は重複
    advanceBishop<UL>(state, from, moves);
    advanceBishop<UR>(state, from, moves);
  }
}

template <osl::Player P>
template <osl::Direction DIR>
void osl::search::QuiescenceGenerator<P>::
advanceBishop(const NumEffectState& state, 
	      const Square from, MoveVector& moves)
{
  const Offset offset = DirectionPlayerTraits<DIR,P>::offset();
  for (Square to=from+offset;; to+=offset)
  {
    if (! state.pieceAt(to).isEmpty())
      break;		// capture は重複．ついでに盤外判定
    if (to.canPromote<P>())
      break;		// promote は重複
    if (state.hasEffectAt<alt(P)>(to))
      continue;
    const Move move(from, to, BISHOP, PTYPE_EMPTY, false, P);
    assert(state.isAlmostValidMove<false>(move));
    moves.push_back(move);
  }
}
      
template <osl::Player P>
template <class EvalT>
void osl::search::QuiescenceGenerator<P>::
escapeFromLastMove(const NumEffectState& state, Move last_move, MoveVector& moves)
{
  if (! last_move.isNormal())
    return;
  if (last_move.ptype() != PAWN)
  {
    escapeFromLastMoveOtherThanPawn<EvalT>(state, last_move, moves);
    return;
  }
  const Square attack_from = last_move.to();
  const Square attack_to = attack_from+DirectionPlayerTraits<D,P>::offset();
  const Piece target = state.pieceOnBoard(attack_to);
  if (! target.isOnBoardByOwner<P>())
    return;

  using namespace move_action;
  MoveVector all_moves;
  GenerateEscape<P>::generate(state, target, all_moves);

  for (Move m: all_moves)
  {
    assert(! m.isDrop());
    const Square to = m.to();
    // 逃げる時には利きの充分なところへ
    const int defense = state.countEffect(P, to);
    const int offense = state.countEffect(alt(P), to);
    if (offense > defense)
      continue;
    if (to == attack_from)
    {
      // 取る手は重複だけど歩の場合は深さによるので入れておく
      if (offense == defense)
	continue;
      assert(m.capturePtype() == PAWN);
      assert(! ShouldPromoteCut::canIgnoreMove<P>(m));
      moves.push_back(m);
      continue;
    }
    assert(m.from() == attack_to);
    if ((offense == defense)
	&& (m.isCapture()))
      continue;

    // TODO: KnightCaptureDepth
      assert(! ShouldPromoteCut::canIgnoreMove<P>(m));
    if (! m.isPromotion())
    {
      moves.push_back(m);
    }
  }
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
dropMajorPiece(const NumEffectState& state, MoveVector& moves) 
{
  move_generator::SafeDropMajorPiece<P>::generateMoves(state, moves);
}

template <osl::Player P>
void osl::search::QuiescenceGenerator<P>::
dropMajorPiece3(const NumEffectState& state, MoveVector& moves, const HistoryTable& table) 
{
  FixedCapacityVector<Move, 27*2> all;
  move_generator::SafeDropMajorPiece<P>::generateMoves(state, all);
  FixedCapacityVector<std::pair<int,Move>, 27*2> selected;
  for (Move m: all) 
    selected.push_back(std::make_pair(table.value(m), m));
  std::sort(selected.begin(), selected.end());
  for (int i=0; i<std::min(3, (int)selected.size()); ++i)
    moves.push_back(selected[selected.size()-1-i].second);
}

#endif /* QUIESCENCEGENERATOR_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
