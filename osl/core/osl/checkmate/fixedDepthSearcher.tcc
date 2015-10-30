/* fixedDepthSearcher.tcc
 */
#ifndef OSL_CHECKMATE_FIXED_DEPTH_SERCHER_TCC
#define OSL_CHECKMATE_FIXED_DEPTH_SERCHER_TCC
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/numEffectState.h"
#include "osl/move_generator/move_action.h"
#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_classifier/check_.h"

namespace osl
{
  namespace checkmate
  {
    template<Player P, class SetPieces>
    struct FixedAttackHelper{
      FixedDepthSearcher &searcher;
      Move move;
      int depth;
      ProofDisproof& pdp;
      PieceStand& pieces;
      FixedAttackHelper(FixedDepthSearcher &s,int d,ProofDisproof& p,
			PieceStand& pi)
	: searcher(s), depth(d), pdp(p), pieces(pi)
      {
      }
      void operator()(Square)
      {
	assert(move.isNormal());
	pdp=searcher.defense<P,SetPieces>(move,depth-1,pieces);
      }
    };
   /**
    * Pは動かす側ではなく攻撃側
    */
    template<Player P, class SetPieces, bool MayUnsafe=false>
    struct FixedDefenseHelper{
      FixedDepthSearcher &searcher;
      int depth;
      ProofDisproof& pdp;
      PieceStand& pieces;
      Move best_move;
      FixedDefenseHelper(FixedDepthSearcher &s,int d,ProofDisproof& p,
			 PieceStand& pi) 
	: searcher(s), depth(d), pdp(p), pieces(pi)
      {
      }
      void operator()(Square)
      {
	if (MayUnsafe)
	  pdp=searcher.attackMayUnsafe<P,SetPieces,false>(depth-1, best_move, pieces);
	else
	  pdp=searcher.attack<P,SetPieces,false>(depth-1, best_move, pieces);
      }
    };
  }
}

template <osl::Player P, class SetPieces, bool HasGuide>
const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSearcher::
attackMayUnsafe(int depth, Move& best_move, PieceStand& proof_pieces)
{
  assert(state->turn() == P);
  const Square target_king
    = state->template kingSquare<alt(P)>();
  if (state->hasEffectAt<P>(target_king))
    return ProofDisproof::NoEscape();
  return attack<P,SetPieces,HasGuide>(depth, best_move, proof_pieces);
}

template <osl::Player P, class SetPieces, bool HasGuide>
const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSearcher::
attack(int depth, Move& best_move, PieceStand& proof_pieces)
{
  assert(state->turn() == P);
  assert ((! HasGuide)
	  || (state->isAlmostValidMove(best_move)
	      && move_classifier::
	      Check<P>::isMember(*state, best_move.ptype(), best_move.from(), 
				 best_move.to())));
  addCount();
  const Square target_king
    = state->template kingSquare<alt(P)>();
  assert(! state->hasEffectAt<P>(target_king));
  const King8Info info(state->Iking8Info(alt(P)));
  if ((! state->inCheck())
      && ImmediateCheckmate::hasCheckmateMove<P>(*state, info, target_king,
						 best_move))
  {
    SetPieces::setAttackLeaf(best_move, proof_pieces);
    return ProofDisproof::Checkmate();
  }
  if (depth <= 0) 
  {
    return SetPieces::attackEstimation(*state, P, info);
  }

  ProofDisproof pdp;
  typedef FixedAttackHelper<P,SetPieces> helper_t;
  helper_t helper(*this,depth,pdp,proof_pieces);
  int minProof = ProofDisproof::PROOF_MAX;
  int sumDisproof=0;
  if (HasGuide)
  {
    helper.move=best_move;
    state->makeUnmakeMove(Player2Type<P>(),best_move,helper);
    if (pdp.isCheckmateSuccess())
    {
      SetPieces::attack(best_move, PieceStand(P,*state), proof_pieces);
      return ProofDisproof::Checkmate();
    }
    minProof = pdp.proof();
    sumDisproof += pdp.disproof();
  }
  
  const Square targetKing
    = state->template kingSquare<alt(P)>();
  CheckMoveVector moves;
  bool has_pawn_checkmate=false;
  {
    move_action::Store store(moves);    
    move_generator::AddEffectWithEffect<move_action::Store>::template generate<P,true>
      (*state,targetKing,store,has_pawn_checkmate);
  }
  if (moves.size()==0){
    if(has_pawn_checkmate)
      return ProofDisproof::PawnCheckmate();
    else
      return ProofDisproof::NoCheckmate();
  }
  if(has_pawn_checkmate)
    minProof=std::min(minProof,(int)ProofDisproof::PAWN_CHECK_MATE_PROOF);
  for (Move move: moves) {
    if (HasGuide && move == best_move)
      continue;
    helper.move=move;
    state->makeUnmakeMove(Player2Type<P>(), move,helper);
    int proof=pdp.proof();
    if (proof<minProof){
      if (proof==0){
	best_move=move;
	SetPieces::attack(best_move, PieceStand(P,*state), proof_pieces);
	return ProofDisproof::Checkmate();
      }
      minProof=proof;
    }
    sumDisproof+=pdp.disproof();
  }
  // depth >= 3 では PawnCheckmateの際にunpromoteを試す必要あり
  return ProofDisproof(minProof,sumDisproof);
}

template <osl::Player P, class SetPieces>
inline
const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSearcher::
defenseEstimation(Move last_move, PieceStand& proof_pieces,
		  Piece attacker_piece, Square target_position) const
{
  assert(state->turn() == alt(P));
  const Player Opponent = alt(P);
  int count=King8Info(state->Iking8Info(Opponent)).libertyCount();
  // multiple checkなので，pawn dropではない
  if (attacker_piece.isEmpty())
  {
    if (count>0){
      return ProofDisproof(count,1);
    }
    return ProofDisproof::NoEscape();
  }
  const Square attack_from=attacker_piece.square();
  count += state->countEffect(alt(P), attack_from);
  if (attack_from.isNeighboring8(target_position))
    --count;
  const EffectContent effect
    = Ptype_Table.getEffect(attacker_piece.ptypeO(), 
			    attack_from, target_position);
  if (! effect.hasUnblockableEffect())
  {
    // this is better approximation than naive enumeration of blocking moves,
    // for counting of disproof number in df-pn,
    ++count;
  }

  if (count==0){
    if (last_move.isValid() && last_move.isDrop() && last_move.ptype()==PAWN)
      return ProofDisproof::PawnCheckmate();
    SetPieces::setLeaf(*state, P, stand(P), proof_pieces);
    return ProofDisproof::NoEscape();
  }
  return ProofDisproof(count, 1);
}

template <osl::Player Defense>
void osl::checkmate::FixedDepthSearcher::
generateBlockingWhenLiberty0(Piece defense_king, Square attack_from,
			     CheckMoveVector& moves) const
{
  assert(state->kingPiece(Defense) == defense_king);
  using namespace move_generator;
  using namespace move_action;
  CheckMoveVector all_moves;
  {
    Store store(all_moves);
    Escape<Store>::
      generateBlockingKing<Defense,false>(*state, defense_king, attack_from,store);
  }

  for (Move move: all_moves)
  {
    if (move.isDrop())
    {
      if (! state->hasEffectAt<Defense>(move.to()))
	continue;
    }
    else
    {
      // move
      if (! move.from().isNeighboring8(defense_king.square()))
      {
	if (! state->hasMultipleEffectAt(Defense, move.to()))
	  continue;
      }
    }
    moves.push_back(move);
  }
}

template <osl::Player Defense> inline
int osl::checkmate::FixedDepthSearcher::
blockEstimation(Square /*attack_from*/, Square /*defense_king*/) const
{
  // 利きのあるマスを数えようと思ったら効果がなかった
  return 1;
}

template <osl::Player P, class SetPieces>
const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSearcher::
defense(Move last_move, int depth, PieceStand& proof_pieces)
{
  assert(state->turn() == alt(P));
  addCount();
  const Player Defense = alt(P);
  const Square attackerKing
    = state->template kingSquare<P>();
  /**
   * 直前の攻め方の手が自殺手
   */
  if (attackerKing.isOnBoard() && state->hasEffectAt<Defense>(attackerKing))
    return ProofDisproof::NoCheckmate();
  const Piece target_king
    = state->template kingPiece<Defense>();
  const Square target_position = target_king.square();
  assert(state->hasEffectAt<P>(target_position));
  Piece attacker_piece;
  state->template findCheckPiece<Defense>(attacker_piece);
  if (depth <= 0)
  {
    return defenseEstimation<P, SetPieces>
      (last_move, proof_pieces, attacker_piece, target_position);
  }

  assert(depth > 0);
  CheckMoveVector moves;
  bool blockable_check = false;
  int nonblock_moves;
  {
    using namespace move_generator;
    using namespace move_action;
    if (attacker_piece.isEmpty()) {
      move_action::Store store(moves);
      Escape<Store>::template
	generateEscape<Defense,KING>(*state,target_king,store);
      nonblock_moves = moves.size();
    }
    else {
      const Square attack_from=attacker_piece.square();
      {
	move_action::Store store(moves);
	Escape<Store>::
	  generateCaptureKing<Defense>(*state, target_king, attack_from, store);
      }
      const int num_captures = moves.size();
      {
	move_action::Store store(moves);
	Escape<Store>::template
	  generateEscape<Defense,KING>(*state, target_king, store);
      }
      nonblock_moves = moves.size();
      blockable_check = ! state->inUnblockableCheck(alt(P));
      if ((depth <= 1) && num_captures && (nonblock_moves > 2))
      {
	if (nonblock_moves > 3)
	{
	  const int block_estimate = blockable_check 
	    ? blockEstimation<Defense>(attack_from, target_position) 
	    : 0;
	  return ProofDisproof(nonblock_moves + block_estimate, 1);
	}
      } 
      if (moves.empty())
	generateBlockingWhenLiberty0<Defense>(target_king, attack_from, moves);
    }
  }
  const size_t initial_moves = moves.size();
  if (moves.empty() && !blockable_check) {
    if (last_move.isValid() && last_move.isDrop() && last_move.ptype()==PAWN)
      return ProofDisproof::PawnCheckmate();
    SetPieces::setLeaf(*state, P, PieceStand(P,*state), proof_pieces);
    return ProofDisproof::NoEscape();
  }
  const bool cut_candidate = (depth <= 1)
    && (nonblock_moves - (state->hasPieceOnStand<GOLD>(P)
			  || state->hasPieceOnStand<SILVER>(P)) > 4);
  if (cut_candidate)
    return ProofDisproof(nonblock_moves, 1);

  typedef FixedDefenseHelper<P,SetPieces> helper_t;
  SetPieces::clear(proof_pieces);
  PieceStand child_proof;
  ProofDisproof pdp;
  helper_t helper(*this, depth, pdp, child_proof);
  int minDisproof = ProofDisproof::DISPROOF_MAX;
  int sumProof = 0;
  size_t i=0, no_promote_moves=0;
start_examine:
  for (;i<moves.size();i++){
    state->makeUnmakeMove(Player2Type<alt(P)>(),moves[i],helper);
    const int disproof=pdp.disproof();
    if (disproof<minDisproof){
      if (disproof==0)
      {
	return pdp;		// maybe PawnCheckmate
      }
      minDisproof=disproof;
    }
    sumProof+=pdp.proof();
    if (sumProof == 0)
    {
      SetPieces::updateMax(child_proof, proof_pieces);
    }
    else
    {
      if (i+1 < moves.size())
      {
	minDisproof = 1;
	if ((int)i < nonblock_moves)
	  sumProof += nonblock_moves - (i+1);
	if (blockable_check)
	  ++sumProof;
      }
      return ProofDisproof(sumProof,minDisproof);
    }
  }
  if (sumProof == 0)
  {
    if (blockable_check && moves.size() == initial_moves)
    {
      using namespace move_generator;
      using namespace move_action;
      const Square attack_from=attacker_piece.square();
      {
	move_action::Store store(moves);
	Escape<Store>::
	  generateBlockingKing<Defense,false>(*state, target_king, attack_from,store);
      }
      if ((int)moves.size() > nonblock_moves)
	goto start_examine;
      if (moves.empty()) {
	assert(! (last_move.isValid() && last_move.isDrop() && last_move.ptype()==PAWN));
	SetPieces::setLeaf(*state, P, PieceStand(P,*state), proof_pieces);
	return ProofDisproof::NoEscape();
      }
    }
    if (no_promote_moves == 0) 
    {
      no_promote_moves = moves.size();
      for (size_t i=0; i<no_promote_moves; ++i) 
	if (moves[i].hasIgnoredUnpromote<Defense>())
	  moves.push_back(moves[i].unpromote());
      if (moves.size() > no_promote_moves)
	goto start_examine;
    }
    if (blockable_check)
      SetPieces::addMonopolizedPieces(*state, P, stand(P), proof_pieces);
  }
  // depth >= 2 では unprmote も試す必要あり
  return ProofDisproof(sumProof,minDisproof);
}

template <osl::Player P>
const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSearcher::
hasEscapeByMove(Move next_move, int depth)
{
  typedef FixedDefenseHelper<P,NoProofPieces,true> helper_t;
  PieceStand proof_pieces;
  ProofDisproof pdp;
  helper_t helper(*this, depth+1, pdp, proof_pieces);
  state->makeUnmakeMove(Player2Type<alt(P)>(),next_move,helper);
  return pdp;
}

#endif /* OSL_CHECKMATE_FIXED_DEPTH_SERCHER_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
