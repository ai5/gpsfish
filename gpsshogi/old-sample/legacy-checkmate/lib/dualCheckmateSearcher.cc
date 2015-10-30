/* dualCheckmateSearcher.cc
 */
#include "dualCheckmateSearcher.h"
#include "dualCheckmateSearcher.tcc"
#include "dominanceTable.h"
#include "osl/state/numEffectState.h"

namespace osl
{
  namespace checkmate
  {
    // explicit template instantiation
    template class DualCheckmateSearcher<DominanceTable,LibertyEstimator,PieceCost2>;

    template bool DualCheckmateSearcher<DominanceTable,LibertyEstimator,PieceCost2>
    ::isWinningStateByOracle<BLACK>
    (NumEffectState&, const HashKey&, const PathEncoding&, Move&, 
     unsigned short&, int);
    template bool DualCheckmateSearcher<DominanceTable,LibertyEstimator,PieceCost2>
    ::isWinningStateByOracle<WHITE>
    (NumEffectState&, const HashKey&, const PathEncoding&, Move&,
     unsigned short&, int);

    template bool DualCheckmateSearcher<DominanceTable,LibertyEstimator,PieceCost2>
    ::isWinningStateByOracleLastMove<BLACK>
    (NumEffectState&, const HashKey&, const PathEncoding&, Move&, Move,
     unsigned short&, int);
    template bool DualCheckmateSearcher<DominanceTable,LibertyEstimator,PieceCost2>
    ::isWinningStateByOracleLastMove<WHITE>
    (NumEffectState&, const HashKey&, const PathEncoding&, Move&, Move,
     unsigned short&, int);
  }
}

osl::checkmate::
SharedOracles::SharedOracles(Player attacker)
  : oracles(attacker), oracles_last_move(attacker),
    oracles_after_attack(attacker)
{
}

void osl::checkmate::
SharedOracles::showStatus() const
{
  std::cerr << oracles.keySize() << ":"
	    << oracles.totalSize() << " "
	    << oracles_after_attack.keySize() << ":"
	    << oracles_after_attack.totalSize();
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
