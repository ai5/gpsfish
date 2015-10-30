/* checkMoveGenerator.h
 */
#ifndef _CHECKMOVEGENERATOR_H
#define _CHECKMOVEGENERATOR_H

#include "osl/state/numEffectState.h"
#include <cstddef>

/**
 * @def DELAY_INTERPOSE
 * 定義すると必要になるまで中合いを考慮しない.
 * (CheckmateSearcher と CheckMoveGenerator の両方の協調が必要)
 */
#define DELAY_INTERPOSE
/** 
 * @def PAWN_CHECKMATE_SENSITIVE
 * 打歩詰と普通の不詰を区別するときに定義.
 * 定義しないと PawnCheckmate を返すべきところで NoCheckmate が帰ることがある
 * 定義すると，打歩詰になるまで，飛角歩が成らない手は読まない最適化を行う
 * (CheckmateSearcher と CheckMoveGenerator の両方の協調が必要)
 */
#define PAWN_CHECKMATE_SENSITIVE

// /**
//  * @def DELAY_SACRIFICE
//  * 捨駒を後回し.
//  * 調整必要
//  */
// #define DELAY_SACRIFICE

namespace osl
{
  namespace checkmate
  {
    class CheckMoveList;
    class CheckMoveListProvider;
    /** 
     * CheckmateSearcher と OracleProver で共通に使う move generator
     * @param P 攻撃側
     */
    template <Player P>
    struct CheckMoveGenerator
    {
      /** 
       * 防御側の move を生成し out に書き出す 
       * @return 駒を取らない王の移動
       */
      static unsigned int generateEscape(const NumEffectState& state, 
					 CheckMoveListProvider& src,
					 CheckMoveList& out);
      
      /** 
       * 攻撃側の move を生成し out に書き出す.
       * last_move がないと，王手がかかっている状況で安全でない手を作ることがある
       */
      static void generateAttack(const NumEffectState& state, 
				 CheckMoveListProvider& src,
				 CheckMoveList& out,
				 bool& has_pawn_checkmate);
    
    };
  } // namespace checkmate
} // namespace osl

#endif /* _CHECKMOVEGENERATOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
