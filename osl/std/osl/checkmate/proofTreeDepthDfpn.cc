/* proofTreeDepthDfpn.cc
 */
#include "osl/checkmate/proofTreeDepthDfpn.h"
#include "osl/checkmate/dfpn.h"
#include "osl/checkmate/dfpnRecord.h"
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/checkmate/fixedDepthSearcher.tcc"
#include <unordered_map>
#include <forward_list>
/**
 * 深さを記憶するテーブル.
 * -1 は探索中
 */
struct osl::checkmate::ProofTreeDepthDfpn::Table
{
  boost::scoped_array<NumEffectState> state;
  typedef std::unordered_map<HashKey, std::pair<int, Move>, std::hash<HashKey>> map_t;
  typedef std::pair<const HashKey, std::pair<int, Move>> entry_t;
  typedef std::forward_list<const entry_t*> list_t;
  typedef std::unordered_map<BoardKey, list_t, std::hash<BoardKey>> index_t;
  map_t depth_table;
  index_t depth_index;
  const DfpnTable& table;
  Table(const DfpnTable& t) : state(new NumEffectState[t.maxDepth()]), table(t)
  {
  }
  void store(const HashKey& key, int depth, Move best_move=Move()) 
  {
    depth_table[key] = std::make_pair(depth, best_move);
    const entry_t& e = *depth_table.find(key);
    depth_index[key.boardKey()].push_front(&e);
  }
  bool find(const HashKey& key, int& depth, Move& best_move) const
  {
    map_t::const_iterator p=depth_table.find(key);
    if (p == depth_table.end())
      return false;
    depth = p->second.first;
    best_move = p->second.second;
    return true;
  }
  bool expectMoreDepth(Player attack, const HashKey& key, int depth) const
  {
    index_t::const_iterator p=depth_index.find(key.boardKey());
    if (p == depth_index.end())
      return true;
    for (const entry_t *q: p->second) {
      assert(q->first.boardKey() == key.boardKey());
      if (attack == BLACK) {
	if (q->first.blackStand().isSuperiorOrEqualTo(key.blackStand())) {
	  if (q->second.first >= depth)
	    return true;
	} else if (key.blackStand().isSuperiorOrEqualTo(q->first.blackStand())) {
	  if (q->second.first < depth)
	    return false;
	}
      }
      else {
	if (q->first.blackStand().isSuperiorOrEqualTo(key.blackStand())) {
	  if (q->second.first < depth)
	    return false;
	} else if (key.blackStand().isSuperiorOrEqualTo(q->first.blackStand())) {
	  if (q->second.first >= depth)
	    return true;
	}
      }
    }
    return true;
  }
  int maxDepth() const { return table.maxDepth(); }
};

osl::checkmate::
ProofTreeDepthDfpn::ProofTreeDepthDfpn(const DfpnTable& dfpn_table)
  : table(new Table(dfpn_table))
{
}

osl::checkmate::
ProofTreeDepthDfpn::~ProofTreeDepthDfpn()
{
}

int osl::checkmate::
ProofTreeDepthDfpn::depth(const HashKey& key, const NumEffectState& state, bool is_or_node) const
{
  Move dummy;
  table->state[0] = state;
  return (is_or_node ? orNode(key, dummy) : andNode(key, dummy));
}

void osl::checkmate::
ProofTreeDepthDfpn::retrievePV
(const NumEffectState& src, bool is_or_node,
 std::vector<Move>& pv) const
{
  table->state[0] = src;
  HashKey key(table->state[0]);
  pv.clear();
  for (int i=0; i<table->maxDepth(); ++i) {
    Move next;
    if (is_or_node ^ (i%2))
      orNode(key, next);
    else
      andNode(key, next);
    if (! next.isNormal())
      return;
    pv.push_back(next);
    table->state[0].makeMove(next);
    key = key.newMakeMove(next);
  }
}

int osl::checkmate::
ProofTreeDepthDfpn::orNode(const HashKey& key, Move& best_move, int height) const
{
  assert(key == HashKey(table->state[height]));  
  best_move = Move();
  if (height >= table->maxDepth())
    return -1;

  // always test ImmediateCheckmate since users do not want to see redundant solutions
  FixedDepthSearcher fixed_searcher(table->state[height]);
  ProofDisproof pdp = fixed_searcher.hasCheckmateMoveOfTurn(0, best_move);
  if (pdp.isCheckmateSuccess()) {
    table->store(key, 1, best_move);
    return 1;
  }
  pdp = fixed_searcher.hasCheckmateMoveOfTurn(2, best_move);
  if (pdp.isCheckmateSuccess()) {
    table->store(key, 3, best_move);
    return 3;
  }

  const PieceStand white_stand = PieceStand(WHITE, table->state[height]);
  DfpnRecord record = table->table.probe(key, white_stand);
  if (! record.proof_disproof.isCheckmateSuccess()) {
    table->store(key, 5, Move()); // XXX
    return 5;
  }
  {
    int recorded;
    if (table->find(key, recorded, best_move)) 
      return recorded;
  }
  table->store(key, -1, Move());

  if (! record.best_move.isNormal())
  {
    // XXX // ImmediateCheckmate
    table->store(key, 1, Move());
  }

  const HashKey new_key = key.newHashWithMove(record.best_move);
  const PieceStand next_white_stand = (table->state[height].turn() == WHITE) 
    ? white_stand.nextStand(WHITE, record.best_move) : white_stand;
  DfpnRecord new_record = table->table.probe(new_key, next_white_stand);
  if (! new_record.proof_disproof.isCheckmateSuccess())
    new_record = table->table.findProofOracle(new_key, next_white_stand, record.best_move);
  if (new_record.proof_disproof.isCheckmateSuccess()) {
    table->state[height+1] = table->state[height];
    table->state[height+1].makeMove(record.best_move);
    Move dummy;
    const int depth = andNode(new_key, dummy, height+1);
    if (depth >= 0)
    {
      best_move = record.best_move;
      table->store(key, depth+1, best_move);
      return depth+1;
    }
  }
  return 0;
}

int osl::checkmate::
ProofTreeDepthDfpn::andNode(const HashKey& key, Move& best_move, int height) const
{
  best_move = Move();
  if (height >= table->maxDepth())
    return -1;
  {
    int recorded;
    if (table->find(key, recorded, best_move)) 
      return recorded;
  }
  table->store(key, -1, Move());

  int result = 0;	// and node で指手がなくて詰 => 逃げられない
  std::unique_ptr<Dfpn::DfpnMoveVector> moves(new Dfpn::DfpnMoveVector);
  if (table->state[height].turn() == BLACK)
    Dfpn::generateEscape<WHITE>(table->state[height], true, Square(), *moves);
  else
    Dfpn::generateEscape<BLACK>(table->state[height], true, Square(), *moves);

  for (size_t i=0; i<moves->size(); ++i)
  {
    const HashKey new_key = key.newHashWithMove((*moves)[i]);
    if (i > 0 && ! table->expectMoreDepth(alt((*moves)[i].player()), new_key, result))
      continue;
    table->state[height+1] = table->state[height];
    table->state[height+1].makeMove((*moves)[i]);
    Move dummy;
    const int depth = orNode(new_key, dummy, height+1);
    if (depth < 0) {
      return depth;		// loop found
    }
    if (result < depth+1) {
      result = depth+1;
      best_move = (*moves)[i];
    }
  }
  
  table->store(key, result, best_move);
  return result;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
