/* loss.h
 */
#ifndef GPSSHOGI_LOSS_H
#define GPSSHOGI_LOSS_H

#include <vector>
#include <valarray>
#include <utility>

namespace gpsshogi
{
  class InstanceData;
  typedef std::vector<std::pair<int,double> > sparse_vector_t;
  struct ValarrayUtil
  {
    typedef std::valarray<double> valarray_t;
    static double dot(const valarray_t& l, const valarray_t& r);
    static double dot(const InstanceData& l, const valarray_t& r);
    static double dot(const sparse_vector_t& l, const valarray_t& r, const std::vector<size_t>& frequency, int min_frequency);
  };

  struct LogLoss : public ValarrayUtil
  {
    /** @return loss for this sample */
    static double addGradient(const valarray_t& w, const valarray_t& x, double y, valarray_t& gradient);
    static double addGradient(const valarray_t& w, const InstanceData&, valarray_t& gradient, double instance_weight=1.0, double margin=0.0);

    static double addGradient(const valarray_t& w, const valarray_t& x, double y, valarray_t& gradient,
			      const valarray_t& v, valarray_t& Hv);
    static double addGradient(const valarray_t& w, const InstanceData&, valarray_t& gradient,
			      const valarray_t& v, valarray_t& Hv);

    // x = b-a
    static double addGradientSep(const valarray_t& w, double adot, const InstanceData& a,
				 const InstanceData& b, valarray_t& gradient, double& agsum);
    static double addGradientSep(const valarray_t& w, double adot, const sparse_vector_t& a,
				 const sparse_vector_t& b, double turn_coef,
				 const std::vector<size_t>& frequency, int min_frequency,
				 valarray_t& gradient, double& agsum,
				 double instance_weight=1.0, double margin=0.0);
    static void addGradientSep(const InstanceData& a, double agsum, valarray_t& gradient);
    static void addGradientSep(const sparse_vector_t& a, 
			       const std::vector<size_t>& frequency, int min_frequency,
			       double agsum, valarray_t& gradient);
  };

  struct HingeLoss : public ValarrayUtil
  {
    static double addGradient(const valarray_t& w, const InstanceData&, valarray_t& gradient);
    static double addGradient(const valarray_t& w, const InstanceData&, valarray_t& gradient,
			      const valarray_t& v, valarray_t& Hv);


    static double addGradientSep(const valarray_t& w, double adot, const sparse_vector_t& a,
				 const sparse_vector_t& b, double turn_coef,
				 const std::vector<size_t>& frequency, int min_frequency,
				 valarray_t& gradient, double& agsum,
				 double instance_weight=1.0, double margin=0.0);
    static void addGradientSep(const sparse_vector_t& a, 
			       const std::vector<size_t>& frequency, int min_frequency,
			       double agsum, valarray_t& gradient);
  };

  struct ExpLoss : public ValarrayUtil
  {
    static double addGradient(const valarray_t& w, const InstanceData&, valarray_t& gradient);
    static double addGradient(const valarray_t& w, const InstanceData&, valarray_t& gradient,
			      const valarray_t& v, valarray_t& Hv);
  };
  struct SigmoidLoss : public ValarrayUtil
  {
    static double addGradient(const valarray_t& w, const InstanceData&, valarray_t& gradient, double instance_weight=1.0, double margin=0.0);
    static double addGradient(const valarray_t& w, const InstanceData&, valarray_t& gradient,
			      const valarray_t& v, valarray_t& Hv);

    // x = b-a
    static double addGradientSep(const valarray_t& w, double adot, const sparse_vector_t& a,
				 const sparse_vector_t& b, double turn_coef,
				 const std::vector<size_t>& frequency, int min_frequency,
				 valarray_t& gradient, double& agsum,
				 double instance_weight=1.0, double margin=0.0);
    static void addGradientSep(const sparse_vector_t& a, 
			       const std::vector<size_t>& frequency, int min_frequency,
			       double agsum, valarray_t& gradient);
  };
  struct HalfSigmoidLoss : public ValarrayUtil
  {
    static double addGradientSep(const valarray_t& w, double adot, const sparse_vector_t& a,
				 const sparse_vector_t& b, double turn_coef,
				 const std::vector<size_t>& frequency, int min_frequency,
				 valarray_t& gradient, double& agsum,
				 double instance_weight=1.0, double margin=0.0);
    static void addGradientSep(const sparse_vector_t& a, 
			       const std::vector<size_t>& frequency, int min_frequency,
			       double agsum, valarray_t& gradient);
  };
}

#endif /* GPSSHOGI_LOSS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
