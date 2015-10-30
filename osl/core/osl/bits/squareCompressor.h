/* squareCompressor.h
 */
#ifndef OSL_POSITIONCOMPRESSOR_H
#define OSL_POSITIONCOMPRESSOR_H

#include "osl/basic_type.h"
#include "osl/container.h"
namespace osl
{

  /**
   * Square を [0..81] に圧縮する
   * 0: 駒台，1..81 盤上
   */
  struct SquareCompressor
  {
  private:
    /** 本当はconst にしたいけど初期化が手間なので後回し */
    static CArray<signed char, Square::SIZE> positionToIndex;
  public:
    class Initializer;
    friend class Initializer;

    static int compress(Square pos)
    {
      const int result = positionToIndex[pos.index()];
      assert(result >= 0);
      return result;
    }
    static Square
#ifdef __GNUC__
__attribute__ ((noinline))
#endif
      melt(int index)
    {
      assert(0 <= index);
      assert(index < 82);
      if (index == 0)
	return Square::STAND();
      --index;
      return Square(index/9+1, index%9+1);
    }
  };

} // namespace osl

#endif /* OSL_POSITIONCOMPRESSOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
