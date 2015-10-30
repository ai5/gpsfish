/* nullCost.h
 */
#ifndef _NULLCOST_H
#define _NULLCOST_H

#include "osl/player.h"
namespace osl
{
  namespace checkmate
  {
    struct CheckMove;
    /** plain df-pn のためのCostの予想 */
    struct NullCost
    {
      /** 攻撃側の move に対する cost を計算する */
      template <class State>
      static void setAttackCost(Player, const State&, CheckMove&)
      {
      }
      /** 防御側の move に対する cost を計算する */
      template <class State>
      static void setDefenseCost(Player, const State&, CheckMove&)
      {
      }
    };
  } // namespace checkmate
} // namespace osl


#endif /* _NULLCOST_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
