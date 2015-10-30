/* fixedDepthSearcher.h
 */
#ifndef OSL_CHECKMATE_FIXED_DEPTH_SERCHER_H
#define OSL_CHECKMATE_FIXED_DEPTH_SERCHER_H
#include "osl/checkmate/proofDisproof.h"
#include "osl/numEffectState.h"
#include "osl/bits/king8Info.h"
#include "osl/bits/pieceStand.h"

namespace osl
{
  namespace checkmate
  {
    struct NoProofPieces
    {
      static void setAttackLeaf(Move /*best_move*/, PieceStand& /*proof_pieces*/) {
      }
      static void attack(Move /*best_move*/, PieceStand, PieceStand& /*proof_pieces*/) {
      }
      static void setLeaf(const NumEffectState&, Player, PieceStand, PieceStand&) {
      }
      static void clear(PieceStand& /*proof_pieces*/) {
      }
      static void updateMax(PieceStand /*child*/, PieceStand& /*proof_pieces*/) {
      }
      static void addMonopolizedPieces(const NumEffectState&, Player, PieceStand,
				       PieceStand&) {
      }
      static ProofDisproof attackEstimation(const NumEffectState&, Player, King8Info) {
	return ProofDisproof();
      }
    };
    /**
     * 深さ固定で，その深さまで depth first searchで読む詰将棋.
     * 深さ0で詰み状態かどうか(攻め手の手番の場合)，王手をかける手がないかを判定可能
     * 深さ1で通常の一手詰みを判定(攻め手の手番の場合)
     * 使うのは深さ3位まで?
     * NumEffectState専用
     */
    class FixedDepthSearcher
    {
    protected:
      NumEffectState *state;
      int count;
    public:
      FixedDepthSearcher() : state(0), count(0)
      {
      }
      explicit FixedDepthSearcher(NumEffectState& s)
	: state(&s), count(0)
      {
      }
      void setState(NumEffectState& s)
      {
	state = &s;
      }
    private:
      void addCount()
      {
	count++; 
      }
    public:
      int getCount() const
      {
	return count; 
      }
      const PieceStand stand(Player P) const
      {
	return PieceStand(P, *state);
      }
    public:
      // private: Note: helper の都合
      template <Player P, class SetPieces, bool HasGuide>
      const ProofDisproof attack(int depth, Move& best_move, PieceStand& proof_pieces);
      template <Player P, class SetPieces, bool HasGuide>
      const ProofDisproof attackMayUnsafe(int depth, Move& best_move, PieceStand& proof_pieces);
      template <Player P, class SetPieces>
      const ProofDisproof defense(Move last_move,int depth,
				  PieceStand& proof_pieces);
    private:
      /**
       * move を作らずに ProofDisproof の予測を計算する
       */
      template <Player P, class SetPieces>
      const ProofDisproof defenseEstimation(Move last_move, PieceStand& proof_pieces,
					    Piece attacker_piece,
					    Square target_position) const;
    public:      
      template <Player P>
      const ProofDisproof hasCheckmateMove(int depth,Move& best_move)
      {
	PieceStand proof_pieces;
	return attack<P,NoProofPieces,false>(depth, best_move, proof_pieces);
      }
      template <Player P>
      const ProofDisproof hasCheckmateMove(int depth)
      {
	Move checkmate_move;
	return hasCheckmateMove<P>(depth, checkmate_move);
      }
      
      template <Player P>
      const ProofDisproof hasEscapeMove(Move last_move,int depth)
      {
	PieceStand proof_pieces;
	return defense<P,NoProofPieces>(last_move, depth, proof_pieces);
      }
      template <Player P>
      const ProofDisproof hasEscapeByMove(Move next_move, int depth);

      const ProofDisproof hasCheckmateMoveOfTurn(int depth,Move& best_move);
      const ProofDisproof hasEscapeMoveOfTurn(Move last_move,int depth);
      const ProofDisproof hasEscapeByMoveOfTurn(Move next_move, int depth);

      /**
       * 無駄合をなるべく生成しない，合駒生成
       */
      template <Player Defense>
      void generateBlockingWhenLiberty0(Piece defense_king, Square attack_from,
					CheckMoveVector& moves) const;
      template <Player Defense>
      int blockEstimation(Square attack_from, Square defense_king) const;
    };
  } // namespace checkmate
} // namespace osl

#endif /* OSL_CHECKMATE_FIXED_DEPTH_SERCHER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
