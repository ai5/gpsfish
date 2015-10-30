/* majorPiece.h
 */
#ifndef _GPSSSHOGI_SAMPLE_EVAL_LIB_MAJORPIECE_H
#define _GPSSSHOGI_SAMPLE_EVAL_LIB_MAJORPIECE_H

#include "eval/eval.h"
#include "eval/indexCache.h"

namespace gpsshogi
{
  using namespace osl;
  template <Ptype MajorPiece>
  class MajorPieceYBase : public EvalComponent
  {
  protected:
    virtual bool isTarget(Piece piece) const = 0;
  public:
    MajorPieceYBase() : EvalComponent(9) { }
    int eval(const osl::NumEffectState &state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showAll(std::ostream &os) const;
    const std::string name() const { return "MajorPieceYBase"; };
  };

  class RookY : public MajorPieceYBase<ROOK>
  {
  protected:
    bool isTarget(Piece rook) const { return !rook.isPromoted(); }
  public:
    virtual const std::string name() const { return "RookY"; };
  };

  class PRookY : public MajorPieceYBase<ROOK>
  {
  protected:
    bool isTarget(Piece rook) const { return rook.isPromoted(); }
  public:
    virtual const std::string name() const { return "PRookY"; };
  };

  class BishopY : public MajorPieceYBase<BISHOP>
  {
  protected:
    bool isTarget(Piece bishop) const { return !bishop.isPromoted(); }
  public:
    virtual const std::string name() const { return "BishopY"; };
  };

  class PBishopY : public MajorPieceYBase<BISHOP>
  {
  protected:
    bool isTarget(Piece bishop) const { return bishop.isPromoted(); }
  public:
    virtual const std::string name() const { return "PBishopY"; };
  };

  class RookPawn : public EvalComponent
  {
  public:
    // different value for rook and prook?
    // how about pawn under rook?
    RookPawn() : EvalComponent(1) { }
    int eval(const osl::NumEffectState &state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os) const;
    const std::string name() const { return "RookPawn"; };
  };

  class RookPawnY : public FeaturesOneNonUniq
  {
  public:
    // rook y, pawn y, promoted
    RookPawnY() : FeaturesOneNonUniq(9 * 10 * 2) { };
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &features) const;
    const std::string name() const { return "RookPawnY"; }
    size_t maxActive() const { return 2; };
  private:
    int index(const NumEffectState &state, const Piece rook) const;
  };

  class RookPawnYX : public FeaturesOneNonUniq
  {
  public:
    // rook y, pawn y, abs_x_diff_with_king, promoted, self/opp
    RookPawnYX() : FeaturesOneNonUniq(9 * 10 * 9 * 2 * 2) { };
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &features) const;
    const std::string name() const { return "RookPawnYX"; }
    size_t maxActive() const { return 2*2; };
  private:
    int index(const NumEffectState &state, const Piece rook,
	      bool attack) const;
  };

  class AllMajor : public EvalComponentMulti
  {
  public:
    AllMajor() : EvalComponentMulti(1) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights&,
		  CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights&) const;
    const std::string name() const { return "AllMajor"; };
  };

  class RookEffect : public EvalComponentMulti
  {
  private:
    template <Player RookOwner>
    static MultiInt evalOne(const NumEffectState &state, const Square king,
		     const Piece rook, const Offset dir, const MultiWeights&);
    static int index(Player p, Square pos, Square king, bool horizontal,
		     bool promoted);
    void featureOne(const NumEffectState &state,
		    const Square king,
		    const Piece rook,
		    const Offset dir,
		    CArray<int, 612>& feature) const;
    bool is_attack;
  public:
    RookEffect(bool attack=true)
      : EvalComponentMulti(612), is_attack(attack) { } // 2 * 2 * 9 * 17
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& w,
		  CArray<MultiInt,2>& saved_state) const;
    MultiInt evalWithUpdate(const NumEffectState &state, Move,
			    MultiInt last_value, const MultiWeights& w,
			    CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showAll(std::ostream &os, const MultiWeights&) const;
    const std::string name() const {
      return is_attack ? "RookEffect" : "RookEffectDefense"; };
  private:
    MultiInt evalOnePiece(int piece_number, 
			  const osl::NumEffectState &state, const MultiWeights& w) const;
  };

  class BishopEffect : public EvalComponentMulti
  {
  private:
    MultiInt evalOne(const NumEffectState &state, const Square king,
		     const Piece bishop, const Direction dir, const MultiWeights&) const;
    int index(Player p, Square pos, Square king, bool ur,
	      bool promoted) const;
    void featureOne(const NumEffectState &state,
		    const Square king,
		    const Piece bishop,
		    const Direction dir,
		    CArray<int, 612>& feature) const;
    bool is_attack;
  public:
    explicit BishopEffect(bool attack=true)
      : EvalComponentMulti(612), is_attack(attack) { } // 2 * 2 * 9 * 17
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& w,
		  CArray<MultiInt,2>& saved_state) const;
    MultiInt evalWithUpdate(const NumEffectState &state, Move,
			    MultiInt last_value, const MultiWeights& w,
			    CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showAll(std::ostream &os, const MultiWeights& w) const;
    const std::string name() const {
      return is_attack ? "BishopEffect" : "BishopEffectDefense"; };
  private:
    MultiInt evalOnePiece(int piece_number, const osl::NumEffectState &state, 
			  const MultiWeights& w) const;
  };

  
  template <osl::Ptype MajorPiece>
  class MajorEffectPieceFeatures : public FeaturesOneNonUniq
  {
  public:
    static PtypeO getPtypeO(const NumEffectState &state,
			    const Square pos, const Player player,
			    const Direction dir);
    static void addPtypeO(const Piece rook,
			  const PtypeO ptypeO,
			  IndexCacheI<MaxActiveWithDuplication> &features);
  public:
    MajorEffectPieceFeatures() : FeaturesOneNonUniq(PTYPEO_SIZE) { } 
    void showAllOne(const Weights&, int n, std::ostream &os) const;
    const std::string name() const
    { return (MajorPiece == ROOK ? "RookEffectPiece" : "BishopEffectPiece"); }
    size_t maxActive() const { return 16; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  public:
    static void featuresOneNonUniq(int piece_number, const NumEffectState &state,
				   IndexCacheI<MaxActiveWithDuplication> &);
    const std::string describe(size_t local_index) const;
  };
  template <osl::Ptype MajorPiece>
  class MajorEffectPieceStages : public EvalComponentStages
  {
  public:
    MajorEffectPieceStages() : EvalComponentStages(new MajorEffectPieceFeatures<MajorPiece>()) { } 
    MultiInt evalMulti(const NumEffectState &state, CArray<MultiInt,2>& save_state) const;
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& save_state) const;
  private:
    MultiInt evalOne(const NumEffectState &state, int piece_number) const;
  };
  class RookEffectPieceStages : public MajorEffectPieceStages<ROOK>
  {
  };
  class BishopEffectPieceStages : public MajorEffectPieceStages<BISHOP>
  {
  };

  template <osl::Ptype MajorPiece>
  class MajorEffectPieceKingRelative : public FeaturesOneNonUniq
  {
  private:
    static int index(Player p, Square pos, Square king,
		     PtypeO ptypeO, bool attack,
		     bool horizontal, bool promoted);
    static void addOne(const NumEffectState &state,
		       const Square pos, const Player player,
		       const Direction dir, const bool promoted,
		       IndexCacheI<MaxActiveWithDuplication> &features);
  public:
    MajorEffectPieceKingRelative()
      // attack, vertical, promoted, x_diff, y_diff
      : FeaturesOneNonUniq(PTYPEO_SIZE * 2 * 2 * 2 * 9 * 17) { } 
    const std::string name() const
    {
      return MajorPiece == ROOK ? "RookEffectPieceKingRelative" :
	"BishopEffectPieceKingRelative";
    }
    size_t maxActive() const { return 32; }
    void showAllOne(const Weights&,
		    int n,
		    std::ostream &os) const;
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  public:
    static void featuresOneNonUniq(int piece_number, const NumEffectState &state,
				   IndexCacheI<MaxActiveWithDuplication> &);
  };

  class RookEffectPieceKingRelative : public EvalComponentStages 
  {
  public:
    RookEffectPieceKingRelative() : EvalComponentStages(new MajorEffectPieceKingRelative<ROOK>)
    {
    }
    MultiInt evalMulti(const NumEffectState &state, CArray<MultiInt,2>& save_state) const;
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& save_state) const;
  };
  class BishopEffectPieceKingRelative : public EvalComponentStages 
  {
  public:
    BishopEffectPieceKingRelative() : EvalComponentStages(new MajorEffectPieceKingRelative<BISHOP>)
    {
    }
    MultiInt evalMulti(const NumEffectState &state, CArray<MultiInt,2>& save_state) const;
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& save_state) const;
  };

  class BishopHead : public FeaturesOneNonUniq
  {
  public:
    // ptypeo.  Only when there's no support
    BishopHead() : FeaturesOneNonUniq(32) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "BishopHead"; }
  private:
    int index(Player player, PtypeO ptypeO) const
    {
      if (player == WHITE && isPiece(ptypeO))
      {
	ptypeO = newPtypeO(alt(getOwner(ptypeO)), getPtype(ptypeO));
      }
      return ptypeOIndex(ptypeO);
    }
  };

  class BishopHeadX : public FeaturesOneNonUniq
  {
  public:
    BishopHeadX() : FeaturesOneNonUniq(32 * 5) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "BishopHeadX"; }
  private:
    int index(Player player, PtypeO ptypeO, int bishop_x) const
    {
      if (bishop_x > 5)
      {
	bishop_x = 10 - bishop_x;
      }
      if (player == WHITE && isPiece(ptypeO))
      {
	ptypeO = newPtypeO(alt(getOwner(ptypeO)), getPtype(ptypeO));
      }
      return bishop_x - 1 + 5 * ptypeOIndex(ptypeO);
    }
  };

  class BishopHeadKingRelative : public FeaturesOneNonUniq
  {
  public:
    // ptypeo * 9 * 17. only self king
    BishopHeadKingRelative() : FeaturesOneNonUniq(32 * 9 * 17) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "BishopHeadKingRelative"; }
  private:
    int index(Player player, PtypeO ptypeO, int x_diff, int y_diff) const
    {
      if (player == WHITE && isPiece(ptypeO))
      {
	ptypeO = newPtypeO(alt(getOwner(ptypeO)), getPtype(ptypeO));
      }
      if (player == WHITE)
      {
	y_diff = -y_diff;
      }
      return (ptypeOIndex(ptypeO) * 9 + x_diff) * 17 + y_diff + 8;
    }
  };


  class RookPromoteDefense : public FeaturesOneNonUniq
  {
  public:
    // Attacked Ptype, Effect Ptype
    RookPromoteDefense() : FeaturesOneNonUniq(PTYPE_SIZE * PTYPE_SIZE) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "RookPromoteDefense"; }
  };

  class RookPromoteDefenseRookH : public FeaturesOneNonUniq
  {
  public:
    // Attacked ptype, rook horizontal mobility
    RookPromoteDefenseRookH() : FeaturesOneNonUniq(PTYPE_SIZE * 9) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "RookPromoteDefenseRookH"; }
  };

  class KingRookBishop : public FeaturesOneNonUniq
  {
  public:
    // rook rel x, y, bishop rel x, y, rook same player, bishop same player
    // rook promoted, bishop promoted (374544)
    KingRookBishop() : FeaturesOneNonUniq(9 * 17 * 9 * 17 * 2 * 2 * 2 * 2) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "KingRookBishop"; }
    size_t maxActive() const { return 24; }
  private:
    template <Player King>
    int index(const Square king, const Piece rook, const Piece bishop) const
    {
      const int rook_x = std::abs(king.x() - rook.square().x());
      const int bishop_x = std::abs(king.x() - bishop.square().x());
      const int rook_y = (King == BLACK ? rook.square().y() - king.y() : king.y() - rook.square().y());
      const int bishop_y = (King == BLACK ? bishop.square().y() - king.y() : king.y() - bishop.square().y());
      return bishop_y + 8 + 17 * (bishop_x + 9 * (rook_y + 8 + 17 * (rook_x + 9 * ((bishop.owner() == King ? 1 : 0) + 2 * ((rook.owner() == King ? 1 : 0) + 2 * (2 * (bishop.isPromoted() ? 1 : 0) + (rook.isPromoted() ? 1 : 0)))))));
    }
  };

  // N: 0 =>self
  //    1 =>opponent
  //    2 =>all
  template <int N>
  class NumPiecesBetweenBishopAndKing : public FeaturesOneNonUniq
  {
  public:
    NumPiecesBetweenBishopAndKing() : FeaturesOneNonUniq(9) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "NumPiecesBetweenBishopAndKing"; }
    size_t maxActive() const { return 2; }
    static int countBetween(const NumEffectState &state,
			    Square king, Piece bishop);
    void showAllOne(const Weights&,
		    int n,
		    std::ostream &os) const;
  };
  class NumPiecesBetweenBishopAndKingSelf
    : public NumPiecesBetweenBishopAndKing<0>
  {
  public:
    const std::string name() const
    {
      return "NumPiecesBetweenBishopAndKingSelf";
    }
  };
  class NumPiecesBetweenBishopAndKingOpp
    : public NumPiecesBetweenBishopAndKing<1>
  {
  public:
    const std::string name() const
    {
      return "NumPiecesBetweenBishopAndKingOpp";
    }
  };
  class NumPiecesBetweenBishopAndKingAll
    : public NumPiecesBetweenBishopAndKing<2>
  {
  public:
    const std::string name() const
    {
      return "NumPiecesBetweenBishopAndKingAll";
    }
  };

  class NumPiecesBetweenBishopAndKingCombination : public FeaturesOneNonUniq
  {
  public:
    NumPiecesBetweenBishopAndKingCombination()
      // self, opp
      : FeaturesOneNonUniq(9 * 9) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "NumPiecesBetweenBishopAndKingCombination"; }
    size_t maxActive() const { return 2; }
    static void countBetween(const NumEffectState &state,
			     Square king, Piece bishop,
			     int &self, int &opp);
  };

  class BishopBishopPiece : public FeaturesOneNonUniq
  {
  public:
    BishopBishopPiece() : FeaturesOneNonUniq(PTYPE_SIZE * 2 * 2) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "BishopBishopPiece"; }
    size_t maxActive() const { return 1; }
    int index(PtypeO ptypeO, bool black_with_support,
	      bool white_with_support) const
    {
      if (getOwner(ptypeO) == BLACK)
	return getPtype(ptypeO) +
	  PTYPE_SIZE * ((black_with_support ? 1 : 0) +
			2 * (white_with_support ? 1 : 0));
      else
	return getPtype(ptypeO) +
	  PTYPE_SIZE * ((white_with_support ? 1 : 0) +
			2 * (black_with_support ? 1 : 0));
    }
  };

  class RookRook : public FeaturesOneNonUniq
  {
  public:
    // Y, Y, promoted?, promoted?, same player
    RookRook() : FeaturesOneNonUniq(10 * 10 * 2 * 2 * 2) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "RookRook"; }
    size_t maxActive() const { return 1; }
  private:
    int index(Piece rook1, Piece rook2, int y1, int y2) const
    {
      return y1 + 10 *
	(y2 + 10 * ((rook1.isPromoted() ? 1 : 0) + 2 *
		    ((rook2.isPromoted() ? 1 : 0) + 2 *
		     (rook1.owner() == rook2.owner() ? 1 : 0))));
    }
  };

  class RookRookPiece : public FeaturesOneNonUniq
  {
  public:
    // ptype in between, with support black, white, horizontal or vertical
    RookRookPiece() : FeaturesOneNonUniq(PTYPE_SIZE * 2 * 2 * 2) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "RookRookPiece"; }
    size_t maxActive() const { return 1; }
    int index(PtypeO ptypeO, bool black_with_support,
	      bool white_with_support, bool vertical) const
    {
      if (getOwner(ptypeO) == BLACK)
	return getPtype(ptypeO) +
	  PTYPE_SIZE * ((black_with_support ? 1 : 0) +
			2 * (white_with_support ? 1 : 0)) +
	  (vertical ? PTYPE_SIZE * 2 * 2 : 0);
      else
	return getPtype(ptypeO) +
	  PTYPE_SIZE * ((white_with_support ? 1 : 0) +
			2 * (black_with_support ? 1 : 0)) +
	  (vertical ? PTYPE_SIZE * 2 * 2 : 0);
    }
  };

  class BishopStandRank5 : public FeaturesOneNonUniq
  {
  public:
    BishopStandRank5() : FeaturesOneNonUniq(PTYPEO_SIZE) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "BishopStandRank5"; }
    size_t maxActive() const { return 2; }
  };

  class MajorCheckWithCapture : public FeaturesOneNonUniq
  {
    enum {
      ONE_DIM = PTYPE_SIZE * 2/*bishop or rook*/ * 2 /*promotable*/
    };
  public:
    MajorCheckWithCapture() : FeaturesOneNonUniq(ONE_DIM) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    void addOne(Player owner, const NumEffectState &state,
		IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "MajorCheckWithCapture"; }
    size_t maxActive() const { return 8; }
    static size_t index(Ptype ptype, bool is_rook, bool can_promote) 
    {
      return ptype * 4 + is_rook * 2 + can_promote;
    }
  };

  class RookSilverKnight : public FeaturesOneNonUniq
  {
    enum {
      ONE_DIM = 5 * 9 * 9 * 9 * 9 * 9
    };
  public:
    RookSilverKnight() : FeaturesOneNonUniq(ONE_DIM) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "RookSilverKnight"; }
    size_t maxActive() const { return 2 * 4 * 4; }
  private:
    static size_t index(int rook_x, int rook_y, int silver_x, int silver_y,
			int knight_x, int knight_y)
    {
      return knight_y + 9 * (knight_x + 9 * (silver_y + 9 * (silver_x + 9 * (rook_y + 9 * rook_x))));
    }
  };

  class BishopSilverKnight : public FeaturesOneNonUniq
  {
    enum {
      ONE_DIM = 5 * 9 * 9 * 9 * 9 * 9
    };
  public:
    BishopSilverKnight() : FeaturesOneNonUniq(ONE_DIM) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "BishopSilverKnight"; }
    size_t maxActive() const { return 2 * 4 * 4; }
  private:
    static size_t index(int bishop_x, int bishop_y, int silver_x, int silver_y,
			int knight_x, int knight_y)
    {
      return knight_y + 9 * (knight_x + 9 * (silver_y + 9 * (silver_x + 9 * (bishop_y + 9 * bishop_x))));
    }
  };

  class AttackMajorsInBase : public FeaturesOneNonUniq
  {
    enum {
      ONE_DIM = PTYPE_SIZE * PTYPE_SIZE * 2 * 2 * 2
    };
  public:
    AttackMajorsInBase() : FeaturesOneNonUniq(ONE_DIM) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "AttackMajorsInBase"; }
    size_t maxActive() const { return 4; }
    static int index(Ptype support, Ptype attack, bool has_gold, 
		     bool rook_support, bool bishop_support)
    {
      return (unpromoteSafe(support)*16 + unpromoteSafe(attack))*8+has_gold*4
	+rook_support*2+bishop_support;
    }
  };
}

#endif /* _GPSSSHOGI_SAMPLE_EVAL_LIB_MAJORPIECE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
