#ifndef _PIECE_MASK_H
#define _PIECE_MASK_H
#include "osl/config.h"
#include "osl/bits/ptypeTraits.h"
static_assert(OSL_WORDSIZE == 64, "word size");
#include "osl/bits/pieceMask64.h"

#include <iosfwd>

namespace osl
{
  typedef PieceMask64 PieceMaskBase;

  /**
   * 駒番号のビットセット.
   * 64bitのMask64を一つもしくは，32bitのMask32を2枚で表現する．
   * 各メソッドの変数名は num は駒番号に，
   * index はマスクのID(0-1)に用いられている．
   */
  class PieceMask : public PieceMaskBase
  {
  public:
    PieceMask() {}
    PieceMask(const PieceMaskBase& base) : PieceMaskBase(base) {}
    static const mask_t numToMask(int num) { 
      return mask_t::makeDirect(1) << PieceMask::numToOffset(num); 
    }
    void setMask(int index,mask_t val) {
      mutableMask(index)=val;
    }
  private:
    mask_t& mutableMaskNum(int num) {
      return mutableMask(numToIndex(num));
    }
    const mask_t getMaskNum(int num) const {
      return getMask(numToIndex(num));
    }
  public:
    void xorMask(int index,mask_t val) {
      mutableMask(index)^=val;
    }
    void orMask(int index,mask_t val) {
      mutableMask(index)|=val;
    }
    bool test(int num) const {
      return (getMaskNum(num)&numToMask(num)).any();
    }
    void set(int num) {
      mutableMaskNum(num)|=numToMask(num);
    }
    void flip(int num) {
      mutableMaskNum(num)^=numToMask(num);
    }
    void reset(int num) {
      mutableMaskNum(num)&= ~numToMask(num);
    }
    bool any() const { return ! none(); }

    const mask_t getMask(int num) const { return PieceMaskBase::getMask(num); }
    /** unpromote(PTYPE) の駒のbit を*含む*mask_tを取り出す */
    template <Ptype PTYPE>
    const mask_t getMask() const { return getMask(PtypeFuns<PTYPE>::indexNum); }
    
    /** unpromote(PTYPE) の駒のbit だけ取り出す */
    template <Ptype PTYPE>
    const mask_t selectBit() const 
    {
      mask_t mask = getMask<PTYPE>();
      mask &= mask_t::makeDirect(PtypeFuns<PtypeFuns<PTYPE>::basicType>::indexMask);
      return mask;
    }
    /** unpromote(PTYPE) の駒のbit を消す */
    template <Ptype PTYPE>
    void clearBit()
    {
      mask_t& mask = mutableMask(PtypeFuns<PTYPE>::indexNum);
      mask &= ~mask_t::makeDirect(PtypeFuns<PtypeFuns<PTYPE>::basicType>::indexMask);
    }
    /** unpromote(PTYPE) の駒のbit を立てる */
    template <Ptype PTYPE>
    void setBit()
    {
      mask_t& mask = mutableMask(PtypeFuns<PTYPE>::indexNum);
      mask |= mask_t::makeDirect(PtypeFuns<PtypeFuns<PTYPE>::basicType>::indexMask);
    }
  };

  inline const PieceMask operator&(const PieceMask &m1, const PieceMask &m2) {
    return PieceMask64(m1.getMask(0)&m2.getMask(0));
  }
  inline const PieceMask operator|(const PieceMask &m1, const PieceMask &m2) {
    return PieceMask64(m1.getMask(0)|m2.getMask(0));
  }
  inline const PieceMask operator~(const PieceMask &m1) {
    return PieceMask64(~m1.getMask(0));
  }

  inline bool operator==(const PieceMask &m1, const PieceMask &m2){
    return m1.getMask(0)==m2.getMask(0) && m1.getMask(1)==m2.getMask(1);
  }
  inline bool operator!=(const PieceMask &m1, const PieceMask &m2)
  {
    return ! (m1 == m2);
  }
  std::ostream& operator<<(std::ostream& os,PieceMask const& pieceMask);
} // namespace osl


#endif /* _PIECE_MASK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
