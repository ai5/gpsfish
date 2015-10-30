/* boardMask.h
 */
#ifndef OSL_BOARDMASK_H
#define OSL_BOARDMASK_H

#include "osl/basic_type.h"
#include "osl/bits/directionTraits.h"
#include "osl/bits/mask.h"
#include "osl/container.h"
#include <iosfwd>

namespace osl 
{
  namespace container
  {
    class BoardMask;
    bool operator==(const BoardMask&, const BoardMask&);
    std::ostream& operator<<(std::ostream&, const BoardMask&);
    /** 11 x 12 */
    class BoardMask
    {
      /** the third one is only for edge */
      CArray<unsigned long long,3> contents;
    public:
      BoardMask() { invalidate(); }
      BoardMask(const BoardMask& src) { 
	contents[0] = src.contents[0]; 
	contents[1] = src.contents[1]; 
      }
      BoardMask& operator=(const BoardMask& src) { 
	if (this != &src) {
	  contents[0] = src.contents[0]; 
	  contents[1] = src.contents[1]; 
	}
	return *this;
      }
      void clear() { contents[0]=contents[1]=0; }
      void invalidate() { contents[0] = static_cast<uint64_t>(-1); }
      bool isInvalid() const { return contents[0] == static_cast<uint64_t>(-1); }
      void set(unsigned int i) {
	int j=(i>>6);
	contents[j]|=(1ull<<(i&63));
      }
      void set(Square pos) {
	set(index(pos));
      }
      void reset(unsigned int i) {
	int j=(i>>6);
	contents[j] &= ~(1ull<<(i&63));
      }
      void reset(Square pos) { reset(index(pos)); }
      bool test(unsigned int i) const {
	int j=(i>>6);
	return (contents[j]&(1ull<<(i&63)))!=0;
      }
      bool test(Square pos) const { return test(index(pos)); }
      bool anyInRange(const BoardMask& mask) const
      {
	return (contents[0] & mask.contents[0]) 
	  || (contents[1] & mask.contents[1]);
      }
      BoardMask& operator|=(const BoardMask& mask)
      {
	contents[0] |= mask.contents[0];
	contents[1] |= mask.contents[1];
	return *this;
      }
      bool any() const
      {
	assert(! isInvalid());
	return contents[0] || contents[1];
      }
      Square takeOneBit()
      {
	assert(! isInvalid() && any());
	if (contents[0])
	  return toSquare(BitOp::takeOneBit(contents[0]));
	return toSquare(BitOp::takeOneBit(contents[1])+64);
      }
      static int index(int x,int y){ return x*12+y+1; }
      static int index(Square pos) {
	int v=pos.index();
	return v-((v>>2)&0x3c);
      }
      template<Direction Dir,Player P>
      static int getIndexOffset() {
	int blackDx=DirectionTraitsGen<Dir>::blackDx;
	int blackDy=DirectionTraitsGen<Dir>::blackDy;
	int val=blackDx*12+blackDy;
	if(P==BLACK) return val;
	else return -val;
      }
      static Square toSquare(int n) { return Square::makeDirect(n+(((n*21)>>8)<<2)); } 
      friend bool operator==(const BoardMask&, const BoardMask&);
    };
    inline const BoardMask operator|(const BoardMask& l, const BoardMask& r)
    {
      BoardMask result = l;
      result |= r;
      return result;
    }
    inline bool operator==(const BoardMask& l, const BoardMask& r)
    {
      return l.contents[0] == r.contents[0]
	&& l.contents[1] == r.contents[1];
    }
    class BoardMaskTable5x5 
    {
      CArray<BoardMask, Square::SIZE> data;
    public:
      BoardMaskTable5x5();
      /** p中心の5x5 の範囲のbitを立てたもの, centeringなし*/
      const BoardMask& mask(Square p) const { return data[p.index()]; }
    };
    extern const BoardMaskTable5x5 Board_Mask_Table5x5;

    class BoardMaskTable3x3 
    {
      CArray<BoardMask, Square::SIZE> data;
    public:
      BoardMaskTable3x3();
      /** p中心の3x3 の範囲のbitを立てたもの, centeringなし*/
      const BoardMask& mask(Square p) const { return data[p.index()]; }
    };
    extern const BoardMaskTable3x3 Board_Mask_Table3x3;

    class BoardMaskTable5x3Center
    {
      CArray<BoardMask, Square::SIZE> data;
    public:
      BoardMaskTable5x3Center();
      /** p中心の5x3 の範囲のbitを立てたもの, centering*/
      const BoardMask& mask(Square p) const { return data[p.index()]; }
    };
    extern const BoardMaskTable5x3Center Board_Mask_Table5x3_Center;
  } // namespace container
  using container::BoardMask;
  using container::Board_Mask_Table5x5;
  using container::Board_Mask_Table5x3_Center;
  using container::Board_Mask_Table3x3;
} // namespace osl


#endif /* OSL_BOARDMASK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
