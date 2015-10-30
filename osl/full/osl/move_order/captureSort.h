/* captureSort.h
 */
#ifndef MOVE_ORDER_CAPTURESORT_H
#define MOVE_ORDER_CAPTURESORT_H

#include "osl/basic_type.h"
#include <algorithm>
namespace osl
{
namespace move_order
{
  /**
   * 取る手を前にもってくる
   */
  struct CaptureSort
  {
    template <class Iterator>
    static void sort(Iterator first, Iterator last)
    {
      while (first < --last)
      {
	while (true)
	{
	  if (first->capturePtype() == PTYPE_EMPTY)
	    break;
	  ++first;
	  if (! (first < last))
	    return;
	}
	assert(first->capturePtype() == PTYPE_EMPTY);
	while (true)
	{
	  if (! (first < last))
	    return;
	  if (last->capturePtype() != PTYPE_EMPTY)
	    break;
	  --last;
	}
	assert(last->capturePtype() != PTYPE_EMPTY);
	assert(first < last);
	std::swap(*first, *last);
	++first;
      }
    }
  };
} // namespace move_order
  using move_order::CaptureSort;
} // namespace osl

#endif /* MOVE_ORDER_CAPTURESORT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
