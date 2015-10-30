/* ptypeEval.h
 */
#ifndef _PTYPE_EVAL_H
#define _PTYPE_EVAL_H

#include "eval/eval.h"
#include "eval/indexCache.h"

namespace gpsshogi
{
  using namespace osl;

  class PtypeCountFeatures : public FeaturesOneNonUniq
  {
  public:
    PtypeCountFeatures() : FeaturesOneNonUniq(160) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "PtypeCount"; }
    void showAllOne(const Weights&,
		    int n,
		    std::ostream &os) const;

    static int index(Ptype ptype, int count);
  };

  class PtypeCount : public EvalComponentStages 
  {
  public:
    PtypeCount() : EvalComponentStages(new PtypeCountFeatures()) {}
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& save_state) const;
  };

  template <bool Attack>
  class PtypeCountXYBase : public FeaturesOneNonUniq
  {
  public:
    // x, x board, y, y board
    PtypeCountXYBase() : FeaturesOneNonUniq(160 * (9 + 5)) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values, const MultiWeights&) const;
    void showAllOne(const Weights&,
		    int n,
		    std::ostream &os) const;
    const std::string name() const 
    { 
      return Attack ? "PtypeCountXYAttack" : "PtypeCountXY"; 
    }
    static int indexX(int x, Ptype ptype, int count);
    static int indexY(int y, Ptype ptype, int count);
    static int indexXBoard(int x, Ptype ptype, int count);
    static int indexYBoard(int y, Ptype ptype, int count);
  };

  class PtypeCountXY : public EvalComponentStages 
  {
  public:
    PtypeCountXY() : EvalComponentStages(new PtypeCountXYBase<false>()) {}
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& save_state) const
    {
      if (moved.isPass())
	return last_values;
      if (moved.ptype() == KING) 
	return evalMulti(state, save_state);
      return static_cast<PtypeCountXYBase<false>&>(*delegate).evalWithUpdateMulti
	(state, moved, last_values, weight);
    }
  };

  class PtypeCountXYAttack : public EvalComponentStages
  {
  public:
    PtypeCountXYAttack() : EvalComponentStages(new PtypeCountXYBase<true>()) {}
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& save_state) const
    {
      if (moved.isPass())
	return last_values;
      if (moved.ptype() == KING) 
	return evalMulti(state, save_state);
      return static_cast<PtypeCountXYBase<true>&>(*delegate).evalWithUpdateMulti
	(state, moved, last_values, weight);
    }
  };

  class PtypeYPawnYFeature : public FeaturesOneNonUniq
  {
  public:
    PtypeYPawnYFeature() : FeaturesOneNonUniq(9 * PTYPE_SIZE * 10) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "PtypeYPawnY"; }
    static int index(Player player, Ptype ptype, int y, int pawn_y)
    {
      if (player == WHITE)
      {
	y = 10 - y;
	pawn_y = (10 - pawn_y) % 10;
      }
      return pawn_y + 10 * (y - 1 + 9 * ptype);
    }
  };
  class PtypeYPawnY : public EvalComponentStages 
  {
  public:
    PtypeYPawnY() : EvalComponentStages(new PtypeYPawnYFeature()) {}
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& save_state) const;
  };

  class PtypeCombination : public FeaturesOneNonUniq
  {
  public:
    PtypeCombination() : FeaturesOneNonUniq(1 << 13) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "PtypeCombination"; }
    size_t maxActive() const { return 2; }
  private:
    template <Ptype PTYPE, int INDEX>
    void updatePtype(const NumEffectState &state, CArray<int, 2> &ptypes) const
    {
      for (int i = PtypeTraits<PTYPE>::indexMin;
	   i < PtypeTraits<PTYPE>::indexLimit; ++i)
      {
	const Piece p = state.pieceOf(i);
	const int index = INDEX - (p.isPromoted() ? 6 : 0);
	ptypes[p.owner()] |= (1 << index);
      }
    }
    const std::string describe(size_t local_index) const;
  };

  class PtypeCombinationY : public FeaturesOneNonUniq
  {
  public:
    PtypeCombinationY() : FeaturesOneNonUniq((1 << 13) * 9) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "PtypeCombinationY"; }
    size_t maxActive() const { return 2; }
  private:
    template <Ptype PTYPE, int INDEX>
    void updatePtype(const NumEffectState &state, CArray<int, 2> &ptypes) const
    {
      for (int i = PtypeTraits<PTYPE>::indexMin;
	   i < PtypeTraits<PTYPE>::indexLimit; ++i)
      {
	const Piece p = state.pieceOf(i);
	const int index = INDEX - (p.isPromoted() ? 6 : 0);
	ptypes[p.owner()] |= (1 << index);
      }
    }
  };

  class PtypeCombinationOnBoard : public FeaturesOneNonUniq
  {
  public:
    PtypeCombinationOnBoard() : FeaturesOneNonUniq(1 << 13) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "PtypeCombinationOnBoard"; }
    size_t maxActive() const { return 2; }
  private:
    template <Ptype PTYPE, int INDEX>
    void updatePtype(const NumEffectState &state, CArray<int, 2> &ptypes) const
    {
      for (int i = PtypeTraits<PTYPE>::indexMin;
	   i < PtypeTraits<PTYPE>::indexLimit; ++i)
      {
	const Piece p = state.pieceOf(i);
	if (!p.isOnBoard())
	  continue;
	const int index = INDEX - (p.isPromoted() ? 6 : 0);
	ptypes[p.owner()] |= (1 << index);
      }
    }
  };

  class PtypeCombinationOnBoardY : public FeaturesOneNonUniq
  {
  public:
    PtypeCombinationOnBoardY() : FeaturesOneNonUniq((1 << 13) * 9) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "PtypeCombinationOnBoardY"; }
    size_t maxActive() const { return 2; }
  private:
    template <Ptype PTYPE, int INDEX>
    void updatePtype(const NumEffectState &state, CArray<int, 2> &ptypes) const
    {
      for (int i = PtypeTraits<PTYPE>::indexMin;
	   i < PtypeTraits<PTYPE>::indexLimit; ++i)
      {
	const Piece p = state.pieceOf(i);
	if (!p.isOnBoard())
	  continue;
	const int index = INDEX - (p.isPromoted() ? 6 : 0);
	ptypes[p.owner()] |= (1 << index);
      }
    }
  };
}

#endif // _PTYPE_EVAL_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
