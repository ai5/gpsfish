/* histogram.cc
 */
#include "osl/stat/histogram.h"
#include <iostream>
#include <iomanip>

osl::stat::
Histogram::Histogram(size_t w, size_t len, int s, bool sd)
  : data(new double[len]), length_(len), width_(w), start_(s),
    show_on_destruct(sd)
{
  std::fill(&data[0], &data[0]+length_, 0);
}
osl::stat::
Histogram::~Histogram()
{
  if (show_on_destruct)
    show(std::cerr);
}
void osl::stat::
Histogram::merge(const Histogram& o)
{
  if ((width_ == o.width_)
      && (length_ == o.length_)
      && (start_ == o.start_))
  {
    for (size_t i=0; i<length_; ++i)
    {
      data[i] += o.data[i];
    }
  }
}

void osl::stat::
Histogram::show(std::ostream& os) const
{
  int value=start_;
  for (size_t i=0; i<length_; ++i, value+=width_)
  {
    os << std::setw(5) << value << " - " << std::setw(5);
    os << value+(int)width_;
    os << " " << std::setw(8) << (size_t)data[i] << "\n";
  }
}

void osl::stat::
Histogram::showRatio(std::ostream& os, const Histogram& o) const
{
  if ((width_ == o.width_)
      && (length_ == o.length_)
      && (start_ == o.start_))
  {
    int value=start_;
    for (size_t i=0; i<length_; ++i, value+=width_)
    {
      os << std::setw(5) << value << " - " << std::setw(5);
      if (i+1 < length_)
	os << value+(int)width_;
      else
	os << " ";
      os << std::setw(8) << o.data[i]
	 << std::setw(8) << data[i]
	 << std::setw(8) << std::setprecision(2)
	 << static_cast<double>(o.data[i])/data[i] << "\n";
    }
  }
}
 
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
