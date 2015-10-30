/* searchWindow.h
 */
#ifndef _SEARCHWINDOW_H
#define _SEARCHWINDOW_H

#include "osl/search/simpleHashRecord.h"
#include "osl/eval/evalTraits.h"
namespace osl
{
  namespace search
  {
    enum TableHit { NO_HIT=0, LOWER_HIT, UPPER_HIT };
    struct AlphaBetaWindow
    {
      int alpha_value, beta_value;
      explicit AlphaBetaWindow(int a, int b) : alpha_value(a), beta_value(b)
      {
	assert(a % 2);
	assert(b % 2);
      }
      int& alpha() { return alpha_value; }
      int& beta()  { return beta_value; }
      int alpha() const { return alpha_value; }
      int beta()  const { return beta_value; }
      const AlphaBetaWindow flipPlayer() const
      {
	return AlphaBetaWindow(beta(), alpha());
      }
      bool isConsistent(Player P) const
      {
	return eval::notLessThan(P, beta(), alpha());
      }
      bool null() const { return alpha() == beta(); }
      void dump() const;
    };

    template <Player P, class EvalBase>
    struct AlphaBetaWindowUtil
    {
      typedef typename EvalBase::eval_t eval_t;
      /**
       * NullWindow の場合と違って window を狭く出来ることがある．
       */
      template <class Recorder>
      static
      TableHit isOutOfWindow(const SimpleHashRecord& record, int limit, 
			     AlphaBetaWindow& w, int& val,
			     const Recorder& recorder) 
      {
#ifdef __APPLE__
	int table_value=0;
#else
	int table_value;
#endif
	if (record.template hasGreaterLowerBound<P>(limit, w.alpha(), 
						    table_value))
	{
	  assert(eval::isConsistentValue(table_value));
	  w.alpha() = table_value + EvalTraits<P>::delta;
	  if (EvalTraits<P>::betterThan(table_value, w.beta()))
	  {
	    recorder.tableHitLowerBound(P, table_value, w.beta(), limit);
	    val = table_value;
	    return LOWER_HIT;
	  }
	} 
	if (record.template hasLesserUpperBound<P>(limit, w.beta(), table_value))
	{
	  assert(eval::isConsistentValue(table_value));
	  w.beta() = table_value - EvalTraits<P>::delta;
	  if (EvalTraits<P>::betterThan(w.alpha(), table_value))
	  {
	    recorder.tableHitUpperBound(P, table_value, w.alpha(), limit);
	    val = table_value;
	    return UPPER_HIT;
	  }
	}
	return NO_HIT;
      }
    };
    

    struct NullWindow
    {
      int value;

      explicit NullWindow(int v) :value(v)
      {
      }
      int& alpha() { return value; }
      int& beta()  { return value; }
      int alpha() const { return value; }
      int beta()  const { return value; }
      bool isConsistent(Player) const { return true; }
      void dump() const;
    };

    template <Player P, class EvalBase, bool best_move_extension>
    struct NullWindowUtil
    {
      typedef typename EvalBase::eval_t eval_t;
      /** 
       * Record と比べて cut できるかどうかを判定する.
       * @return cut できるかどうか
       * @param val cut 出来る場合は upper/lower bound が入る
       * @param best_move_extension 真の場合，window を越えていても
       *   かなり深く読んだ結果でないと cut しない．一度同じ深さで探索した手を，
       *   これから読む手の確率を高くとることで延長する時に使用するため
       */
      template <class Recorder>
      static
      TableHit isOutOfWindow(const SimpleHashRecord& record, int limit, 
			     NullWindow w,
			     int& val, const Recorder& recorder)
      {
	const int lookUpLimit = (best_move_extension ? limit + 200 : limit);
#ifdef __APPLE__
	int table_value=0;
#else
	int table_value;
#endif
	if (record.template hasGreaterLowerBound<P>
	    (lookUpLimit, w.beta(), table_value))
	{
	  assert(eval::isConsistentValue(table_value));
	  recorder.tableHitLowerBound(P, table_value, w.beta(), limit);

	  val = table_value;
	  return LOWER_HIT;
	} 
	if (record.template hasLesserUpperBound<P>
	    (lookUpLimit, w.alpha(), table_value))
	{
	  assert(eval::isConsistentValue(table_value));
	  recorder.tableHitUpperBound(P, table_value, w.alpha(), limit);

	  val = table_value;
	  return UPPER_HIT;
	}
	return NO_HIT;
      }

    };
    
  } // namespace search
} // namespace osl


#endif /* _SEARCHWINDOW_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
