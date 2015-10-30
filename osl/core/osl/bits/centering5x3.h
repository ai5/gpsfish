/* centering5x3.h
 */
#ifndef OSL_CENTERING5X3_H
#define OSL_CENTERING5X3_H

#include "osl/basic_type.h"
#include "osl/container.h"

namespace osl
{
  /**
   * 5x3が盤上におさまるように中心を調整
   */
  struct Centering5x3
  {
    struct Table 
    {
      CArray<Square,Square::SIZE> centers;
      Table();
    };
    static const Square adjustCenterNaive(Square);
    static const Table table;
    static const Square adjustCenter(Square src)
    {
      return table.centers[src.index()];
    }
  };

} // namespace osl

#endif /* OSL_CENTERING5X3_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
