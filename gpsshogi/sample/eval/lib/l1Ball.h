/* l1Ball.h
 */
#ifndef GPSSHOGI_L1BALL_H
#define GPSSHOGI_L1BALL_H

#include <vector>
#include <valarray>

namespace gpsshogi
{
  class InstanceData;
  /**
   * http://www.conflate.net/icml/paper/2008/361
   */
  class L1BallBase
  {
  public:
    typedef std::valarray<double> valarray_t;
  protected:
    double eta0, tolerance, mean_abs_weight, eta_increment;
    int max_iteration;

    int last_iteration;
    double last_error;
    valarray_t last_eta;

  public:
    L1BallBase();
    virtual ~L1BallBase();

    void setEta0(double new_eta0) { eta0 = new_eta0; }
    void setEtaIncrement(double new_eta_increment) { eta_increment = new_eta_increment; }
    void setTolerance(double new_tolerance) { tolerance = new_tolerance; }
    void setMaxIteration(double new_max_iteration) { max_iteration = new_max_iteration; }
    void setMeanAbsWeight(double new_mean_abs_weight) { mean_abs_weight = new_mean_abs_weight; }

    int lastIteration() const { return last_iteration; }
    double lastError() const { return last_error; }
    const valarray_t& lastEta() const { return last_eta; }

    static double l1norm(const valarray_t&);
    static double l2norm(const valarray_t&);
    static valarray_t projection(const valarray_t& w, double z);
    static double findTheta(const valarray_t& v, double z);

    virtual void iterationHead(int i, const valarray_t& w, double prev_error);
    virtual void solve(valarray_t& w)=0;

    virtual void initializeEta(valarray_t& eta, double eta0) const;
    virtual void reduceEta(valarray_t& eta, int update_failed, const valarray_t& prev_sign, const valarray_t& gradient) const;
  };
  
  /** solve by heuristic eta update */
  class L1Ball : public L1BallBase
  {
    bool fix_step_size;
  public:
    L1Ball();
    ~L1Ball();

    void setFixStepSize(bool new_fix_step_size) { fix_step_size = new_fix_step_size; }
    void solve(valarray_t& w);
  protected:
    void updateWeight(valarray_t& w, valarray_t& eta, const valarray_t& gradient,
		      bool adjust_eta=false) const;
    virtual void makeGradient(const valarray_t& w, valarray_t& gradient, double& error) = 0;
  };

  /** solve by stochastic meta descent */
  class L1BallSMD : public L1BallBase
  {
  public:
    L1BallSMD();
    ~L1BallSMD();

    void solve(valarray_t& w);

    struct Params;
  protected:
    /** make gradient as well as Hessian-vector product */
    virtual void makeGradient(const valarray_t& w, valarray_t& gradient, double& error,
			      const valarray_t& v, valarray_t& Hv) = 0;
  };
}

#endif /* GPSSHOGI_L1BALL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
