/* quadInt.h
 */
#ifndef EVAL_CONTAINER_QUAD_INT_H
#define EVAL_CONTAINER_QUAD_INT_H

#include "osl/config.h"
#include "osl/container.h"
#include "osl/bits/align16New.h"

#if (defined __INTEL_COMPILER || defined __clang__)
#  include <emmintrin.h>
#  ifdef __INTEL_COMPILER
#    define __builtin_ia32_pxor128 _mm_xor_pd
#  endif
#  define __builtin_ia32_psubd128 _mm_sub_epi32
#  define __builtin_ia32_paddd128 _mm_add_epi32
#endif

#ifndef OSL_NO_SSE
#if (defined __x86_64__) || (defined __i386__)
#  ifndef OSL_USE_SSE
#  define OSL_USE_SSE 1
#  endif
#else
#  warning "QuadInt without SSE"
#endif
#endif

namespace osl
{
  namespace container
  {
#ifdef OSL_USE_SSE
#  ifdef __INTEL_COMPILER
    typedef __v4si v4si;
    typedef __v2di v2di;
#  else
    typedef int v4si __attribute__ ((vector_size (16)));
    typedef long long v2di __attribute__ ((vector_size (16)));
#  endif
#endif
    struct QuadInt : public misc::Align16New
    {
      union XMM{
	CArray<int,4> iv;
	CArray<long long,2> llv;
#ifdef OSL_USE_SSE
	v4si v4;
	v2di v2;
#endif
      } v
#ifdef OSL_USE_SSE
      __attribute__((aligned(16)))
#endif
	;
      QuadInt(){
	clear();
      }
      QuadInt(QuadInt const& si){
#if OSL_USE_SSE
	v.v4=si.v.v4;
#else
	v.llv = si.v.llv;
#endif
      }
      QuadInt& operator=(QuadInt const& si) 
      {
#if OSL_USE_SSE
	v.v4=si.v.v4;
#else
	v.llv = si.v.llv;
#endif
	return *this;
      }
      void clear()
      {
#if OSL_USE_SSE
	v.v4=(v4si){ 0, 0, 0, 0 };
#else
	v.llv[0] = v.llv[1] = 0;
#endif
      }
      int& operator[](int i) { 
	return v.iv[i]; 
      }
      const int& operator[](int i) const { 
	return v.iv[i]; 
      }
      QuadInt operator-() const{
	QuadInt ret;
	ret -= *this;
	return ret;
      }
      QuadInt& operator+=(QuadInt const& si){
#if OSL_USE_SSE
	v.v4=__builtin_ia32_paddd128(v.v4,si.v.v4);
#else
	for(int i=0;i<4;i++) v.iv[i]+=si.v.iv[i];
#endif	  
	return *this;
      }
      QuadInt& operator-=(QuadInt const& si){
#if OSL_USE_SSE
	v.v4=__builtin_ia32_psubd128(v.v4,si.v.v4);
#else
	for(int i=0;i<4;i++) v.iv[i]-=si.v.iv[i];
#endif	  
	return *this;
      }
      QuadInt& operator*=(int scale){
#if OSL_USE_SSE41
	XMM val;
	unsigned long long scalescale=(unsigned long long )((unsigned int)scale);
	scalescale|=scalescale<<32ull;
	val.v2=__builtin_ia32_vec_set_v2di(val.v2,(long long)scalescale,0);
	val.v2=__builtin_ia32_vec_set_v2di(val.v2,(long long)scalescale,1);
	v.v4=__builtin_ia32_pmulld128(v.v4,val.v4);
#else
	for(int i=0;i<4;i++) v.iv[i]*=scale;
#endif
	return *this;
      }
      static size_t size() { return 4; }
    };
    inline QuadInt operator+(QuadInt const& si0,QuadInt const& si1)
    {
      QuadInt ret(si0);
      ret+=si1;
      return ret;
    }
    inline QuadInt operator-(QuadInt const& si0,QuadInt const& si1)
    {
      QuadInt ret(si0);
      ret-=si1;
      return ret;
    }
    inline QuadInt operator*(QuadInt const& si0,int scale)
    {
      QuadInt ret(si0);
      ret*=scale;
      return ret;
    }
    inline bool operator==(QuadInt const& l,QuadInt const& r)
    {
      return l.v.llv[0] == r.v.llv[0] && l.v.llv[1] == r.v.llv[1];
    }
    inline bool operator<(QuadInt const& l,QuadInt const& r)
    {
      if (l.v.llv[0] != r.v.llv[0])
	return (l.v.llv[0] < r.v.llv[0]);
      return l.v.llv[1] < r.v.llv[1];
    }    

    class QuadIntPair
    {
      CArray<QuadInt,2> v;
    public:
      QuadIntPair() {}
      const QuadInt& operator[](int i) const{
	return v[i];
      }
      const QuadInt& operator[](Player pl) const{
	return v[pl];
      }
      QuadInt& operator[](int i){
	return v[i];
      }
      QuadInt& operator[](Player pl){
	return v[pl];
      }
      QuadIntPair& operator+=(QuadIntPair const& a){
	v[0]+=a.v[0];
	v[1]+=a.v[1];
	return *this;
      }
      QuadIntPair& operator-=(QuadIntPair const& a){
	v[0]-=a.v[0];
	v[1]-=a.v[1];
	return *this;
      }
    };
    inline QuadIntPair operator+(QuadIntPair const& si0,QuadIntPair const& si1)
    {
      QuadIntPair ret(si0);
      ret+=si1;
      return ret;
    }
    inline QuadIntPair operator-(QuadIntPair const& si0,QuadIntPair const& si1)
    {
      QuadIntPair ret(si0);
      ret-=si1;
      return ret;
    }
    inline bool operator==(QuadIntPair const& l,QuadIntPair const& r)
    {
      return l[0] == r[0] && l[1] == r[1];
    }
  }
  
  using container::QuadInt;
  using container::QuadIntPair;
}
#endif // EVAL_CONTAINER_QUAD_INT_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
