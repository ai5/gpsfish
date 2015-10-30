/* weights.h
 */
#ifndef OSL_EVAL_WEIGHTS_H
#define OSL_EVAL_WEIGHTS_H

#include "osl/eval/midgame.h"
#include <vector>
#include <valarray>
#include <cassert>

namespace osl
{
  namespace eval
  {
    namespace ml
    {
      struct Weights
      {
      protected:
	std::valarray<signed short> values;
	size_t dim;
      public:
	explicit Weights(size_t dim=0);
	virtual ~Weights();

	void resetDimension(size_t new_dim);
	int value(size_t index) const { assert(index < dim); return values[index]; }
	void setRandom();
	size_t dimension() const { return dim; }

	void setValue(size_t index, int value) 
	{
	  assert(index < dim);
	  values[index] = value;
	  assert(values[index] == value);
	}
      };

      class MultiWeights
      {
      protected:
	std::vector<MultiInt> values;
	size_t one_dim;
      public:
	explicit MultiWeights(size_t one_dim=0);
	virtual ~MultiWeights();

	void resetDimension(size_t one_dim);
	const MultiInt& value(size_t index) const { assert(index < one_dim); return values[index]; }
	void setRandom();
	size_t oneDimension() const { return one_dim; }
	void setValue(size_t index, MultiInt value) 
	{
	  assert(index < one_dim);
	  values[index] = value;
	}
      };
    }
  }
}

#endif /* OSL_EVAL_WEIGHTS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
