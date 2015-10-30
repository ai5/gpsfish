#include "twinTable.h"
#include "twinList.h"
#include "checkMoveListProvider.h"
#include "osl/stl/hash_set.h"

namespace osl
{
  namespace checkmate
  {
#ifdef OSL_SMP
    static const int initial_capacity = 0;
#else
    static const int initial_capacity = 100;
#endif

    /**
     * set にいれたデータの一部を変更するための mutable wrapper.
     * set の同一性を path だけで見るのでpath さえ変更しなければ
     * 整合性は保たれる.
     * TODO: 本質的には hash_map 改造クラスを作るべき
     */
    struct TwinAgeMutable
    {
      mutable TwinAgeEntry body;
      TwinAgeMutable(const TwinAgeEntry& e) : body(e)
      {
      }
    };
    
    struct TwinPathHash
    {
      unsigned long operator()(const TwinAgeMutable& e) const
      {
	return e.body.entry.path.getPath();
      }
    };
    struct TwinPathEqual
    {
      bool operator()(const TwinAgeMutable& l, const TwinAgeMutable& r) const
      {
	return l.body.entry.path == r.body.entry.path;
      }
    };

    typedef hash_set<TwinAgeMutable,TwinPathHash,TwinPathEqual> hash_set_t;

    struct TwinTable::Table : public hash_set_t
    {
      explicit Table(size_t capacity) : hash_set_t(capacity)
      {
      }
    };
  } // namespace checkmate
} // namespace osl

osl::checkmate::  
TwinTable::TwinTable() : table(new Table(initial_capacity))
{
}

osl::checkmate::  
TwinTable::~TwinTable()
{
}

void osl::checkmate::
TwinTable::addTwin(const TwinEntry& e)
{
  const TwinAgeEntry ae(e.path, e.move, e.loopTo);
  std::pair<Table::iterator,bool> ib = table->insert(TwinAgeMutable(ae));
  if (! ib.second) 
  {
    ib.first->body.setTwinEntry(e);
  }
}

void osl::checkmate::  
TwinTable::clearTwins()
{
  table->clear();
}

const osl::checkmate::TwinEntry * osl::checkmate::  
TwinTable::findTwin(const PathEncoding& key) const
{
  Table::const_iterator p=table->find(TwinAgeMutable(TwinAgeEntry(key))); 
  if ((p == table->end()) 
      || (! p->body.hasTwinEntry()))
    return 0;
  return &(p->body.entry);
}

osl::checkmate::TwinAgeEntry& osl::checkmate::  
TwinTable::allocateTwin(const PathEncoding& key)
{
  return table->insert(TwinAgeMutable(TwinAgeEntry((key)))).first->body;
}

size_t osl::checkmate::  
TwinTable::size() const
{
  return table->size();
}

/* ------------------------------------------------------------------------- */

osl::checkmate::  
TwinTableHolder::TwinTableHolder() : list_provider(new CheckMoveListProvider())
{
}

osl::checkmate::  
TwinTableHolder::~TwinTableHolder()
{
}

void osl::checkmate::  
TwinTableHolder::clear()
{
  list_provider->clear();
  clearTwins();
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
