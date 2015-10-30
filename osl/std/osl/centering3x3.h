/* centering3x3.h
 */
#ifndef OSL_CENTERING3X3_H
#define OSL_CENTERING3X3_H

#include "osl/container.h"

namespace osl
{
  /**
   * 3x3が盤上におさまるように中心を調整
   */
  struct Centering3x3
  {
    struct Table 
    {
      CArray<Square,Square::SIZE> centers;
      void init();
    };
    static const Square adjustCenterNaive(Square);
    static Table table;
    static const Square adjustCenter(Square src)
    {
      return table.centers[src.index()];
    }
  };

} // namespace osl

#endif /* OSL_CENTERING3X3_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
