/* immediateCheckmate.h
 */
#ifndef OSL_CHECKMATE_IMMEDIATE_CHECKMATE_H
#define OSL_CHECKMATE_IMMEDIATE_CHECKMATE_H
#include "osl/numEffectState.h"

namespace osl
{
  namespace checkmate
  {
    class ImmediateCheckmate
    {
    private:
      template<Player P,bool setBestMove>
      static bool hasCheckmateDrop(NumEffectState const& state,Square target,
				   King8Info mask,Move& bestMove);

    public:
      template<Player P,bool setBestMove>
      static bool slowHasCheckmateMoveDirPiece(NumEffectState const& state,Square target,
					       King8Info mask,Direction d,Square pos,Piece p,Ptype ptype,Move& bestMove);

      template<Player P,bool setBestMove>
      static bool hasCheckmateMoveDirPiece(NumEffectState const& state,Square target,
					   King8Info mask,Direction d,Square pos,Piece p,Move& bestMove);

      template<Player P,bool setBestMove>
      static bool hasCheckmateMoveDir(NumEffectState const& state,Square target,
				      King8Info mask,Direction d,Move& bestMove);

      template<Player P,bool setBestMove>
      static bool hasCheckmateMove(NumEffectState const& state,Square target,
				   King8Info mask,Move& bestMove);

      /**
       * 一手詰めがある局面かどうか判定(move).
       * 手番の側に王手がかかっている場合は除く
       * 長い利きによる王手は生成しない．
       * pinされている駒の利きがないために詰みになる例も扱わない．
       * @param P(template) - 攻撃側(手番側)のプレイヤー
       * @param state - 局面
       */
      template<Player P>
      static bool hasCheckmateMove(NumEffectState const& state);
      template<Player P>
      static bool hasCheckmateMove(NumEffectState const& state, King8Info);

      /**
       * 一手詰めがある局面かどうか判定(move).
       * 手番の側に王手がかかっている場合は除く
       * 長い利きによる王手は生成しない．
       * pinされている駒の利きがないために詰みになる例も扱わない．
       * @param P(template) - 攻撃側(手番側)のプレイヤー
       * @param state - 局面
       * @param best_move - ある場合に詰めの手を返す
       */
      template<Player P>
      static bool hasCheckmateMove(NumEffectState const& state,Move &bestMove);
      template<Player P>
      static bool hasCheckmateMove(NumEffectState const& state, 
				   King8Info canMoveMask,
				   Square king, Move& bestMove);
      /**
       *
       */
      static bool hasCheckmateMove(Player pl,NumEffectState const& state);
      static bool hasCheckmateMove(Player pl,NumEffectState const& state,Move& bestMove);

    };
  } // namespace checkmate
  using checkmate::ImmediateCheckmate;
} // namespace osl
#endif /* _CHECKMATE_IMMEDIATE_CHECKMATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

