/* checkMoveGenerator.cc
 */

#include "checkMoveGenerator.h"
#include "checkMoveGenerator.tcc"

namespace osl
{
  // explicit template instantiation
  template class checkmate::CheckMoveGenerator<BLACK>;
  template class checkmate::CheckMoveGenerator<WHITE>;
} // namespace osl

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
