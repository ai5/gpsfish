/* ptypeAttacked.h
 */
#ifndef _PTYPEATTACKED_H
#define _PTYPEATTACKED_H

#include "eval/pieceFeature.h"

namespace gpsshogi
{
  using namespace osl;

  class MajorGoldSilverAttacked : public EvalComponent
  {
  private:
    template <Ptype PTYPE>
    int evalOne(const NumEffectState &state) const;
    template <Ptype PTYPE>
    void featureOne(const NumEffectState &state,
		    CArray<int, PTYPE_SIZE * 2> &features) const;
    int index(const NumEffectState &state, Piece piece) const;
  public:
    MajorGoldSilverAttacked() : EvalComponent(PTYPE_SIZE * 2) { }
    ~MajorGoldSilverAttacked() { }
    int eval(const NumEffectState &state) const;

    void features(const NumEffectState &state, 
    		  index_list_t &diffs, int offset) const;

    void showSummary(std::ostream &os) const;
    const std::string name() const { return "MajorGoldSilverAttacked"; };
  };

  class NonPawnAttacked : public FeaturesOneNonUniq
  {
  private:
    int index(const NumEffectState &state, Piece piece) const
    {
      const bool has_support = state.hasEffectAt(piece.owner(),
						 piece.square());
      const bool same_turn = state.turn() == piece.owner();
      return piece.ptype() + (same_turn ? 0 : PTYPE_SIZE) +
	(has_support ? 0 : PTYPE_SIZE * 2);
    }
  public:
    // turn, with self support
    NonPawnAttacked() : FeaturesOneNonUniq(PTYPE_SIZE * 2 * 2) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "NonPawnAttacked"; }
    const std::string describe(size_t local_index) const
    {
      const Ptype ptype = static_cast<Ptype>(local_index % PTYPE_SIZE);
      local_index /= PTYPE_SIZE;
      const bool same_turn = local_index % 2;
      const bool has_support = local_index / 2;
      return std::string(Ptype_Table.getCsaName(ptype)) 
	+ (has_support ? " support" : "") + (same_turn ? " turn" : "");
    }
  };

  class NonPawnAttackedKingRelatve : public FeaturesOneNonUniq
  {
  private:
    int index(const NumEffectState &state, Piece piece, bool attack) const
    {
      const Square king =
	state.kingSquare(attack ? alt(piece.owner()) : piece.owner());
      const int x_diff = std::abs(piece.square().x() - king.x());
      const int y_diff = (piece.owner() == BLACK ?
			  piece.square().y() - king.y() :
			  king.y() - piece.square().y());
      const bool has_support = state.hasEffectAt(piece.owner(),
						 piece.square());
      const bool same_turn = state.turn() == piece.owner();
      return ((piece.ptype() + (same_turn ? 0 : PTYPE_SIZE) +
	       (has_support ? 0 : PTYPE_SIZE * 2)) * 9 + x_diff) * 17 +
	y_diff + 8 + (attack ? 0 : 9792);
    }
  public:
    // turn, with self support, x, y, attack, defense
    NonPawnAttackedKingRelatve()
      : FeaturesOneNonUniq(PTYPE_SIZE * 2 * 2 * 9 * 17 * 2) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "NonPawnAttackedKingRelatve"; }
    size_t maxActive() const { return 160; }
  };

  class NonPawnAttackedPtype : public FeaturesOneNonUniq
  {
  public:
    // ptype, turn, with self support, attacking ptype
    NonPawnAttackedPtype()
      : FeaturesOneNonUniq(PTYPE_SIZE * 2 * 2 * PTYPE_SIZE) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "NonPawnAttackedPtype"; }
  private:
    int index(const NumEffectState &state, Piece piece,
	      Ptype attack_ptype) const
    {
      const bool has_support = state.hasEffectAt(piece.owner(),
						 piece.square());
      const bool same_turn = state.turn() == piece.owner();
      return (piece.ptype() + (same_turn ? 0 : PTYPE_SIZE) +
	      (has_support ? 0 : PTYPE_SIZE * 2)) * 16 + attack_ptype;
    }
  };

  class NonPawnAttackedPtypePair : public FeaturesOneNonUniq
  {
  public:
    // (ptype, with self support, attacking ptype)^2
    NonPawnAttackedPtypePair()
      : FeaturesOneNonUniq((PTYPE_SIZE * 2 * PTYPE_SIZE)*(PTYPE_SIZE * 2 * PTYPE_SIZE)) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "NonPawnAttackedPtypePair"; }
    size_t maxActive() const { return 40*40*2; }
  private:
    static int index1(const NumEffectState &state, Piece piece)
    {
      const Ptype attack_ptype
	= state.findCheapAttack(alt(piece.owner()), piece.square()).ptype();
      const bool has_support = state.hasEffectAt(piece.owner(),
						 piece.square());
      return (piece.ptype() + 
	      (has_support ? 0 : PTYPE_SIZE)) * PTYPE_SIZE + attack_ptype;
    }
    static int index2(int i0, int i1) 
    {
      if (i0 > i1) 
	std::swap(i0, i1);
      return i0 * PTYPE_SIZE * 2 * PTYPE_SIZE + i1;
    }
  };
}
#endif /* _PTYPEATTACKED_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:


