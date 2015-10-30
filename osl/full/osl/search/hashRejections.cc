/* hashRejections.cc
 */
#include "osl/search/hashRejections.h"
#include <unordered_map>

struct osl::search::HashRejections::Table
{
  struct Entry
  {
    PieceStand black_stand;    
  };
  typedef std::unordered_map<BoardKey, Entry, std::hash<BoardKey>> table_t;
  table_t table;
};

struct osl::search::HashRejections::RootTable
{
  struct Entry
  {
    PieceStand black_stand;
    HashKey parent;
  };
  typedef std::unordered_map<BoardKey, Entry, std::hash<BoardKey>> table_t;
  table_t table;
};

osl::search::HashRejections::
HashRejections() : root_table(new RootTable), table(new Table)
{
}
osl::search::HashRejections::
HashRejections(const HashRejections& src) : root_table(src.root_table), table(new Table(*src.table))
{
}
osl::search::
HashRejections::~HashRejections()
{
}
osl::search::HashRejections&
osl::search::HashRejections::
operator=(const HashRejections& src)
{
  if (this != &src) {
    root_table = src.root_table;
    table.reset();
    table.reset(new Table(*src.table));
  }
  return *this;
}

void osl::search::
HashRejections::addRejectionRoot(const NumEffectState& parent, const HashKey& key, Move move)
{
  MoveVector moves;
  parent.generateLegal(moves);

  assert(HashKey(parent) == key);
  for (Move m: moves) {
    if (m == move)
      continue;
    const HashKey new_key = key.newHashWithMove(m);
    RootTable::Entry& e = root_table->table[new_key.boardKey()];
    e.parent = key;
    e.black_stand = new_key.blackStand();
  }
}

void osl::search::
HashRejections::clearRejectionRoot(const NumEffectState& parent, const HashKey& key, Move move)
{
  MoveVector moves;
  parent.generateLegal(moves);

  for (Move m: moves) {
    if (m == move)
      continue;
    const HashKey new_key = key.newHashWithMove(m);
    root_table->table.erase(new_key.boardKey());
  }
}

void osl::search::
HashRejections::addRejection(const NumEffectState& parent, const HashKey& key, Move move)
{
  MoveVector moves;
  parent.generateLegal(moves);

  for (Move m: moves) {
    if (m == move)
      continue;
    const HashKey new_key = key.newHashWithMove(m);
    Table::Entry& e = table->table[new_key.boardKey()];
    e.black_stand = new_key.blackStand();
  }
}

void osl::search::
HashRejections::clearRejection(const NumEffectState& parent, const HashKey& key, Move move)
{
  MoveVector moves;
  parent.generateLegal(moves);

  for (Move m: moves) {
    if (m == move)
      continue;
    const HashKey new_key = key.newHashWithMove(m);
    table->table.erase(new_key.boardKey());
  }
}

bool osl::search::
HashRejections::rejectionProbe(const HashKey& cur, const HashKey& parent) const
{
  {
    RootTable::table_t::const_iterator p = root_table->table.find(cur.boardKey());
    if (p != root_table->table.end() && p->second.parent != parent) {
      if (cur.turn() == BLACK) 
      {
	if (cur.blackStand().isSuperiorOrEqualTo(p->second.black_stand))
	  return true;
      }
      else 
      {
	if (p->second.black_stand.isSuperiorOrEqualTo(cur.blackStand()))
	  return true;
      }
    }
  }
  {
    Table::table_t::const_iterator p = table->table.find(cur.boardKey());
    if (p != table->table.end()) {
      if (cur.turn() == BLACK) 
      {
	if (cur.blackStand().isSuperiorOrEqualTo(p->second.black_stand))
	  return true;
      }
      else 
      {
	if (p->second.black_stand.isSuperiorOrEqualTo(cur.blackStand()))
	  return true;
      }
    }
  }
  return false;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
