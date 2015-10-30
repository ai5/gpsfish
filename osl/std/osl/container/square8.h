/* position8.h
 */
#ifndef OSL_POSITION8_H
#define OSL_POSITION8_H

#include "osl/basic_type.h"
#include "osl/container.h"
#include <cstdint>
#include <iosfwd>

namespace osl
{
  namespace container
  {
    struct Square8 : public FixedCapacityVector<uint8_t,8>
    {
      typedef FixedCapacityVector<uint8_t,8> base_t;
      void push_back(Square position)
      {
	base_t::push_back(position.uintValue());
      }
      bool isMember(Square position) const
      {
	return base_t::isMember(position.uintValue());
      }
      const Square operator[](size_t i) const
      {
	return Square::makeDirect(base_t::operator[](i));
      }
    };
    std::ostream& operator<<(std::ostream&, const Square8&);
  } // namespace container
  using container::Square8;
} // namespace osl

#endif /* OSL_POSITION8_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
