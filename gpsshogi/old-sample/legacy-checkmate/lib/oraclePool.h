/* oraclePool.h
 */
#ifndef _ORACLEPOOL_H
#define _ORACLEPOOL_H

#include "osl/state/numEffectState.h"
#include "osl/position.h"
#include "osl/pieceStand.h"
#include <boost/scoped_ptr.hpp>
#include <cstddef>

namespace osl
{
  namespace checkmate
  {
    class CheckHashRecord;
    class OraclePool
    {
      class Table;
      std::unique_ptr<Table> proof_oracles;
      std::unique_ptr<Table> disproof_oracles;
    public:
      explicit OraclePool(Player attacker);
      ~OraclePool();

      void addProofOracle(const NumEffectState&, const CheckHashRecord*,
			  int node_count);
      void addDisproofOracle(const NumEffectState&, const CheckHashRecord*,
			     int node_count);
      const CheckHashRecord *findProofOracle(const NumEffectState&, 
					     PieceStand black_stand,
					     unsigned short& oracle_age) const;
      const CheckHashRecord *findDisproofOracle(const NumEffectState&, 
						PieceStand black_stand,
						unsigned short& oracle_age) const;
      size_t totalSize() const;
      size_t keySize() const;
    };

  } // namespace checkmate
} // namespace osl

#endif /* _ORACLEPOOL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
