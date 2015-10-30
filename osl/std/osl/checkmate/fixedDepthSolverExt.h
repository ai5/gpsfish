/* fixedDepthSolverExt.h
 */
#ifndef OSL_FIXEDDEPTHSOLVEREXT_H
#define OSL_FIXEDDEPTHSOLVEREXT_H
#include "osl/checkmate/fixedDepthSearcher.h"

namespace osl
{
  namespace checkmate
  {
    class FixedDepthSolverExt : public FixedDepthSearcher
    {
    public:
      FixedDepthSolverExt();
      explicit FixedDepthSolverExt(NumEffectState& s);

      /**
       * stateがPから詰む局面かを返す.
       * stateの手番はPと一致しているという前提
       */
      template <Player P>
      const ProofDisproof hasCheckmateMove(int depth, Move& best_move,
					   PieceStand& proof_pieces);
      /**
       * guide を最初に試す．
       * guide.isNormal() である必要はあるが，その局面でvalid でなくても良い
       */
      template <Player P>
      const ProofDisproof hasCheckmateWithGuide(int depth, Move& guide,
						PieceStand& proof_pieces);
      /**
       * stateがPによって詰んでいる局面かを返す.
       * 王手がかかっていない時には呼ばない
       * stateの手番はalt(P)と一致しているという前提
       * stateはPによって王手がかかっているという前提
       * @param last_move 打ち歩詰めの判定に必要
       */
      template <Player P>
      const ProofDisproof hasEscapeMove(Move last_move,int depth,
					PieceStand& proof_pieces);
      /**
       * next_move を指して逃げられるかどうかを調べる
       * @param check_move 詰の場合の攻撃側の指手
       * @param depth next_move を指した後からカウント
       */
      template <Player P>
      const ProofDisproof hasEscapeByMove(Move next_move, int depth,
					  Move& check_move,
					  PieceStand& proof_pieces);
      const ProofDisproof hasCheckmateMoveOfTurn(int depth,Move& best_move,
						 PieceStand& proof_pieces);
      const ProofDisproof hasCheckmateWithGuideOfTurn(int depth, Move& guide,
						      PieceStand& proof_pieces);
      const ProofDisproof hasEscapeByMoveOfTurn(Move next_move, int depth,
						Move& check_move,
						PieceStand& proof_pieces);

      class SetProofPieces;
    };
  }
}


#endif /* _FIXEDDEPTHSOLVEREXT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
