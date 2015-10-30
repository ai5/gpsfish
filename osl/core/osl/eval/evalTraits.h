/* evalTraits.h
 */
#ifndef _EVAL_TRAITS_H
#define _EVAL_TRAITS_H

#include "osl/basic_type.h"
#include <algorithm>
namespace osl
{
  namespace eval
  {
    template<Player P>
    struct EvalTraits;

    template<>
    struct EvalTraits<BLACK>
    {
      static const int delta=1;
      static const int MAX_VALUE = 250000000;
      // infty become specific to Evaluation class
      static int max(int v1,int v2){ return std::max(v1,v2); }
      static int min(int v1,int v2){ return std::min(v1,v2); }

      static bool betterThan(int v1,int v2) 
      {
	return v1 > v2;
      }
      static bool notLessThan(int v1,int v2) {
	return v1 >= v2;
      }
      /**
       * @param value BLACKのための値
       */
      static int convert(int value)
      {
	assert(value >= 0);
	return value;
      }
    };

    template<>
    struct EvalTraits<WHITE>
    {
      static const int delta= -EvalTraits<BLACK>::delta;
      static const int MAX_VALUE= -EvalTraits<BLACK>::MAX_VALUE;
      static int max(int v1,int v2){ return std::min(v1,v2); }
      static int min(int v1,int v2){ return std::max(v1,v2); }

      static bool betterThan(int v1,int v2) 
      {
	return v1 < v2;
      }
      static bool notLessThan(int v1,int v2) 
      {
	return v1 <= v2;
      }
      /**
       * @param value BLACKのための値
       */
      static int convert(int value)
      {
	assert(value >= 0);
	return -value;
      }
    };

    inline bool betterThan(Player p, int v1,int v2)
    {
      assert(isValid(p));
      if (p == BLACK)
	return EvalTraits<BLACK>::betterThan(v1,v2);
      else
	return EvalTraits<WHITE>::betterThan(v1,v2);
    }
    inline bool notLessThan(Player p, int v1,int v2)
    {
      assert(isValid(p));
      if (p == BLACK)
	return EvalTraits<BLACK>::notLessThan(v1,v2);
      else
	return EvalTraits<WHITE>::notLessThan(v1,v2);
    }

    inline int max(Player p, int v1, int v2)
    {
      assert(isValid(p));
      if (p == BLACK)
	return EvalTraits<BLACK>::max(v1,v2);
      else
	return EvalTraits<WHITE>::max(v1,v2);
    }
    inline int min(Player p, int v1, int v2)
    {
      assert(isValid(p));
      if (p == BLACK)
	return EvalTraits<BLACK>::min(v1,v2);
      else
	return EvalTraits<WHITE>::min(v1,v2);
    }

    /**
     * playerにとってちょっと高い値
     */
    inline int delta(Player p)
    {
      assert(isValid(p));
      if (p == BLACK)
	return EvalTraits<BLACK>::delta;
      else
	return EvalTraits<WHITE>::delta;
    }

    /**
     * @param value BLACKのための値
     */
    inline int convert(Player P, int value)
    {
      assert(value >= 0);
      return value*delta(P);
    }

    /**
     * 詰がからんでいない局面での通常の評価値
     */
    template <class Eval>
    inline bool isConsistentValueForNormalState(int value)
    {
      const int infty = Eval::infty();
      return ((value % 2) == 0)
	&& EvalTraits<BLACK>::betterThan(value, EvalTraits<WHITE>::convert(infty))
	&& EvalTraits<WHITE>::betterThan(value, infty);
    }
    inline bool isConsistentValue(int value)
    {
      return (value % 2) == 0
	&& (EvalTraits<BLACK>::MAX_VALUE >= value)
	&& (EvalTraits<WHITE>::MAX_VALUE <= value);
    }
  } // namespace eval
  using eval::EvalTraits;
} // namespace osl

#endif /* _EVAL_TRAITS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
