/* mobilityTable.h
 */
#ifndef EVAL_MOBILITYTABLE_H
#define EVAL_MOBILITYTABLE_H
#include "osl/container.h"
namespace osl
{
  namespace eval
  {
    class MobilityTable
    {
    public:
      static const CArray<int, 9> rookVertical;
      static const CArray<int, 9> rookHorizontal;
      static const CArray<int, 9> prookVertical;
      static const CArray<int, 9> prookHorizontal;
      static const CArray<int, 17> bishop;
      static const CArray<int, 17> pbishop;
      static const CArray<int, 9> lance;
    };
  }
}
#endif /* EVAL_MOBILITYTABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
