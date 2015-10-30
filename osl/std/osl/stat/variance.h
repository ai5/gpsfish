/* variance.h
 */
#ifndef _VARIANCE_H
#define _VARIANCE_H

#include "osl/stat/average.h"
namespace osl
{
  namespace stat
  {
    /**
     * incrementaly maintain average and variance of data sequence
     */
    class Variance : private Average
    {
      double m_variance;
      typedef Average base_t;
    public:
      // CREATORS
      Variance() : m_variance(0)
      {
      }
      // MANIPULATORS
      void add(const double& x)
      {
	const double diff = base_t::add(x);
	const double adjuster 
	  = static_cast<double>(numElements()-1)/numElements();
	m_variance += diff*diff*adjuster;
      }

      // ACCESSORS
      double variance() const { return m_variance/numElements(); }
      using base_t::average;
      using base_t::numElements;
    };
  } // namespace stat
  using stat::Average;
} // namespace osl


#endif /* _VARIANCE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
