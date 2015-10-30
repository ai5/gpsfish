/* hashRandom.h
 */
#ifndef OSL_HASHRANDOM_H
#define OSL_HASHRANDOM_H

#include "osl/hashKey.h"
#include "osl/container.h"

namespace osl
{
  namespace hash
  {
    class HashRandom
    {
    public:
      static const size_t Length = 0x1000;
    private:
      static CArray<int,Length> table;
    public:
      static void setUp(double sigma);
      static int value(size_t key) 
      {
	return table[key % Length];
      }
      static int value(const HashKey& key) 
      {
	return value(key.signature());
      }
    };
  }
  using hash::HashRandom;
}

#endif /* OSL_HASHRANDOM_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
