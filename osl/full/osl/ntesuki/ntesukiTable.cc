/* ntesukiTable.cc
 */
#include "osl/ntesuki/ntesukiTable.h"
#include "osl/ntesuki/ntesukiTable.tcc"
#include "osl/ntesuki/ntesukiRecord.h"
#include "osl/ntesuki/ntesukiSearcher.h"
#include "osl/state/numEffectState.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/record/csaRecord.h"
#ifdef NDEBUG
# include "osl/ntesuki/ntesukiRecord.tcc"
#endif
#include <iostream>
#include <queue>
#include <set>

int osl::ntesuki::NtesukiTable::Table::
largeGCCount = 0;

osl::ntesuki::NtesukiTable::Table::
Table(unsigned int c,
      unsigned int g,
      bool v)
  : ntesuki_hash_map(c), capacity(c), default_gc_size(g),
    verbose(v), no_gc(false), gc_request(false),
    numEntry(0), numCacheHit(0), gcCount(0)
{
}

osl::ntesuki::NtesukiTable::Table::
~Table()
{
}
  
osl::ntesuki::NtesukiRecord *
osl::ntesuki::NtesukiTable::Table::
allocate(const HashKey& key,
	 const PieceStand& white_stand,
	 signed short distance)
{
  int full = 0;
 reallocate:
  if (numEntry < capacity || no_gc)
  {
    if (numEntry >= capacity) gc_request = true;
    NtesukiRecord::RecordList *record_list =
      &(ntesuki_hash_map::operator[](key.getSignatureKey()));
    for (NtesukiRecord::RecordList::iterator p = record_list->begin();
	 p != record_list->end(); ++p)
    {
      if (p->key.getPieceStand() == key.getPieceStand())
      {
	++numCacheHit;
	return &(*p);
      }
    }
    
    ++numEntry;
    const NtesukiRecord r(distance, key, white_stand, record_list);
    record_list->push_front(r);
    NtesukiRecord *result = &(record_list->front());
    return result;
  }
  
  NtesukiRecord *result = find(key);
  if (result == 0 && full++ == 0 && capacity)
  {
    if (default_gc_size > 0)
    {
      collectGarbage(default_gc_size);
      goto reallocate;
    }
  }
  return result;
}

struct CompareChildSize
{
  bool operator()(const osl::ntesuki::NtesukiRecord *lhs,
		  const osl::ntesuki::NtesukiRecord *rhs)
  {
    return lhs->getChildCount() < rhs->getChildCount();
  }
};

struct RecordPrinter
{
  /* Prints each node.
   * Leaf nodes are not printed.
   */
  osl::state::NumEffectState &state;
  osl::ntesuki::NtesukiTable::Table &table;
  std::vector<osl::ntesuki::NtesukiRecord*> records;
  std::set<HashKey> read_keys;
  int depth, pass_count, pass_depth;

  RecordPrinter(osl::state::NumEffectState &s,
		osl::ntesuki::NtesukiTable::Table &t,
		osl::ntesuki::NtesukiRecord *r)
    : state(s), table(t),
      depth(-1), pass_count(0)
  {
  }

  void enter(osl::ntesuki::NtesukiRecord *r)
  {
    read_keys.insert(r->key);
    records.push_back(r);
    ++depth;
  }
  void exit()
  {
    read_keys.erase(records.back()->key);
    records.pop_back();
    if (pass_depth) pass_count--;
    --depth;
  }
  bool withChildMove(const osl::ntesuki::NtesukiMove& move,
		     osl::ntesuki::NtesukiRecord *child)
  {
    if (move.isPass())
    {
      if (pass_count) return false;
      pass_count++;
      pass_depth = depth;
    }

    if (read_keys.find(child->key) != read_keys.end())
    {
      return false;
    }

    int i = 0;
    for (; i <= depth; i++) std::cerr << "*";
    std::cerr << record::csa::show(move.getMove());
    for (; i <= 11; i++) std::cerr << " ";

    if (child->isVisited()) std::cerr << "(*)";
    if (depth > 8)
    {
      std::cerr << "\t" << child->getChildCount() << "\t"
		<< child->getValue<BLACK>(0) << " \t"
		<< child->getValue<BLACK>(1) << " |\n";
      return false;
    }
    else
    {
      std::cerr << "\t" << child->getChildCount() << "\t"
		<< child->getValue<BLACK>(0) << " \t"
		<< child->getValue<BLACK>(1) << "\n";
    }
    return true;
  }

  void noChildMove(const osl::ntesuki::NtesukiMove& move)
  {
  }

  bool operator()(const osl::ntesuki::NtesukiMove& lhs,
		  const osl::ntesuki::NtesukiMove& rhs)
  {
    osl::ntesuki::NtesukiRecord *record = records.back();
    ntesuki_assert(record);
    NtesukiRecord *lchild = table.find(record->key.newHashWithMove(lhs.getMove()));
    if (!lchild) return false;
    NtesukiRecord *rchild = table.find(record->key.newHashWithMove(rhs.getMove()));
    if (!rchild) return true;

    if (lchild->isVisited()) return true;
    if (rchild->isVisited()) return false;
    return lchild->getChildCount() > rchild->getChildCount();
  }
  struct Compare
  {
    bool operator()(const osl::ntesuki::NtesukiMove& lhs,
		    const osl::ntesuki::NtesukiMove& rhs)
    {
      return lhs.getMove() < rhs.getMove();
    }
  };
};

struct RecordPrinter2
{
  /* Prints each node.
   * Leaf nodes are printed as well.
   */

  osl::state::NumEffectState &state;
  osl::ntesuki::NtesukiTable::Table &table;
  std::vector<osl::ntesuki::NtesukiRecord*> records;
  std::set<HashKey> read_keys;
  int depth, pass_count, pass_depth, depth_visited;

  RecordPrinter2(osl::state::NumEffectState &s,
		 osl::ntesuki::NtesukiTable::Table &t,
		 osl::ntesuki::NtesukiRecord *r)
    : state(s), table(t),
      depth(-1), pass_count(0), pass_depth(0), depth_visited(0)
  {
  }

  void enter(osl::ntesuki::NtesukiRecord *r)
  {
    if (r->isVisited()) ++depth_visited;
    read_keys.insert(r->key);
    records.push_back(r);
    ++depth;
  }
  void exit()
  {
    read_keys.erase(records.back()->key);
    records.pop_back();
    if (pass_depth) pass_count--;
    --depth;
  }
  bool withChildMove(const osl::ntesuki::NtesukiMove& move,
		     osl::ntesuki::NtesukiRecord *child)
  {
    if (move.isPass())
    {
      if (pass_count) return false;
      pass_count++;
      pass_depth = depth;
    }

    bool result = true;

    if (depth < depth_visited)
    {
      return false;
    }

    int i = 0;
    for (; i <= depth; i++) std::cerr << "*";
    for (; i <= 10; i++) std::cerr << " ";
    std::cerr << record::csa::show(move.getMove());
    if (child->isVisited()) std::cerr << "(*)";
    if (read_keys.find(child->key) != read_keys.end())
    {
      std::cerr << "loop";
      result = false;
    }

    std::cerr << "\t" << child->getChildCount() << "\t"
	      << child->getValue<osl::BLACK>(0) << "\t"
	      << child->getValue<osl::BLACK>(1) << "\n";

    return result;
  }

  void noChildMove(const osl::ntesuki::NtesukiMove& move)
  {
    if (depth < depth_visited)
    {
      return;
    }
    int i = 0;
    for (; i <= depth; i++) std::cerr << "*";
    for (; i <= 10; i++) std::cerr << " ";
    std::cerr << record::csa::show(move.getMove())
	      << "|\t"
	      << move.h_a_proof << "/"  << move.h_a_disproof << "\t"
	      << move.h_d_proof << "/"  << move.h_d_disproof
	      << "\n";
  }

  bool operator()(const osl::ntesuki::NtesukiMove& lhs,
		  const osl::ntesuki::NtesukiMove& rhs)
  {
    osl::ntesuki::NtesukiRecord *record = records.back();
    ntesuki_assert(record);
    NtesukiRecord *lchild = table.find(record->key.newHashWithMove(lhs.getMove()));
    if (!lchild) return false;
    NtesukiRecord *rchild = table.find(record->key.newHashWithMove(rhs.getMove()));
    if (!rchild) return true;

    if (lchild->isVisited()) return true;
    if (rchild->isVisited()) return false;
    return lchild->getChildCount() > rchild->getChildCount();
  }

  struct Compare
  {
    bool operator()(const osl::ntesuki::NtesukiMove& lhs,
		    const osl::ntesuki::NtesukiMove& rhs)
    {
      return lhs.getMove() < rhs.getMove();
    }
  };
};

struct
MarkAndSweep
{
  osl::state::NumEffectState &state;
  osl::ntesuki::NtesukiTable::Table &table;
  std::set<HashKey> reachable_keys;
  int depth;

  MarkAndSweep(osl::state::NumEffectState &s,
	       osl::ntesuki::NtesukiTable::Table &t,
	       osl::ntesuki::NtesukiRecord *r)
    : state(s), table(t)
  {
  }

  ~MarkAndSweep()
  {
    typedef std::vector<HashKey> keys_t;
    keys_t garbage_list;
    for (osl::ntesuki::NtesukiTable::Table::iterator it = table.begin();
	 it != table.end(); ++it)
    {
      for (NtesukiRecord::RecordList::iterator p = it->second.begin();
	   p != it->second.end(); ++p)
      {
	NtesukiRecord *record = &(*p);
	if (reachable_keys.find(record->key) == reachable_keys.end())
	{
	  garbage_list.push_back(record->key);
	}
	else
	{
	  reachable_keys.erase(record->key);
	}
      }
    }
    for (keys_t::iterator it = garbage_list.begin();
	 it != garbage_list.end(); ++it)
    {
      table.erase(*it);
    }
  }
  void enter(osl::ntesuki::NtesukiRecord *r)
  {
    reachable_keys.insert(r->key);
  }
  void exit()
  {
  }

  bool withChildMove(const osl::ntesuki::NtesukiMove& move,
		     osl::ntesuki::NtesukiRecord *child)
  {
    return reachable_keys.find(child->key) == reachable_keys.end();
  }

  void noChildMove(const osl::ntesuki::NtesukiMove& move)
  {
  }

  struct Compare
  {
    bool operator()(const osl::ntesuki::NtesukiMove& lhs,
		    const osl::ntesuki::NtesukiMove& rhs)
    {
      return lhs.getMove() < rhs.getMove();
    }
  };
};

void
osl::ntesuki::NtesukiTable::Table::
collectGarbage(unsigned int gc_size)
{
  if (gc_size == 0)
  {
    throw TableFull();
  }
  if (largeGCCount >= 0 && 
      gcCount >= static_cast<unsigned int>(largeGCCount))
  {
    if (verbose)
    {
      std::cerr << "\ntoo many GCs, failing\n\n";
      //forEachRecordFromRoot<RecordPrinter>();//DEBUG
    }
    throw TableFull();
  }
  ++gcCount;
#if 0
  const unsigned int garbage_size = numEntry - gc_size;
  if (verbose)
  {
    std::cerr << "GC: ";
    for (int i = 0; i < 2; i++)
    {
      std::cerr << root->getValue<BLACK>(i) << "\t"
		<< root->getValue<WHITE>(i) << "\t";
    }
    std::cerr << " " << numEntry << " -> ";
  }

  std::priority_queue<NtesukiRecord *,
    std::vector<NtesukiRecord *>,
    CompareChildSize> garbage_list;

  for (iterator it = begin(); it != end(); ++it)
  {
    for (NtesukiRecord::RecordList::iterator p = it->second.begin();
	 p != it->second.end(); ++p)
    {
      NtesukiRecord *record = &(*p);
      if (record->isVisited()
	  //|| record->isFinal()
	  )
      {
	record->addChildCount(garbage_size);
	continue;
      }

      garbage_list.push(record);
      if (garbage_list.size() > garbage_size)
      {
	garbage_list.pop();
      }
    }
  }

  std::cerr << "\n*before GC\n";//DEBUG
  //forEachRecordFromRoot<RecordPrinter>();//DEBUG
  forEachRecordFromRoot<RecordPrinter2>();//DEBUG

  while (!garbage_list.empty())
  {
    NtesukiRecord *garbage = garbage_list.top();
    erase(garbage->key);
    garbage_list.pop();
  }
  
  if (verbose)
  {
    std::cerr << numEntry << "\n";
  }
  std::cerr << "*after GC\n";//DEBUG
  forEachRecordFromRoot<RecordPrinter2>();//DEBUG
  //forEachRecordFromRoot<RecordPrinter>();//DEBUG
  //throw TableFull();//DEBUG
#else
  if (verbose)
  {
    std::cerr << "GC: ";
    for (int i = 0; i < 2; i++)
    {
      std::cerr << root->getValue<BLACK>(i) << "\t"
		<< root->getValue<WHITE>(i) << "\t";
    }
    std::cerr << "\n";
  }
  //std::cerr << "\n*before GC\n";//DEBUG
  //forEachRecordFromRoot<RecordPrinter>();//DEBUG
  //forEachRecordFromRoot<RecordPrinter2>();//DEBUG

  unsigned int orig_size = numEntry;
  unsigned int small_subtree_size = 1,
    garbage_size = 0;
  while (numEntry > gc_size)
  {
    for (iterator list_it = begin();
	 list_it != end(); ++list_it)
    {
      /* テーブルに登録された record の list について，
       * リストの各要素の subtree のサイズを見て，必要なら collect する.
       * ここでの list は所謂 same board list.
       */

      /* 対象とする record が削除された場合には，その record への iterator は無効になるので，
       * 先頭から調べ直す.
       */
    retry_collect_list:
      for (NtesukiRecord::RecordList::iterator r = list_it->second.begin();
	   r != list_it->second.end(); ++r)
      {
	NtesukiRecord *record = &(*r);
	
	if (record->isVisited())
	{
	  continue;
	}
	if (record->getChildCount() < small_subtree_size
	    && record->rev_refcount == 0)
	{
	  ++garbage_size;
	  --numEntry;
	  for (NtesukiRecord::RecordPList::const_iterator pit = record->parents.begin();
	       pit != record->parents.end(); ++pit)
	  {
	    (*pit)->rev_refcount--;
	  }
	  list_it->second.erase(r); /* この時点で r は無効 */
	  goto retry_collect_list;
	}
      }
    }
#ifdef MARK_AND_SWEEP_AFTER_SMALLTREEGC
    unsigned int before_mark_and_sweep = numEntry;
    forEachRecordFromRoot<MarkAndSweep>();
    garbage_size += before_mark_and_sweep - numEntry;
#endif
    //std::cerr << small_subtree_size << "\t" << garbage_size << "\t" << numEntry << "\n";//DEBUG
    small_subtree_size += 4;
  }
  small_subtree_size -= 4;
  
  for (iterator it = begin(); it != end(); ++it)
  {
    for (NtesukiRecord::RecordList::iterator p = it->second.begin();
	 p != it->second.end(); ++p)
    {
      NtesukiRecord *record = &(*p);
      if (record->isVisited())
      {
	record->addChildCount(garbage_size);
      }
    }
  }

  if (verbose)
  {
    std::cerr << numEntry << "\tcollected up to " << small_subtree_size << "\n";
    std::cerr << " " << orig_size << " -> " << numEntry << "\n";
  }
  //std::cerr << "*after GC\n";//DEBUG
  //forEachRecordFromRoot<RecordPrinter2>();//DEBUG
  //forEachRecordFromRoot<RecordPrinter>();//DEBUG
  //if (largeGCCount >= 0 && gc_count >= largeGCCount) throw TableFull();//DEBUG
  //throw TableFull();//DEBUG
#endif
}

osl::ntesuki::NtesukiRecord *
osl::ntesuki::NtesukiTable::Table::
find(const HashKey& key)
{
  ntesuki_hash_map::iterator hit =
    ntesuki_hash_map::find(key.getSignatureKey());
  if (hit == ntesuki_hash_map::end())
  {
    return 0;
  }
  for (NtesukiRecord::RecordList::iterator p = hit->second.begin();
       p != hit->second.end(); ++p)
  {
    if (p->key.getPieceStand() == key.getPieceStand())
    {
      ++numCacheHit;
      return &(*p);
    }
  }
  return 0;
}

void
osl::ntesuki::NtesukiTable::Table::
erase(const HashKey key)
{
  ntesuki_hash_map::iterator hit =
    ntesuki_hash_map::find(key.getSignatureKey());
  if (hit == ntesuki_hash_map::end())
  {
    return;
  }
  for (NtesukiRecord::RecordList::iterator p = hit->second.begin();
       p != hit->second.end(); ++p)
  {
    if (p->key.getPieceStand() == key.getPieceStand())
    {
      hit->second.erase(p);
      --numEntry;
      return;
    }
  }
}

osl::ntesuki::NtesukiTable::
NtesukiTable(unsigned int capacity,
	     unsigned int gc_size,
	     bool verbose)
  : table(new Table(capacity, gc_size, verbose)),
    verbose(verbose),
    depths(200,0)
 {}

osl::ntesuki::NtesukiTable::
~NtesukiTable()
{
  //forEachRecordFromRoot<RecordPrinter>();//DEBUG

  if (verbose)
  {
    std::cerr << "~NtesukiTable size " << size()
	      << ", cache hit " << table->numCacheHit
	      << ", table full " << table->gcCount << "\n";

    ntesuki_hash_map::const_iterator hit = this->begin();
    while (hit != this->end())
    {
      const NtesukiRecord::RecordList& record_list = hit->second;
      for (NtesukiRecord::RecordList::const_iterator p = record_list.begin();
	   p != record_list.end(); ++p)
      {
	const NtesukiRecord& r = *p;
	const unsigned short d = r.distance;
	if (d >= 200)
	{
	  std::cerr << d << r << "\n";
	}
	depths[d]++;
      }
      hit++;
    }

    for (int depth = 0; depth < 200; depth++)
    {
      if (depths[depth] != 0)
      {
	std::cerr << "depth: " << depth << " " << depths[depth] << "\n";
      }
    }
  }
}

bool osl::ntesuki::NtesukiTable::
isVerbose() const
{
  return verbose;
}
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
