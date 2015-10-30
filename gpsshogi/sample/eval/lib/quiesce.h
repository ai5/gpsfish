/* quiesce.h
 */
#ifndef GPSSHOGI_LEARN_QUIESCE_H
#define GPSSHOGI_LEARN_QUIESCE_H

#include "eval/eval.h"
#include "pvVector.h"
#include "osl/hashKey.h"
#include "osl/state/historyState.h"
#include "osl/container.h"
#include <unordered_map>
#include <memory>

namespace gpsshogi
{
  using namespace osl;
  struct Record
  {
    int lower_bound, upper_bound;
    int lower_depth, upper_depth;
    Move best_move;    
    size_t update_time;
    Record() : lower_bound(0), upper_bound(0), lower_depth(-16), upper_depth(-16), update_time(0)
    {
    }
    void setLowerBound(size_t time, int depth, int value, bool overwrite)
    {
      if (overwrite || depth > lower_depth) {
	update_time = time;
	lower_bound = value;
	lower_depth = depth;
      }
    }
    void setUpperBound(size_t time, int depth, int value, bool overwrite)
    {
      if (overwrite || depth > upper_depth) {
	update_time = time;
	upper_bound = value;
	upper_depth = depth;
      }
    }
    void setValue(size_t time, int depth, int value)
    {
      update_time = time;
      lower_bound = upper_bound = value;
      lower_depth = upper_depth = depth;
    }
  };
  class Table
  {
    typedef std::unordered_map<HashKey, Record, std::hash<HashKey>> map_t;
    map_t table;
  public:
    Table();
    ~Table();

    Record *allocate(const HashKey&);
    void clear();
  };
  class BigramTable
  {
    typedef std::unordered_map<int, Move> map_t;
    map_t table;
  public:
    BigramTable();
    ~BigramTable();
    Move find(const NumEffectState& state, Move key) const {
      map_t::const_iterator p = table.find(key.intValue());
      if (p != table.end())
	if (state.isAlmostValidMove<false>(p->second))
	  return p->second;
      return Move();
    }
    void add(Move key, Move value) {
      if (! key.isNormal()
	  || ! value.isNormal() || value.isCaptureOrPromotion())
	return;
      table[key.intValue()] = value;
    }
    void clear();
  };
  
  class Quiesce
  {
  private:
    Eval *eval;			// acquaintance
    HistoryState history_state;
    HashKey key;
    Table table;
    BigramTable bigram_table;
    int all_moves_depth, quiesce_depth;
    int root_depth_left, root_history_size;
    PVVector history;
    CArray<Move, PvMaxDepth> killers;
    size_t time;
    std::vector<PVVector> pv;
    uint64_t node_count;
    std::unique_ptr<EvalValueStack> eval_value;
  public:
    explicit Quiesce(Eval *, int all_moves_depth=0, int quiesce_depth=3);
    ~Quiesce();
    
    /** return false if pv is sceptical */
    bool quiesce(NumEffectState& state,
		 int& value, PVVector& pv);
    bool quiesce(NumEffectState& state,
		 int& value, PVVector& pv, int alpha, int beta);
    void setDepth(int all, int quiesce) 
    {
      assert(all >= 0);
      if (quiesce < 0)
	quiesce = PvMaxDepth - all - 1; // quiesce = 4;
      all_moves_depth = all;
      quiesce_depth = quiesce;
    }
    int fullWidthDepth() const { return all_moves_depth; }
    int quiesceDepth() const { return quiesce_depth; }
    bool bonanzaCompatible() const 
    {
      return all_moves_depth + quiesce_depth == PvMaxDepth-1;
    }
  private:
    int search(int alpha, int beta, int depth_left);
    void generateAllMoves(MoveVector&, int depth_left) const;
    void generateTacticalMoves(MoveVector&) const;
    void generateEscapeFromLastMove(MoveVector&, Move last_move) const;
    void generateTakeBack(MoveVector&, Square) const;
    void selectSeePlus(const NumEffectState&, const MoveVector&, MoveVector&, int threshold=0) const;
  public:
    void clear();
    size_t nodeCount() const { return node_count; }
    void addBigram(Move key, Move value) { bigram_table.add(key, value); }
    
    static const int black_infty = 2000000000;
    static int infty(Player turn) 
    {
      return (turn == BLACK) ? black_infty : -black_infty;
    }

    const NumEffectState *state() const { return &history_state.state(); }
    Move makeTakeBack(Move last_move) const;
  };
  std::ostream& operator<<(std::ostream&, const gpsshogi::PVVector&);
}

#endif /* GPSSHOGI_LEARN_QUIESCE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
