/* generalSimpleHashTable.cc
 */
#include "osl/search/generalSimpleHashTable.h"
#include "osl/hashKey.h"
#include "osl/config.h"
#ifdef OSL_SMP
#  include "osl/misc/lightMutex.h"
#  include <iostream>
#endif
#include <unordered_map>

template <typename Record>
struct osl::container::GeneralSimpleHashTable<Record>::Table 
{
  typedef std::unordered_map<HashKey, Record, std::hash<HashKey>> table_t;
  typedef typename table_t::const_iterator const_iterator;
#ifdef OSL_SMP
  typedef osl::misc::LightMutex Mutex;
  static const unsigned int DIVSIZE=16;
  CArray<Mutex,DIVSIZE> mutex;
#else
  static const unsigned int DIVSIZE=1;
#endif
  CArray<table_t,DIVSIZE> tables;

  /** icc のhash_map がcapacity を持っていなかった気がするので自分で持つ */
  const size_t capacity;
  int num_cache_hit, num_record_after_full;

  Table(size_t c) 
    : capacity(c), num_cache_hit(0), num_record_after_full(0)
  {
  }
  ~Table()
  {
  }
  void clear()
  {
    for(size_t i=0;i<DIVSIZE;i++){
      tables[i].clear();
    }
    num_cache_hit = 0;
    num_record_after_full = 0;
  }
  size_t size() const
  {
    size_t ret=0;
    for(size_t i=0;i<DIVSIZE;i++)
      ret+=tables[i].size();
    return ret;
  }
private:
  Record *findInLock(const HashKey& key,int i)
  {
    typename table_t::iterator pos = tables[i].find(key);
    if (pos == tables[i].end())
      return 0;
#ifndef OSL_USE_RACE_DETECTOR
    ++num_cache_hit;
#endif
    return &pos->second;
  }
  static int keyToIndex(const HashKey& key)
  {
    unsigned long val=key.signature();
    return (val>>24)%DIVSIZE;
  }
public:
  Record *find(const HashKey& key)
  {
    int i=keyToIndex(key);
#if (defined OSL_SMP) && (! defined USE_TBB_HASH)
    SCOPED_LOCK(lk,mutex[i]);
#endif
    return findInLock(key,i);
  }
    
  Record *allocate(const HashKey& key)
  {
    const int i=keyToIndex(key);
#if (defined OSL_SMP) && (! defined USE_TBB_HASH)
    SCOPED_LOCK(lk,mutex[i]);
#endif
    const size_t current_size = tables[i].size();
    if (current_size < capacity/DIVSIZE)
    {
      Record *record = &tables[i].operator[](key);
#ifndef OSL_USE_RACE_DETECTOR
      if (current_size == tables[i].size())
	++num_cache_hit;
#endif
      return record;
    }
    // サイズを増やさないように探す
    Record *result = findInLock(key,i);
    if ((result == 0) && capacity)
    {
#ifdef OSL_SMP
      if (capacity > 10000)
	std::cerr << "table full " << size() << " " << capacity << "\n";
      // SMP環境では全てのthreadに投げる必要がある
      ++num_record_after_full;
      throw TableFull();
#else
      if (num_record_after_full++ == 0) 
	throw TableFull();
#endif
    }
    return result;
  }
};

  
template <typename Record>
osl::container::GeneralSimpleHashTable<Record>::
GeneralSimpleHashTable(size_t capacity) 
  : table(new Table(capacity))
{
}
  
template <typename Record>
osl::container::GeneralSimpleHashTable<Record>::
~GeneralSimpleHashTable() {
}

template <typename Record>
void osl::container::GeneralSimpleHashTable<Record>::
clear()
{
  table->clear();
}

template <typename Record>
Record * 
osl::container::GeneralSimpleHashTable<Record>::
allocate(const HashKey& key)
{
  return table->allocate(key);
}

template <typename Record>
Record * 
osl::container::GeneralSimpleHashTable<Record>::
find(const HashKey& key)
{
  return table->find(key);
}

template <typename Record>
const Record * 
osl::container::GeneralSimpleHashTable<Record>::
find(const HashKey& key) const
{
  return table->find(key);
}

template <typename Record>
size_t osl::container::GeneralSimpleHashTable<Record>::
size() const
{
  return table->size();
}

template <typename Record>
size_t osl::container::GeneralSimpleHashTable<Record>::
capacity() const
{
  return table->capacity;
}

template <typename Record>
int osl::container::GeneralSimpleHashTable<Record>::
numCacheHit() const
{
  return table->num_cache_hit;
}

template <typename Record>
int osl::container::GeneralSimpleHashTable<Record>::
numRecordAfterFull() const
{
  return table->num_record_after_full;
}

template <typename Record>
int osl::container::GeneralSimpleHashTable<Record>::
divSize() const
{
  return Table::DIVSIZE;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
