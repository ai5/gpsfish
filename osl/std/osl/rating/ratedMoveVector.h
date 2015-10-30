/* ratedMoveVector.h
 */
#ifndef OSL_RATEDMOVEVECTOR_H
#define OSL_RATEDMOVEVECTOR_H

#include "osl/rating/ratedMove.h"
#include "osl/container.h"
#include <iosfwd>

namespace osl
{
  namespace rating
  {
    typedef FixedCapacityVector<RatedMove,Move::MaxUniqMoves> RatedMoveVectorBase;

    class RatedMoveVector : public RatedMoveVectorBase
    {
      typedef RatedMoveVectorBase base_t;
    public:
      /** ratingが高い順にsort */
      void sort();
      const RatedMove* find(Move) const;
    };
    std::ostream& operator<<(std::ostream& os, RatedMoveVector const&);
    bool operator==(const RatedMoveVector& l, const RatedMoveVector& r);
  } // namespace container
  using rating::RatedMoveVector;
} // namespace osl

#endif /* OSL_RATEDMOVEVECTOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:


