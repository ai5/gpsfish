/* checkMoveListProvider.h
 */
#ifndef _CHECKMOVELISTPROVIDER_H
#define _CHECKMOVELISTPROVIDER_H

#include "checkMove.h"
#include <ext/slist>

namespace osl
{
  namespace checkmate
  {
    class CheckMoveListProvider
    {
      typedef __gnu_cxx::slist<CheckMove*> list_t;
      list_t data;
      list_t::iterator cur;
      size_t index;
#ifdef OSL_SMP
      enum { BucketSize = 128 };
#else
      enum { BucketSize = 2048*16 };
#endif
    public:
      CheckMoveListProvider();
      ~CheckMoveListProvider();
    private:
      void newBucket(size_t length);
    public:
      CheckMove* alloc(size_t length) 
      {
	if (length == 0)
	  return 0;
	if (index + length > BucketSize)
	  newBucket(std::max(length, (size_t)BucketSize));
	assert(cur != data.end());
	assert(index + length <= BucketSize);
	CheckMove *result = &(*cur)[index];
	index += length;
	return result;
      }
      void clear();
    };
  }
}


#endif /* _CHECKMOVELISTPROVIDER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
