/* checkMoveList.h
 */
#ifndef _CHECK_MOVELIST_H
#define _CHECK_MOVELIST_H

#include "checkMove.h"
#include "osl/stl/vector.h"
#include <cassert>
#include <iosfwd>

namespace osl
{
  namespace checkmate
  {
    class CheckMoveListProvider;
    class CheckMoveListBase
    {
      CheckMove *first, *last;
    public:
      CheckMoveListBase() : first(0), last(0)
      {
      }

      void setSize(size_t length, CheckMoveListProvider&);
      void shrinkSize(size_t length)
      {
	assert(length <= static_cast<size_t>(last - first));
	last = first + length;
      }
      void setOne(const CheckMove& data, CheckMoveListProvider&);

      typedef const CheckMove* const_iterator;
      typedef CheckMove* iterator;
      
      CheckMove *begin() { return first; }
      CheckMove *end() { return last; }

      const CheckMove *begin() const { return first; }
      const CheckMove *end() const { return last; }

      CheckMove& operator[](size_t i) { 
	assert(first+i<last); return first[i]; 
      }
      const CheckMove& operator[](size_t i) const { 
	assert(first+i<last); return first[i]; 
      }

      void clear() { first = last = 0; }

      size_t size() const { return last - first; }
      bool empty() const { return last == first; }
    };
    /** 
     * 詰将棋で使う指手
     */
    class CheckMoveList : public CheckMoveListBase
    {
    public:
      const CheckMove* find(Move move) const;
      CheckMove* find(Move move);
      void dump(std::ostream&) const;
    };

    std::ostream& operator<<(std::ostream&, const CheckMoveList&);
  } // namespace checkmate
} // namespace osl

#endif /* _CHECK_MOVELIST_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
