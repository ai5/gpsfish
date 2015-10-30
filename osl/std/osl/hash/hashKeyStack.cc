/* hashKeyStack.cc
 */
#include "osl/hash/hashKeyStack.h"
#include <iostream>

osl::hash::
HashKeyStack::HashKeyStack(size_t capacity)
{
  data.reserve(capacity);
}

osl::hash::
HashKeyStack::~HashKeyStack()
{
}

void osl::hash::
HashKeyStack::push(const HashKey& key)
{
  data.push_back(key);
}

void osl::hash::
HashKeyStack::dump() const
{
#ifndef MINIMAL
  for (auto& key: data) {
    std::cerr << key << std::endl;
  }
#endif
}

bool osl::hash::operator==(const HashKeyStack& l, const HashKeyStack& r)
{
  return l.data == r.data;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
