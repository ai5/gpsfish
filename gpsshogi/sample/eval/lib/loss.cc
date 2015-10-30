/* loss.cc
 */
#include "loss.h"
#include "instanceData.h"
#include <cassert>
double gpsshogi::ValarrayUtil::dot(const valarray_t& l, const valarray_t& r) 
{
  double sum = 0.0;
  assert(l.size() == r.size());
  for (size_t i=0; i<l.size(); ++i)
    sum += l[i]*r[i];
  return sum;
}

double gpsshogi::ValarrayUtil::dot(const InstanceData& x, const valarray_t& w) 
{
  double sum = 0.0;
  for (size_t i=0; i<x.size(); ++i) {
    size_t j = x.index[i];
    double v = x.value[i];
    sum += w[j]*v;
  }
  return sum;
}

double gpsshogi::ValarrayUtil::
dot(const sparse_vector_t& l, const valarray_t& r, const std::vector<size_t>& frequency, int min_frequency)
{
  double sum = 0.0;
  for (size_t i=0; i<l.size(); ++i) {
    size_t j = l[i].first;
    double v = l[i].second;
    if (min_frequency && frequency[j] < min_frequency)
      continue;
    sum += r[j]*v;
  }
  return sum;
}

/* ------------------------------------------------------------------------- */

double gpsshogi::LogLoss::
addGradient(const valarray_t& w, const valarray_t& x, double y, valarray_t& gradient)
{
  assert(gradient.size() == x.size());
  double d = dot(w, x), B = -y*d, A = exp(B), err = log(1.0+A);
  for (size_t j=0; j<w.size(); ++j)
    gradient[j] += -1.0/(1.0+A)*A*y*x[j];
  return err;
}

double gpsshogi::LogLoss::addGradient(const valarray_t& w, const InstanceData& x, valarray_t& gradient, double instance_weight, double margin)
{
  assert(gradient.size() == w.size());
  assert(margin >= 0);
  double d = dot(x, w);
  double y = x.y;
  if (x.y == 0) 
    y = -1;		// [0,1] => [-1,1]
  assert(y == 1 || y == -1);
  double B = -y*d+margin, A = exp(B), err = log(1.0+A);
  for (size_t i=0; i<x.size(); ++i) {
    size_t j = x.index[i];
    double v = x.value[i];
    gradient[j] += -1.0/(1.0+A)*A*y*v*instance_weight;
  }
  return err;
}

double gpsshogi::LogLoss::addGradientSep(const valarray_t& w, double adot, const InstanceData& a,
					 const InstanceData& b, valarray_t& gradient, double& agsum)
{
  // x = b-a
  assert(dot(a, w) == adot);
  assert(gradient.size() == w.size());
  double d = dot(b, w) - adot;
  double y = b.y;
  if (y == 0) 
    y = -1;		// [0,1] => [-1,1]
  assert(y == 1 || y == -1);
  const double B = -y*d, A = exp(B);
  double err = log(1.0+A), g = 1.0/(1.0+A)*A;
  if (B>30) {
    err = B;
    g = 1.0;
  }
  for (size_t i=0; i<b.size(); ++i) {
    size_t j = b.index[i];
    double v = b.value[i];
    gradient[j] += -g*y*v;
  }
  agsum += g*y;
  return err;
}
void gpsshogi::LogLoss::addGradientSep(const InstanceData& a, double agsum, valarray_t& gradient)
{
  // x = b-a
  for (size_t i=0; i<a.size(); ++i) {
    size_t j = a.index[i];
    double v = a.value[i];
    gradient[j] += agsum*v;
  }
}

double gpsshogi::LogLoss::addGradientSep(const valarray_t& w, double adot, const sparse_vector_t& a,
					 const sparse_vector_t& b, double turn_coef,
					 const std::vector<size_t>& frequency, int min_frequency,
					 valarray_t& gradient, double& agsum, 
					 double instance_weight, double margin)
{
  const double y = -turn_coef;
  assert(y == 1 || y == -1);
  assert(margin >= 0.0);
  // x = b-a
  assert(a.empty()||abs(dot(a, w, frequency, min_frequency) - adot) < 0.01);
  assert(gradient.size() == w.size());
  const double d = dot(b, w, frequency, min_frequency) - adot;
  const double B = -y*d + margin, A = exp(B);
  double err = log(1.0+A), g = 1.0/(1.0+A)*A;
  if (B>30) {
    err = B;
    g = 1.0;
  }
  for (size_t i=0; i<b.size(); ++i) {
    const size_t j = b[i].first;
    const double v = b[i].second;
    if (min_frequency && frequency[j] < min_frequency)
      continue;
    gradient[j] += -g*y*v*instance_weight;
  }
  agsum += g*y*instance_weight;
  return err;
}
void gpsshogi::LogLoss::addGradientSep(const sparse_vector_t& a, 
				       const std::vector<size_t>& frequency, int min_frequency,
				       double agsum, valarray_t& gradient)
{
  // x = b-a
  for (size_t i=0; i<a.size(); ++i) {
    size_t j = a[i].first;
    double v = a[i].second;
    if (min_frequency && frequency[j] < min_frequency)
      continue;
    gradient[j] += agsum*v;
  }
}

double gpsshogi::LogLoss::
addGradient(const valarray_t& w, const valarray_t& x, double y, valarray_t& gradient, 
	    const valarray_t& v, valarray_t& Hv)
{
  assert(gradient.size() == x.size());
  double d = dot(w, x), B = -y*d, A = exp(B), err = log(1.0+A);
  double C = 1.0/(1+A)*(-y)*dot(x, v);
  for (size_t j=0; j<w.size(); ++j) {
    double g = -1.0/(1.0+A)*A*y*x[j];
    gradient[j] += g;
    Hv[j] += g*C;
  }
  return err;
}

double gpsshogi::LogLoss::addGradient(const valarray_t& w, const InstanceData& x, valarray_t& gradient,
				      const valarray_t& v, valarray_t& Hv)
{
  assert(gradient.size() == w.size());
  double d = dot(x, w);
  double y = x.y;
  if (x.y == 0) 
    y = -1;		// [0,1] => [-1,1]
  assert(y == 1 || y == -1);
  double B = -y*d, A = exp(B), err = log(1.0+A);
  double C = 1.0/(1+A)*(-y)*dot(x, v);
  for (size_t i=0; i<x.size(); ++i) {
    size_t j = x.index[i];
    double v = x.value[i];
    double g = -1.0/(1.0+A)*A*y*v;
    gradient[j] += g;
    Hv[j] += g*C;
  }
  return err;
}

/* ------------------------------------------------------------------------- */
double gpsshogi::ExpLoss::addGradient(const valarray_t& w, const InstanceData& x, valarray_t& gradient)
{
  assert(gradient.size() == w.size());
  double y = x.y;
  if (x.y == 0) 
    y = -1;		// [0,1] => [-1,1]
  assert(y == 1 || y == -1);
  double d = dot(x, w), B = -y*d, err = exp(B);
  for (size_t i=0; i<x.size(); ++i) {
    size_t j = x.index[i];
    double v = x.value[i];
    gradient[j] += err * (-y)*v;
  }
  return err;
}

double gpsshogi::ExpLoss::addGradient(const valarray_t& w, const InstanceData& x, valarray_t& gradient,
				      const valarray_t& v, valarray_t& Hv)
{
  assert(gradient.size() == w.size());
  double y = x.y;
  if (x.y == 0) 
    y = -1;		// [0,1] => [-1,1]
  assert(y == 1 || y == -1);
  double d = dot(x, w), B = -y*d, err = exp(B);
  double C = (-y)*dot(x, v);
  for (size_t i=0; i<x.size(); ++i) {
    size_t j = x.index[i];
    double v = x.value[i];
    double g = err * (-y)*v;
    gradient[j] += g;
    Hv[j] += g*C;
  }
  return err;
}

/* ------------------------------------------------------------------------- */
double gpsshogi::SigmoidLoss::addGradient(const valarray_t& w, const InstanceData& x, valarray_t& gradient, double instance_weight, double margin)
{
  assert(gradient.size() == w.size());
  double y = x.y;
  if (x.y == 0) 
    y = -1;		// [0,1] => [-1,1]
  assert(y == 1 || y == -1);
  double d = dot(x, w), B = y*d - margin;
  double err = 1.0/(1.0+exp(B)), A = err*(1.0-err);
  for (size_t i=0; i<x.size(); ++i) {
    size_t j = x.index[i];
    double v = x.value[i];
    gradient[j] += A * (-y)*v * instance_weight;
  }
  return err;
}

double gpsshogi::SigmoidLoss::
addGradientSep(const valarray_t& w, double adot, const sparse_vector_t& a,
	       const sparse_vector_t& b, double turn_coef,
	       const std::vector<size_t>& frequency, int min_frequency,
	       valarray_t& gradient, double& agsum,
	       double instance_weight, double margin)
{
  const double y = -turn_coef;
  assert(y == 1 || y == -1);
  // x = b-a
  assert(a.empty()||abs(dot(a, w, frequency, min_frequency) - adot) < 0.01);
  assert(gradient.size() == w.size());
  const double d = dot(b, w, frequency, min_frequency) - adot;
  const double B = y*d - margin, err = 1.0/(1.0+exp(B)), A = err*(1.0-err);

  for (size_t i=0; i<b.size(); ++i) {
    const size_t j = b[i].first;
    const double v = b[i].second;
    if (min_frequency && frequency[j] < min_frequency)
      continue;
    gradient[j] += A * (-y)*v*instance_weight;
  }
  agsum += A*y*instance_weight;
  return err;
}

void gpsshogi::SigmoidLoss::
addGradientSep(const sparse_vector_t& a, 
		    const std::vector<size_t>& frequency, int min_frequency,
		    double agsum, valarray_t& gradient)
{
  LogLoss::addGradientSep(a, frequency, min_frequency, agsum, gradient);
}

double gpsshogi::SigmoidLoss::addGradient(const valarray_t& w, const InstanceData& x, valarray_t& gradient,
					  const valarray_t& v, valarray_t& Hv)
{
  assert(gradient.size() == w.size());
  double y = x.y;
  if (x.y == 0) 
    y = -1;		// [0,1] => [-1,1]
  assert(y == 1 || y == -1);
  double d = dot(x, w), B = y*d;
  double err = 1.0/(1.0+exp(B)), A = err*(1.0-err);
  double C = y*(2.0*err-1.0)*dot(x, v);
  for (size_t i=0; i<x.size(); ++i) {
    size_t j = x.index[i];
    double v = x.value[i];
    double g = A * (-y)*v;
    gradient[j] += g;
    Hv[j] += g*C;
  }
  return err;
}

/* ------------------------------------------------------------------------- */
double gpsshogi::HingeLoss::addGradient(const valarray_t& w, const InstanceData& x, valarray_t& gradient)
{
  assert(gradient.size() == w.size());
  double y = x.y;
  if (x.y == 0) 
    y = -1;		// [0,1] => [-1,1]
  assert(y == 1 || y == -1);
  double d = dot(x, w), err = 1-y*d;
  if (err <= 0)
    return 0;
  for (size_t i=0; i<x.size(); ++i) {
    size_t j = x.index[i];
    double v = x.value[i];
    gradient[j] += -y*v;
  }
  return err;
}

void gpsshogi::HingeLoss::
addGradientSep(const sparse_vector_t& a, 
	       const std::vector<size_t>& frequency, int min_frequency,
	       double agsum, valarray_t& gradient)
{
  LogLoss::addGradientSep(a, frequency, min_frequency, agsum, gradient);
}

double gpsshogi::HingeLoss::
addGradientSep(const valarray_t& w, double adot, const sparse_vector_t& a,
	       const sparse_vector_t& b, double turn_coef,
	       const std::vector<size_t>& frequency, int min_frequency,
	       valarray_t& gradient, double& agsum,
	       double instance_weight, double margin)
{
  const double y = -turn_coef;
  assert(y == 1 || y == -1);
  // x = b-a
  assert(a.empty()||abs(dot(a, w, frequency, min_frequency) - adot) < 0.01);
  assert(gradient.size() == w.size());
  const double d = dot(b, w, frequency, min_frequency) - adot;
  const double err = margin-y*d;
  if (err <= 0)
    return 0;

  for (size_t i=0; i<b.size(); ++i) {
    const size_t j = b[i].first;
    const double v = b[i].second;
    if (min_frequency && frequency[j] < min_frequency)
      continue;
    gradient[j] += (-y)*v*instance_weight;
  }
  agsum += y*instance_weight;
  return err;
}

double gpsshogi::HingeLoss::addGradient(const valarray_t& w, const InstanceData& x, valarray_t& gradient,
					const valarray_t& /*v*/, valarray_t& /*Hv*/)
{
  // H = [0]
  return addGradient(w, x, gradient);
}

/* ------------------------------------------------------------------------- */
double gpsshogi::HalfSigmoidLoss::
addGradientSep(const valarray_t& w, double adot, const sparse_vector_t& a,
	       const sparse_vector_t& b, double turn_coef,
	       const std::vector<size_t>& frequency, int min_frequency,
	       valarray_t& gradient, double& agsum,
	       double instance_weight, double margin)
{
  const double y = -turn_coef;
  assert(y == 1 || y == -1);
  // x = b-a
  assert(a.empty()||abs(dot(a, w, frequency, min_frequency) - adot) < 0.01);
  assert(gradient.size() == w.size());
  const double d = dot(b, w, frequency, min_frequency) - adot;
  const double B = y*d - margin;
  if (B > 0)
    return 0.0;
  
  const double err = 1.0/(1.0+exp(B)), A = err*(1.0-err);

  for (size_t i=0; i<b.size(); ++i) {
    const size_t j = b[i].first;
    const double v = b[i].second;
    if (min_frequency && frequency[j] < min_frequency)
      continue;
    gradient[j] += A * (-y)*v*instance_weight;
  }
  agsum += A*y*instance_weight;
  return err;
}

void gpsshogi::HalfSigmoidLoss::
addGradientSep(const sparse_vector_t& a, 
		    const std::vector<size_t>& frequency, int min_frequency,
		    double agsum, valarray_t& gradient)
{
  LogLoss::addGradientSep(a, frequency, min_frequency, agsum, gradient);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
