/* simpleHashTable.cc
 */
#include "osl/search/simpleHashTable.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/search/analyzer/recordSet_.h"
#include "osl/search/generalSimpleHashTable.tcc"
#include <iostream>

namespace osl
{
  template class container::GeneralSimpleHashTable <SimpleHashRecord>;
} // namespace osl
  
osl::search::SimpleHashTable::
SimpleHashTable(size_t capacity, int minimum_recordlimit, int v) 
  : GeneralSimpleHashTable<SimpleHashRecord>(capacity),
    minimum_limit(minimum_recordlimit), verbose(v)
{
}

uint64_t osl::search::SimpleHashTable::
memoryUse() const
{
  return 1ull * (sizeof(SimpleHashRecord)+sizeof(HashKey)+sizeof(SimpleHashRecord*)) * size();
}

osl::search::SimpleHashTable::
~SimpleHashTable() 
{
  const uint64_t memory_use = memoryUse();
  if ((verbose > 1 && size()) || memory_use > (1024*(1ull<<20)))
  {
    std::cerr << "SimpleHashTable size: " << size() << " ("
	      << memory_use / (1ull<<20) << "MB)"
	      << ", cache hit " << table->num_cache_hit
	      << ", table full " << table->num_record_after_full << "\n";
  }
}

void osl::search::SimpleHashTable::
setVerbose(int v)
{
  verbose = v;
}

void osl::search::SimpleHashTable::
setMinimumRecordLimit(int new_limit)
{
  minimum_limit = new_limit;
}

int osl::search::SimpleHashTable::
minimumRecordLimit() const
{
  return minimum_limit;
}

osl::search::SimpleHashRecord * 
osl::search::SimpleHashTable::
allocate(const HashKey& key, int limit)
{
  if (limit < minimumRecordLimit())
    return find(key);
  return GeneralSimpleHashTable <SimpleHashRecord>::allocate (key);
}

int osl::search::SimpleHashTable::
verboseLevel() const
{
  return verbose;
}

bool osl::search::SimpleHashTable::
isConsistent() const
{
  return true;
}

int osl::search::SimpleHashTable::
divSize() const
{
  return GeneralSimpleHashTable<SimpleHashRecord>::divSize();
}

#ifndef MINIMAL
void osl::search::SimpleHashTable::
getPV(const HashKey& root, MoveVector& out, size_t *quiesce_start) const
{
  analyzer::RecordSet dejavu;
  HashKey key = root;
  const SimpleHashRecord *record;
  while (true) {
    record = table->find(key);
    if (! record || dejavu.find(record) != dejavu.end()) {
      break;
    }
    const Move best_move = record->bestMove().move();
    if (best_move.isInvalid()) {
      break;
    }
    dejavu.insert(record);
    out.push_back(best_move);
    key = key.newHashWithMove(best_move);
  }
  if (! quiesce_start || ! record)
    return;
  *quiesce_start = out.size();
  while (true) {
    const Move best_move = record->qrecord.bestMove();
    if (best_move.isInvalid()) {
      break;
    }
    out.push_back(best_move);

    key = key.newHashWithMove(best_move);
    record = table->find(key);
    if (! record || dejavu.find(record) != dejavu.end()) {
      break;
    }
    dejavu.insert(record);
  }
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
