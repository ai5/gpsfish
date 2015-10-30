/* simpleHashRecord.h
 */
#ifndef OSL_SEARCH_SIMPLEHASHRECORD_H
#define OSL_SEARCH_SIMPLEHASHRECORD_H

#include "osl/search/quiescenceRecord.h"
#include "osl/search/searchTable.h"
#include "osl/eval/evalTraits.h"
#include "osl/moveLogProb.h"
#ifdef OSL_SMP
#  include "osl/misc/lightMutex.h"
#endif
#include <cassert>

namespace osl
{
  namespace search
  {
    /**
     * SimpleHashTable の中に記録するエントリ
     */
    class SimpleHashRecord
    {
    public:
      /** 静止探索用 */
      QuiescenceRecord qrecord;
    private:
      MoveLogProb best_move;
      int upper_bound;
      int lower_bound;
      signed short upper_limit;
      signed short lower_limit;
      /** CAVEAT: 32bit では 100万局面/秒で1時間ちょっと考えると溢れる */
      unsigned int search_nodes;
    public:
      SimpleHashRecord() 
	: upper_limit(-1), lower_limit(-1), search_nodes(0)
      {
      }
      void addNodeCount(size_t diff)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,qrecord.mutex);
#endif
	search_nodes += diff;
      }

      /**
       * 値を書き込む. 詰将棋や千日手対策を想定．
       */
      void setAbsoluteValue(Move best_move, int value, int limit)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,qrecord.mutex);
#endif
	lower_limit = limit;
	lower_bound = value;
	upper_limit = limit;
	upper_bound = value;
	if (best_move.isNormal())
	  this->best_move = MoveLogProb(best_move, 100);
	else
	  this->best_move = MoveLogProb();
      }
      void setBestMove(Move m, int logprob=100) 
      {
	assert(! m.isInvalid());
	// 普通の更新はsetLowerBoundで。
	best_move = MoveLogProb(m, logprob);
	qrecord.best_moves.add(m);
      }
      void setAbsoluteValue(MoveLogProb best_move, int value, int limit)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,qrecord.mutex);
#endif
	lower_limit = limit;
	lower_bound = value;
	upper_limit = limit;
	upper_bound = value;
	this->best_move = best_move;
      }

      /**
       * lowerBound の設定.
       * - 深さ優先
       * - 同じ深さなら良い値優先
       */
      void setLowerBound(Player P, int limit, const MoveLogProb& best_move, 
			 int value)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,qrecord.mutex);
#endif
	assert((value % 2) == 0);
	assert(limit >= 0);
	if (limit < lower_limit)
	  return;
	if (best_move.validMove())
	  this->best_move= best_move;
	lower_bound = value;
	lower_limit = limit;
	if (hasUpperBound(0))
	  makeConsistent<true>(P);
      }

      /**
       * upperBound の設定.
       * - 深さ優先
       * - 同じ深さなら悪い値優先
       */
      void setUpperBound(Player P, int limit, const MoveLogProb& best_move, 
			 int value)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,qrecord.mutex);
#endif
	assert((value % 2) == 0);
	assert(limit >= 0);
	if (limit < upper_limit)
	  return;
	if (best_move.validMove()
	    && (! this->best_move.validMove()))
	{
	  // best move はupper boundでは基本的には更新しない
	  this->best_move= best_move;
	}
	upper_bound = value;
	upper_limit = limit;
	if (hasLowerBound(0))
	  makeConsistent<false>(P);
      }
    private:
      /**
       * 上限と下限に矛盾があったら調整する.
       * 同じ深さだったら後から来た方を優先しないと，再々探索の結果を記録できない
       * @param PreferLowerBound 同じ深さだったら lower bound を優先する
       */
      template <bool PreferLowerBound>
      void makeConsistent(Player P)
      {
	assert(hasLowerBound(0) && hasUpperBound(0));
	if (! eval::betterThan(P, lower_bound, upper_bound))
	  return;
	if ((upper_limit < lower_limit)
	    || (PreferLowerBound && (upper_limit == lower_limit)))
	{
	  upper_bound = lower_bound;
	  assert((upper_bound % 2) == 0);
	}
	else
	{
	  lower_bound = upper_bound;
	  assert((lower_bound % 2) == 0);
	}
      }
    public:
      /** upperBound を記録したときのlimit */
      int upperLimit() const { return upper_limit; }
      /** 手番のプレイヤからみた評価値の上限 */
      int upperBound() const { return upper_bound; }
      /** lowerBound を記録したときのlimit */
      int lowerLimit() const { return lower_limit; }
      /** 手番のプレイヤからみた評価値の下限 */
      int lowerBound() const { return lower_bound; }
      /** 今までに詰将棋で探したノード数 */
      int checkmateNodes() const { return qrecord.checkmate_nodes; }
      int threatmateNodes() const { return qrecord.threatmate_nodes; }
      const MoveLogProb bestMove() const { 
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,qrecord.mutex);
#endif
	return best_move; 
      }

      bool hasUpperBound(int limit) const
      {
	return (upperLimit() >= limit);
      }
      bool hasLowerBound(int limit) const
      {
	return (lowerLimit() >= limit);
      }
      bool hasAbsoluteUpperBound(Player p, int limit) const
      {
	return (p == BLACK) ? hasUpperBound(limit) : hasLowerBound(limit);
      }
      bool hasAbsoluteLowerBound(Player p, int limit) const
      {
	return (p == BLACK) ? hasLowerBound(limit) : hasUpperBound(limit);
      }
      int absoluteUpperBound(Player p) const
      {
	return (p == BLACK) ? upperBound() : lowerBound();
      }
      int absoluteLowerBound(Player p) const
      {
	return (p == BLACK) ? lowerBound() : upperBound();
      }
      void resetValue()
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,qrecord.mutex);
#endif
	lower_limit = -1;
	upper_limit = -1;
      }
      /** high fail するか */
      template <Player P>
      bool hasGreaterLowerBound(int limit, int threshold, int& val) const
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,qrecord.mutex);
#endif
	if ((lowerLimit() < limit)
	    || (EvalTraits<P>::betterThan(threshold, lowerBound())))
	  return false;
	val = lowerBound();
	return true;
      }
      /** low fail するか */
      template <Player P>
      bool hasLesserUpperBound(int limit, int threshold, int& val) const
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,qrecord.mutex);
#endif
	if ((upperLimit() < limit)
	    || (EvalTraits<P>::betterThan(upperBound(), threshold)))
	  return false;
	val = upperBound();
	return true;
      }

      const DualThreatmateState& threatmate() const { return qrecord.threatmate; }
      DualThreatmateState& threatmate() { return qrecord.threatmate; }

      void dump(std::ostream&) const;

      size_t nodeCount() const { return search_nodes; }
      bool inCheck() const
      {
#ifdef OSL_USE_RACE_DETECTOR
	SCOPED_LOCK_CHAR(lk,qrecord.mutex);
#endif
	return qrecord.threatmate.flags.is_king_in_check;
      }
      void setInCheck(bool new_value)
      {
#ifdef OSL_USE_RACE_DETECTOR
	SCOPED_LOCK_CHAR(lk,qrecord.mutex);
#endif
	qrecord.threatmate.flags.is_king_in_check = new_value;
      }
    };
  } // namespace search

  using search::SimpleHashRecord;
} // namespace osl


#endif /* OSL_SEARCH_SIMPLEHASHRECORD_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
