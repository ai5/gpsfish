/* histogram.h
 */
#ifndef _HISTOGRAM_H
#define _HISTOGRAM_H

#include <boost/scoped_array.hpp>
#include <iosfwd>
namespace osl
{
  namespace stat
  {
    /**
     * ヒストグラム
     */
    class Histogram
    {
      boost::scoped_array<double> data;
      size_t length_, width_;
      int start_;
      bool show_on_destruct;
    public:
      Histogram(size_t w, size_t len, int start=0, bool show_on_destruct=false);
      ~Histogram();
      size_t safeIndex(size_t i) const
      {
	return (i >= length_) ? length_-1 : i;
      }
      double& frequency(size_t i) { return data[safeIndex(i)]; }
      void add(int value, double weight=1.0) 
      {
	if (value < start_)
	  value = 0;
	else
	  value -= start_;
	frequency(value/width_) += weight; 
      }
      double frequency(size_t i) const { return data[safeIndex(i)]; }
      void show(std::ostream& os) const;

      size_t length() const { return length_; }
      size_t width() const { return width_; }
      int start() const { return start_; }

      /** 結果を合算する length や width が異なっていたら何もしない*/
      void merge(const Histogram&);
      /** \frac{*this}{numerator} を表示 length や width が異なっていたら何もしない*/
      void showRatio(std::ostream& os, const Histogram& numerator) const;
    };
  } // namespace stat
  using stat::Histogram;
} // namespace osl

#endif /* _HISTOGRAM_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
