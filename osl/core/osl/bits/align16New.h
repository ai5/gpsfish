/* align16new.h
 */
#ifndef OSL_ALIGN16NEW_H
#define OSL_ALIGN16NEW_H
#include <memory>
#include <cstddef>
namespace osl
{
  namespace misc
  {
    struct Align16New 
    {
      static const int Alignment = 16;
      static void *operator new(size_t size);
      static void *operator new[](size_t size);
      static void operator delete(void *ptr, size_t size);
      static void operator delete[](void *ptr, size_t size);
    protected:
      ~Align16New() {}		// for safety in public inheritance
    };
  }
}

#endif /* OSL_ALIGN16NEW_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
