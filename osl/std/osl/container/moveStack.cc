#include "osl/container/moveStack.h"
#include "osl/csa.h"
#include <iostream>

osl::MoveStack::MoveStack()
{
  data.reserve(16);
  clear();
}

osl::MoveStack::~MoveStack()
{
}

void osl::MoveStack::reserve(size_t capacity)
{
  data.reserve(capacity);
}

void osl::MoveStack::clear()
{
  data.clear(); 
  data.push_back(Move::INVALID());
  data.push_back(Move::INVALID());
}

#ifndef MINIMAL
void osl::MoveStack::dump(std::ostream& os, size_t last_n) const
{
  const size_t start = (last_n == 0) ? 0 : size() - last_n;
  os << "move stack";
  vector_t::const_iterator p=data.begin();
  ++p;				// skip first element, pass
  for (size_t i=0; p!=data.end(); ++p, ++i)
  {
    if (i < start)
      continue;
    os << " " << csa::show(*p);
  }
  os << "\n";
}

void osl::MoveStack::dump(size_t last_n) const
{
  dump(std::cerr, last_n);
}
#endif
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
