/* average.h
 */
#ifndef _AVERAGE_H
#define _AVERAGE_H

namespace osl
{
  namespace stat
  {
    /**
     * incrementaly maintain average of data sequence
     */
    class Average
    {
      double mean;
      int elements;
    public:
      // CREATORS
      Average() : mean(0), elements(0)
      {
      }
      // MANIPULATORS
      /**
       * Add an element x
       * @return difference between x and (old) mean
       */
      double add(const double& x)
      {
	++elements;
	const double diff = x - mean;
	mean += diff/elements;
	return diff;
      }
      void merge(const Average& r) 
      {
	if (r.elements == 0)
	  return;
	const double sum = mean*elements + r.mean*r.elements;
	elements += r.elements;
	mean = sum / elements;
      }
      void clear(double a=0.0, int e=0)
      {
	mean = a;
	elements = e;
      }
      // ACCESSORS
      double average() const	{ return mean; }
      int numElements() const	{ return elements; }
    };
  } // namespace stat
  using stat::Average;
} // namespace osl


#endif /* _AVERAGE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
