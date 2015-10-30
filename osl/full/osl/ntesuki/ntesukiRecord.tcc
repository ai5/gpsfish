#include "ntesukiRecord.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "osl/ntesuki/ntesukiTable.h"
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/checkmate/fixedDepthSearcher.tcc"
#include "osl/checkmate/libertyEstimator.h"
#include "osl/threatmate/richPredictor.h"
#include "osl/threatmate/kfendPredictor.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/checkmate/pawnCheckmateMoves.h"
#include "osl/effect_util/unblockableCheck.h"
#include "osl/pieceStand.h"

template <osl::Player P>
void
osl::ntesuki::NtesukiRecord::
propagate_proof(int pass_left)
{
  for (RecordList::iterator it = same_board_list->begin();
       it != same_board_list->end(); it++)
  {
    if (&(*it) == this)
    {
      continue;
    }
    if (it->isDominatedByProofPieces<P>(this, pass_left))
    {
      if (!it->getValue<P>(pass_left).isFinal())
      {
	PieceStand ps = getPDPieces<P>(pass_left);
	TRY_DFPN;
	it->setResult<P>(pass_left, getValue<P>(pass_left),
			 getBestMove<P>(pass_left),
			 false, &ps);
	CATCH_DFPN;
      }
      else
      {
	if (!it->getValue<P>(pass_left).isCheckmateSuccess())
	{
#ifdef DEBUG
	  std::cerr << <"for " < P << "\tat pass_left:" << pass_left << "\tcontradiction occured\n" << *it << *this;
	  ntesuki_assert(pass_left >0 ||
			 it->getValue<P>(pass_left) == ProofDisproof::LoopDetection() ||
			 getValue<P>(pass_left) == ProofDisproof::LoopDetection());
#endif
	}
      }
    }
  }
}

template <osl::Player P>
void
osl::ntesuki::NtesukiRecord::
propagate_disproof(int pass_left)
{
  for (RecordList::iterator it = same_board_list->begin();
       it != same_board_list->end(); it++)
  {
    if (&(*it) == this)
    {
      continue;
    }
    if (it->isDominatedByDisproofPieces<P>(this, pass_left))
    {
      if (!it->getValue<P>(pass_left).isFinal())
      {
	PieceStand ps = getPDPieces<P>(pass_left);
	TRY_DFPN;
	it->setResult<P>(pass_left, getValue<P>(pass_left),
			 getBestMove<P>(pass_left),
			 false, &ps);
	CATCH_DFPN;
      }
      else
      {
	if (!it->getValue<P>(pass_left).isCheckmateFail())
	{
#ifdef DEBUG
	  std::cerr << <"for " < P << "\tat pass_left:" << pass_left << "\tcontradiction occured\n" << *it << *this;
	  ntesuki_assert(it->getValue<P>(pass_left) == ProofDisproof::LoopDetection() ||
			 getValue<P>(pass_left) == ProofDisproof::LoopDetection());
#endif
	}
      }
    }
  }
}

namespace osl
{
  namespace ntesuki
  {
    template <class NumEffectState>
    Ptype getCheapestDrop(Player turn, const NumEffectState& state)
    {
      if (state.hasPieceOnStand(turn, PAWN))
	return PAWN;
      if (state.hasPieceOnStand(turn, LANCE))
	return LANCE;
      if (state.hasPieceOnStand(turn, KNIGHT))
	return KNIGHT;
      if (state.hasPieceOnStand(turn, SILVER))
	return SILVER;
      if (state.hasPieceOnStand(turn, GOLD))
	return GOLD;
      if (state.hasPieceOnStand(turn, BISHOP))
	return BISHOP;
      if (state.hasPieceOnStand(turn, ROOK))
	return ROOK;
      return PTYPE_EMPTY;
    }
    template <class NumEffectState>
    Ptype getSecondCheapestDrop(Player turn, const NumEffectState& state,
				Ptype cheapest)
    {
      if (cheapest == PAWN)
      {
	if (state.hasPieceOnStand(turn, LANCE))
	  return LANCE;
	if (state.hasPieceOnStand(turn, KNIGHT))
	  return KNIGHT;
	if (state.hasPieceOnStand(turn, SILVER))
	  return SILVER;
	if (state.hasPieceOnStand(turn, GOLD))
	  return GOLD;
	if (state.hasPieceOnStand(turn, BISHOP))
	  return BISHOP;
	if (state.hasPieceOnStand(turn, ROOK))
	  return ROOK;
      }
      return PTYPE_EMPTY;
    }
  }
}

/* ノードの準備
 * - 深さ固定 checkmate searcher を呼び出す
 */
template <osl::Player T>
bool
osl::ntesuki::NtesukiRecord::
setUpNode()
{
  ntesuki_assert(state->turn() == turn());
#ifndef NDEBUG
  const Player O = PlayerTraits<T>::opponent;
#endif
  const bool under_attack = state->inCheck(T);

  ntesuki_assert(!state->inCheck(O));

  if (already_set_up)
  {
    return false;
  }
  already_set_up = true;

  /* 黒が攻めるときの Rzone は白の玉の位置 */
  rzone<BLACK>()[0] = Rzone(*state, WHITE);
  rzone<BLACK>()[1] = rzone<BLACK>()[0];
  /* 白が攻めるときの Rzone は黒の玉の位置 */
  rzone<WHITE>()[0] = Rzone(*state, BLACK);
  rzone<WHITE>()[1] = rzone<WHITE>()[0];

  if (!under_attack)
  {
    setUpAttackNode<T>();
  }
  else
  {
    setUpDefenseNode<T>();
  }
  return true;
}

template <osl::Player T>
void
osl::ntesuki::NtesukiRecord::
setUpAttackNode()
{
  const Player O = PlayerTraits<T>::opponent;

  /* 王手がかかっていないときは，相手の手番の即詰は失敗している.
   */
  NtesukiMove pass(Move::PASS(T));
  values<O>()[0] = ProofDisproof::NoCheckmate();
  best_move<O>()[0] = pass;

  if (!values<T>()[0].isFinal() &&
      state->template kingSquare<O>().isOnBoard())
  {
    /* 深さ固定 checkmate searcher を呼び出す */
    FixedDepthSearcher fixed_searcher(*state);
    PieceStand proof_pieces;
    Move check_move;
      
    const NtesukiResult result_fixed =
      fixed_searcher.hasCheckmateMove<T>(fixed_search_depth,
					 check_move,
					 proof_pieces);
    if (result_fixed.isCheckmateSuccess())
    {
      NtesukiMove best_move(check_move);
      best_move.setCheck();
      best_move.setImmediateCheckmate<T>();
	
      TRY_DFPN;
      setResult<T>(0, result_fixed,
		   best_move, false, &proof_pieces);
      CATCH_DFPN;

    }
    else if (result_fixed.isCheckmateFail())
    {
      PieceStand disproof_pieces = getPieceStand<O>();
      TRY_DFPN;
      setResult<T>(0, result_fixed,
		   NtesukiMove::INVALID(), false, &disproof_pieces);
      CATCH_DFPN;
    }
    else
    {
      TRY_DFPN;
      setResult<T>(0, result_fixed,
		   NtesukiMove::INVALID(), false);
      if (!values<T>()[1].isFinal())
      {
	setResult<T>(1, ProofDisproof(1, result_fixed.disproof()),
		     NtesukiMove::INVALID(), false);
      }
      CATCH_DFPN;
    }
  }
}

template <osl::Player T>
void
osl::ntesuki::NtesukiRecord::
setUpDefenseNode()
{
  const Player O = PlayerTraits<T>::opponent;

  /* 王手がかかっているので自動的に相手からの n 手すきがかかっている.
   */
  for (size_t i = 0; i < SIZE; ++i)
  {
    setNtesuki<O>(i);
  }

  /* もし自玉が盤面上にあるのなら, fixed depth searcher を呼び出す */
  if (!values<O>()[0].isFinal() &&
      state->template kingSquare<T>().isOnBoard())
  {
    /* 深さ固定 checkmate searcher を呼び出す */
    FixedDepthSearcher fixed_searcher(*state);
    PieceStand proof_pieces;
    //when called with Move::INVALID() as last move, PawnDropCheckmate is not checked for
    // the last move
    const NtesukiResult result_fixed =
      fixed_searcher.hasEscapeMove<O>(Move::INVALID(),
				      fixed_search_depth,
				      proof_pieces);
    if (result_fixed.isCheckmateSuccess())
    {
      TRY_DFPN;
      setResult<O>(0, result_fixed,
		   NtesukiMove::INVALID(), false, &proof_pieces);
      CATCH_DFPN;
    }
    else if (result_fixed.isCheckmateFail())
    {
      PieceStand disproof_pieces = getPieceStand<T>();
      NtesukiMove best_move = NtesukiMove::INVALID();
      /* might want to know the true best move, or
	 mark best move as immediateCheckmate */
      TRY_DFPN;
      setResult<O>(0, result_fixed,
		   best_move, false, &disproof_pieces);
      CATCH_DFPN;
    }
    else
    {
      TRY_DFPN;
      setResult<O>(0, result_fixed,
		   NtesukiMove::INVALID(), false);
      if (!values<O>()[1].isFinal())
      {
	setResult<O>(1, ProofDisproof(1, result_fixed.disproof()),
		     NtesukiMove::INVALID(), false);
      }
      CATCH_DFPN;
    }
  }
}

/* 手生成を行う
 */
template <osl::Player P>
void
osl::ntesuki::NtesukiRecord::
generateMoves(NtesukiMoveList& move_list,
	      int pass_left,
	      bool all_moves)
{
  const Player O = PlayerTraits<P>::opponent;

  if (all_moves)
  {
    mg->generateSlow(P, *state, move_list);
  }
  else
  {
    mg->generateWithRzoneSlow(P, *state, this, pass_left, move_list);
  }
  const Ptype cheapest = getCheapestDrop(P, *state);
  const Ptype secondCheapest = getSecondCheapestDrop(P, *state, cheapest);

  for (NtesukiMoveList::iterator move_it = move_list.begin();
       move_it != move_list.end(); move_it++)
  {
    const Move move = move_it->getMove();

    if (move_it->isPass())
    {
      unsigned int p_a = 1, d_a = 1, p_d = 1, d_d = 1;
      move_it->setHEstimates(p_a, d_a, p_d, d_d);
      continue;
    }

    unsigned int p_a = 1, d_a = 1, p_d = 1, d_d = 1;
#if 0
    if (state->template kingSquare<O>().isOnBoard())
    {
      checkmate::LibertyEstimator::attackH(P, *state, move_it->getMove(),
					   p_a, d_a);
    }
    if (state->template kingSquare<P>().isOnBoard())
    {
      checkmate::LibertyEstimator::defenseH(P, *state, move_it->getMove(),
					    p_d, d_d);
    }
#endif

    move_it->setHEstimates(p_a, d_a, p_d, d_d);

    const Square from = move.from();
    const Square to = move.to();
    const Ptype ptype = move.ptype();

    /* 駒のただ捨て(無駄合を含む)を記録 */
    if (state->hasEffectAt(O, to))
    {
      if (from.isPieceStand())
      {
	if ((ptype != cheapest)  && (ptype != secondCheapest))
	{
	  move_it->setInterpose();
	}
	else if((! state->hasEffectAt(P, to))	// 自分の利きがない
		&& (! state->hasMultipleEffectAt(O, to)))// 焦点でもない
	{
	  move_it->setInterpose();
	}
      }
      else if ((ptype != KING) &&
	       (move.capturePtype() == PTYPE_EMPTY) &&
	       (! state->hasMultipleEffectAt(P, to)))
      {
	move_it->setInterpose();
      }
    }

    /* 無駄な遠い利き */
    if (delay_lame_long &&
	from.isPieceStand() &&
	(isMajor(ptype) || ptype == LANCE) &&
	(! state->hasMultipleEffectAt(P, to)))
    {
      const Square opKingSquare = state->template kingSquare<O>();
      const Square myKingSquare = state->template kingSquare<P>();
      bool close_to_king = false;
      if (opKingSquare.isOnBoard())
      {
	int distance = (opKingSquare.x() - to.x()) * (opKingSquare.x() - to.x()) +
	  (opKingSquare.y() - to.y()) * (opKingSquare.y() - to.y());
	if (distance < 19)
	{
	  close_to_king = true;
	}
      }
      if (myKingSquare.isOnBoard())
      {
	int distance = (myKingSquare.x() - to.x()) * (myKingSquare.x() - to.x()) +
	  (myKingSquare.y() - to.y()) * (myKingSquare.y() - to.y());
	if (distance < 19)
	{
	  close_to_king = true;
	}
      }

      if (!close_to_king)
      {
	move_it->setLameLong();
	ntesuki_assert(move_it->isLameLong());
      }
    }

    /* 不成を記録 */
    if (from.isOnBoard() &&
	PawnCheckmateMoves::effectiveOnlyIfPawnCheckmate<P>(ptype, from, to))
    {
      move_it->setNoPromote();
    }
  }
}

struct
DifferentMove
{
  const osl::ntesuki::NtesukiMove* move;

  DifferentMove(const osl::ntesuki::NtesukiMove* move)
    :move(move) {}

  bool operator()(const osl::ntesuki::NtesukiMove& m)
  {
    return m.getMove() != move->getMove();
  }
};

template <osl::Player P>
osl::PieceStand
osl::ntesuki::NtesukiRecord::
calcProofPiecesOr(int pass_left,
		  const osl::ntesuki::NtesukiMove& move)
{
  ntesuki_assert(turn() == P);
  PieceStand proof_pieces;

  const NtesukiRecord* record_child = table->findWithMove(this, move);
  ntesuki_assert(record_child);
  proof_pieces = record_child->getPDPieces<P>(pass_left);

  if (move.isDrop())
  {
    proof_pieces.add(move.ptype());
  }
  else if (move.getMove().capturePtype() != PTYPE_EMPTY)
  {
    proof_pieces.trySub(unpromote(move.getMove().capturePtype()));
  }
  return proof_pieces;
}

template <osl::Player P>
osl::PieceStand
osl::ntesuki::NtesukiRecord::
calcProofPiecesAnd(int pass_left)
{
  ntesuki_assert(state->turn() == turn());

  const Player O = PlayerTraits<P>::opponent;
  ntesuki_assert(turn() != P);
  PieceStand proof_pieces;

  NtesukiMoveList moves;
  mg->generate<O>(*state, moves);

  for (NtesukiMoveList::iterator move_it = moves.begin();
       move_it != moves.end(); move_it++)
  {
    const NtesukiMove& move = *move_it;
    const NtesukiRecord* record_child = table->findWithMove(this, move);
    if (!record_child)
    {
      if (move.isCheck() ||
	  (0 == pass_left &&
	   (!move.isCheck() || move.isNoPromote())))
      {
	continue;
      }
      else
      {
	return piece_stand<P>();
      }
    }
    else if (!record_child->getValue<P>(pass_left).isCheckmateSuccess())
    {
      continue;
    }

    PieceStand proof_pieces_child = record_child->getPDPieces<P>(pass_left);
    proof_pieces = proof_pieces.max(proof_pieces_child);
  }
  
  /* monopolized pieces */
  ntesuki_assert(state);
  
  if (! effect_util::UnblockableCheck::isMember(O, *state))
  {
    for (unsigned int i=0; i<PieceStand::order.size(); ++i)
    {
      const Ptype ptype = PieceStand::order[i];
      if (!state->hasPieceOnStand(O, ptype))
      {
	const int diff = state->countPiecesOnStand(P, ptype) - proof_pieces.get(ptype);
	ntesuki_assert(diff >= 0);
	if (diff) proof_pieces.add(ptype, diff);
      }
    }
  }
  return proof_pieces;
}

template <osl::Player A>
void
osl::ntesuki::NtesukiRecord::
setProofPieces(int pass_left,
	       const NtesukiResult& r,
	       const NtesukiMove& best_move,
	       const PieceStand* ps)
{
  const Player D = PlayerTraits<A>::opponent;
  PieceStand proof_pieces;

  if (ps)
  {
    /* Immediate checkmate or dominance or oracel prover light
     */
    proof_pieces = *ps;
  }
  else if (best_move.isPass())
  {
    const NtesukiRecord* record_pass = table->findWithMove(this, best_move);
    ntesuki_assert(record_pass);
    proof_pieces = record_pass->getPDPieces<A>(pass_left - 1);
  }
  else if (best_move.isValid())
  {
    proof_pieces = calcProofPiecesOr<A>(pass_left, best_move);
  }
  else
  {
    /* set_proof_tree_AND
     */
    proof_pieces = calcProofPiecesAnd<A>(pass_left);
  }

  ntesuki_assert(piece_stand<A>().template hasMoreThan<BLACK>(proof_pieces));
  for (unsigned int j = pass_left; j < SIZE; j++)
  {
    setPDPieces<A>(j, proof_pieces);
    setPDPieces<D>(j, proof_pieces);//for attack back
  }
}

template <osl::Player P>
void
osl::ntesuki::NtesukiRecord::
setDisproofPieces(int pass_left,
		  const NtesukiResult& r,
		  const NtesukiMove& m,
		  const PieceStand* ps)
{
  const Player O = PlayerTraits<P>::opponent;
  PieceStand disproof_pieces;

  if (ps)
  {
    /* Immediate checkmate or dominance
     */
    disproof_pieces = *ps;
  }
  else if (m.isPass())
  {
    const NtesukiRecord* record_pass = table->findWithMove(this, m);
    ntesuki_assert(record_pass);
    
    disproof_pieces = record_pass->getPDPieces<P>(pass_left - 1);
  }
  else if (m.isValid())
  {
    /* set_disproof_tree_OR
     */
    ntesuki_assert(turn() != P);
    
    const NtesukiRecord* record_child = table->findWithMove(this, m);
    ntesuki_assert(record_child);
    disproof_pieces = record_child->getPDPieces<P>(pass_left);
    
    if (m.isDrop())
    {
      disproof_pieces.add(m.ptype());
    }
    else if (m.getMove().capturePtype() != PTYPE_EMPTY)
    {
      disproof_pieces.trySub(unpromote(m.getMove().capturePtype()));
    }
  }
  else
  {
    /* set_disproof_tree_AND
     */
    ntesuki_assert(turn() == P);

    NtesukiMoveList moves;
    generateMoves<P>(moves, 0, true);

    for (NtesukiMoveList::iterator move_it = moves.begin();
	 move_it != moves.end(); move_it++)
    {
      const NtesukiRecord* record_child = table->findWithMove(this, *move_it);
      if (!record_child)
      {
	if (move_it->isPass() ||
	    (0 == pass_left &&
	     (!move_it->isCheck() || move_it->isNoPromote())))
	{
	  continue;
	}
	else
	{
	  setPDPieces<P>(pass_left, piece_stand<O>());
	  return;
	}
	
      }

      PieceStand disproof_pieces_child = record_child->getPDPieces<P>(pass_left);

      disproof_pieces = disproof_pieces.max(disproof_pieces_child);
    }

    /* monopolized pieces */
    ntesuki_assert(state);
    ntesuki_assert(state->turn() == turn());

    //if (! effect_util::UnblockableCheck::isMember(P, *state))
    if (true)
    {
      for (unsigned int i=0; i<PieceStand::order.size(); ++i)
      {
	const Ptype ptype = PieceStand::order[i];
	if (!state->hasPieceOnStand(P, ptype))
	{
	  const int diff = state->countPiecesOnStand(O, ptype) - disproof_pieces.get(ptype);
	  ntesuki_assert(diff >= 0);
	  if (diff) disproof_pieces.add(ptype, diff);
	}
      }
    }
  }

#ifndef NDEBUG
  ntesuki_assert(piece_stand<O>().isSuperiorOrEqualTo(disproof_pieces))
#endif
  setPDPieces<P>(pass_left, disproof_pieces);
}

template <osl::Player P>
void
osl::ntesuki::NtesukiRecord::
setFinal(int pass_left,
	 const NtesukiResult& r,
	 const NtesukiMove& m,
	 const PieceStand* ps)
{
  if (r.isCheckmateSuccess() && pass_count)
  {
    final = true;
  }
  const Player O = PlayerTraits<P>::opponent;

  if (r.isCheckmateSuccess())
  {
    TRY_DFPN;
    setProofPieces<P>(pass_left, r, m, ps);
    CATCH_DFPN;

    for (unsigned int j = pass_left; j < SIZE; j++)
    {
      values<P>   ()[j] = r;
      best_move<P>()[j] = m;
      
      if (!values<O>()[j].isCheckmateFail())
      {
	ntesuki_assert(!values<O>()[j].isCheckmateSuccess());
	
	values<O>   ()[j] = ProofDisproof::AttackBack();
	best_move<O>()[j] = m;
      }
    }

    if (use_dominance)
    {
      propagate_proof<P>(pass_left);
    }
    
#ifdef COLLECT_GARBAGE
    //collect garbage
    if (turn() == P)
    {
      DifferentMove different_move(best_move<P>()[pass_left]);
      moves.remove_if(different_move);
    }
#endif
  }
  else //fail
  {
    ntesuki_assert(r.isCheckmateFail());
    setDisproofPieces<P>(pass_left, r, m, ps);

    values<P>()[pass_left] = r;
    best_move<P>()[pass_left] = m;

    if (pass_left != 0 &&
	!values<P>()[pass_left - 1].isCheckmateFail()
	)
    {
      setFinal<P>(pass_left - 1, r, m, ps);
    }

    if (use_dominance)
    {
      propagate_disproof<P>(pass_left);
    }

#ifdef COLLECT_GARBAGE
    //collect garbage
    if (turn() != P)
    {
      for (NtesukiMoveList::iterator move_it = moves.begin();
	   move_it != moves.end(); move_it++)
      {
	bool not_best_move = true;
	for (size_t i = 0; i < SIZE; i++)
	{
	  if (&(*move_it) != best_move<O>()[pass_left]) not_best_move = false;
	}

	if (not_best_move)
	{
	  move_it->clearRecord();
	}
      }
    }
#endif
  }
}

/* set result
 *  also propagate values to deeper threats if checkmate success
 *  P is the attacker
 */
template <osl::Player P>
void
osl::ntesuki::NtesukiRecord::
setResult(int i,			//i tesuki
	  const NtesukiResult& r,	//pn/dn of this node at i
	  const NtesukiMove& m,		//best move
	  bool bs,			//by simulation
	  const PieceStand* ps		//when set by fixed depth searcher
	  )
{
  ++written_count;
  /* TODO something that should dissapear,
   * as this is a result of a loop gaining/losing pieces
   */
  ntesuki_assert(!values<P>()[i].isFinal());
  ntesuki_assert(best_move<P>()[i].isInvalid());

  by_simulation = bs;

  if (r.isFinal())
  {
    value_before_final = values<P>()[i];//remember the last pdp for latter use
    setFinal<P>(i, r, m, ps);
  }
  else
  {
    values<P>()[i] = r;
    /* Dominance between lambda order */
    int order = 0;
    for (; order < i; order++)
    {
      if (values<P>()[order].disproof() >  r.disproof())
      {
	values<P>()[order] = ProofDisproof(values<P>()[order].proof(),
					   r.disproof());
      }
    }
    ++order;
    for (; order < (int)SIZE; order++)
    {
      /* L^order が AttackBack等で既に disproof されている場合 */
      if (values<P>()[order].isCheckmateFail()) continue;
      
      if (values<P>()[order].disproof() <  r.disproof())
      {
	values<P>()[order] = ProofDisproof(values<P>()[order].proof(),
					   r.disproof());
      }
    }
  }
  
#ifndef NDEBUG
  if (by_simulation) ntesuki_assert(r.isFinal());

  if (m.isValid())
  {
    ntesuki_assert(r.isFinal());
  }

  if (key.turn() == P &&
      values<P>()[i].isCheckmateSuccess())
  {
    ntesuki_assert(m.isValid());
  }
  else if (key.turn() != P &&
	   values<P>()[i].isCheckmateFail())
  {
    /* ntesuki_assert(m); */
  }
#endif
}

/* See if this is dominated by the proof pieces for record
 */
template <osl::Player P>
bool
osl::ntesuki::NtesukiRecord::
isDominatedByProofPieces(const NtesukiRecord* record,
			 int pass_left) const
{
  if (!record->getValue<P>(pass_left).isCheckmateSuccess()) return false;

  const PieceStand& my_stand = piece_stand<P>();
  const PieceStand& other_pp = record->getPDPieces<P>(pass_left);

  return my_stand.isSuperiorOrEqualTo(other_pp);
}

/* See if this is dominated by the disproof pieces for record
 */
template <osl::Player P>
bool
osl::ntesuki::NtesukiRecord::
isDominatedByDisproofPieces(const NtesukiRecord* record,
			    int pass_left) const
{
  const Player O = PlayerTraits<P>::opponent;
  if (!record->getValue<P>(pass_left).isCheckmateFail()) return false;
  
  const PieceStand& my_stand = piece_stand<O>();
  const PieceStand& other_dp = record->getPDPieces<P>(pass_left);

  return my_stand.isSuperiorOrEqualTo(other_dp);
}

/* See if this dominated by record
 */
template <osl::Player P>
bool
osl::ntesuki::NtesukiRecord::
isBetterFor(NtesukiRecord* record)
{
  const PieceStand& mystand = key.getPieceStand();
  const PieceStand& opstand = record->key.getPieceStand();
  
  if (mystand == opstand) return false;
  return mystand.hasMoreThan<P>(opstand);
}

/* =============================================================================
 * accessors
 */
template <osl::Player A>
bool osl::ntesuki::NtesukiRecord::
useOld(int pass_left) const
{
  return use_old<A>()[pass_left];
}

template <osl::Player A>
void osl::ntesuki::NtesukiRecord::
setUseOld(int pass_left,
	   bool value)
{
  use_old<A>()[pass_left] = value;
}


template <osl::Player P>
bool osl::ntesuki::NtesukiRecord::
isLoopWithPath(int pass_left,
	       const PathEncoding& path) const
{
  typedef osl::ntesuki::PathEncodingList list_t;
  const list_t& list = loop_path_list<P>()[pass_left];
  for (list_t::const_iterator it = list.begin(); it != list.end(); it++)
  {
    if (*it == path)
    {
      return true;
    }
  }
  return false;
}

template <osl::Player P>
void  osl::ntesuki::NtesukiRecord::
setLoopWithPath(int pass_left,
		const PathEncoding& path)
{
  typedef osl::ntesuki::PathEncodingList list_t;
  list_t& list = loop_path_list<P>()[pass_left];
  list.push_front(path);
}

template <osl::Player P>
const osl::ntesuki::NtesukiResult
osl::ntesuki::NtesukiRecord::
getValue(int i) const
{
  ++read_count;
  return  values<P>()[i];
}

template <osl::Player P>
const osl::ntesuki::NtesukiResult
osl::ntesuki::NtesukiRecord::
getValueWithPath(int i,
		 const PathEncoding path) const
{
  ++read_count;
  if (values<P>()[i].isFinal()) return values<P>()[i];
  if (isLoopWithPath<P>(i, path))
  {
    return ProofDisproof::LoopDetection();
  }
  return values<P>()[i];
}


template <osl::Player P>
const osl::ntesuki::NtesukiResult
osl::ntesuki::NtesukiRecord::
getValueOr(int max_pass_left,
	   const PathEncoding path,
	   IWScheme iwscheme) const
{
  ++read_count;
  if (values<P>()[max_pass_left].isFinal()) return values<P>()[max_pass_left];
  if (isLoopWithPath<P>(max_pass_left, path))
  {
    return ProofDisproof::LoopDetection();
  }

  NtesukiResult ret = values<P>()[max_pass_left];

  if (iwscheme == pn_iw)
  {
    unsigned int min_proof = ret.proof();
    for (int pass_left = 0; pass_left < max_pass_left; ++pass_left)
    {
      if (isLoopWithPath<P>(pass_left, path)) continue;
      const NtesukiResult result = values<P>()[pass_left];
      ntesuki_assert(result.disproof() <= ret.disproof());
      min_proof = std::min(min_proof, result.proof());
    }
    ret = NtesukiResult(min_proof, ret.disproof());
  }
  else if (iwscheme == strict_iw)
  {
    for (int pass_left = 0; pass_left < max_pass_left; ++pass_left)
    {
      if (isLoopWithPath<P>(pass_left, path)) continue;
      const NtesukiResult result = values<P>()[pass_left];
      if (!result.isCheckmateFail())
      {
	ret = result;
	break;
      }
    }
  }
  return  ret;
}

template <osl::Player P>
const osl::ntesuki::NtesukiResult
osl::ntesuki::NtesukiRecord::
getValueAnd(int max_pass_left,
	    const PathEncoding path,
	    IWScheme iwscheme,
	    PSScheme psscheme) const
{
  ++read_count;
  if (values<P>()[max_pass_left].isFinal()) return values<P>()[max_pass_left];
  if (isLoopWithPath<P>(max_pass_left, path))
  {
    return ProofDisproof::LoopDetection();
  }

  NtesukiResult ret = values<P>()[max_pass_left];
  if (psscheme && max_pass_left != 0)
  {
    /* max_pass_left - 1 まで敵の PN を見て，
     * こちらの DN より小さかったら先に読む.
     */
    const Player O = PlayerTraits<P>::opponent;
    const NtesukiResult result_opponent = getValueOr<O>(max_pass_left - 1,
							  path,
							  iwscheme);
    if (!result_opponent.isFinal() &&
	result_opponent.proof() + inversion_cost < ret.disproof())
    {
      ret = ProofDisproof(result_opponent.disproof(), //最も単純なモデルでは ret.proof()
			  result_opponent.proof() + inversion_cost);
    }
  }
  return  ret;
}

template <osl::Player P>
const osl::ntesuki::NtesukiMove&
osl::ntesuki::NtesukiRecord::
getBestMove(int i) const
{
  return best_move<P>()[i];
}

template <osl::Player P>
bool
osl::ntesuki::NtesukiRecord::
isNtesuki(int pass_left) const
{
  return is_ntesuki<P>()[pass_left];
}

template <osl::Player P>
void
osl::ntesuki::NtesukiRecord::
setNtesuki(int pass_left)
{
  //ntesuki_assert(false == is_ntesuki<P>()[pass_left]);
  is_ntesuki<P>()[pass_left] = true;
}

template <osl::Player P>
bool
osl::ntesuki::NtesukiRecord::
hasTriedPropagatedOracle(int pass_left) const
{
  return propagated_oracle<P>()[pass_left];
}

template <osl::Player P>
void
osl::ntesuki::NtesukiRecord::
triedPropagatedOracle(int pass_left)
{
  assert(false == propagated_oracle<P>()[pass_left]);
  propagated_oracle<P>()[pass_left] = true;
}

template <osl::Player P>
bool
osl::ntesuki::NtesukiRecord::
isByFixed() const
{
  return by_fixed<P>();
}

template <osl::Player P>
osl::PieceStand
osl::ntesuki::NtesukiRecord::
getPDPieces(int pass_left) const
{
  return pdpieces<P>()[pass_left];
}

template <osl::Player P>
void
osl::ntesuki::NtesukiRecord::
setPDPieces(int pass_left, const PieceStand p)
{
  pdpieces<P>()[pass_left] = p;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
