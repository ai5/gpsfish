/* analyzer.h
 */
#ifndef GPSSHOGI_LEARN_ANALYZER_H
#define GPSSHOGI_LEARN_ANALYZER_H

#include "moveData.h"
#include "instanceData.h"
#include "loss.h"
#include "pvVector.h"
#include "osl/state/historyState.h"
#include "osl/container.h"
#include "osl/stat/average.h"

#include <memory>
#include <cmath>

namespace gpsshogi
{  
  using osl::NumEffectState;
  
  class Eval;
  class Quiesce;
  struct SigmoidUtil
  {
    static constexpr double eps = 0.00001;
    static double alphax(double x, double alpha) 
    {
      return 1.0/(1.0+exp(-x*alpha));
    }
    static double tx(double x, double pawn) 
    {
      return 1.0/(1.0+exp(-x*3.0/pawn));
    }
    /** differential of tx */
    static double tpx(double x, double pawn) 
    {
      const double v = tx(x, pawn);
      return 3.0/pawn*v*(1.0-v);
    }
  };

  /** framework of analyses of sibling relation */
  class Analyzer : protected SigmoidUtil
  {
  public:
    static void analyzeLeaf(const NumEffectState& state, 
			    const PVVector& pv, Eval&,
			    MoveData& data, bool bonanza_compatible = false);
    static int leafValue(const NumEffectState& state, 
			 const PVVector& pv, Eval&);
    static void makeLeaf(NumEffectState& state, const PVVector& pv);

    static void
    makeInstanceSorted(double turn_coef, 
		       const sparse_vector_t& selected, const sparse_vector_t& sibling,
		       const std::vector<size_t>& frequency, int min_frequency, InstanceData& out);
  };
}

#endif /* GPSSHOGI_LEARN_ANALYZER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
