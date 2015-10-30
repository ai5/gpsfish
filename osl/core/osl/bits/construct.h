/* construct.h
 */
#ifndef OSL_CONSTRUCT_H
#define OSL_CONSTRUCT_H

#include <boost/type_traits/has_trivial_destructor.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/utility/enable_if.hpp>
#include <iterator>
#include <memory>
#include <cassert>
namespace osl
{
  class Piece;
  class Move;
  class Square;
  namespace rating
  {
    class RatedMove;
  }
  namespace misc
  {
    namespace detail
    {
      /** use raw memory copy instead of placement new not to test a given pointer is null */
      template <typename T>
      struct BitCopyTraits 
      {
	static const bool value=boost::is_pod<T>::value; 
      };

      template <> struct BitCopyTraits<Move> { static const bool value=true; };
      template <> struct BitCopyTraits<Piece> { static const bool value=true; };
      template <> struct BitCopyTraits<Square> { static const bool value=true; };
      template <> struct BitCopyTraits<rating::RatedMove> { static const bool value=true; };
    }

    template <typename T1, typename T2>
    inline
    void construct(T1* ptr, const T2& value, 
		   typename boost::enable_if<detail::BitCopyTraits<T1> >::type * =0)
    {
      assert(ptr);
      *ptr = T1(value);
    }

    template <typename T1, typename T2>
    inline
    void construct(T1* ptr, const T2& value, 
		   typename boost::disable_if<detail::BitCopyTraits<T1> >::type * =0)
    {
      assert(ptr);
      ::new(ptr) T1(value);
    }

    template <typename T>
    inline void destroy(T *ptr) 
    {
      ptr->~T(); 
    }

    template <typename ForwardIterator>
    inline void destroy(ForwardIterator first, ForwardIterator last)
    {
      typedef typename std::iterator_traits<ForwardIterator>::value_type
	value_type;
      if (boost::has_trivial_destructor<value_type>::value)
	return;
      for (; first != last; ++first)
	destroy(&*first);
    }
  }
}


#endif /* OSL_CONSTRUCT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
