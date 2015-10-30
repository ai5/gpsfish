/* twinList.h
 */
#ifndef CHECKMATE_TWINLIST_H
#define CHECKMATE_TWINLIST_H

#include "twinEntry.h"
#include "checkAssert.h"
#include "osl/stl/slist.h"
#include <iosfwd>

namespace osl
{
  namespace checkmate
  {
    class TwinList : private slist<TwinEntry>
    {
      unsigned int size_cache;
    public:
      TwinList() : size_cache(0)
      {
      }
      using slist<TwinEntry>::const_iterator;
      using slist<TwinEntry>::begin;
      using slist<TwinEntry>::end;
      using slist<TwinEntry>::empty;
      unsigned int size() const { return size_cache; }
      void clear();
      const_iterator find(const PathEncoding& path) const
      {
	for (const_iterator p=begin(); p!=end(); ++p)
	{
	  if (p->path == path)
	    return p;
	}
	return end();
      }
      const TwinEntry* findLoopTo(const CheckHashRecord *record) const
      {
	for (const_iterator p=begin(); p!=end(); ++p)
	{
	  if (p->loopTo == record)
	    return &*p;
	}
	return 0;
      }
      /** @param move 受方の場合のloopに導く手 */
      void addLoopDetection(const PathEncoding& path, const CheckMove& move,
			    const CheckHashRecord *loopTo)
      {
	check_assert(find(path) == end());
	++size_cache;
	push_front(TwinEntry(path, move, loopTo));
      }

      void dump(std::ostream&) const;
    };
  }
}


#endif /* CHECKMATE_TWINLIST_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
