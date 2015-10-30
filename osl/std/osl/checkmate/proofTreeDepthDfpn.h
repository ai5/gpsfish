/* proofTreeDepthDfpn.h
 */
#ifndef OSL_PROOFTREEDEPTHDFPN_H
#define OSL_PROOFTREEDEPTHDFPN_H

#include "osl/numEffectState.h"
#include "osl/hashKey.h"
#include <vector>

namespace osl
{
  namespace checkmate
  {
    class CheckHashRecord;
    class DfpnTable;
    /**
     * 詰までの手数を数える.
     * 詰将棋ルーチン次第で，無駄合なども含まれるため
     * 人間の感覚と一致するとは限らない．
     */
    class ProofTreeDepthDfpn
    {
      struct Table;
      std::unique_ptr<Table> table;
    public:
      explicit ProofTreeDepthDfpn(const DfpnTable& table);
      ~ProofTreeDepthDfpn();
      int depth(const HashKey& key, const NumEffectState& state, bool is_or_node) const;

      void retrievePV(const NumEffectState& state, bool is_or_node,
		      std::vector<Move>& pv) const;
    private:
      int orNode(const HashKey& key, Move& best_move, int height=0) const;
      int andNode(const HashKey& key, Move& best_move, int height=0) const;
    };
  } // namespace checkmate
}


#endif /* OSL_PROOFTREEDEPTHDFPN_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
