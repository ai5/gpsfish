/* pieceEvalComponent.h
 */
#ifndef _PIECEEVALCOMPONENT_H
#define _PIECEEVALCOMPONENT_H

#include "eval/eval.h"
namespace gpsshogi
{
  class PieceEvalComponent : public EvalComponent
  {
  private:
    PieceEval piece_eval;
  public:
    PieceEvalComponent() : EvalComponent(osl::PTYPE_SIZE) {
      for (size_t i=0; i<dimension(); ++i)
	setValue(i, piece_eval.flatValue(i));
    }
    int eval(const NumEffectState& state) const {
      return piece_eval.eval(state);
    }
    void featuresNonUniq(const NumEffectState& state, 
			 index_list_t &out, int offset) const {
      MoveData data;
      piece_eval.features(state, data, offset);
      for (size_t i=0; i<data.diffs.size(); ++i)
	out.add(data.diffs[i].first, data.diffs[i].second); // XXX
    }
    // int value(size_t index) const { return piece_eval.value(index); }
    void setWeightScale(const double *w, double scale) {
      piece_eval.setWeightScale(w, scale);
      for (size_t i=0; i<dimension(); ++i)
	setValue(i, piece_eval.flatValue(i));
    }
    size_t dimension() const { return piece_eval.dimension(); }
    size_t maxActive() const { return piece_eval.maxActive(); }
    void saveWeight(double*w) const { 
      piece_eval.saveWeight(w); 
    }
    void setRandom() { 
      piece_eval.setRandom(); 
      for (size_t i=0; i<dimension(); ++i)
	setValue(i, piece_eval.flatValue(i));
    }
    int evalWithUpdate(const NumEffectState& /*state*/,
		       Move moved, int last_value) const {
      return last_value + piece_eval.diff(moved);
    }
    void showSummary(std::ostream& os) const {
      piece_eval.showSummary(os);
    }
    void showAll(std::ostream& os) const {
      piece_eval.showAll(os);
    }
    int pieceValue(const NumEffectState& state, Piece p) const {
      return piece_eval.pieceValue(state, p);
    }
    bool hasPieceValue() const {
      return piece_eval.hasPieceValue();
    }
    void showEvalSummary(const NumEffectState& state) const {
      piece_eval.showEvalSummary(state);
    }
    const std::string name() const { return "PieceEvalComponent"; };
    void setZero()
    {
      std::vector<double> zero(osl::PTYPE_SIZE);
      setWeightScale(&zero[0], 1.0);
    }
  };
}


#endif /* _PIECEEVALCOMPONENT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
