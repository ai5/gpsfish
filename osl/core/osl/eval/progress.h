#ifndef EVAL_ML_PROGRESS_H
#define EVAL_ML_PROGRESS_H

#include "osl/progress.h"
#include "osl/eval/weights.h"

namespace osl
{
  namespace eval
  {
    namespace ml
    {
      class ProgressBonus
      {
      public:
	enum { DIM = 256 };
	static int eval(Progress16 black, Progress16 white);
	static void setUp(const Weights &weights);
      private:
	static int index(Progress16 black, Progress16 white)
	{
	  return black.value() * 16 + white.value();
	}
	static CArray<int, 256> table;
      };
      class ProgressAttackDefense
      {
      public:
	enum { DIM = 256 };
	static int eval(Progress16 black_attack, Progress16 white_defense,
			Progress16 white_attack, Progress16 black_defense);
	static void setUp(const Weights &weights);
      private:
	static int index(Progress16 attack, Progress16 defense)
	{
	  return attack.value() * 16 + defense.value();
	}
	static CArray<int, 256> table;
      };

      class ProgressAttackDefenseAll
      {
      public:
	enum { DIM = 65536 };
	static int eval(Progress16 black_attack,
			Progress16 white_defense,
			Progress16 white_attack, Progress16 black_defense);
	static void setUp(const Weights &weights);
      private:
	static int index(Progress16 black_attack, Progress16 white_defense,
			 Progress16 white_attack, Progress16 black_defense)
	{
	  return white_attack.value() +
	    16 * (black_defense.value() +
		  16 * (black_attack.value() * 16 + white_defense.value()));
	}
	static CArray<int, 65536> table;
      };
    }
  }
}

#endif // EVAL_ML_PROGRESS_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

