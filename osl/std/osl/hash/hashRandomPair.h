/* hashRandom.h
 */
#ifndef OSL_HASHRANDOMPAIR_H
#define OSL_HASHRANDOMPAIR_H

#include "osl/hashKey.h"

namespace osl
{
  namespace hash
  {
    class HashRandomPair
    {
    public:
      static const size_t Length = 0x100000;
    private:
      static std::pair<char,char> table[Length];
      static bool is_initialized;
    public:
      static void setUp(unsigned int seed, unsigned int prob100);
      static std::pair<char,char> value(size_t key) 
      {
	return table[key % Length];
      }
      static std::pair<char,char> value(const HashKey& key) 
      {
	return value(key.signature());
      }
      static bool initialized() { return is_initialized; }
    };
  }
  using hash::HashRandomPair;
}

#endif /* OSL_HASHRANDOMPAIR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
