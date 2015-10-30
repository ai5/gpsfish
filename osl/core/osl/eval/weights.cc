/* weights.cc
 */
#include "osl/eval/weights.h"
#include "osl/random.h"

osl::eval::ml::
Weights::Weights(size_t idim)
  : values(idim), dim(idim)
{
  std::fill(&values[0], &values[0]+dim, 0);
}
osl::eval::ml::
Weights::~Weights()
{
}

void osl::eval::ml::Weights::
setRandom()
{
  for (size_t i=0; i<dim; ++i)
    values[i] = osl::random() % 256 - 128;
}


void osl::eval::ml::
Weights::resetDimension(size_t new_dim)
{
  dim = new_dim;
  values.resize(new_dim);
  std::fill(&values[0], &values[0]+dim, 0);
}



osl::eval::ml::
MultiWeights::MultiWeights(size_t idim)
  : values(idim), one_dim(idim)
{
}
osl::eval::ml::
MultiWeights::~MultiWeights()
{
}

void osl::eval::ml::
MultiWeights::resetDimension(size_t new_dim)
{
  one_dim = new_dim;
  values.resize(new_dim);
  std::fill(&values[0], &values[0]+one_dim, MultiInt());
}

void osl::eval::ml::MultiWeights::
setRandom()
{
  for (size_t i=0; i<one_dim; ++i)
    for (size_t s=0; s<MultiInt::size(); ++s)
      values[i][s] = osl::random() % 256 - 128;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
