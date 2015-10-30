#ifndef GPSSHOGI_SAMPLE_EVAL_LIB_PIECESTAND_H
#define GPSSHOGI_SAMPLE_EVAL_LIB_PIECESTAND_H

#include "eval/pieceFeature.h"

namespace gpsshogi
{
  class PieceStand : public EvalComponentMulti
  {
  public:
    PieceStand() : EvalComponentMulti(osl::Piece::SIZE) {
    }
    MultiInt eval(const NumEffectState& state, const MultiWeights& w,
      CArray<MultiInt,2>& saved_state) const;
    MultiInt evalWithUpdate(const NumEffectState& state, Move moved, MultiInt last_value, const MultiWeights& w,
      CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights&) const;
    const std::string name() const { return "PieceStand"; };
  };

  class NonPawnPieceStand : public EvalComponentMulti
  {
  public:
    NonPawnPieceStand() : EvalComponentMulti(21) { }  // 0 to 20 (40 - 18 - 2).
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
      CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights& weights) const;
    const std::string name() const { return "NonPawnPieceStand"; };
  };

  class NonPawnPieceStandCombination : public FeaturesOneNonUniq
  {
  public:
    // 3 * 3 * 5 * 5 * 5 * 5
    // rook bishop gold silver knight lance
    NonPawnPieceStandCombination() : FeaturesOneNonUniq(5625) { }
    const std::string name() const { return "NonPawnPieceStandCombination"; }
    size_t maxActive() const { return 4; };

    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication>&) const;
    int index(int rook, int bishop, int gold, int silver,
	       int knight, int lance) const
    {
      return lance +
	5 * (knight + 5 * (silver + 5 * (gold + 5 * (3 * bishop + rook))));
    }
    void showAllOne(const Weights&,
		    int n,
		    std::ostream &os) const;
  };

  class NonPawnPieceStandCombinationEach : public FeaturesOneNonUniq
  {
  public:
    // 3 * 3 * 5 * 5 * 5 * 5
    // rook bishop gold silver knight lance
    NonPawnPieceStandCombinationEach() : FeaturesOneNonUniq(5625) { }
    const std::string name() const
    {
      return "NonPawnPieceStandCombinationEach";
    }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication>&) const;
    int index(int rook, int bishop, int gold, int silver,
	       int knight, int lance) const
    {
      return lance +
	5 * (knight + 5 * (silver + 5 * (gold + 5 * (3 * bishop + rook))));
    }
    void showAllOne(const Weights &weights,
		    int n, std::ostream&) const;
  private:
    template <Player P>
    void featuresPlayer(const NumEffectState &,
			IndexCacheI<MaxActiveWithDuplication>&) const;
  };

  class CanCheckNonPawnPieceStandCombinationEach : public FeaturesOneNonUniq
  {
  public:
    // 3 * 3 * 5 * 5 * 5 * 5
    // rook bishop gold silver knight lance
    CanCheckNonPawnPieceStandCombinationEach() : FeaturesOneNonUniq(5625) { }
    const std::string name() const
    {
      return "CanCheckNonPawnPieceStandCombinationEach";
    }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication>&) const;
    int index(int rook, int bishop, int gold, int silver,
	       int knight, int lance) const
    {
      return lance +
	5 * (knight + 5 * (silver + 5 * (gold + 5 * (3 * bishop + rook))));
    }
  private:
    template <Player P>
    void featuresPlayer(const NumEffectState &,
			IndexCacheI<MaxActiveWithDuplication>&) const;
  };

  class CanCheckNonPawnPieceStandCombination
    : public NonPawnPieceStandCombination
  {
  public:
    const std::string name() const {
      return "CanCheckNonPawnPieceStandCombination";
    }
  protected:
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication>&) const;
  };

  class NonPawnPieceStandTurn : public FeaturesOneNonUniq
  {
  public:
    NonPawnPieceStandTurn() : FeaturesOneNonUniq(44) { }
    const std::string name() const { return "NonPawnPieceStandTurn"; }
    void showSummary(const Weights&, std::ostream &os) const;

    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    int index(Player player, Player turn, Ptype ptype, int count) const
    {
      return Ptype_Table.getIndexMin(ptype) - 18 + count +
	(turn == player ? 22 : 0);
    }
  };

  class PieceStandY : public FeaturesOneNonUniq
  {
  public:
    PieceStandY() : FeaturesOneNonUniq(osl::Piece::SIZE * 9 * 2) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication>&) const;
    const std::string name() const { return "PieceStandY"; };
    size_t maxActive() const { return Piece::SIZE*2; };
    static int index(Ptype ptype, Player player, Square king, int count,
		     bool attack) 
    {
      const int king_y = (player == BLACK ? king.y() : 10 - king.y());
      return (king_y - 1) * 40 + Ptype_Table.getIndexMin(ptype) + count +
	(attack ? 0 : osl::Piece::SIZE * 9);
    }
    void showAllOne(const Weights &weights,
		    int n, std::ostream&) const;
  };

  class PieceStandCombinationBoth : public FeaturesOneNonUniq
  {
  public:
    PieceStandCombinationBoth() : FeaturesOneNonUniq(1 << 14) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication>&) const;
    const std::string name() const { return "PieceStandCombinationBoth"; };
    size_t maxActive() const { return 1; };
    const std::string describe(size_t local_index) const;
  };

  class PieceStandOnBoard : public FeaturesOneNonUniq
  {
  public:
    // 1080
    PieceStandOnBoard() : FeaturesOneNonUniq(7 * 5 * 9 * PTYPEO_SIZE) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication>&) const;
    const std::string name() const { return "PieceStandOnBoard"; };
    size_t maxActive() const { return 14 * 40; };
  private:
    template <Player Owner>
    int index(int order, Piece on_board_piece) const
    {
      int x = on_board_piece.square().x();
      int y = on_board_piece.square().y();
      PtypeO ptypeO = on_board_piece.ptypeO();
      if (x > 5)
      {
	x = 10 - x;
      }
      if (Owner == WHITE)
      {
	y = 10 - y;
	ptypeO = NEW_PTYPEO(alt(getOwner(ptypeO)), getPtype(ptypeO));
      }
      return x - 1 + 5 * (y - 1 + 9 * (order + 7 * ptypeOIndex(ptypeO)));
    }
  };
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
#endif // GPSSHOGI_SAMPLE_EVAL_LIB_PIECESTAND_H
