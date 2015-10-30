/* repetitionCounter.cc
 */
#include "osl/repetitionCounter.h"
#include "osl/hashKey.h"
#include "osl/numEffectState.h"
#include <unordered_map>
#include <iostream>


typedef osl::RepetitionCounter::list_t list_t;
typedef std::unordered_map<osl::HashKey,list_t,std::hash<osl::HashKey>> map_t;

struct osl::RepetitionCounter::Table : public map_t
{
};

static const int initial_capacity = 256;

void osl::RepetitionCounter::
clear() 
{
  table->clear();
  continuous_check[0].clear();
  continuous_check[1].clear();
  hash_history.clear();

  continuous_check[0].reserve(initial_capacity);
  continuous_check[1].reserve(initial_capacity);
  
  continuous_check[0].push_back(0);
  continuous_check[1].push_back(0);
}

osl::RepetitionCounter::
RepetitionCounter() : table(new Table()), hash_history(initial_capacity)
{
  clear();
}

osl::RepetitionCounter::
RepetitionCounter(const RepetitionCounter& c)
  : continuous_check(c.continuous_check), 
    hash_history(c.hash_history)
{
  if (c.table)
    table.reset(new Table(*c.table));
  assert(isConsistent());
}


osl::RepetitionCounter::
RepetitionCounter(const NumEffectState& initial)
  : table(new Table()), hash_history(initial_capacity)
{
  clear();
  const HashKey key(initial);
  push(key, initial);
}

osl::RepetitionCounter::
~RepetitionCounter()
{
}

void osl::RepetitionCounter::
push(const HashKey& new_key, bool is_check)
{
  const Player last_turn = alt(new_key.turn());
  if (is_check)
  {
    continuous_check[last_turn].push_back(checkCount(last_turn)+1);
  }
  else
  {
    continuous_check[last_turn].push_back(0);
  }

  const Table::iterator p=table->find(new_key);
  if (p == table->end())
  {
    (*table)[new_key].push_front(order());
  }
  else
  {
    list_t& l = p->second;
    l.push_front(order());
  }
  hash_history.push(new_key);
}

void osl::RepetitionCounter::
push(const HashKey& key, const NumEffectState& state)
{
  const bool is_check = state.inCheck();
  push(key, is_check);
}

void osl::RepetitionCounter::
push(const NumEffectState& state)
{
  push(HashKey(state), state);
}

void osl::RepetitionCounter::
push(const NumEffectState& state, Move move)
{
  assert(move.isValidOrPass());
  assert(state.turn() == move.player());
  
  HashKey key(state);
  key = key.newHashWithMove(move);

  // 指した後の王手を検出
  const bool is_check 
    = (!move.isPass()) && state.isCheck(move);
  push(key, is_check);
}

void osl::RepetitionCounter::
pop()
{
  assert(order());
  assert(hash_history.size()>0);
  const HashKey last_key = hash_history.top();
  hash_history.pop();
  
  const Player last_turn = alt(last_key.turn());
  assert(! continuous_check[last_turn].empty());
  continuous_check[last_turn].pop_back();

  const Table::iterator p=table->find(last_key);
  assert(p != table->end());

#ifndef NDEBUG
  const list_t::iterator q = p->second.begin();
  assert(q != p->second.end());
  assert(*q == order());
#endif
  p->second.pop_front();
  if (p->second.empty())
    table->erase(p);
}

int osl::RepetitionCounter::
getLastMove(const HashKey& key) const
{
  const Table::const_iterator p=table->find(key);
  if (p == table->end())
    return -1;
  return p->second.front();
}
int osl::RepetitionCounter::
getFirstMove(const HashKey& key) const
{
  const Table::const_iterator p=table->find(key);
  if (p == table->end())
    return -1;
  list_t::const_iterator q = p->second.begin();
  assert(q != p->second.end());
  int result = *q++;
  while (q != p->second.end())
    result = *q++;
  return result;
}

const osl::Sennichite osl::RepetitionCounter::
isSennichite(const NumEffectState& state, Move move) const
{
  HashKey key(state);
  key = key.newHashWithMove(move);
  const Table::const_iterator p=table->find(key);
  if (p == table->end())
    return Sennichite::NORMAL();

  // 現在3だと次で4
  if (p->second.size() < 3)
    return Sennichite::NORMAL();
  return isAlmostSennichite(key);
}

const std::pair<osl::Sennichite,int> osl::RepetitionCounter::
distanceToSennichite(const HashKey& key) const
{
  const Table::const_iterator p=table->find(key);
  if (p == table->end())
    return std::make_pair(Sennichite::NORMAL(), 0);
  return std::make_pair(isAlmostSennichite(key), p->second.size());
}

unsigned int osl::RepetitionCounter::
countRepetition(const HashKey& key) const
{
  const Table::const_iterator p=table->find(key);
  if (p == table->end())
    return 0;
  return p->second.size();
}

const list_t osl::RepetitionCounter::
getRepetitions(const HashKey& key) const
{
  Table::const_iterator p=table->find(key);
  if (p == table->end())
    return list_t();
  return p->second;
}

#ifndef MINIMAL
void osl::RepetitionCounter::
printMatches(const HashKey& key) const
{
  Table::const_iterator p=table->find(key);
  if (p == table->end())
    return;
  for (int q: p->second) {
    std::cerr << q << " ";
  }
  std::cerr << "\n";
}

bool osl::RepetitionCounter::
isConsistent() const
{
  HashKeyStack history = hash_history;
  Table table(*this->table);
  CArray<std::vector<int>, 2> continuous_check = this->continuous_check;
  while (history.empty())
  {
    const HashKey last_key = history.top();
    history.pop();
  
    const Player last_turn = alt(last_key.turn());
    assert(! continuous_check[last_turn].empty());
    continuous_check[last_turn].pop_back();

    const Table::iterator p=table.find(last_key);
    if (p == table.end())
    {
      std::cerr << "oops, " << this << "\n";
      return false;
    }
    assert(p != table.end());

#ifndef NDEBUG
    const list_t::iterator q = p->second.begin();
    assert(q != p->second.end());
    assert(*q == order());
#endif
    p->second.pop_front();
    if (p->second.empty())
      table.erase(p);
  }
  return true;
}

bool osl::RepetitionCounter::maybeEqual(const RepetitionCounter& l, const RepetitionCounter& r)
{
#if ! (__GNUC__ >= 4 && __GNUC_MINOR__ >=3)
  // oops
  if (*l.table != *r.table)
    return false;
#endif
  if (l.continuous_check[0] != r.continuous_check[0])
    return false;
  if (l.continuous_check[1] != r.continuous_check[1])
    return false;
  return l.hash_history == r.hash_history;
}
#endif

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
