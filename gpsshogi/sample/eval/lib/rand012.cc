/* rand012.cc
 */
#include "rand012.h"
#include "instanceData.h"
#include "osl/container.h"
#include "osl/random.h"
#include <iostream>
#include <cassert>
#include <cmath>

gpsshogi::
Rand012::Rand012() : max_iteration(64)
{
}

gpsshogi::
Rand012::~Rand012()
{
}

void gpsshogi::
Rand012::iterationHead(int, const valarray_t&, double)
{
}

int gpsshogi::
Rand012::rand012()
{
  int v = osl::random() % 4;
  return (v >= 2) ? v - 1 : v;
}

void gpsshogi::
Rand012::updateWeight(valarray_t& w, valarray_t& gradient) const
{
  osl::FixedCapacityVector<std::pair<double,int>,osl::PTYPE_SIZE> piece_gradient;
  for (int i=osl::PTYPE_PIECE_MIN; i<osl::PTYPE_SIZE; ++i)
    if (i != osl::KING)
      piece_gradient.push_back(std::make_pair(gradient[i], i));
  std::sort(piece_gradient.begin(), piece_gradient.end());
  std::random_shuffle(piece_gradient.begin(), piece_gradient.begin()+6);
  std::random_shuffle(piece_gradient.begin()+7, piece_gradient.end());
  assert(piece_gradient.size() == 13);
  w[piece_gradient[ 0].second] +=  2;
  w[piece_gradient[ 1].second] +=  2;
  w[piece_gradient[ 2].second] +=  1;
  w[piece_gradient[ 3].second] +=  1;
  w[piece_gradient[ 4].second] +=  1;
  w[piece_gradient[ 8].second] += -1;
  w[piece_gradient[ 9].second] += -1;
  w[piece_gradient[10].second] += -1;
  w[piece_gradient[11].second] += -2;
  w[piece_gradient[12].second] += -2;
  for (size_t i=osl::PTYPE_SIZE; i<gradient.size(); ++i) {
    if (gradient[i] > 0) 
      w[i] -= rand012();
    else if (gradient[i] < 0) 
      w[i] += rand012();
  }
}

void gpsshogi::
Rand012::solve(valarray_t& w)
{
  double prev_error = 0;
  int t=0; for (; t<max_iteration; ++t) {
    iterationHead(t, w, prev_error);
    valarray_t gradient;
    double err;
    makeGradient(w, gradient, err);
    if (! std::isnormal(err)) {
      std::cerr << "oops\n";
      return;
    }
    prev_error = err;
    updateWeight(w, gradient);
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
