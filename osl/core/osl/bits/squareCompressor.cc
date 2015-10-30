/* squareCompressor.cc
 */
#include "osl/bits/squareCompressor.h"
#include "osl/basic_type.h"
#include <algorithm>
namespace osl
{
  CArray<signed char, Square::SIZE> SquareCompressor::positionToIndex;

  class SquareCompressor::Initializer
  {
  public:
    Initializer()
    {
      std::fill(positionToIndex.begin(), positionToIndex.end(), -1);

      int cur = 0;
      positionToIndex[0] = cur++;
      for (int x=1; x<=9; ++x)
      {
	for (int y=1; y<=9; ++y)
	{
	  positionToIndex[Square(x,y).index()] = cur++;
	}
      }
      assert(cur == 82);
    }
  };
    
  namespace 
  {
    SquareCompressor::Initializer init;
  } // anonymous namespace
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
