/* simpleCheckHashTable.h
 */
#ifndef _SIMPLE_CHECK_HASHTABLE_H
#define _SIMPLE_CHECK_HASHTABLE_H

#include "checkHashRecord.h"
#include "twinTable.h"
#include "visitedCounter.h"
#include <boost/scoped_ptr.hpp>

namespace osl
{
  namespace hash
  {
    class HashKey;
  }

  namespace checkmate
  {
    using hash::HashKey;
    /**
     * 詰将棋用のテーブル.
     * DominanceTable, ArrayCheckHashTable と違って類似局面の利用はできない
     */
    class SimpleCheckHashTable : public TwinTableHolder, public VisitedCounter
    {
      class Table;
      std::unique_ptr<Table> table;
      const Player attacker;
      CheckHashRecord rootNode;
    public:
      explicit SimpleCheckHashTable(Player attacker);
      ~SimpleCheckHashTable();

      CheckHashRecord *find(const HashKey& key);
      CheckHashRecord *allocate(const HashKey& key, 
				const PieceStand& white_stand,
				const PathEncoding&);

      void clear();
      CheckHashRecord *root() { return &rootNode; }

      const CheckHashRecord *find(const HashKey& key) const;
      size_t size() const;
      Player getAttacker() const { return attacker; }
      void confirmNoVisitedRecords() const;
    };
    
  } // namespace checkmate
} // namespace osl

#endif /* _SIMPLE_CHECK_HASHTABLE_H_ */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
