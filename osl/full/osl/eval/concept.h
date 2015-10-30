#ifndef EVAL_CONCEPT_H
#define EVAL_CONCEPT_H

#include "osl/player.h"
#include <boost/concept_check.hpp>

namespace osl
{
  namespace eval
  {
    /**
     * 評価関数のinterface (人間用)
     */
    class EvaluationFunction
    {
    public:
      int getVal() const;
    };

    /**
     * EvaluationFunction の制約.
     * http://www.boost.org/libs/concept_check/concept_check.htm
     */
    template <class T>
    struct Concept
    {
      /** 
       * 制約.
       * 他に， ApplyMoveOfTurn できる必要がある．
       */
      void constraints() 
      {
	const int value  = eval.value();
	boost::ignore_unused_variable_warning(value);
	const int infty = T::infty();
	boost::ignore_unused_variable_warning(infty);
	const int capture_val = T::captureValue(ptypeo);
	boost::ignore_unused_variable_warning(capture_val);
      }
      T eval;
      PtypeO ptypeo;
    };
  } // namespace move_action
} // namespace osl


#endif /* EVAL_CONCEPT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
