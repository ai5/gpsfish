#include "osl/random.h"
#include <random>
#include <time.h>
unsigned int osl::misc::random()
{
  static std::mt19937 mt_random;
  return mt_random();
}

unsigned int osl::misc::time_seeded_random()
{
  static std::mt19937 mt_random(time(0));
  return mt_random();
}
