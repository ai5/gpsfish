/* bitXmask.h
 */
#ifndef OSL_BITXMASK_H
#define OSL_BITXMASK_H

#include "osl/basic_type.h"
#include <iosfwd>

namespace osl 
{
  namespace container
  {
    /**
     * X座標のbitset
     */
  class BitXmask
  {
    int mask;
  public:
    BitXmask() : mask(0) {}
    void clearAll() { mask = 0; }
    void set(int x)   { mask |= (1 << x); }
    void clear(int x) { mask &=  ~(1 << x); }

    void set(Square position)   { set(position.x()); }
    void clear(Square position) { clear(position.x()); }
  
    bool isSet(int x) const { return mask & (1<<x); }

    int intValue() const { return mask; }
  };

  inline bool operator==(BitXmask l, BitXmask r)
  {
    return l.intValue() == r.intValue();
  }
  inline bool operator!=(BitXmask l, BitXmask r)
  {
    return ! (l == r);
  }
  inline bool operator<(BitXmask l, BitXmask r)
  {
    return l < r;
  }
  
  std::ostream& operator<<(std::ostream&,const BitXmask);
  } // namespace container
  using container::BitXmask;
} // namespace osl

#endif /* OSL_BITXMASK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
