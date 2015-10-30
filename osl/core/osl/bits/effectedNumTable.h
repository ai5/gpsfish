#ifndef _EFFECTED_NUM_TABLE_H
#define _EFFECTED_NUM_TABLE_H
#include "osl/simpleState.h"
#include <iosfwd>
namespace osl
{
  namespace effect
  {
    union Byte8 {
      unsigned long long lv;
      CArray<unsigned char,8> uc;
    }
#ifdef __GNUC__
    __attribute__((aligned(8)))
#endif
      ;
    /**
     * 盤面上の駒が「黒から見た」方向に長い利きをつけられている時に，
     * 利きをつけている駒の番号を得る
     * たとえば，Uの時は下から上方向の長い利きがついているものとする．
     * その方向の利きがついていない場合はEMPTY_NUM(0x80)を入れておく．
     */
    class EffectedNum
    {
    private:
      Byte8 b8;
    public:
      EffectedNum() { clear(); }
      void clear(){
#define E(n) ((static_cast<unsigned long long>(EMPTY_NUM)<<((n)*8)))
	b8.lv= E(0)|E(1)|E(2)|E(3)|E(4)|E(5)|E(6)|E(7);
#undef E
      }
      int operator[](Direction d) const{
	assert(0<=d && d<=7);
	return b8.uc[d];
      }
      unsigned char& operator[](Direction d){
	assert(0<=d && d<=7);
	return b8.uc[d];
      }
    };
    class EffectedNumTable
    {
      CArray<EffectedNum,40> contents
#ifdef __GNUC__
      __attribute__((aligned(16)))
#endif
	;
    public:
      EffectedNumTable() { clear(); }
      EffectedNumTable(SimpleState const&);
      const EffectedNum& operator[](int i) const {
	return contents[i];
      }
      void clear();
      EffectedNum& operator[](int i){
	return contents[i];
      }
    };
    bool operator==(const EffectedNumTable&,const EffectedNumTable&);
    std::ostream& operator<<(std::ostream&,const EffectedNumTable&);
  }
  using effect::EffectedNumTable;
}

#endif // _EFFECTED_NUM_TABLE_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
