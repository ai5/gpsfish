#include "simpleCheckHashTable.h"
#include "checkHashRecord.h"
#include "osl/stl/hash_map.h"
#include "osl/hash/hashKey.h"
#include <algorithm>
#include <iostream>

#ifndef MINIMAL
namespace osl
{
  namespace checkmate
  {
    static const int initial_capacity = 1000000;
    typedef hash_map<HashKey,CheckHashRecord> hash_map_t;
    struct SimpleCheckHashTable::Table : public hash_map_t
    {
      explicit Table(size_t capacity) : hash_map_t(capacity)
      {
      }
    };
  
    SimpleCheckHashTable::SimpleCheckHashTable(Player a)
      : table(new Table(initial_capacity)), attacker(a)
    {
    }
    SimpleCheckHashTable::~SimpleCheckHashTable()
    {
#ifdef CHECKMATE_DEBUG
      confirmNoVisitedRecords();
#endif
    }

    CheckHashRecord * SimpleCheckHashTable::find(const HashKey& key)
    {
      Table::iterator it=table->find(key);
      if (it==table->end()) 
	return 0;
      return &it->second;
    }
  
    CheckHashRecord * SimpleCheckHashTable::
    allocate(const HashKey& key, const PieceStand& white_stand,
	     const PathEncoding&)
    {
      CheckHashRecord *result = &(*table)[key];
      if ((result->stand(BLACK) != key.blackStand())
	  || (result->stand(WHITE) != white_stand))
	*result = CheckHashRecord(key.blackStand(), white_stand);
      return result;
    }

    void SimpleCheckHashTable::clear(){
      TwinTableHolder::clear();
      table->clear();
    }

    const CheckHashRecord * SimpleCheckHashTable::find(const HashKey& key) const
    {
      Table::const_iterator it=table->find(key);
      if (it==table->end()) 
	return 0;
      return &it->second;
    }
    size_t SimpleCheckHashTable::size() const{
      return table->size();
    }
    void SimpleCheckHashTable::confirmNoVisitedRecords() const
    {
      size_t visited = 0;
      for (Table::const_iterator p=table->begin(); p!=table->end(); ++p)
      {
	if (p->second.isVisited)
	{
	  ++visited;
	  std::cerr << p->first << " " << &p->second << "\n";
	}
      }
      if (visited)
      {
	std::cerr << "SimpleCheckHashTable::confirmNoVisitedRecords " 
		  << visited << " visited nodes found\n";
      }
    }
  } // namespace checkmate
} // namespace osl
#endif
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
