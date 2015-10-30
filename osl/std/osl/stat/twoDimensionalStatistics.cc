/* TwoDimensionalStatistics.cc
 */
#include "osl/stat/twoDimensionalStatistics.h"
#include "osl/stat/average.h"
#include <cmath>

class osl::stat::TwoDimensionalStatistics::Data 
{
public:
  Average m_x, m_y, m_x2, m_y2, m_xy;

  // MANIPULATORS
  void add(const double& x, const double& y) 
  {
    m_x.add(x);
    m_y.add(y);
    m_x2.add(x*x);
    m_y2.add(y*y);
    m_xy.add(x*y);
  }
  void merge(const Data& r)
  {
    m_x.merge(r.m_x);
    m_y.merge(r.m_y);
    m_x2.merge(r.m_x2);
    m_y2.merge(r.m_y2);
    m_xy.merge(r.m_xy);
  }
  void clear()
  {
    m_x.clear();
    m_y.clear();
    m_x2.clear();
    m_y2.clear();
    m_xy.clear();
  }
};

// CREATORS
osl::stat::TwoDimensionalStatistics::
TwoDimensionalStatistics() : m_data(new Data())
{
}

osl::stat::TwoDimensionalStatistics::
~TwoDimensionalStatistics()
{
}

// MANIPULATORS
void osl::stat::TwoDimensionalStatistics::
add(const double& x, const double& y)
{
  m_data->add(x,y);
}

void osl::stat::TwoDimensionalStatistics::
merge(const TwoDimensionalStatistics& r)
{
  m_data->merge(*r.m_data);
}

void osl::stat::TwoDimensionalStatistics::
clear()
{
  m_data->clear();
}


// ACCESSORS
size_t osl::stat::TwoDimensionalStatistics::
size() const
{
  return m_data->m_x.numElements();
}

double osl::stat::TwoDimensionalStatistics::
averageX() const
{
  return m_data->m_x.average();
}

double osl::stat::TwoDimensionalStatistics::
averageY() const
{
  return m_data->m_y.average();
}

double osl::stat::TwoDimensionalStatistics::
averageX2() const
{
  return m_data->m_x2.average();
}

double osl::stat::TwoDimensionalStatistics::
averageY2() const
{
  return m_data->m_y2.average();
}

double osl::stat::TwoDimensionalStatistics::
averageXY() const
{
  return m_data->m_xy.average();
}

double osl::stat::TwoDimensionalStatistics::
meanSquaredErrors() const
{
  return averageX2() - 2*averageXY() + averageY2();
}

double osl::stat::TwoDimensionalStatistics::
meanSquaredErrorsAdjustConstant() const
{
  return averageX2() - averageX()*averageX()
    -2*(averageXY() - averageX()*averageY())
    + averageY2() - averageY()*averageY();
}

double osl::stat::TwoDimensionalStatistics::
correlation() const
{
  return (averageXY() - averageX()*averageY())
    / sqrt((averageX2() - averageX()*averageX())
	   * (averageY2() - averageY()*averageY()));
}

void osl::stat::TwoDimensionalStatistics::
fitting(double &a, double &b, double &residual) const
{
  double d = averageX2() - averageX()*averageX();
  double n = averageXY() - averageX()*averageY();
  if (std::abs(d) < 1e-8) {
    a = 0.0;
    b = averageY();
    residual = averageY2() - averageY()*averageY();
    return;
  }
  a = n / d;
  b = (averageX2()*averageY() - averageXY()*averageX()) / d;
  residual = averageY2()-averageY()*averageY()
    - n * n / d;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
