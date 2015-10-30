/* offset32.h
 */
#ifndef OSL_OFFSET32_H
#define OSL_OFFSET32_H

#include "osl/basic_type.h"

namespace osl 
{
  /**
   * 差が uniqになるような座標の差分.
   * x*32+y同士の差を取る
   * ちょっとだけ溢れても良い
   */
  template <int Width, int Width2>
  class Offset32Base
  {
    enum {
      MIN = -(Width*32+Width),
      MAX = (Width*32+Width),
    };
  public:
    static const unsigned int SIZE=(MAX-MIN+1);
  private:
    int offset32;
    explicit Offset32Base(int o) : offset32(o)
    {
    }
  public:
    Offset32Base(Square to, Square from)
      : offset32(to.indexForOffset32()-from.indexForOffset32())
    {
      assert((to.x()-from.x() >= -Width) && (to.x()-from.x() <= Width) 
	     && (to.y()-from.y() >= -Width) && (to.y()-from.y() <= Width));
      assert(MIN<=offset32 && offset32<=MAX);
    }
    Offset32Base(int dx,int dy) : offset32(dx*32+dy) {
      assert(-Width2<=dx && dx<=Width2 && -Width2<=dy && dy<=Width2);
    }
    unsigned int index() const
    {
      return offset32 - MIN;
    }
    bool isValid() const
    {
      return MIN <=offset32 && offset32 <= MAX;
    }
    /**
     * Player P からみた offset を黒番のものに変更する
     */
    template<Player P>
    const Offset32Base blackOffset32() const { 
      return P == BLACK ? *this : Offset32Base(-offset32); 
    }
    const Offset32Base operator-() const { return Offset32Base(-offset32); }
  private:
    // these functions are *intentionally* unimplemented for the moment.
    // don't forget the fact that x or y can be negative.
    int dx(Offset32Base offset32);
    int dy(Offset32Base offset32);
  };

  typedef Offset32Base<8,9> Offset32;
  typedef Offset32Base<10,10> Offset32Wide;
} // namespace osl


#endif /* OSL_OFFSET32_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
