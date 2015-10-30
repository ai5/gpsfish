/* libertyEstimator.h
 */
#ifndef _LIBERTYESTIMATOR_H
#define _LIBERTYESTIMATOR_H

#include "osl/checkmate/proofNumberTable.h"

namespace osl
{
  namespace checkmate
  {
    struct PureLibertyEstimator
    {
      /** @return move 後の玉のまわりの利きのないマス(の予想) */
      template <class State>
      static void attackH(Player attacker, const State& state, 
			  King8Info info, Move move, 
			  unsigned int& proof_number, 
			  unsigned int& disproof_number)
      {
	const Player defender = alt(attacker);
	const Square king_position = state.kingSquare(defender);
	proof_number = Proof_Number_Table.countLiberty
	  (state, info.libertyCount(), move, king_position, info);
	disproof_number = 1;
      }
      /** @return move 後の玉のまわりの利きのあるマス(の予想) */
      template <class State>
      static void defenseH(Player /*attacker*/, const State&, Move /*move*/, 
			   unsigned int& proof_number, 
			   unsigned int& disproof_number)
      {
	proof_number = 1;
	disproof_number = 1;
      }

    };
    /**
     * 玉の動ける場所を基本としたHの推定.
     * 駒を取る/捨てるなども多少考慮する.
     */
    struct LibertyEstimator
    {

      /** 攻撃側の move に対する proof_number と disproof_number を予想する */
      template <class State>
      static void attackH(Player attacker, const State&, King8Info, Move move, 
			  unsigned int& proof_number, unsigned int& disproof_number);
      /** 防御側の move に対する proof_number と disproof_number を予想する */
      template <class State>
      static void defenseH(Player attacker, const State&, Move move, 
			   unsigned int& proof_number, unsigned int& disproof_number);
    };

  } // namespace checkmate
} // namespace osl


template<typename State>
void osl::checkmate::LibertyEstimator::
attackH(Player attacker, const State& state, King8Info info, Move move, 
	unsigned int& proof_number, unsigned int& disproof_number)
{
  const Player defender = alt(attacker);
  PureLibertyEstimator::attackH
    (attacker, state, info, move, proof_number, disproof_number);

  // 功罪はあるが，速くなる問題の方が多そう
  if (state.hasMultipleEffectAt(defender, move.to()))
    ++proof_number;

  const Square from=move.from();
  const Square to=move.to();
  const int attack_support = state.countEffect(attacker,to);
  const int defense_support = state.countEffect(defender,to);
  if ((attack_support + (from.isPieceStand() ? 1 : 0)) > defense_support)
  {
    /** 効きが上回っていれば先にやってみる */
    disproof_number=2;
  }
  else if (move.capturePtype()!=PTYPE_EMPTY)
  {
    /** 駒を取る */
    Ptype capturePtype=unpromote(move.capturePtype());
    if ((capturePtype == SILVER)
	|| (capturePtype == GOLD))
    {
      disproof_number=2;
    }
    else
    {
      proof_number+=1;
      disproof_number=1;
    }
  }
  else
  {
    proof_number+=1;
    disproof_number=1;
  }
}

template<typename State>
void osl::checkmate::LibertyEstimator::
defenseH(Player attacker, const State& state, Move move, 
		 unsigned int& proof_number, unsigned int& disproof_number)
{
  /** captureは価値が高い */
  if (move.capturePtype()!=PTYPE_EMPTY)
  {
    proof_number=2;
    disproof_number=1;
    return;
  }
  if (move.ptype()==KING)
  {
    proof_number=1;
    disproof_number=1;
    return;
  }
  const Square to = move.to();
  if ((state.countEffect(attacker,to) + (move.isDrop() ? 1 : 0))
      <= state.countEffect(alt(attacker),to))
  {
    proof_number=2;
    disproof_number=1;
    return;
  }
  proof_number=1;
  disproof_number = 2;
}

#endif /* _LIBERTYESTIMATOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
