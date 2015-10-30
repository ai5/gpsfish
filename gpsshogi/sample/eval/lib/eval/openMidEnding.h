/* openMidEnding.h
 */
#ifndef _OPENMIDENDING_H
#define _OPENMIDENDING_H

#include "eval/eval.h"

namespace gpsshogi
{
  template <class Eval> struct OMDStackBase;
  class OpenMidEndingBase : public Eval
  {
  protected:
    boost::ptr_vector<EvalComponent> flat;
    mutable boost::ptr_vector<EvalComponentMulti> legacy;
    boost::ptr_vector<MultiWeights> legacy_weights;
    boost::ptr_vector<EvalComponentStages> triple;
  private:
    size_t dim, max_active, legacy_one_dim;
    bool progress_max_16;
  protected:
    explicit OpenMidEndingBase(bool progress_max_16);
    ~OpenMidEndingBase();
    /** call once in constructor of subclass, after all EvalComponents added */
    void addFinished();
    size_t legacyOneDim() const { return legacy_one_dim; }
  public:
    int maxProgress() const;
    int progress(const NumEffectState &state) const;
    int progress(int progress_value) const;

    friend class OMDStackBase<OpenMidEndingBase>;
    EvalValueStack *newStack(const NumEffectState& state);
    int pawnValue() const;
    int eval(const NumEffectState&) const;


    bool load(const char *filename);
    void save(const char *filename) const;
    void setWeight(const double*);
    void setWeightScale(const double*, double);
    void saveWeight(double*) const;

    const std::string describe(const std::string& feature, size_t local_index) const;
    const std::string describeAll(const std::string& feature) const;
    int flatValue(size_t index) const {
      for (size_t i=0; i<flat.size(); ++i) {
	if (index < flat[i].dimension())
	  return flat[i].value(index);
	index -= flat[i].dimension();
      }
      for (size_t s=0; s<MultiInt::size(); ++s) {
	for (size_t i=0; i<legacy.size(); ++i) {
	  if (index < legacy[i].dimensionOne())
	    return legacy_weights[i].value(index)[s];
	  index -= legacy[i].dimensionOne();
	}
      }
      for (size_t i=0; i<triple.size(); ++i) {
	if (index < triple[i].dimension())
	  return triple[i].value(index);
	index -= triple[i].dimension();
      }
      assert(0);
      return 0;
    }
    std::tuple<std::string, int, int> findFeature(size_t index) const;
    
    size_t dimension() const {
      return dim;
    }
    size_t lambdaStart() const { return osl::PTYPE_SIZE; };
    void features(const NumEffectState&, MoveData&, int) const;
    void showSummary(std::ostream&) const;
    void showAll(std::ostream&) const;
    void setRandom();

    size_t maxActive() const;

    int pieceValue(const NumEffectState& state, Piece p) const;
    bool hasPieceValue() const;
    void showEvalSummary(const NumEffectState&) const;

    // for debug
    int progressIndependentValue(const NumEffectState& state) const;
    int openingValue(const NumEffectState& state) const;
    int midgameValue(const NumEffectState& state) const;
#ifdef EVAL_QUAD
    int midgame2Value(const NumEffectState& state) const;
#endif
    int endgameValue(const NumEffectState& state) const;
    MultiInt legacyValue(size_t i, const NumEffectState& state) const;

    int compose(int flat, MultiInt value, int progress) const
    {
      return compose(flat, value, progress, maxProgress(), progress_max_16);
    }
    static int compose(int flat, MultiInt value, int progress, int progress_max,
		       bool progress_max_16)
    {
#ifdef EVAL_QUAD
      const int c0 = progress_max/3, c1 = c0*2, w2 = progress_max - c1;
      int c0_c = c0, w2_c = w2;
      if (! progress_max_16) {
	c0_c = w2_c = 1;
	assert(c0 == w2);
      }
      int sum = flat * c0 * w2_c;
      if (progress < c0) {
	const int scale = w2_c * (c0 - progress);
	sum += value[0] * scale;
      }
      if (progress > 0 && progress < c1) {
	const int scale = (progress < c0) ? (w2_c * progress) : (w2_c * (c1 - progress));
	sum += value[1] * scale;
      }
      if (progress > c0) {
	const int scale = (progress < c1) ? (w2_c * (progress-c0)) : (c0_c * (progress_max - progress));
	sum += value[2] * scale;
      }
      if (progress > c1) {
	const int scale = c0_c * (progress - c1);
	sum += value[3] * scale;
      }
      return sum;
#else
      const int progress_max/2;
      if (progress < c) 
      {
	return flat * progress_max + value[0] * 2*(c - progress) + value[1] * 2*progress;
      }
      else 
      {
	return flat * progress_max
	  + value[1] * 2*(progress_max - progress)
	  + value[2] * 2*(progress - c);
      }
#endif    
    }
    void debug(const NumEffectState& state) const;
  };

  class OpenMidEndingForTest : public OpenMidEndingBase
  {
  public:
    explicit OpenMidEndingForTest(int richness);
    ~OpenMidEndingForTest();
  };
  
  class KOpenMidEnding : public OpenMidEndingBase
  {
  public:
    KOpenMidEnding();
    ~KOpenMidEnding();
  };

  class StableOpenMidEnding : public OpenMidEndingBase
  {
  public:
    explicit StableOpenMidEnding();
    ~StableOpenMidEnding();
  };

  
  class OslOpenMidEnding : public Eval
  {
    std::unique_ptr<StableOpenMidEnding> stable_eval;
    boost::scoped_array<int> values;
  public:
    OslOpenMidEnding();
    ~OslOpenMidEnding();

    int maxProgress() const;
    int progress(const NumEffectState &state) const;
    int progress(int progress_value) const;

    int eval(const NumEffectState&) const;
    int newValue(const NumEffectState& new_state, Move moved, int old_value) const;

    bool load(const char *filename);
    void save(const char *filename) const;
    void setWeight(const double*);
    void setWeightScale(const double*, double);
    void saveWeight(double*) const;
    int flatValue(size_t index) const;
    size_t dimension() const;
    size_t lambdaStart() const { return osl::PTYPE_SIZE; };
    void features(const NumEffectState&, MoveData&, int) const;
    void showSummary(std::ostream&) const;
    void showAll(std::ostream&) const;
    bool hasPieceValue() const;
    void setRandom();

    size_t maxActive() const;
    int pawnValue() const;

    class Stack;
    EvalValueStack *newStack(const NumEffectState& state);
    int roundUp() const { return 2; }
    // for debug
    int openingValue(const NumEffectState& state) const;
    int midgameValue(const NumEffectState& state) const;
#ifdef EVAL_QUAD
    int midgame2Value(const NumEffectState& state) const;
#endif
    int endgameValue(const NumEffectState& state) const;
    int progressIndependentValue(const NumEffectState& state) const;
    std::tuple<std::string, int, int> findFeature(size_t index) const;
    const std::string describe(const std::string& feature, size_t local_index) const;
  };
}

#endif /* _OPENMIDENDING_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
