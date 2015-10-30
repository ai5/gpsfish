#ifndef OSL_NUM_BITMAP_EFFECT_H
#define OSL_NUM_BITMAP_EFFECT_H

#include "osl/basic_type.h"
#include "osl/bits/ptypeTraits.h"
#include "osl/bits/pieceMask.h"

namespace osl 
{
  namespace effect
  {
    /**
     * 現在の定義 (2005/3/4以降)
     * - 0-39 : 0-39の利き
     * - 40-47 : 32-39の長い利き
     * - 48-53 : 黒の利きの数(sentinelを合わせて6bit)
     * - 54-59 : 白の利きの数(sentinelを合わせて6bit)
     *
     * 以前の定義 (2004/4/13以降)
     * - 0-39 : 0-39の利き
     * - 40-47 : 32-39の長い利き
     *
     * 以前の定義
     * - 0-31 : 0-31の直接効き
     * - 32-39 : 32-39の短い効き(obsolete および32-39の短い間接効き)
     * - 40-47 : 32-39の長い効き(obsoleteおよび32-39の長い間接効き)
     */
    class NumBitmapEffect : public PieceMask
    {
    public:
      NumBitmapEffect(){
	resetAll();
      }
      template<Player P>
      static NumBitmapEffect playerEffect(){
	NumBitmapEffect ret;
	if (P == BLACK) ret.flip(48);
	else ret.flip(54);
	return ret;
      }
      static NumBitmapEffect playerEffect(Player pl){
	mask_t mask1=numToMask(54);
	mask1-=numToMask(48);
	mask1&=mask_t::makeDirect(pl);
	mask1+=numToMask(48);
	NumBitmapEffect ret;
	ret.setMask(1,mask1);
	assert((pl==BLACK && ret==playerEffect<BLACK>()) ||
	       (pl==WHITE && ret==playerEffect<WHITE>()));
	return ret;
      }
      template<Player P>
      static mask_t playerEffectMask(){
	if (P == BLACK) {
	  mask_t mask1=numToMask(54);
	  mask1-=numToMask(48);
	  return mask1;
	} else {
	  mask_t mask1=numToMask(60);
	  mask1-=numToMask(54);
	  return mask1;
	}
      }

      static mask_t playerEffectMask(Player pl){
	mask_t mask1=numToMask(60);
	mask1-=numToMask(48);
	mask1&=mask_t::makeDirect(pl);
	// pl==BLACK -> mask1 = 0
	// pl==WHITE -> mask1 = 0x0fff0000(32bit), 0x0fff000000000000(64bit)
	mask_t mask2=numToMask(54);
	mask2-=numToMask(48);
	// mask2 = 0x3f0000(32bit), 0x3f000000000000(64bit)
	mask1^=mask2;
	// pl==BLACK -> mask1 = 0x3f0000(32bit), 0x3f000000000000(64bit)
	// pl==WHITE -> mask2 = 0x0fc00000(32bit), 0x0fc0000000000000(64bit)
	assert((pl==BLACK && mask1==playerEffectMask<BLACK>()) ||
	       (pl==WHITE && mask1==playerEffectMask<WHITE>()));
	return mask1;
      }
      int countEffect(Player pl) const {
	int shift=48+(6&pl);
	mask_t mask=getMask(1);
	mask>>=numToOffset(shift);
	mask&=mask_t::makeDirect(0x3f);
	return static_cast<int>(mask.value());
      }

      template<Player P>
      static NumBitmapEffect makeEffect(int num){
	NumBitmapEffect effect=playerEffect<P>();
	effect.flip(num);
	return effect;
      }
      enum Op{
	Add,Sub,
      };
      template<Op OP>
      NumBitmapEffect& opEqual(NumBitmapEffect const& rhs){
	if (OP == Add)
	  *this+=rhs;
	else 
	  *this-=rhs;
	return *this;
      }

      static const mask_t longEffectMask() {
#if OSL_WORDSIZE == 64
	return mask_t::makeDirect(0xff0000000000uLL);
#elif OSL_WORDSIZE == 32
	return mask_t::makeDirect(0xff00u);
#endif  
      }
#if OSL_WORDSIZE == 64
      static const int longToNumOffset=-8;
#elif OSL_WORDSIZE == 32
      static const int longToNumOffset=32-8;
#endif
      static const mask_t makeLongMask(int num)
      {
	return mask_t::makeDirect(0x101) << PieceMask::numToOffset(num);
      }
      template<Player P>
      static NumBitmapEffect makeLongEffect(int num){
	assert(32<=num && num<=39);
	NumBitmapEffect effect=NumBitmapEffect::playerEffect<P>();
	effect.orMask(1,makeLongMask(num));
	return effect;
      }
      static NumBitmapEffect makeLongEffect(Player pl,int num){
	assert(32<=num && num<=39);
	NumBitmapEffect effect=NumBitmapEffect::playerEffect(pl);
	effect.orMask(1,makeLongMask(num));
	return effect;
      }

      // utility methods
      const mask_t selectLong() const 
      {
	return (getMask(1) & longEffectMask());
      }
      bool hasLong() const 
      {
	return selectLong().any();
      }
      template <Ptype PTYPE> const mask_t selectLong() const 
      {
	return selectLong()
		& mask_t::makeDirect(PtypeFuns<PTYPE>::indexMask << 8);
      }
      template <Ptype PTYPE> bool hasLong() const 
      {
	return selectLong<PTYPE>().any();
      }
      template <Ptype PTYPE> bool hasAny() const 
      {
	return (getMask(PtypeFuns<PTYPE>::indexNum)
		& mask_t::makeDirect(PtypeFuns<PTYPE>::indexMask)).any();
      }
    };
  } // namespace effect
} // namespace osl
#endif // _NUM_BITMAP_EFFECT_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
