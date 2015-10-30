/* mobilityTable.h
 */
#ifndef _MOBILITY_TABLE_H
#define _MOBILITY_TABLE_H
#include "osl/basic_type.h"
#include "osl/simpleState.h"
#include <cassert>
#include <iosfwd>

namespace osl
{
  namespace mobility
  {
    union V4 {
      unsigned int lv;
      CArray<unsigned char,4> uc;
    }
#ifdef __GNUC__
  __attribute__((aligned(4)))
#endif
    ;
    /**
     * 駒毎に指定の方向の利きを持つ最後のSquare.
     * 自分の駒への利きも含む
     * EDGEまでいく
     * 方向は「黒」から見た方向に固定
     * そもそもそちらに利きがない場合やSTANDにある場合は0
     */
    class MobilityContent
    {
      V4 v;
    public:
      MobilityContent() {
	clear();
      }
      void clear(){
	v.lv=0u;
      }
      const Square get(Direction d) const{
	return Square::makeDirect(v.uc[((unsigned int)d)>>1]);
      }
      void set(Direction d,Square pos){
	v.uc[((unsigned int)d)>>1]=static_cast<unsigned char>(pos.uintValue());
      }
    };
    std::ostream& operator<<(std::ostream& os,MobilityContent const& mc);

    /**
     * 駒番号からMobilityContentを得る
     */
    class MobilityTable
    {
      CArray<MobilityContent,8> table
#ifdef __GNUC__
      __attribute__((aligned(16)))
#endif
	;
    public:
      MobilityTable(){}
      MobilityTable(SimpleState const& state);
      void set(Direction d,int num,Square pos){
	assert(0<=(int)d && (int)d<=7);
	return table[num-32].set(d,pos);
      }
      const Square get(Direction d,int num) const{
	assert(0<=(int)d && (int)d<=7);
	return table[num-32].get(d);
      }
      friend bool operator==(const MobilityTable& mt1,const MobilityTable& mt2);
    };
    std::ostream& operator<<(std::ostream& os,MobilityTable const& mt);
    bool operator==(const MobilityTable&,const MobilityTable&);
  }
  using mobility::MobilityTable;
}
#endif /* _MOBILITY_TABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
