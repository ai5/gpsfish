/* loss.t.cc
 */
#include "loss.h"
#include "analyzer.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <cmath>

using namespace gpsshogi;
using namespace osl;
typedef ValarrayUtil::valarray_t valarray_t;
static const size_t dim = 1024;

static const InstanceData make(const sparse_vector_t& selected, const sparse_vector_t& sibling,
			       const std::vector<size_t>& frequency, int min_frequency)
{
  sparse_vector_t  s0 = selected, s1 = sibling;
  std::sort(s0.begin(), s0.end());
  std::sort(s1.begin(), s1.end());
  InstanceData data;
  Analyzer::makeInstanceSorted(1, s0, s1, frequency, min_frequency, data);
  return data;
}

static void separation_test(double weight, double margin)
{
  const std::vector<size_t> count(dim, (size_t)1);
  const int min_count = 0;
  valarray_t w(dim);
  sparse_vector_t selected, sibling;
  for (size_t i=0; i<dim; ++i)
    w[i] = (random()%1024)/1024.0;
  for (size_t i=0; i<dim/2; ++i)
  {
    selected.push_back(std::make_pair(random()%dim, (random()%1024)/1024.0));
    sibling.push_back(std::make_pair(random()%dim, (random()%1024)/1024.0));
  }
  InstanceData combined = make(selected, sibling, count, min_count);
  const CArray<int,2> turns = {{ 1,-1 }};
  // sigmoid
  for (int z=0; z<2; ++z)
  {
    valarray_t gradient_sep(dim), gradient_naive(dim);
    gradient_sep = gradient_naive = 0.0;
    double agsum = 0.0;
    const double adot = ValarrayUtil::dot(selected, w, count, min_count);
    const double loss = SigmoidLoss::addGradientSep
      (w, adot, selected, sibling, turns[z], count, min_count,
       gradient_sep, agsum, weight, margin);
    SigmoidLoss::addGradientSep(selected, count, min_count, agsum, 
				gradient_sep);
    combined.y = 1-std::max(0, turns[z]);
    const double loss_naive
      = SigmoidLoss::addGradient(w, combined, gradient_naive, weight, margin);
    
    const double eps = 1e-12;
    BOOST_CHECK_SMALL(loss - loss_naive, eps);
    for (size_t i=0; i<dim; ++i)
      BOOST_CHECK_SMALL(gradient_sep[i] - gradient_naive[i], eps);
  }
  // logloss
  for (int z=0; z<2; ++z)
  {
    valarray_t gradient_sep(dim), gradient_naive(dim);
    gradient_sep = gradient_naive = 0.0;
    double agsum = 0.0;
    const double adot = ValarrayUtil::dot(selected, w, count, min_count);
    const double loss = LogLoss::addGradientSep
      (w, adot, selected, sibling, turns[z], count, min_count,
       gradient_sep, agsum, weight, margin);
    LogLoss::addGradientSep(selected, count, min_count, agsum, 
			    gradient_sep);

    combined.y = 1-std::max(0, turns[z]);
    const double loss_naive
      = LogLoss::addGradient(w, combined, gradient_naive, weight, margin);
    
    const double eps = 1e-8;
    BOOST_CHECK_SMALL(loss - loss_naive, eps);
    for (size_t i=0; i<dim; ++i)
      BOOST_CHECK_SMALL(gradient_sep[i] - gradient_naive[i], eps);
  }
}

BOOST_AUTO_TEST_CASE(LossTestSeparationTest)
{
  separation_test(1.0, 0.0);
  separation_test(200.0, 0.0);
  separation_test(1.0, 1.0);
}




// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
