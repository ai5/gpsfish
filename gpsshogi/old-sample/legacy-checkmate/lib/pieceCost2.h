/* pieceCost2.h
 */
#ifndef _PIECECOST2_H
#define _PIECECOST2_H
#include "osl/checkmate/pieceCost.h"
namespace osl
{
  namespace checkmate
  {
    struct PieceCost2 : public PieceCost
    {	    
      /** ����¦�� move ���Ф��� cost ��׻����� */
      template <class State>
      static void setAttackCost(Player attacker, const State&, 
				CheckMove& move);
      /** �ɸ�¦�� move ���Ф��� cost ��׻����� */
      template <class State>
      static void setDefenseCost(Player, const State&, CheckMove&)
      {
      }
    };
  }
}

template<typename State> inline
void osl::checkmate::PieceCost2::
setAttackCost(Player attacker, const State& state, CheckMove& move)
{
  const Square from=move.move.from();
  const Square to=move.move.to();
  const Ptype capturePtype = move.move.capturePtype();
  const Player defender = alt(attacker);
  // ����ʳ��ǡ������ǤȤ����ϸ��
  // �����餯��Ƭ�ˤ������ʤ��ɤ��Ǥ��ڤ򥻥åȤˤ���Ȥʤ��ɤ�
  if (capturePtype == PTYPE_EMPTY)
  {
    const int a = (state.countEffect(attacker,to) 
		   + (from.isPieceStand() ? 1 : 0));
    int d = state.countEffect(defender,to);
    if (a <= d)
    {
      const Ptype ptype = move.move.ptype();
      move.cost_proof = attack_sacrifice_cost[ptype] /* *8 */;
      if ((d >= 2) && (a == d))	// �ɲ������Ȥ����������줿��Ȥ�
	move.cost_proof /= 2;
    }
  }
}
#endif /* _PIECECOST2_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
