/* pieceMask64.h
 */
#ifndef PIECEMASK64_H
#define PIECEMASK64_H
#include "osl/bits/mask.h"

namespace osl
{
  namespace container
  {
  class PieceMask64
  {
  protected:
    Mask64 mask;
  public:
    static int numToIndex(int) { return 0; }
    static int numToOffset(int num) { return num; }
    PieceMask64() { resetAll(); }
    explicit PieceMask64(misc::Mask64 const& m) : mask(m) {}
  protected:
    misc::Mask64& mutableMask(int) { return mask; }
  public:
    const misc::Mask64& getMask(int) const { return mask; }
    void resetAll()
    {
      mask=misc::Mask64::makeDirect(0uLL);
    }
    void setAll()
    {
      mask=misc::Mask64::makeDirect(0xffffffffffuLL);
    }
    PieceMask64& operator^=(const PieceMask64& o)
    {
      mask ^= o.mask;
      return *this;
    }
    PieceMask64& operator&=(const PieceMask64& o)
    {
      mask &= o.mask;
      return *this;
    }
    PieceMask64& operator|=(const PieceMask64& o)
    {
      mask |= o.mask;
      return *this;
    }
    PieceMask64& operator-=(const PieceMask64& o)
    {
      mask -= o.mask;
      return *this;
    }
    PieceMask64& operator+=(const PieceMask64& o)
    {
      mask += o.mask;
      return *this;
    }
    bool none() const { return mask.none(); }
    bool hasMultipleBit() const
    {
      if (none()) 
	return false;
      return mask.hasMultipleBit();
    }
    /**
     * bit の数を2まで数える
     * @return 0,1,2 (2の場合は2以上)
     */
    int countBit2() const 
    {
      if (none()) 
	return 0;
      return mask.countBit2();
    }
    int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
    countBit() const 
    {
      return mask.countBit();
    }
    int takeOneBit()
    {
      assert(!none());
      return mask.takeOneBit();
    }
  };
} // namespace container
  using container::PieceMask64;
} // namespace osl


#endif /* PIECEMASK64_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
