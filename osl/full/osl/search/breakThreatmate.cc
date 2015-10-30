/* breakThreatmate.cc
 */
#include "osl/search/breakThreatmate.h"
#include "osl/container/moveLogProbVector.h"
#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/move_generator/capture_.h"
#include "osl/move_generator/pieceOnBoard.h"
#include "osl/move_generator/escape_.h"
#include "osl/bits/king8Info.h"

void osl::search::
BreakThreatmate::generateBreakDrop(int limit,
				   const NumEffectState& state, Square to,
				   int default_prob,
				   MoveLogProbVector& out)
{
  const Player Turn = state.turn();
  default_prob = std::max(default_prob, 150);
  if (state.hasPieceOnStand<PAWN>(Turn))
  {
    if (! state.isPawnMaskSet(Turn, to.x())
	&& Ptype_Table.canDropTo(Turn, PAWN, to)
	&& limit >= default_prob-100)
      out.push_back(MoveLogProb(Move(to, PAWN, Turn), default_prob-100));
  }
  if (state.hasPieceOnStand<LANCE>(Turn)
      && Ptype_Table.canDropTo(Turn, LANCE, to)
      && limit >= default_prob-50)
    out.push_back(MoveLogProb(Move(to, LANCE, Turn), default_prob-50));
  if (default_prob > limit)
    return;

  if (state.hasPieceOnStand<KNIGHT>(Turn)
      && Ptype_Table.canDropTo(Turn, KNIGHT, to))
    out.push_back(MoveLogProb(Move(to, KNIGHT, Turn), default_prob));
  if (state.hasPieceOnStand<SILVER>(Turn))
    out.push_back(MoveLogProb(Move(to, SILVER, Turn), default_prob));
  if (state.hasPieceOnStand<GOLD>(Turn))
    out.push_back(MoveLogProb(Move(to, GOLD, Turn), default_prob));
  if (state.hasPieceOnStand<BISHOP>(Turn))
    out.push_back(MoveLogProb(Move(to, BISHOP, Turn), default_prob));
  if (state.hasPieceOnStand<ROOK>(Turn))
    out.push_back(MoveLogProb(Move(to, ROOK, Turn), default_prob));
}

void osl::search::BreakThreatmate::findBlockLong(const NumEffectState& state, Move threatmate_move,
						 MoveVector& out)
{
  const Player Turn = alt(threatmate_move.player());
  const Piece my_king = state.kingPiece(Turn);
  const Square target = threatmate_move.to();
  out.clear();
  move_action::Store store(out);
  // block additional
  if (! threatmate_move.isDrop()
      && threatmate_move.oldPtype() != KNIGHT)
  {
    const Offset step = Board_Table.getShortOffsetNotKnight(Offset32(threatmate_move.from(), target));
    Square p=threatmate_move.from()+step;
    Piece piece=state.pieceAt(p);
    for (; piece.isEmpty(); p+=step, piece=state.pieceAt(p))
      ;
    if (piece.isPiece() && piece.owner() == alt(Turn)
	&& state.hasEffectByPiece(piece, threatmate_move.from())) {
      if (Turn == BLACK)
	move_generator::Escape<move_action::Store>::generateBlocking<BLACK,true>
	  (state, my_king, threatmate_move.from(), piece.square(), store);
      else
	move_generator::Escape<move_action::Store>::generateBlocking<WHITE,true>
	  (state, my_king, threatmate_move.from(), piece.square(), store);
    }
  }
  // block long
  {
    mask_t attack = state.longEffectAt(target, alt(Turn));
    while (attack.any()) {
      static_assert(PtypeFuns<LANCE>::indexNum == PtypeFuns<BISHOP>::indexNum, "");
      static_assert(PtypeFuns<LANCE>::indexNum == PtypeFuns<ROOK>::indexNum, "");
      const int num = (attack.takeOneBit()+((PtypeFuns<LANCE>::indexNum)<<5));
      if (Turn == BLACK)
	move_generator::Escape<move_action::Store>::generateBlocking<BLACK,true>
	  (state, my_king, target, state.pieceOf(num).square(), store);
      else
	move_generator::Escape<move_action::Store>::generateBlocking<WHITE,true>
	  (state, my_king, target, state.pieceOf(num).square(), store);
    }
  }
}

void osl::search::BreakThreatmate::
generateAddEffect(int limit, const NumEffectState& state, Square target,
		  const MoveVector& all_moves, MoveLogProbVector& out)
{
  const Player Turn = state.turn();
  const int max_prob = state.king8Info(state.turn()).liberty() ? limit : 100;
  for (Move move: all_moves)
  {
    const Ptype ptype = move.ptype();
    if (ptype == KING)
      continue;			// KING_WALK will generate
    if (! move.isDrop())
    {
      if (isMajor(ptype)
	  && state.hasEffectByPiece(state.pieceOnBoard(move.from()), target))
	continue;			// already have effect
    }
    const Square to = move.to();
    const int me = state.countEffect(Turn, to) + (move.isDrop() ? 1 : 0);
    const int op = state.countEffect(alt(Turn), to);
    int prob = (move.isDrop() ? 100 : 100); // delay drop
    if (move.isCapture())
    {
      prob -= 50;
    }
    else
    { 
      if (isMajor(ptype)
	  || ((ptype == GOLD || ptype == SILVER)
	      && (to.x() == 1 || to.x() == 9)))
      {
	prob += 50;
      }
      if (! ((me >= 2) || (op == 0)))
      {
	prob += 300;
      }
    }
    prob = std::min(prob, max_prob);
    if (prob <= limit)
      out.push_back(MoveLogProb(move, prob));
  }
}

// yoshiki's suggestion 駒取り 50, 普通 100, 駒捨て 400 
void osl::search::BreakThreatmate::
generate(int limit, const NumEffectState& state, Move threatmate_move,
	 MoveLogProbVector& out)
{
  assert(threatmate_move.isNormal());
  const Player Turn = state.turn();
  
  MoveVector all_moves;
  assert(threatmate_move.isNormal());
  const Square target = threatmate_move.to();
  move_generator::GenerateAddEffectWithEffect::generate<false>
      (Turn, state, target, all_moves);
  generateAddEffect(limit, state, target, all_moves, out);

  if (threatmate_move.isDrop())
  {
    const int drop_prob = (state.hasEffectAt(alt(Turn), target) ? 400 : 100);
    generateBreakDrop(limit, state, target, drop_prob, out);
  }
  else
  {
    // not drop
    const Square from = threatmate_move.from();
    const Offset offset
      = Board_Table.getShortOffsetNotKnight(Offset32(target, from));
    if (! offset.zero())
    {
      for (Square to = from + offset; to != target; to += offset) 
      {
	assert(to.isOnBoard());
	assert(state.pieceOnBoard(to) == Piece::EMPTY());
	const int drop_prob = (state.hasEffectAt(Turn, to) ? 100 : 400);
	generateBreakDrop(limit, state, to, drop_prob, out);

	const int move_prob = (state.hasMultipleEffectAt(Turn, to) ? 100 : 400);
	if (move_prob > limit)
	  continue;
	all_moves.clear();
	move_generator::GenerateCapture::generate
	  (Turn, state, to, all_moves);
 	for (Move move: all_moves)
	{
	  out.push_back(MoveLogProb(move, move_prob));
	}
      }
    }
  }
  const Piece my_king = state.kingPiece(Turn);
  if (my_king.square()
      != target+Board_Table.getShortOffset(Offset32(my_king.square(),target)))
  {
    const checkmate::King8Info king8info = state.king8Info(Turn);
    unsigned int drop_candidate = king8info.dropCandidate();
    if (drop_candidate) {
      const int d = misc::BitOp::bsf(drop_candidate);
      const Square to = my_king.square()
	+ Board_Table.getOffset(Turn, static_cast<Direction>(d));
      if (to != target) {
	all_moves.clear();
	move_generator::GenerateAddEffectWithEffect::generate<false>
	  (Turn, state, to, all_moves);
	generateAddEffect(limit, state, to, all_moves, out);
      }
    }
  }
  // king walk
  const int king_prob = 100;
  if (king_prob <= limit)
  {
    all_moves.clear();
    {
      GeneratePieceOnBoard::generate(Turn, state, my_king, all_moves);
    }
    for (Move move: all_moves)
    {
      if (state.hasEffectAt(alt(Turn), move.to()))
	continue;
      out.push_back(MoveLogProb(move, king_prob));
    }
  }
  
  // open king road
  const Square center = my_king.square();
  generateOpenRoad(limit, state, center + DirectionTraits<U>::blackOffset(), out);
  generateOpenRoad(limit, state, center + DirectionTraits<UL>::blackOffset(), out);
  generateOpenRoad(limit, state, center + DirectionTraits<UR>::blackOffset(), out);
  generateOpenRoad(limit, state, center + DirectionTraits<L>::blackOffset(), out);
  generateOpenRoad(limit, state, center + DirectionTraits<R>::blackOffset(), out);
  generateOpenRoad(limit, state, center + DirectionTraits<D>::blackOffset(), out);
  generateOpenRoad(limit, state, center + DirectionTraits<DL>::blackOffset(), out);
  generateOpenRoad(limit, state, center + DirectionTraits<DR>::blackOffset(), out);

  // block
  findBlockLong(state, threatmate_move, all_moves);
  if (! all_moves.empty()) 
  {
    Ptype cheapest = PTYPE_EMPTY;
    if (state.hasPieceOnStand<PAWN>(Turn)) cheapest = PAWN;
    else if (state.hasPieceOnStand<LANCE>(Turn)) cheapest = LANCE;
    else if (state.hasPieceOnStand<KNIGHT>(Turn)) cheapest = KNIGHT;
    int added = 0;
    Move chuai_reserve;
    for (Move m: all_moves) {
      const int d = state.countEffect(Turn, m.to()) + m.isDrop();
      const int a = state.countEffect(alt(Turn), m.to());
      if (d == 1
	  || (m.ptype() != cheapest && cheapest != PTYPE_EMPTY
	      && Ptype_Table.canDropTo(Turn, cheapest, m.to())
	      && (cheapest != PAWN || ! state.isPawnMaskSet(m.player(), m.to().x()))))
	continue;
      if (a >= d) {
	if (! chuai_reserve.isNormal()) 
	  chuai_reserve = m;
	continue;
      }
      const int prob = 150+added*50;
      if (prob > limit)
	break;
      out.push_back(MoveLogProb(m, prob));
    }
    if (added == 0 && chuai_reserve.isNormal() && limit >= 250) {
      out.push_back(MoveLogProb(chuai_reserve, 250));
      if (chuai_reserve.isDrop() && chuai_reserve.ptype() == KNIGHT && 300 <= limit) {
	if (state.hasPieceOnStand<SILVER>(Turn))
	  out.push_back(MoveLogProb(Move(chuai_reserve.to(),SILVER,Turn), 300));
	else if (state.hasPieceOnStand<GOLD>(Turn))
	  out.push_back(MoveLogProb(Move(chuai_reserve.to(),GOLD,Turn), 300));
	else if (state.hasPieceOnStand<BISHOP>(Turn))
	  out.push_back(MoveLogProb(Move(chuai_reserve.to(),BISHOP,Turn), 300));
	else if (state.hasPieceOnStand<ROOK>(Turn))
	  out.push_back(MoveLogProb(Move(chuai_reserve.to(),ROOK,Turn), 300));
      }
    }
  }
}

void osl::search::BreakThreatmate::
generateOpenRoad(int limit, const NumEffectState& state, 
		 Square from, MoveLogProbVector& out)
{
  const Piece target = state.pieceAt(from);
  if (! target.isPiece())
    return;
  const Player Turn = state.turn();
  if (target.owner() != Turn)
    return;

  const int capture_prob = 50;
  const int default_prob = 100;
  const int sacrifice_prob = 400;
  if (limit < capture_prob)
    return;

  MoveVector moves;
  GeneratePieceOnBoard::generate(Turn, state, target, moves);

  for (Move move: moves)
  {
    const bool capture = move.isCapture();
    const bool sacrifice = state.hasEffectAt(alt(Turn), move.to());
    const int prob = capture ? capture_prob
      : (sacrifice ? sacrifice_prob : default_prob);
    if (prob <= limit)
      out.push_back(MoveLogProb(move, prob));
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
