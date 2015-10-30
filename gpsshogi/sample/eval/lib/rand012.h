/* rand012.h
 */
#ifndef GPSSHOGI_RAND012_H
#define GPSSHOGI_RAND012_H

#include <vector>
#include <valarray>

namespace gpsshogi
{
  class InstanceData;
  class Rand012
  {
    int max_iteration;
  public:
    typedef std::valarray<double> valarray_t;
    Rand012();
    virtual ~Rand012();

    void setMaxIteration(double new_max_iteration) { max_iteration = new_max_iteration; }
    void solve(valarray_t& w);
    static int rand012();
  private:
    void updateWeight(valarray_t& w, valarray_t& gradient) const;
  protected:
    virtual void iterationHead(int i, const valarray_t& w, double prev_error);
    virtual void makeGradient(const valarray_t& w, valarray_t& gradient, double& error) = 0;
  };
  
}

#endif /* GPSSHOGI_RAND012_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
