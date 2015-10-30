/* tripleInt.h
 */
#ifndef EVAL_CONTAINER_TRIPLE_INT_H
#define EVAL_CONTAINER_TRIPLE_INT_H
#include "osl/carray.h"
#include "osl/align16New.h"
#include "osl/config.h"
#include <iosfwd>

#if (defined __INTEL_COMPILER || defined __clang__)
#  include <emmintrin.h>
#  define __builtin_ia32_pxor128 _mm_xor_si128
#  define __builtin_ia32_psubd128 _mm_sub_epi32
#  define __builtin_ia32_paddd128 _mm_add_epi32
#endif

#ifndef OSL_NO_SSE
#if (defined __x86_64__) || (defined __i386__)
#  ifndef OSL_USE_SSE
#    define OSL_USE_SSE 1
#  endif
#else
#  warning "TripleInt without SSE"
#endif
#endif

namespace osl
{
  namespace container
  {
#ifndef OSL_USE_SSE
    typedef CArray<int32_t,4> v4si;
    typedef CArray<int64_t,2> v2di;
#elif defined __INTEL_COMPILER
    typedef __v4si v4si;
    typedef __v2di v2di;
#else
    typedef int v4si __attribute__ ((vector_size (16)));
    typedef long long v2di __attribute__ ((vector_size (16)));
#endif
    struct TripleInt : public misc::Align16New
    {
      union XMM{
	CArray<int,4> iv;
	v4si v4;
	v2di v2;
      } v
#ifdef __GNUC__
      __attribute__((aligned(16)))
#endif
	;
      TripleInt(){
#if OSL_USE_SSE
	assert(reinterpret_cast<size_t>(this) % 16 == 0);
#endif
	clear();
      }
      TripleInt(TripleInt const& si){
#if OSL_USE_SSE
	assert(reinterpret_cast<size_t>(this) % 16 == 0);
	v.v4=si.v.v4;
#else
	for(int i=0;i<3;i++) v.iv[i]=si.v.iv[i];
#endif
      }
      TripleInt(int a, int b, int c){
#if OSL_USE_SSE
	assert(reinterpret_cast<size_t>(this) % 16 == 0);
	v.v4 = (v4si){a, b, c, 0};	
#elif __GNUC__
	v.iv = (CArray<int,4>){{a, b, c, 0}};
#else
	v.iv[0] = a, v.iv[1] = b, v.iv[2] = c;
#endif
      }
      void clear()
      {
#if OSL_USE_SSE
	v.v4=(v4si){ 0, 0, 0, 0 };
#else
	for(int i=0;i<3;i++) v.iv[i]=0;
#endif
      }
      int& operator[](int i) { 
	return v.iv[i]; 
      }
      const int& operator[](int i) const { 
	return v.iv[i]; 
      }
      TripleInt operator-() const{
	TripleInt ret;
#if OSL_USE_SSE
	ret.v.v4=__builtin_ia32_psubd128(ret.v.v4,v.v4);
#else
	for(int i=0;i<3;i++) ret.v.iv[i]= -v.iv[i];
#endif	  
	return ret;
      }
      TripleInt& operator+=(TripleInt const& si){
#if OSL_USE_SSE
	v.v4=__builtin_ia32_paddd128(v.v4,si.v.v4);
#else
	for(int i=0;i<3;i++) v.iv[i]+=si.v.iv[i];
#endif	  
	return *this;
      }
      TripleInt& operator-=(TripleInt const& si){
#if OSL_USE_SSE
	v.v4=__builtin_ia32_psubd128(v.v4,si.v.v4);
#else
	for(int i=0;i<3;i++) v.iv[i]-=si.v.iv[i];
#endif	  
	return *this;
      }
      TripleInt& operator*=(int scale){
#if OSL_USE_SSE41
	XMM val;
	unsigned long long scalescale=(unsigned long long )((unsigned int)scale);
	scalescale|=scalescale<<32ull;
	val.v2=__builtin_ia32_vec_set_v2di(val.v2,(long long)scalescale,0);
	val.v2=__builtin_ia32_vec_set_v2di(val.v2,(long long)scalescale,1);
	v.v4=__builtin_ia32_pmulld128(v.v4,val.v4);
#else
	for(int i=0;i<3;i++) v.iv[i]*=scale;
#endif
	return *this;
      }
      TripleInt& operator/=(int div)
      {
	for(int i=0;i<3;i++) v.iv[i] /= div;
	return *this;
      }
      TripleInt& operator>>=(int shift)
      {
#if OSL_USE_SSE
	v.v4= __builtin_ia32_psradi128 (v.v4, shift);
#else
	for(int i=0;i<3;i++) v.iv[i] >>= shift;
#endif	  
	return *this;
      }
      static size_t size() { return 3; }
    };
    inline TripleInt operator+(TripleInt const& si0,TripleInt const& si1)
    {
      TripleInt ret(si0);
      ret+=si1;
      return ret;
    }
    inline TripleInt operator-(TripleInt const& si0,TripleInt const& si1)
    {
      TripleInt ret(si0);
      ret-=si1;
      return ret;
    }
    inline TripleInt operator*(TripleInt const& si0,int scale)
    {
      TripleInt ret(si0);
      ret*=scale;
      return ret;
    }
    inline bool operator==(TripleInt const& l,TripleInt const& r)
    {
      for(int i=0;i<3;i++) 
	if (l[i] != r[i])
	  return false;
      return true;
    }
    
    class TripleIntPair{
      CArray<TripleInt,2> v;
    public:
      TripleIntPair() {}
      const TripleInt& operator[](int i) const{
	return v[i];
      }
      const TripleInt& operator[](Player pl) const{
	return v[pl];
      }
      TripleInt& operator[](int i){
	return v[i];
      }
      TripleInt& operator[](Player pl){
	return v[pl];
      }
      TripleIntPair& operator+=(TripleIntPair const& a){
	v[0]+=a.v[0];
	v[1]+=a.v[1];
	return *this;
      }
      TripleIntPair& operator-=(TripleIntPair const& a){
	v[0]-=a.v[0];
	v[1]-=a.v[1];
	return *this;
      }
    };
    inline TripleIntPair operator+(TripleIntPair const& si0,TripleIntPair const& si1)
    {
      TripleIntPair ret(si0);
      ret+=si1;
      return ret;
    }
    inline TripleIntPair operator-(TripleIntPair const& si0,TripleIntPair const& si1)
    {
      TripleIntPair ret(si0);
      ret-=si1;
      return ret;
    }
    inline bool operator==(TripleIntPair const& l,TripleIntPair const& r)
    {
      return l[0] == r[0] && l[1] == r[1];
    }
    std::ostream& operator<<(std::ostream& os,TripleInt const& ti);
  }
  using container::TripleInt;
  using container::TripleIntPair;
}
#endif // EVAL_CONTAINER_TRIPLE_INT_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
