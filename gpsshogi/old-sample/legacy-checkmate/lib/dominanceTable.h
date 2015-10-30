/* dominanceTable.h
 */
#ifndef _DOMINANCE_TABLE_H
#define _DOMINANCE_TABLE_H

#include "twinTable.h"
#include "visitedCounter.h"
#include "osl/player.h"
#include <boost/scoped_ptr.hpp>

namespace osl
{
  namespace hash
  {
    class HashKey;
  }
  class PathEncoding;
  class PieceStand;
  namespace checkmate
  {
    class CheckHashRecord;
    /**
     * 詰将棋用のテーブル.
     * ArrayCheckHashTable の簡易版
     */
    class DominanceTable : public TwinTableHolder, public VisitedCounter
    {
      class Table;
      std::unique_ptr<Table> table;
      const Player attacker;
      std::unique_ptr<CheckHashRecord> rootNode;
    public:
      typedef hash::HashKey HashKey;

      explicit DominanceTable(Player attacker);
      ~DominanceTable();

      CheckHashRecord *find(const HashKey& key);
      CheckHashRecord *allocate(const HashKey& key, 
				const PieceStand& white_stand,
				const PathEncoding& path);

      void clear();
      CheckHashRecord *root() { return rootNode.get(); }

      const CheckHashRecord *find(const HashKey& key) const;
      size_t size() const;
      Player getAttacker() const { return attacker; }
      void confirmNoVisitedRecords() const;
    };
    
  } // namespace checkmate
} // namespace osl

#endif /* _DOMINANCE_TABLE_H_ */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
