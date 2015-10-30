/* nullEstimator.h
 */
#ifndef _NULLESTIMATOR_H
#define _NULLESTIMATOR_H

#include "osl/checkmate/king8Info.h"
#include "osl/move.h"
namespace osl
{
  namespace checkmate
  {
    /** plain df-pn のためのHの予想 */
    struct NullEstimator
    {
      /** 攻撃側の move に対する proof_number と disproof_number を予想する */
      template <class State>
      static void attackH(Player, const State&, King8Info, Move, 
			  unsigned int& proof_number, unsigned int& disproof_number)
      {
	proof_number = 1;
	disproof_number = 1;
      }
      
      /** 防御側の move に対する proof_number と disproof_number を予想する */
      template <class State>
      static void defenseH(Player attacker, const State&, Move move, 
			   unsigned int& proof_number, unsigned int& disproof_number)
      {
	proof_number = 1;
	disproof_number = 1;
      }
    };
  } // namespace checkmate
} // namespace osl

#endif /* _NULLESTIMATOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
