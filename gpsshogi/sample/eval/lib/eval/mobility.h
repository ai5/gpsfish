/* mobility.h
 */
#ifndef _GPSSSHOGI_SAMPLE_EVAL_LIB_MOBILITY_H
#define _GPSSSHOGI_SAMPLE_EVAL_LIB_MOBILITY_H

#include "eval/eval.h"
namespace gpsshogi
{
  class RookMobility : public EvalComponentMulti
  {
  private:
    int index(const osl::NumEffectState &state,
	      const osl::Piece &rook, bool vertical) const;
  public:
    // vectical and horizontal, unpromoted and promoted
    RookMobility() : EvalComponentMulti(18 * 2) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
		  CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights& weights) const;
    size_t maxActiveOne() const { return 4; }
    const std::string name() const { return "RookMobility"; }
  };

  class RookMobilityX : public FeaturesOneNonUniq
  {
  private:
    int index(Piece rook, int count, bool vertical) const
    {
      const int x = (rook.square().x() > 5 ?
		     10 - rook.square().x() : rook.square().x());
      return x - 1 + 5 * ((rook.isPromoted() ? 1 : 0) +
			  2 * ((vertical ? 1 : 0) + 2 * count));
    }
  public:
    RookMobilityX() : FeaturesOneNonUniq(9 * 2 * 2 * 5) { }
    size_t maxActive() const { return 4; }
    const std::string name() const { return "RookMobilityX"; }
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &features) const;
  };

  class RookMobilityXKingX : public FeaturesOneNonUniq
  {
  private:
    static int index(Piece rook, Piece king, int count, bool vertical)
    {
      assert(! rook.isPromoted());
      const Square r = rook.owner() == BLACK ? rook.square() : rook.square().rotate180();
      const Square k = rook.owner() == BLACK ? king.square() : king.square().rotate180();
      const bool flip = r.x() > 5;
      const int x = (flip ? 10 - r.x() : r.x());
      const int king_x = (flip ? 10 - k.x() : k.x());
      return king_x - 1 + 9 * (x - 1 + 5 * ((vertical ? 1 : 0) + 2 * count));
    }
  public:
    RookMobilityXKingX() : FeaturesOneNonUniq(9 * 2 * 5 * 9) { }
    size_t maxActive() const { return 4; }
    const std::string name() const { return "RookMobilityXKingX"; }
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &features) const;
  };

  class RookMobilityY : public FeaturesOneNonUniq
  {
  private:
    int index(Piece rook, int count, bool vertical) const
    {
      const int y = (rook.owner() == BLACK ?
		     rook.square().y() : 10 - rook.square().y());
      return y - 1 + 9 * ((rook.isPromoted() ? 1 : 0) +
			  2 * ((vertical ? 1 : 0) + 2 * count));
    }
  public:
    RookMobilityY() : FeaturesOneNonUniq(9 * 2 * 2 * 9) { }
    size_t maxActive() const { return 4; }
    const std::string name() const { return "RookMobilityY"; }
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &features) const;
  };

  class RookMobilitySum : public FeaturesOneNonUniq
  {
  public:
    RookMobilitySum() : FeaturesOneNonUniq(17 * 2) { }
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &features) const;
    const std::string name() const { return "RookMobilitySum"; }
    size_t maxActive() const { return 2; };
  };

  class RookMobilitySumKingX : public FeaturesOneNonUniq
  {
  public:
    RookMobilitySumKingX() : FeaturesOneNonUniq(17 * 9) { }
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &features) const;
    const std::string name() const { return "RookMobilitySumKingX"; }
    size_t maxActive() const { return 2; };
  };

  class BishopMobility : public EvalComponentMulti
  {
  private:
    int index(const osl::NumEffectState &state,
	      const osl::Piece &bishop) const;
  public:
    // unpromoted and promoted
    BishopMobility() : EvalComponentMulti(18 * 2) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
		  CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights& weights) const;
    size_t maxActiveOne() const { return 2; }
    const std::string name() const { return "BishopMobility"; }
  };

  class BishopMobilityEach : public FeaturesOneNonUniq
  {
  public:
    BishopMobilityEach() : FeaturesOneNonUniq(9 * 2) { }
    size_t maxActive() const { return 4; }
    const std::string name() const { return "BishopMobilityEach"; }
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &features) const;
  };

  class LanceMobility : public EvalComponentMulti
  {
  public:
    LanceMobility() : EvalComponentMulti(9) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
		  CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights& weights) const;
    size_t maxActiveOne() const { return 4; }
    const std::string name() const { return "LanceMobility"; }
  };
}

#endif /* _GPSSSHOGI_SAMPLE_EVAL_LIB_MOBILITYy_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
