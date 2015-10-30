/* align16New.cc
 */
#include "osl/bits/align16New.h"
#include <cassert>
#include <cstdlib>

void * osl::misc::Align16New::operator new(size_t size)
{
  char *ptr = ::new char[size+Alignment];
  for (int i=0; i<Alignment; ++i) {
    if (reinterpret_cast<unsigned long>(ptr + i + 1) % Alignment == 0) {
      *(ptr + i) = i + 1;
      // std::cerr << ">> " << (long)ptr << " => " << (long)(ptr + i + 1) << "\n";
      return ptr + i + 1;
    }
  }
  assert(0);
  abort();
}

void * osl::misc::Align16New::operator new[](size_t size)
{
  return operator new(size);
}

void osl::misc::Align16New::operator delete(void *ptr, size_t /*size*/)
{
  char *p = static_cast<char*>(ptr);
  int offset = *(p-1);
  ::delete(p - offset);
  // std::cerr << "<< " << (long)p << " => " << (long)(p - offset) << "\n";
}

void osl::misc::Align16New::operator delete[](void *ptr, size_t size)
{
  return operator delete(ptr, size);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
