#ifndef OSL_MISC_MATH_H
#define OSL_MISC_MATH_H

#include <numeric>
#include <cmath>
#include <algorithm>
#include <functional>

namespace osl
{
  namespace misc
  {
    /**
     * Reference: C++ Cookbook, Stephens, Diggins, Turkanis and Cogswell, O'Reilly, 
     */
    template<unsigned int N, class T>
    T nthPower(T x)
    {
      if (N==0)
        return 1;

      const T temp = nthPower<N/2>(x);
      if (N%2 == 0)
        return temp * temp;
      else
        return  temp * temp * x;
    }

    template<class T, int N>
    struct SumDiffNthPower
    {
      T mean;
      SumDiffNthPower(T mean) : mean(mean) {}
      T operator()(T sum, T current)
      {
        return sum + nthPower<N>(current - mean);
      }
    };

    template<class T, int N, class Iter_T>
    T nthMoment(Iter_T first, Iter_T last, T mean)
    {
      const size_t cnt = distance(first, last);
      return accumulate(first, last, T(), SumDiffNthPower<T, N>(mean))/cnt;
    }

    template<class T, class Iter_T>
    T computeVariance(Iter_T first, Iter_T last, T mean)
    {
      return nthMoment<T, 2>(first, last, mean);
    }

    template<class T, class Iter_T>
    T computeStdDev(Iter_T first, Iter_T last, T mean)
    {
      return sqrt(computeVariance(first, last, mean));
    }

    template<class T, class Iter_T>
    T computeSkew(Iter_T first, Iter_T last, T mean)
    {
      const T m3 = nthMoment<T, 3>(first, last, mean);
      const T m2 = nthMoment<T, 2>(first, last, mean);
      return m3 / (m2 * sqrt(m2));
    }

    template<class T, class Iter_T>
    T computeKurtosisExcess(Iter_T first, Iter_T last, T mean)
    {
      const T m4 = nthMoment<T, 4>(first, last, mean);
      const T m2 = nthMoment<T, 2>(first, last, mean);
      return m4 / (m2 * m2) - 3;
    }

    template<class T, class Iter_T>
    void computeStats(Iter_T first, Iter_T last, 
                   T& sum, T& mean, T&var, T&std_dev, T& skew, T& kurt)
    {
      const size_t cnt = distance(first, last);
      sum     = accumulate(first, last, T());
      mean    = sum / cnt;
      var     = computeVariance(first, last, mean);
      std_dev = sqrt(var);
      skew    = computeSkew(first, last, mean);
      kurt    = computeKurtosisExcess(first, last, mean);
    }
  }
}
#endif /* _MISC_MATH_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
