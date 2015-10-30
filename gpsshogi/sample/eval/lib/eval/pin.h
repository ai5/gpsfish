/* pin.h
 */
#ifndef _GPSSSHOGI_SAMPLE_EVAL_LIB_PIN_H
#define _GPSSSHOGI_SAMPLE_EVAL_LIB_PIN_H

#include "eval/eval.h"
#include <iomanip>

namespace gpsshogi
{
  template <int DIM, class IndexFunc>
  class PinBase : public EvalComponentMulti
  {
  private:
    IndexFunc f;
  public:
    PinBase() : EvalComponentMulti(DIM) { }
    MultiInt eval(const NumEffectState& state, const MultiWeights& w,
		  CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights&) const;
    size_t maxActiveOne() const { return 16; }
    const std::string name() const { return "PinBase"; };
  };

  struct PinF
  {
    int operator()(const Square king, const Piece piece) const
    {
      return (piece.ptype() - osl::PTYPE_PIECE_MIN) * 17 * 9 +
	std::abs(king.x() - piece.square().x()) * 17 + 8 +
	(king.y() - piece.square().y()) * (piece.owner() == BLACK ? 1 : -1);
    }
  };

  class Pin : public PinBase<(osl::PTYPE_MAX- osl::PTYPE_PIECE_MIN + 1) * 17 * 9,
			     PinF>
  {
  public:
    const std::string name() const { return "Pin"; };
    void showAll(std::ostream &os, const MultiWeights& weights) const;
  };

  struct PinYF
  {
    int operator()(const Square king, const Piece piece) const
    {
      const int king_y = (piece.owner() == BLACK ? king.y() : 10 - king.y());
      const int piece_y = (piece.owner() == BLACK ?
			   piece.square().y() : 10 - piece.square().y());
      return (piece.ptype() - osl::PTYPE_PIECE_MIN) * 9 * 9 * 9 +
	std::abs(king.x() - piece.square().x()) * 9 * 9 +
	(piece_y - 1) * 9 + king_y - 1;
    }
  };

  class PinY : public PinBase<(osl::PTYPE_MAX- osl::PTYPE_PIECE_MIN + 1) * 9 * 9 * 9,
			     PinYF>
  {
    const std::string name() const { return "PinY"; };
  };

  class PinPtype : public FeaturesOneNonUniq
  {
  public:
    // rook v, rook h, bishop u, bishop d, lance
    PinPtype() : FeaturesOneNonUniq(PTYPE_SIZE * 5) { }
    const std::string name() const { return "PinPtype"; }
    size_t maxActive() const { return 16; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  private:
    template <Player Defense>
    void featuresOneKing(const NumEffectState &state,
			 IndexCacheI<MaxActiveWithDuplication> &) const;
  };

  class PinPtypeDistance : public FeaturesOneNonUniq
  {
  public:
    // rook v, rook h, bishop u, bishop d, lance, distance [1-7]
    PinPtypeDistance() : FeaturesOneNonUniq(PTYPE_SIZE * 5 * 7) { }
    const std::string name() const { return "PinPtypeDistance"; }
    size_t maxActive() const { return 16; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  private:
    template <Player Defense>
    void featuresOneKing(const NumEffectState &state,
			 IndexCacheI<MaxActiveWithDuplication> &) const;
  };

  class PinPtypePawnAttack : public FeaturesOneNonUniq
  {
  public:
    // rook h, bishop u, bishop d
    PinPtypePawnAttack() : FeaturesOneNonUniq(PTYPE_SIZE * 3) { }
    const std::string name() const { return "PinPtypePawnAttack"; }
    size_t maxActive() const { return 16; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  private:
    template <Player Defense>
    void featuresOneKing(const NumEffectState &state,
			 IndexCacheI<MaxActiveWithDuplication> &) const;
  };

  class CheckShadowPtype : public FeaturesOneNonUniq
  {
  public:
    // rook v, rook h, bishop u, bishop d, lance
    CheckShadowPtype() : FeaturesOneNonUniq(PTYPE_SIZE * 5) { }
    const std::string name() const { return "CheckShadowPtype"; }
    size_t maxActive() const { return 16; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  private:
    template <Player Defense>
    void featuresOneKing(const NumEffectState &state,
			 IndexCacheI<MaxActiveWithDuplication> &) const;
  };

}

#endif /* _GPSSSHOGI_SAMPLE_EVAL_LIB_PIN_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
