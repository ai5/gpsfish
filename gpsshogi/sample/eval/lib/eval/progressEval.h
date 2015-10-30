/* progressEval.h
 */
#ifndef GPSSHOGI_PROGRESSEVAL_H
#define GPSSHOGI_PROGRESSEVAL_H

#include "eval/eval.h"
#include <boost/scoped_array.hpp>
namespace gpsshogi
{
  class ProgressEvalBase : public Eval
  {
    boost::scoped_array<int> values;
  protected:
    ProgressEvalBase();
    ~ProgressEvalBase();
  public:
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
    int roundUp() const { return 64*16; }

    class Stack;
    friend class Stack;
    EvalValueStack *newStack(const NumEffectState& state);
    // for debug
    int openingValue(const NumEffectState& state) const;
    int endgameValue(const NumEffectState& state) const;
  };
  
  class KProgressEval : public ProgressEvalBase
  {
  public:
    KProgressEval();
    ~KProgressEval();
  };

  class StableProgressEval : public ProgressEvalBase
  {
  public:
    StableProgressEval();
    ~StableProgressEval();
  };
}

#endif /* GPSSHOGI_PROGRESSEVAL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
