/* probability.h
 */
#ifndef _STAT_PROBABILITY_H
#define _STAT_PROBABILITY_H

#include <algorithm>
#include <cmath>
namespace osl
{
  namespace stat
  {
    struct Probability
    {
      unsigned long numerator, denominator;
      explicit Probability(unsigned long n=0, unsigned long d=0)
	: numerator(n), denominator(d)
      {
      }
      double probability(double stabilizer=1.0) const
      {
	return std::max(stabilizer,numerator-stabilizer)
	  / std::max(stabilizer,static_cast<double>(denominator));
      }
      double logProb(unsigned int stabilizer=1u) const
      {
	const double prob = probability(stabilizer);
	return log(prob)/log(0.5)*100;
      }

      void merge(const Probability& other)
      {
	numerator += other.numerator;
	denominator += other.denominator;
      }
    };
  } // namespace stat
} // namespace osl

#endif /* _STAT_PROBABILITY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:


