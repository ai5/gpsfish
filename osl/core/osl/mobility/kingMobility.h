/* kingMobility.h
 */
#ifndef _KING_MOBILITY_H
#define _KING_MOBILITY_H

#include "osl/basic_type.h"
#include "osl/container.h"
#include "osl/config.h"
#include <cassert>

#ifndef OSL_USE_SSE
#if !(defined _MSC_VER) && ! defined OSL_NO_SSE
#define OSL_USE_SSE 1
#endif
#endif

namespace osl
{
  namespace mobility
  {
#if OSL_USE_SSE
    typedef long long v2di __attribute__ ((vector_size (16)));
#endif
    class KingMobility{
      union b128{
	CArray<CArray<unsigned char,8>,2> uc16;
	unsigned long long ul[2];
#if OSL_USE_SSE
	v2di v2;
#endif
      } v
#ifdef __GNUC__
      __attribute__((aligned(16)))
#endif
	;
    public:
      KingMobility() {
	assert(reinterpret_cast<size_t>(this) % 16 == 0);
      }
      const CArray<unsigned char,8>& operator[](Player p) const{
	return v.uc16[p];
      }
      CArray<unsigned char,8>& operator[](Player p){
	return v.uc16[p];
      }
      KingMobility& operator=(KingMobility const& km){
#if OSL_USE_SSE
	v.v2=km.v.v2;
#else
	v.uc16=km.v.uc16;
#endif
	return *this;
      }
      bool operator==(KingMobility const& km) const{
#if 0 && OSL_USE_SSE41
	return __builtin_ia32_ptestz128(v.v2,km.v.v2);
#else
	return ((v.ul[0]^km.v.ul[0])|(v.ul[1]^km.v.ul[1]))==0;
#endif
      }
    };
  }
  using mobility::KingMobility;
}
#endif /* _KING_MOBILITY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
