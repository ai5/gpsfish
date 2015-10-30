/* dominanceTable.cc
 */
#include "dominanceTable.h"
#include "checkHashRecord.h"
#include "sameBoardList.h"
#include "osl/stat/histogram.h"
#include "osl/hash/hashKey.h"
#include "osl/stl/hash_map.h"
#include <algorithm>
#include <iostream>

struct osl::checkmate::DominanceTable::Table : public hash_map<BoardKey,checkmate::SameBoardList>
{
#ifdef OSL_SMP
  static const int initial_capacity = 100;
#else
  static const int initial_capacity = 1000000;
#endif
  typedef hash_map<BoardKey,checkmate::SameBoardList> hash_map_t;

  explicit Table(size_t capacity) : hash_map_t(capacity)
  {
  }
};

osl::checkmate::  
DominanceTable::DominanceTable(Player a)
  : table(new Table(Table::initial_capacity)), attacker(a), 
    rootNode(new CheckHashRecord)
{
}
osl::checkmate::  
DominanceTable::~DominanceTable()
{
#ifdef CHECKMATE_DEBUG
  confirmNoVisitedRecords();
#endif
#ifdef DOMINANCETABLE_SHOW_STAT
  if (! table->empty())
  {
    stat::Histogram h(1,100);
    for (Table::iterator p=table->begin(); p!=table->end(); ++p)
    {
      h.add(p->second.size());
    }
    h.show(std::cerr);
  }
#endif
}

osl::checkmate::CheckHashRecord * osl::checkmate::  
DominanceTable::find(const HashKey& key)
{
  Table::iterator p=table->find(key.boardKey());
  if (p==table->end()) 
    return 0;
  return p->second.find(key.blackStand());
}
  
osl::checkmate::CheckHashRecord * osl::checkmate::  
DominanceTable::allocate(const HashKey& key, const PieceStand& white_stand,
			 const PathEncoding& path)
{
  size_t count = 0;
  SameBoardList& l = (*table)[key.boardKey()];
  if (attacker == BLACK)
    return l.allocate<BLACK>(key.blackStand(), white_stand, path, count);
  else
    return l.allocate<WHITE>(key.blackStand(), white_stand, path, count);
}

void osl::checkmate::  
DominanceTable::clear()
{
  TwinTableHolder::clear();
  table->clear();
}

const osl::checkmate::CheckHashRecord * osl::checkmate::  
DominanceTable::find(const HashKey& key) const
{
  Table::const_iterator p=table->find(key.boardKey());
  if (p==table->end()) 
    return 0;
  return p->second.find(key.blackStand());
}
size_t osl::checkmate::  
DominanceTable::size() const
{
  return table->size();
}

void osl::checkmate::  
DominanceTable::confirmNoVisitedRecords() const
{
  int visited = 0;
  for (Table::const_iterator p=table->begin(); p!=table->end(); ++p)
  {
    const int curVisited = p->second.confirmNoVisitedRecords();
#ifdef CHECKMATE_EXTRA_DEBUG
    if (curVisited)
      std::cerr << p->first.getSignature()
		<< " " << p->first.getBoardKey()[0] 
		<< "\n";
#endif
    visited += curVisited;
  }
  if (visited != countVisited())
  {
    // rev. 1705 以降，王手の連続の棋譜を渡した場合は
    // 詰将棋の外からisVisitedを書き込んだ局面が残っている．
    std::cerr << "DominanceTable::confirmNoVisitedRecords " 
	      << visited << " visited nodes found\n";
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
