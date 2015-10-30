/* l1Ball.cc
 */
#include "l1Ball.h"
#include "instanceData.h"
#include <vector>
#include <iostream>
#include <cassert>
#include <cmath>

inline double sign(double a) 
{
  return (a >= 0.0) ? 1.0 : -1.0;
}
/* ------------------------------------------------------------------------- */

gpsshogi::
L1BallBase::L1BallBase() : eta0(1e-4), tolerance(1e-6), mean_abs_weight(1.0), eta_increment(1.2), max_iteration(64)
{
}

gpsshogi::
L1BallBase::~L1BallBase()
{
}

inline int pick(const gpsshogi::L1Ball::valarray_t& v, const std::vector<int>& U) 
{
  // better to use random()?
  double k[3] = { v[U.front()], v[U[U.size()/2]], v[U.back()] };
  std::sort(k, k+3);
  if (k[1] == v[U.front()]) return U.front();
  if (k[1] == v[U.back()]) return U.back();
  return U[U.size()/2];
}
double gpsshogi::L1BallBase::findTheta(const valarray_t& v, double z)
{
  std::vector<int> U(v.size()), G, L;
  for (size_t i=0; i<v.size(); ++i)
    U[i] = i;
  double s = 0;
  int rho = 0;
  while (! U.empty()) {
    int k = pick(v, U);
    double ds = 0.0;
    G.clear();
    L.clear();
    for (size_t i=0; i<U.size(); ++i) {
      if (v[U[i]] < v[k]) {
	L.push_back(U[i]);
	continue;
      }
      if (U[i] != k)
	G.push_back(U[i]);
      ds += v[U[i]];
    }
    if (s + ds - (rho + G.size()+1)*v[k] < z) {
      s += ds;
      rho += G.size()+1;
      U.swap(L);
    }
    else {
      U.swap(G);
    }
  }
  return (s-z)/rho;
}

gpsshogi::L1BallBase::valarray_t 
gpsshogi::L1BallBase::projection(const valarray_t& v, double z)
{
  valarray_t mu(v);
  double vsum = 0.0;
  for (size_t i=0; i<v.size(); ++i) {
    mu[i] = std::abs(v[i]);
    vsum += mu[i];
  }
  if (vsum <= z)
    return v;
  std::cerr << '^';
  double theta = findTheta(mu, z);
  // reuse mu
  for (size_t i=0; i<v.size(); ++i)
    mu[i] = sign(v[i])*std::max(0.0, std::abs(v[i]) - theta);
  return mu;    
}

void gpsshogi::L1BallBase::iterationHead(int /*i*/, const valarray_t& /*w*/, double)
{
}

double gpsshogi::L1BallBase::l1norm(const valarray_t& w)
{
  double sum = 0.0;
  for (size_t i=0; i<w.size(); ++i)
    sum += std::abs(w[i]);
  return sum;
}

double gpsshogi::L1BallBase::l2norm(const valarray_t& w)
{
  double sum = 0.0;
  for (size_t i=0; i<w.size(); ++i)
    sum += w[i]*w[i];
  return sum;
}

void gpsshogi::L1BallBase::initializeEta(valarray_t& eta, double eta0) const
{
  eta = eta0;
}

void gpsshogi::L1BallBase::reduceEta(valarray_t& eta, int /*update_failed*/, const valarray_t& /*prev_sign*/, const valarray_t& /*gradient*/) const
{
  eta *= 0.125;
}

/* ------------------------------------------------------------------------- */

gpsshogi::
L1Ball::L1Ball() : fix_step_size(false)
{
}
gpsshogi::
L1Ball::~L1Ball()
{
}

void gpsshogi::
L1Ball::updateWeight(valarray_t& w, valarray_t& eta, const valarray_t& gradient,
		     bool adjust_eta) const
{
  if (fix_step_size) {
    valarray_t signs(gradient.size());
    for (size_t i=0; i<gradient.size(); ++i) {
      if (gradient[i] > 0) 
	signs[i] = 1;
      else if (gradient[i] < 0) 
	signs[i] = -1;
    }
    w = projection(w - eta*signs, mean_abs_weight*w.size());
  }
  else {
    if (adjust_eta) {
      const double initial_w = l2norm(w);
      while (initial_w > 0) {
	valarray_t wn = projection(w - eta*gradient, mean_abs_weight*w.size());
	double changed = l2norm(w - wn);
	if (changed < initial_w/4)
	  break;
	eta *= 0.5;
      } 
    }
    w = projection(w - eta*gradient, mean_abs_weight*w.size());
  }
}

const double perturbation = 1.000;
void gpsshogi::
L1Ball::solve(valarray_t& w)
{
  const double initial_norm = l2norm(w);
  valarray_t best_w(w), best_w_gradient(w.size()), initial_w(w);
  valarray_t eta(w.size()), prev_sign(w.size());
  std::vector<int> same_signs(w.size()), sign_changes(w.size());
  int max_same_signs = 0;

  double prev_error = 0, min_error = 1e8;
  initializeEta(eta, eta0);
  bool adjust_eta = false;
  int update_failed=0;
  prev_sign = 0.0;
  
  int t=0; for (; t<max_iteration; ++t) {
    iterationHead(t, w, prev_error/perturbation);
    valarray_t gradient;
    double err;
    makeGradient(w, gradient, err);
#if 0
    std::cerr << "L1Ball " << t << " " << err
	      << ' '<< prev_error
	      << ' ' << abs(w).sum() 
	      << " [" << w.min() << ',' << w.max() << "] " <<"\n";
    for (size_t i=0; i<w.size(); ++i)
      std::cerr << " " << w[i];
    std::cerr << "\n";
    for (size_t i=0; i<eta.size(); ++i)
      std::cerr << " " << eta[i];
    std::cerr << "\n";
#endif
    if (err < tolerance) {
      std::cerr << "error < tolerance " << err << ' ' << tolerance << "\n";
      break;
    }
    if (! std::isnormal(err) || (t && err > prev_error*perturbation)) {
      w = best_w;
      ++update_failed;
      reduceEta(eta, update_failed, prev_sign, gradient);
      std::fill(same_signs.begin(), same_signs.end(), 0.0);
      adjust_eta = false;
      updateWeight(w, eta, best_w_gradient);
      continue;
    }
    update_failed = 0;
    if (t == 0 || err < min_error*perturbation) {
      best_w = w;
      best_w_gradient = gradient;
      min_error = err;
    }
    updateWeight(w, eta, gradient, t==0);
    for (size_t i=0; i<w.size(); ++i) {
      if (adjust_eta) {
	if (prev_sign[i] != sign(gradient[i])) {
	  same_signs[i] = 0;
	  eta[i] *= 0.5;
	  sign_changes[i]++;
	}
	else {
	  same_signs[i] += 1;
	  max_same_signs = std::max(max_same_signs, same_signs[i]);
	  if (same_signs[i] == 1)
	    eta[i] *= 1.1;
	  else
	    eta[i] *= eta_increment;
	}
      }
      prev_sign[i] = sign(gradient[i]);
    }
    adjust_eta = true;
    prev_error = err;
    const double changed = l2norm(w - initial_w);
    if (changed > initial_norm && initial_norm > 0) {
      std::cerr << "early stopping by norm2 " << changed
		<< " > " << initial_norm << "\n";
      eta *= 0.25;
      break;
    }
  }
  w = best_w;
  last_iteration = t;
  last_error = prev_error;
  last_eta.resize(eta.size());
  last_eta = eta;
#ifdef L1BALL_VERBOSE
  // stat: large max_same_signs and #features without changes of sign indicate that we should increase eta_increment 
  std::vector<int>::iterator non_zero_end = std::remove(sign_changes.begin(), sign_changes.end(), 0);  
  std::sort(sign_changes.begin(), non_zero_end);
  std::cerr << "$" << max_same_signs << '%' << sign_changes.end() - non_zero_end - std::count(&w[0], &w[0]+w.size(), 0.0);
  for (size_t i=0; i<std::min((size_t)4, sign_changes.size()); ++i)
    std::cerr << ':' << sign_changes[i];
#endif
}

/* ------------------------------------------------------------------------- */

gpsshogi::
L1BallSMD::L1BallSMD()
{
}
gpsshogi::
L1BallSMD::~L1BallSMD()
{
}

struct gpsshogi::L1BallSMD::Params
{
  valarray_t w, eta, v, Hv, gradient;
  explicit Params(size_t l) : w(l), eta(l), v(l), Hv(l), gradient(l)
  {
  }
};

void gpsshogi::
L1BallSMD::solve(valarray_t& weight)
{
  Params prev(weight.size()), cur(weight.size()), next(weight.size());
  double lambda_for_v = 0.3 /*eta0*/, meta_mu = 0.1;
  double prev_error = 0, min_error = 1e8;
  int successive_improvement = 0, total_failure = 0;
  cur.eta = cur.v = eta0;
  cur.w = weight;
  prev = cur;
  
  int t=0; for (; t<max_iteration; ++t) {
    iterationHead(t, cur.w, prev_error);
    next = cur;
    double err;
    if (t)
      next.v = lambda_for_v*cur.v - cur.eta*(cur.gradient + lambda_for_v*cur.Hv);

    makeGradient(cur.w, next.gradient, err, next.v, next.Hv);
#ifdef L1BALL_SMD_VERBOSE
    std::cerr << "t " << t << "\n";
    std::cerr << "w ";
    for (size_t i=0; i<cur.w.size(); ++i) std::cerr << " " << cur.w[i];
    std::cerr << "\n";
    std::cerr << "e ";
    for (size_t i=0; i<cur.w.size(); ++i) std::cerr << " " << next.eta[i];
    std::cerr << "\n";
    std::cerr << "v ";
    for (size_t i=0; i<cur.w.size(); ++i) std::cerr << " " << next.v[i];
    std::cerr << "\n";
    std::cerr << "g ";
    for (size_t i=0; i<cur.w.size(); ++i) std::cerr << " " << next.gradient[i];
    std::cerr << "\n";
    std::cerr << "Hv ";
    for (size_t i=0; i<cur.w.size(); ++i) std::cerr << " " << next.Hv[i];
    std::cerr << "\n";
#endif
    for (size_t i=0; i<cur.w.size(); ++i) 
      if (lambda_for_v > 1e-4 && std::abs(next.Hv[i]) > 1e6) {
	std::cerr << 'l';
	lambda_for_v /= 8.0;
	break;
      }
    
    if (err < tolerance) 
      break;
    if (! std::isnormal(err) || (t && err > prev_error)) {
      if (t == 0)
	prev.eta *= 0.125;
      else
	prev.eta *= 0.5;
      cur = prev;
      // meta_mu *= 0.125;
      --t;
      if (successive_improvement) {
	successive_improvement = 0;
	++total_failure;	// count failure after success
      }
      continue;
    }
    ++successive_improvement;
    // improvement in previous iteration confirmed
    prev = cur;
    cur = next;
    if (t == 0 || err < min_error)
      min_error = err;

    double max_inc = 0.0;
    for (size_t i=0; i<cur.eta.size(); ++i) 
      max_inc = std::max(max_inc, -meta_mu*cur.gradient[i]*cur.v[i]);
    if (max_inc > 0.25) // test for approximation of e(x) =~ 1+x
      meta_mu *= 0.25/max_inc;
    for (size_t i=0; i<cur.eta.size(); ++i) {
      cur.eta[i] = prev.eta[i]*std::max(0.5, 1-meta_mu*cur.gradient[i]*cur.v[i]);
    }
    // quick aid for too small initial values
    if (successive_improvement > total_failure*4+1)
      cur.eta *= eta_increment;
    cur.w = projection(cur.w - cur.eta*cur.gradient, mean_abs_weight*cur.w.size());
    prev_error = err;
  }
  weight = prev.w;
  last_iteration = t;
  last_error = prev_error;
  last_eta.resize(prev.eta.size());
  last_eta = prev.eta;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
