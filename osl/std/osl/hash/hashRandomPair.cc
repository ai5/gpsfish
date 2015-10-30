/* hashRandom.cc
 */
#include "osl/hash/hashRandomPair.h"
#include <random>

std::pair<char,char>
osl::hash::HashRandomPair::table[osl::hash::HashRandomPair::Length];
bool osl::hash::HashRandomPair::is_initialized = 0;

void osl::hash::HashRandomPair::setUp(unsigned int seed, unsigned int prob100)
{
  std::mt19937 mt19937(seed);
  std::uniform_int_distribution<int> uniform100(0, 99);

  for (size_t i=0; i<Length; ++i) {
    const unsigned int u = uniform100(mt19937);
    if (u < prob100)
      table[i] = std::make_pair(1,0);
    else if (u < prob100*2)
      table[i] = std::make_pair(0,1);
    else 
      table[i] = std::make_pair(0,0);
  }
  is_initialized = 1;
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
