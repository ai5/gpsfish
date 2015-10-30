/* hashKeyStack.h
 */
#ifndef HASH_HASHKEYSTACK_H
#define HASH_HASHKEYSTACK_H
#include "osl/hashKey.h"
#include <vector>
namespace osl
{
  namespace hash
  {
    class HashKeyStack 
    {
      typedef std::vector<HashKey> vector_t;
      vector_t data;
    public:
      explicit HashKeyStack(size_t capacity=0);
      ~HashKeyStack();

      void push(const HashKey&);
      void pop() { assert(! data.empty()); data.pop_back(); }
      void clear() { data.clear(); }

      const HashKey& top(size_t n=0) const
      {
	assert(n < size());
	vector_t::const_reverse_iterator p=data.rbegin()+n;
	return *p;
      }
      bool empty() const { return data.empty(); }
      size_t size() const { return data.size(); }

      void dump() const;

      friend bool operator==(const HashKeyStack&, const HashKeyStack&);
    };
  } // namespace hash
  using hash::HashKeyStack;
} // namespace osl

#endif /* HASH_HASHKEYSTACK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
