/* TwoDimensionalStatistics.h
 */
#ifndef _STAT_TWODIMENSIONALSTATISTICS_H
#define _STAT_TWODIMENSIONALSTATISTICS_H

#include <memory>
#include <cstddef>

namespace osl
{
  namespace stat
  {
    class TwoDimensionalStatistics
    {
      class Data;
      std::unique_ptr<Data> m_data;
    public:
      // CREATORS
      TwoDimensionalStatistics();
      ~TwoDimensionalStatistics();
      // MANIPULATORS
      void add(const double& x, const double& y);
      void merge(const TwoDimensionalStatistics&);
      void clear();
      // ACCESSORS
      double averageX() const;
      double averageY() const;
      double averageX2() const;
      double averageY2() const;
      double averageXY() const;
      double meanSquaredErrors() const;
      double meanSquaredErrorsAdjustConstant() const;
      size_t size() const;
      double correlation() const;
      /** ax + b = y */
      void fitting(double& a, double& b, double& residual) const;
    private:
      // NoCopy
      TwoDimensionalStatistics(const TwoDimensionalStatistics&);
      TwoDimensionalStatistics& operator=(const TwoDimensionalStatistics&);
    };

  } // namespace stat
} // namespace osl


#endif /* _STAT_TWODIMENSIONALSTATISTICS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
