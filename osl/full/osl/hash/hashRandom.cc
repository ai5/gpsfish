/* hashRandom.cc
 */
#include "osl/hash/hashRandom.h"
#include "osl/misc/milliSeconds.h"
#include <random>

osl::CArray<int,osl::hash::HashRandom::Length> osl::hash::HashRandom::table;

void osl::hash::HashRandom::setUp(double sigma)
{
  static std::mt19937 mt_random(clock::now().time_since_epoch().count());
  std::normal_distribution<double> n(0, sigma);
  for (size_t i=0; i<Length; ++i)
      table[i] = static_cast<int>(n(mt_random))/2*2;
}

