/* featureSet.h
 */
#ifndef OSL_MOVE_PROBABILITY_FEATURESET_H
#define OSL_MOVE_PROBABILITY_FEATURESET_H

#include "osl/move_probability/stateInfo.h"
#include "osl/numEffectState.h"
#include "osl/container/moveLogProbVector.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_array.hpp>
#include <vector>

namespace osl
{
  namespace move_probability
  {
    class Feature;
    typedef std::pair<double,Move> WeightedMove;
    typedef FixedCapacityVector<WeightedMove,Move::MaxUniqMoves> WeightedMoveVector;

    class FeatureSet
    {
      boost::ptr_vector<Feature> features;
      std::vector<int> offsets, light_features;
    protected:
      FeatureSet();
    public:
      ~FeatureSet();
      void pushBack(Feature *, bool light=false);
      void addFinished();
      int dimension() const { return offsets.back(); }
    public:
      double matchExp(const StateInfo&, Move, const double *weights) const;
      double matchNoExp(const StateInfo&, Move, const double *weights) const;    
      void generateLogProb(const StateInfo& state,
			   MoveLogProbVector& out, const double *weights) const;
      double matchLight(const StateInfo&, Move, const double *weights) const;

      bool load(const char *base_filename, double *weights) const;
      bool load_binary(const char *base_filename, double *weights) const;
      void showSummary(const double *weights) const;
      void analyze(const StateInfo& state, Move move, const double *weights) const;
      // for fine control
      double generateRating(const StateInfo& state,
			    WeightedMoveVector& out, const double *weights) const;
      static void ratingToLogProb(const WeightedMoveVector& rating,
				  double sum, MoveLogProbVector& out);
    };

    class StandardFeatureSet : public FeatureSet
    {
      static boost::scoped_array<double> weights, tactical_weights;
      bool initialized;
    public:
      StandardFeatureSet();
      ~StandardFeatureSet();
      
      static const StandardFeatureSet& instance(bool verbose=false);
      static bool healthCheck();
      void generateLogProb(const StateInfo& state, MoveLogProbVector& out) const;
      void generateLogProb2(const StateInfo& state, MoveLogProbVector& out) const;
      void generateLogProb(const StateInfo& state, int limit, MoveLogProbVector& out, bool in_pv) const;
      int logProbTakeBack(const StateInfo& state, Move target) const;
      int logProbSeePlus(const StateInfo& state, Move target) const;      
      double matchLight(const StateInfo&, Move) const;    
      double matchExp(const StateInfo&, Move) const;
      double matchNoExp(const StateInfo&, Move) const;
      bool setUp(bool verbose=false);
      bool ok() const { return initialized; }
    private:
      int tacticalLogProb(int offset, double sum) const;
    };
  }
}
#endif /* OSL_MOVE_PROBABILITY_FEATURESET_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
