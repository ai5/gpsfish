/* featureSet.h
 */
#ifndef GPSSHOGI_MOVE_PROBABILITY_FEATURESET_H
#define GPSSHOGI_MOVE_PROBABILITY_FEATURESET_H

#include "indexList.h"
#include "stateInfo.h"
#include "osl/numEffectState.h"
#include "osl/container/moveLogProbVector.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <valarray>
namespace gpsshogi
{
  class Feature;
  typedef std::valarray<weight_t> valarray_t;
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
    weight_t match(const StateInfo&, Move, index_list_t&, const valarray_t& weights) const;    
    weight_t matchExp(const StateInfo&, Move, index_list_t&, const valarray_t& weights) const;    
    weight_t matchLight(const StateInfo&, Move, index_list_t&, const valarray_t& weights) const;    
    void generateLogProb(const StateInfo& state,
			 MoveLogProbVector& out, const valarray_t& weights) const;
    static weight_t accumulate(const index_list_t& features, const valarray_t& weights);
    static weight_t accumulateExp(const index_list_t& features, const valarray_t& weights);
    void save(const char *base_filename, const valarray_t& weights) const;
    bool load(const char *base_filename, valarray_t& weights) const;
    int logLikelihood(const StateInfo& state,
		      Move move, const valarray_t& weights) const;
    void showSummary(const valarray_t& weights) const;
    static weight_t sumToProbability(double sum) { return sum/(1.0+sum); }
    void analyze(const StateInfo& state, Move move, const valarray_t& weights) const;
    // for fine control
    weight_t generateRating(const StateInfo& state,
			WeightedMoveVector& out, const valarray_t& weights) const;
    static void ratingToLogProb(const WeightedMoveVector& rating,
				weight_t sum,
				MoveLogProbVector& out);
  };

  class StandardFeatureSet : public FeatureSet
  {
  public:
    StandardFeatureSet();
  };  

  struct PredictionModelLight 
  {
    static int logProbTakeBack(const StateInfo&, double sum, const valarray_t& w);
    static int logProbSeePlus(const StateInfo&, double sum, const valarray_t& w);
    static int predict(int offset, double sum, const valarray_t& w);

    static size_t dimension() { return 4*8; }
    static bool load(const char *filename, valarray_t&);
    static void save(const char *filename, const valarray_t&);
    static weight_t addGradientTakeBack(int progress8, Move next, Move target, double sum, const valarray_t& w,
				    valarray_t& partial);
    static weight_t addGradientSeePlus(const StateInfo&, Move next, Move target, double sum, const valarray_t& w,
				   valarray_t& partial);
  private:
    static weight_t addGradient(Move next, Move target, double sum, const valarray_t& w,
				valarray_t& partial, int offset);
  };
  
}

#endif /* GPSSHOGI_MOVE_PROBABILITY_FEATURESET_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
