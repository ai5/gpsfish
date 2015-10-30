#ifndef OSL_MISC_MASK_H
#define OSL_MISC_MASK_H
#include "osl/config.h"
#include <cassert>
#include <iosfwd>

namespace osl
{
  namespace misc
  {
  /**
   * x86 bsf 命令
   */
  template <class Integer> struct Bsf;

  template <> 
  struct Bsf<unsigned int>
  {
    static int bsf(unsigned int mask)
    {
      assert(mask);
#ifdef __x86_64__
      int ret;
      __asm__("bsfl %1,%0" : "=r"(ret) : "r"(mask));
      return ret;
#else /* これ以下は 32bit 環境 */
#  ifdef __i386__
      int ret;
      __asm__("bsfl %1,%0" : "=r"(ret) : "r"(mask));
      return ret;
#  elif defined __GNUC__
      return __builtin_ctzl(mask);
#  else
      for (int i=0;i<32;i++)
	if (mask & (1<<i))
	  return i;
#  endif
#endif
      assert(0);
      return -1;
    }
  };
  template <> 
  struct Bsf<unsigned short>
  {
    static unsigned short bsf(unsigned short mask)
    {
      assert(mask);
#ifdef __x86_64__
      unsigned short ret;
      __asm__("bsfw %1,%0" : "=r"(ret) : "r"(mask));
      return ret;
#else /* これ以下は 32bit 環境 */
#  ifdef __i386__
      unsigned short ret;
      __asm__("bsfw %1,%0" : "=r"(ret) : "r"(mask));
      return ret;
#  else
      return __builtin_ctzl(mask);
#  endif
#endif
    }
  };
  
  template <> 
  struct Bsf<unsigned long long>
  {
    static int bsf(unsigned long long mask)
    {
      assert(mask);
#ifdef __x86_64__
      long long ret;
      __asm__("bsfq %1,%0" : "=r"(ret) : "r"(mask));
      return static_cast<int>(ret);
#else /* これ以下は 32bit 環境 */
      unsigned int mask32 = static_cast<unsigned int>(mask);
      if (mask32)
	return Bsf<unsigned int>::bsf(mask32);
      mask32 = static_cast<unsigned int>(mask >> 32);
      return 32+Bsf<unsigned int>::bsf(mask32);
#endif
    }
  };

  template <class Integer> struct Bsr;

  template <> 
  struct Bsr<unsigned int>
  {
    static int bsr(unsigned int mask)
    {
      assert(mask);
#ifdef __x86_64__
      int ret;
      __asm__("bsrl %1,%0" : "=r"(ret) : "r"(mask));
      return ret;
#else /* これ以下は 32bit 環境 */
#  ifdef __i386__
      int ret;
      __asm__("bsrl %1,%0" : "=r"(ret) : "r"(mask));
      return ret;
#  elif __GNUC__
      return __builtin_clzl(mask);
#  else
      for (int i=31; i>=0; --i)
	if (mask & (1<<i))
	  return i;
#  endif
#endif
      assert(0);
      return -1;
    }
  };
  
  template <> 
  struct Bsr<unsigned long long>
  {
    static int bsr(unsigned long long mask)
    {
      assert(mask);
#ifdef __x86_64__
      long long ret;
      __asm__("bsrq %1,%0" : "=r"(ret) : "r"(mask));
      return static_cast<int>(ret);
#else /* これ以下は 32bit 環境 */
      unsigned int mask32 = static_cast<unsigned int>(mask >> 32);
      if (mask32)
	return 32+Bsr<unsigned int>::bsr(mask32);
      mask32 = static_cast<unsigned int>(mask);
      return Bsr<unsigned int>::bsr(mask32);
#endif
    }
  };
  
  struct BitOp
  {
    template <class Integer>
    static int bsf(Integer mask)
    {
      return Bsf<Integer>::bsf(mask);
    }
    template <class Integer>
    static int bsr(Integer mask)
    {
      return Bsr<Integer>::bsr(mask);
    }
    template <class Integer>
    static int takeOneBit(Integer& mask){
      assert(mask);
      const int num=bsf(mask);
      mask &= mask-1;
      return num;
    }

    template <class Integer>
    static int
#ifdef __GNUC__
	__attribute__ ((const))
#endif
    countBit(Integer mask)
    {
      int count=0;
      while (mask) 
      {
	++count;
	mask &= (mask-1);
      }
      return count;
    }
    template <class Integer>
    static bool hasMultipleBit(Integer mask){
      assert(mask);
      return (mask & (mask-1));
    }
    /**
     * non-zeroのmaskのsetされているビットをLSBから探し，そのビットだけがsetされたmaskを返す.
     */
    template <class Integer>
    static Integer lowestBit(Integer mask){
      assert(mask);
      return static_cast<Integer>(mask & (-mask));
    }
  };
  
#if 0
  /**
   * bit数が多い時は?
   * population countは
   * Cray, CDC Cyber series, Alliant FX/80.
   * UltraSPARC, Alpha 21264
   * Intel x86 has the parity of a byte
   *#define bitcount(n)                                                     \
   * (tmp = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111), \
   * tmp = ((tmp + (tmp >> 3)) & 030707070707),                        \
   * tmp =  (tmp + (tmp >> 6)),                                        \
   * tmp = (tmp + (tmp >> 12) + (tmp >> 24)) & 077)
   *
   */
  inline int countBitDense(unsigned int mask)
  {
    mask = ((mask>>1)&0x55555555)+(mask&0x55555555);
    mask = ((mask>>2)&0x33333333)+(mask&0x33333333);
    mask = ((mask>>4)+mask)&0xf0f0f0f;
    mask = (mask>>8)+mask;
    return ((mask>>16)+mask)&0x3f;
  }
#endif
  } // namespace misc
  namespace misc
  {
    template <class Integer>
    class GeneralMask
    {
      Integer mask;
    private:
      GeneralMask(Integer value) : mask(value) {}
    public:
      GeneralMask() : mask(0) {}
      static const GeneralMask makeDirect(Integer value) { return GeneralMask(value); }
      GeneralMask& operator&=(const GeneralMask& r)
      {
	mask &= r.mask;
	return *this;
      }
      GeneralMask& operator|=(const GeneralMask& r)
      {
	mask |= r.mask;
	return *this;
      }
      GeneralMask& operator^=(const GeneralMask& r)
      {
	mask ^= r.mask;
	return *this;
      }
      GeneralMask& operator-=(const GeneralMask& r)
      {
	mask -= r.mask;
	return *this;
      }
      GeneralMask& operator+=(const GeneralMask& r)
      {
	mask += r.mask;
	return *this;
      }
      GeneralMask& operator<<=(int shift)
      {
	mask <<= shift;
	return *this;
      }
      GeneralMask& operator>>=(int shift)
      {
	mask >>= shift;
	return *this;
      }
      const GeneralMask operator~() const { return GeneralMask(~mask); }

      int bsf() const { return BitOp::bsf(mask); }
      int bsr() const { return BitOp::bsr(mask); }
      /**
       * non-zeroのmaskのsetされているビットをLSBから探し，その番号を返す
       * 副作用としてmaskの対応するビットをクリアする
       * @param mask - 対象とするデータ(non-zero)
       * @return - どのビットか
       */
      int takeOneBit() { return BitOp::takeOneBit(mask); }
  
      /**
       * non-zeroのmaskが複数ビットセットされているかどうかを返す.
       * @param mask - 対象とするデータ(non-zero)
       * @return - 複数ビットがセットされているか？
       */
      bool hasMultipleBit() const { return BitOp::hasMultipleBit(mask); }
      /**
       * non-zeroのmaskにセットされているビットの数を2まで数える．
       * @param mask - 対象とするデータ(non-zero)
       * @return 1,2 (2の場合は2以上)
       */
      int countBit2() const {
	assert(mask);
	return (mask & (mask-1)) ? 2 : 1;
      }
      /**
       * mask にセットされているビットの数を数える．
       * あまり速くない．
       */
      int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      countBit() const { return BitOp::countBit(mask); }
      /**
       * non-zeroのmaskのsetされているビットをLSBから探し，そのビットだけがsetされたmaskを返す.
       * @param mask - 対象とするデータ(non-zero)
       * @return - そのビットだけがsetされたmask
       */
      GeneralMask lowestBit() const { return BitOp::lowestBit(mask); }
      bool none() const { return mask == 0; }
      bool any() const { return ! none(); }
      Integer value() const { return mask; }
    };

    template <class Integer> inline
    bool operator==(const GeneralMask<Integer>& l, const GeneralMask<Integer>& r)
    {
      return l.value() == r.value();
    }
    template <class Integer> inline
    bool operator!=(const GeneralMask<Integer>& l, const GeneralMask<Integer>& r)
    {
      return ! (l == r);
    }
    template <class Integer> inline
    bool operator<(const GeneralMask<Integer>& l, const GeneralMask<Integer>& r)
    {
      return l.value() < r.value();
    }

    template <class Integer> inline
    const GeneralMask<Integer> operator&(GeneralMask<Integer> l,
					GeneralMask<Integer> r) {
      GeneralMask<Integer> result = l;
      return result &= r;
    }
    template <class Integer> inline
    const GeneralMask<Integer> operator|(GeneralMask<Integer> l,
					GeneralMask<Integer> r) {
      GeneralMask<Integer> result = l;
      return result |= r;
    }
    template <class Integer> inline
    const GeneralMask<Integer> operator^(GeneralMask<Integer> l,
					GeneralMask<Integer> r) {
      GeneralMask<Integer> result = l;
      return result ^= r;
    }
    template <class Integer> inline
    const GeneralMask<Integer> operator<<(GeneralMask<Integer> m, int shift) {
      GeneralMask<Integer> result = m;
      return result <<= shift;
    }
    template <class Integer> inline
    const GeneralMask<Integer> operator>>(GeneralMask<Integer> m, int shift) {
      GeneralMask<Integer> result = m;
      return result >>= shift;
    }
  
    typedef GeneralMask<unsigned long long> Mask64;


    typedef unsigned long long mask_int_t;
    typedef GeneralMask<mask_int_t> mask_t;

    std::ostream& operator<<(std::ostream&, const mask_t&);
  } // namespace misc
  using misc::mask_int_t;
  using misc::mask_t;
  using misc::Mask64;
  using misc::BitOp;
} // namespace osl

#endif
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
