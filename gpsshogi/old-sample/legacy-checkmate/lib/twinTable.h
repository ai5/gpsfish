/* twinTable.h
 */
#ifndef _TWIN_HASHTABLE_H
#define _TWIN_HASHTABLE_H

#include "twinEntry.h"
#include <boost/scoped_ptr.hpp>

namespace osl
{
  class PathEncoding;

  namespace checkmate
  {
    class CheckMoveListProvider;
    /**
     * 詰将棋のGHI対策用テーブル.
     * TwinListに入れなかったものが入る
     */
    class TwinTable
    {
      class Table;
      std::unique_ptr<Table> table;
    public:
      TwinTable();
      ~TwinTable();

      void addTwin(const TwinEntry&);
      void clearTwins();

      const TwinEntry *findTwin(const PathEncoding&) const;
      TwinAgeEntry& allocateTwin(const PathEncoding&);
      size_t size() const;
    };

    class TwinTableHolder : private TwinTable
    {
      std::unique_ptr<CheckMoveListProvider> list_provider;
    protected:
      ~TwinTableHolder();
    public:
      TwinTableHolder();
      using TwinTable::addTwin;
      using TwinTable::clearTwins;
      using TwinTable::findTwin;
      using TwinTable::size;
      using TwinTable::allocateTwin;

      const TwinTable& getTwinTable() const { return *this; }
      void addLoopDetection(const PathEncoding& path, const CheckMove& move,
			    const CheckHashRecord *loopTo)
      {
	addTwin(TwinEntry(path, move, loopTo));
      }
      void addLoopDetection(const PathEncoding& path, const CheckHashRecord *loopTo=0)
      {
	addTwin(TwinEntry(path, CheckMove(), loopTo));
      }
      CheckMoveListProvider& listProvider() { return *list_provider; }
      void clear();
    };
    
    
  } // namespace checkmate
} // namespace osl

#endif /* _TWIN_HASHTABLE_H_ */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
