/* minorPiece.h
 */
#ifndef GPSSSHOGI_SAMPLE_EVAL_LIB_MINORPIECE_H
#define GPSSSHOGI_SAMPLE_EVAL_LIB_MINORPIECE_H

#include "eval/eval.h"

namespace gpsshogi
{
  class SilverRetreat : public EvalComponentMulti
  {
  private:
    bool canRetreat(const osl::NumEffectState &state,
		    const osl::Piece silver) const;
    int index(Player P, Square pos) const
    {
      return (P == BLACK ? (pos.y() - 1) : (9 - pos.y()));
    }
    enum { DIM = 9 };
  public:
    SilverRetreat() : EvalComponentMulti(DIM) { }
    osl::MultiInt eval(const osl::NumEffectState &state, const MultiWeights&,
		       CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights&) const;
    const std::string name() const { return "SilverRetreat"; };
  };

  class GoldRetreat : public EvalComponentMulti
  {
  private:
    bool canRetreat(const osl::NumEffectState &state,
		    const osl::Piece gold) const;
    int index(Player P, Square pos) const
    {
      return (P == BLACK ? (pos.y() - 1) : (9 - pos.y()));
    }
    enum { DIM = 9 };
  public:
    GoldRetreat() : EvalComponentMulti(DIM) { }
    osl::MultiInt eval(const osl::NumEffectState &state, const MultiWeights&,
		       CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights&) const;
    const std::string name() const { return "GoldRetreat"; };
  };

  class GoldSideMove : public FeaturesOneNonUniq
  {
  private:
    bool canMove(const osl::NumEffectState &state,
		 const osl::Piece gold) const;
    int indexX(Square pos) const
    {
      return (pos.x() > 5 ? 9 - pos.x() : pos.x() - 1);
    }
    int indexY(Player P, Square pos) const
    {
      return (P == BLACK ? (pos.y() - 1) : (9 - pos.y())) + 5;
    }
    enum { DIM = 14 };
  public:
    GoldSideMove() : FeaturesOneNonUniq(DIM) { }
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    const std::string name() const { return "GoldSideMove"; };
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  };

  class SilverFork : public FeaturesOneNonUniq
  {
    static std::pair<int,int> matchRook(const NumEffectState& state, Piece rook,
			 const CArray<bool,2>& has_silver);
    static std::pair<int,int> matchGold(const NumEffectState& state, Piece gold, 
			 const CArray<bool,2>& has_silver);
  public:
    enum { OWNER_DIM = 5, DIM = OWNER_DIM*2 };
    SilverFork() : FeaturesOneNonUniq(DIM) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "SilverFork"; };
    const std::string describe(size_t local_index) const;
  };
  class BishopRookFork : public FeaturesOneNonUniq
  {
  public:
    enum { DROP_DIM = PTYPE_SIZE*PTYPE_SIZE, OWNER_DIM = DROP_DIM*2, DIM = OWNER_DIM*2 };
    BishopRookFork() : FeaturesOneNonUniq(DIM) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "BishopRookFork"; };
    static bool isBishopForkSquare(const NumEffectState& state, Player defense, const Square a, const Square b);
    static bool isRookForkSquare(const NumEffectState& state, Player defense, const Square a, const Square b);
    static int bishopIndex(Ptype a, Ptype b)
    {
      if (a > b)
	std::swap(a,b);
      return a * PTYPE_SIZE + b;
    }
    static int rookIndex(Ptype a, Ptype b)
    {
      return bishopIndex(a,b) + DROP_DIM;
    }
    const std::string describe(size_t local_index) const;
  private:
    static bool findDropInLine(const NumEffectState& state, Player defense, 
			       const Square a, const Square b, Piece king);
    static bool testCenter(const NumEffectState& state, Player defense, 
			   const Square a, const Square b, Piece king,
			   Square center);
  };

  class KnightFork : public FeaturesOneNonUniq
  {
  public:
    enum { DROP_DIM = PTYPE_SIZE*PTYPE_SIZE, OWNER_DIM = DROP_DIM*2, DIM = OWNER_DIM*2 };
    KnightFork() : FeaturesOneNonUniq(DIM) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "KnightFork"; };
    static bool isForkSquare(const NumEffectState& state, Player defense, 
			       int y, int x0, int x1);
    static int index(Ptype a, Ptype b)
    {
      if (a > b)
	std::swap(a,b);
      return a * PTYPE_SIZE + b;
    }
  };

  class PawnAdvance : public EvalComponentMulti
  {
    bool cantAdvance(const NumEffectState &state, const Piece pawn) const
    {
      return cantAdvance(state, pawn.ptypeO(), pawn.square());
    }
    bool cantAdvance(const NumEffectState &state,
		     const PtypeO ptypeO, const Square position) const;
    int index(Player P, Square pos) const
    {
      return (P == BLACK ? (pos.y() - 1) : (9 - pos.y()));
    }
    enum { DIM = 9 };
  public:
    PawnAdvance() : EvalComponentMulti(DIM) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
		  CArray<MultiInt,2>& saved_state) const;
    MultiInt evalWithUpdate(const osl::NumEffectState &state,
		       osl::Move moved, MultiInt last_value, const MultiWeights& weights,
			    CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights& weights) const;
    const std::string name() const { return "PawnAdvance"; };
  };

  class KnightAdvance : public EvalComponentMulti
  {
    bool cantAdvance(const NumEffectState &state, const Piece pawn) const
    {
      return cantAdvance(state, pawn.ptypeO(), pawn.square());
    }
    bool cantAdvance(const NumEffectState &state,
		     const PtypeO ptypeO, const Square position) const;
    enum { DIM = 9 };
  public:
    KnightAdvance() : EvalComponentMulti(DIM) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights&,
		  CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights&) const;
    const std::string name() const { return "KnightAdvance"; };
  };

  class KnightCheck : public EvalComponentMulti
  {
  public:
    KnightCheck() : EvalComponentMulti(1) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
		  CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights& weights) const;
    const std::string name() const { return "KnightCheck"; };
    static bool canCheck(const NumEffectState &state, const Piece king);
  };

  class KnightCheckY : public FeaturesOneNonUniq
  {
  public:
    KnightCheckY() : FeaturesOneNonUniq(9) { }
    const std::string name() const { return "KnightCheckY"; }
    void showSummary(const Weights&, std::ostream &os) const;
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  };

  class PawnDrop : public EvalComponentMulti
  {
  private:
    bool is_defense;
  public:
    PawnDrop(bool defense=true) : EvalComponentMulti(9), is_defense(defense) { }
    MultiInt eval(const NumEffectState& state, const MultiWeights& w,
		  CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights&) const;
    const std::string name() const {
      return std::string("PawnDrop") + (is_defense ? "Defense" : "Attack"); };
  };

  class PawnDropX : public FeaturesOneNonUniq
  {
  public:
    // king x, x, attack defense
    PawnDropX() : FeaturesOneNonUniq(5 * 9 * 2) { }
    const std::string name() const { return "PawnDropX"; };
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  private:
    int index(const Piece king, int x, bool attack) const
    {
      const int king_x = king.square().x();
      const int target_x = (king_x > 5 ? 10 - king_x : king_x);
      if (king_x >= 6 || (king.owner() == WHITE && king_x == 5))
	x = 10 - x;
      return (x - 1) * 5 + target_x - 1 + (attack ? 0 : 45);
    }
  };

  class PawnDropY : public EvalComponentMulti
  {
  private:
    bool is_defense;
    int index(const Piece king, int x) const
    {
      const int king_y = (king.owner() == BLACK ?
			  king.square().y() : 10 - king.square().y());
      return std::abs(x - king.square().x()) * 9 + king_y - 1;
    }
  public:
    PawnDropY(bool defense=true) : EvalComponentMulti(81), is_defense(defense) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
		  CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showAll(std::ostream &os, const MultiWeights& weights) const;
    const std::string name() const {
      return std::string("PawnDropY") + (is_defense ? "Defense" : "Attack"); };
  };

  class PawnStateKingRelative : public FeaturesOneNonUniq
  {
    enum { BOTH_ON_BOARD, SELF_ON_BOARD, OPP_ON_BOARD, BOTH_ON_STAND };
  public:
    PawnStateKingRelative() : FeaturesOneNonUniq(9 * 4) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "PawnStateKingRelative"; }
  };

  class PawnDropNonDrop : public FeaturesOneNonUniq
  {
  public:
    PawnDropNonDrop() : FeaturesOneNonUniq(5 * 2) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "PawnDropNonDrop"; }
  };

  class PawnDropPawnStand : public FeaturesOneNonUniq
  {
  public:
    PawnDropPawnStand() : FeaturesOneNonUniq(18) { }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "PawnDropPawnStand"; };
  private:
    template <bool Attack>
    int index(const Square king, int x) const
    {
      return std::abs(king.x() - x) + (Attack ? 0 : 9);
    }
  };

  class PawnDropPawnStandX : public FeaturesOneNonUniq
  {
  public:
    // king x, x, attack defense
    PawnDropPawnStandX() : FeaturesOneNonUniq(5 * 9 * 2) { }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "PawnDropPawnStandX"; };
  private:
    template <bool Attack>
    int index(const Piece king, int x) const
    {
      const int king_x = king.square().x();
      const int target_x = (king_x > 5 ? 10 - king_x : king_x);
      if (king_x >= 6 || (king.owner() == WHITE && king_x == 5))
	x = 10 - x;
      return (x - 1) * 5 + target_x - 1 + (Attack ? 0 : 45);
    }
  };

  class PawnDropPawnStandY : public FeaturesOneNonUniq
  {
  public:
    // king y, x_diff, attack defense
    PawnDropPawnStandY() : FeaturesOneNonUniq(9 * 9 * 2) { }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "PawnDropPawnStandY"; };
  private:
    template <bool Attack>
    int index(const Piece king, int x) const
    {
      const int king_y = (king.owner() == BLACK ?
			  king.square().y() : 10 - king.square().y());
      return std::abs(x - king.square().x()) * 9 + king_y - 1 +
	(Attack ? 0 : 81);
    }
  };

  class NoPawnOnStand :  public EvalComponentMulti
  {
  private:
    enum State { OTHER = 0, BLACK = 1, WHITE = -1 };
    State getPawnState(const NumEffectState &state) const;
  public:
    NoPawnOnStand() : EvalComponentMulti(1) { }
    MultiInt eval(const NumEffectState& state, const MultiWeights& w,
		  CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights&) const;
    const std::string name() const { return "NoPawnOnStand"; };
  };

  class PawnAndEmptyAndPieceBase : public EvalComponent
  {
  public:
    PawnAndEmptyAndPieceBase(int dim) : EvalComponent(dim) { }
    int eval(const osl::NumEffectState &state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    const std::string name() const { return "PawnAndEmptyAndPieceBase"; }
    size_t maxActive() const { return 20; }
  protected:
    static int ptypeOIndex(const NumEffectState &state,
			   const Piece pawn,
			   const Piece target);
    virtual int index(const NumEffectState &state,
		      const Piece pawn,
		      const Piece target) const = 0;
  };

  class PawnAndEmptyAndPiece : public PawnAndEmptyAndPieceBase
  {
  public:
    PawnAndEmptyAndPiece() : PawnAndEmptyAndPieceBase(PTYPEO_SIZE) { }
  protected:
    int index(const NumEffectState &state,
	      const Piece pawn,
	      const Piece target) const;
    void showSummary(std::ostream &os) const;
    const std::string name() const { return "PawnAndEmptyAndPiece"; }
  };

  class PawnAndEmptyAndPieceY : public PawnAndEmptyAndPieceBase
  {
  public:
    PawnAndEmptyAndPieceY() : PawnAndEmptyAndPieceBase(PTYPEO_SIZE * 9) { }
  protected:
    int index(const NumEffectState &state,
	      const Piece pawn,
	      const Piece target) const;
    const std::string name() const { return "PawnAndEmptyAndPieceY"; }
  };

  class PawnAndEmptyAndPieceX : public PawnAndEmptyAndPieceBase
  {
  public:
    PawnAndEmptyAndPieceX() : PawnAndEmptyAndPieceBase(PTYPEO_SIZE * 5) { }
  protected:
    int index(const NumEffectState &state,
	      const Piece pawn,
	      const Piece target) const;
    const std::string name() const { return "PawnAndEmptyAndPieceX"; }
  };

  class PawnPtypeOPtypeO : public FeaturesOneNonUniq
  {
  public:
    // PtypeO, PtypeO
    PawnPtypeOPtypeO() : FeaturesOneNonUniq(1024) { }
    const std::string name() const { return "PawnPtypePtype"; }
    size_t maxActive() const { return 40; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  };
  class PawnPtypeOPtypeOStages : public EvalComponentStages
  {
  public:
    explicit PawnPtypeOPtypeOStages() : EvalComponentStages(new PawnPtypeOPtypeO())
    {
    }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;
  };

  class PawnPtypeOPtypeOY : public FeaturesOneNonUniq
  {
  public:
    // PtypeO, PtypeO, pawn y
    PawnPtypeOPtypeOY() : FeaturesOneNonUniq(1024 * 9) { }
    const std::string name() const { return "PawnPtypePtypeY"; }
    size_t maxActive() const { return 40; }
    void showAllOne(const Weights&,
		    int n,
		    std::ostream &os) const;
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  };
  class PawnPtypeOPtypeOYStages : public EvalComponentStages
  {
  public:
    explicit PawnPtypeOPtypeOYStages() : EvalComponentStages(new PawnPtypeOPtypeOY())
    {
    }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;
  };

  class AllGold : public EvalComponentMulti
  {
  public:
    AllGold() : EvalComponentMulti(1) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
		  CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights& weights) const;
    const std::string name() const { return "AllGold"; };
  };

  class PtypeY : public EvalComponentMulti
  {
  private:
    int index(const Piece piece) const
    {
      return index(piece.owner(), piece.ptype(), piece.square());
    }
    int index(const Player, const Ptype ptype, const Square pos) const;
  public:
    PtypeY() : EvalComponentMulti(9 * PTYPE_SIZE) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
		  CArray<MultiInt,2>& saved_state) const;
    MultiInt evalWithUpdate(const NumEffectState& state, Move moved, MultiInt last_value, const MultiWeights& w,
			    CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights& weights) const;
    const std::string name() const { return "PtypeY"; };
    const std::string describe(size_t local_index) const;
  };

  class PtypeX : public EvalComponentMulti
  {
  private:
    int index(const Piece piece) const
    {
      return index(piece.owner(), piece.ptype(), piece.square());
    }
    int index(const Player, const Ptype ptype, const Square pos) const;
  public:
    PtypeX() : EvalComponentMulti(5 * PTYPE_SIZE) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
		  CArray<MultiInt,2>& saved_state) const;
    MultiInt evalWithUpdate(const NumEffectState& state, Move moved, MultiInt last_value, const MultiWeights& w,
			    CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights& weights) const;
    const std::string name() const { return "PtypeX"; };
  };

  class PromotedMinorPieces : public FeaturesOneNonUniq
  {
  public:
    PromotedMinorPieces() : FeaturesOneNonUniq(9) { }
    const std::string name() const { return "PromotedMinorPieces"; }
    size_t maxActive() const { return 60; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  private:
    template <Player P>
    void featuresX(const NumEffectState &state,
		   const PieceMask promoted,
		   IndexCacheI<MaxActiveWithDuplication> &features) const;
  };

  class PromotedMinorPiecesY : public FeaturesOneNonUniq
  {
  public:
    // King Y, Attack and Defense
    PromotedMinorPiecesY() : FeaturesOneNonUniq(9 * 9 * 2) { }
    const std::string name() const { return "PromotedMinorPiecesY"; }
    size_t maxActive() const { return 60; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  private:
    template <Player P>
    void featuresX(const NumEffectState &state,
		   const PieceMask promoted,
		   IndexCacheI<MaxActiveWithDuplication> &features) const;
    int index(bool attack, const Square king,
	      const Player owner, int x_diff) const
    {
      const int y = (owner == BLACK ? king.y() : 10 - king.y());
      return x_diff + (y - 1) * 9 + (attack ? 0 : 81);
    }
  };

  class KnightHead : public FeaturesOneNonUniq
  {
  public:
    KnightHead() : FeaturesOneNonUniq(9) { }
    const std::string name() const { return "KnightHead"; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  };

  class KnightHeadOppPiecePawnOnStand : public FeaturesOneNonUniq
  {
  public:
    KnightHeadOppPiecePawnOnStand() : FeaturesOneNonUniq(PTYPE_SIZE * 9) { }
    const std::string name() const { return "KnightHeadOppPiecePawnOnStand"; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  };

  class LanceEffectPieceKingRelative : public FeaturesOneNonUniq
  {
  private:
    int index(Player p, Square pos, Square king,
	      PtypeO ptypeO, bool attack) const
    {
      const int y_diff = (p == BLACK ? king.y() - pos.y() : pos.y() - king.y());
      const int x_diff = std::abs(king.x() - pos.x());
      if (p == WHITE)
      {
	ptypeO = newPtypeO(alt(getOwner(ptypeO)), getPtype(ptypeO));
      }
      return y_diff + 8 + x_diff * 17 + (ptypeO - PTYPEO_MIN) * 17 * 9 +
	(attack ? 0 : 4896);
    }

    void addOne(const NumEffectState &state,
		const Square pos, const Player player,
		IndexCacheI<MaxActiveWithDuplication> &features) const;
  public:
    LanceEffectPieceKingRelative()
      // attack, x_diff, y_diff
      : FeaturesOneNonUniq(PTYPEO_SIZE * 2 * 9 * 17) { } 
    const std::string name() const { return "LanceEffectPieceKingRelative"; }
    size_t maxActive() const { return 8; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  };

  class SilverHeadPawnKingRelative : public FeaturesOneNonUniq
  {
  public:
    SilverHeadPawnKingRelative() : FeaturesOneNonUniq(9 * 17) { }
    const std::string name() const { return "SilverHeadPawnKingRelative"; }
    size_t maxActive() const { return 4; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  };

  class GoldKnightKingRelative : public FeaturesOneNonUniq
  {
  public:
    GoldKnightKingRelative() : FeaturesOneNonUniq(9 * 17) { }
    const std::string name() const { return "GoldKnightKingRelative"; }
    size_t maxActive() const { return 4; }
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  };

  class PawnAttackBase : public FeaturesOneNonUniq
  {
    enum SupportState
    {
      NO_SUPPORT,
      KING_ONLY,
      MAJOR_ONLY,
      MINOR
    };
  public:
    // king x, king y, pawn x, pawn y, opp pawn effect, support state
    PawnAttackBase() : FeaturesOneNonUniq(5 * 9 * 9 * 9 * 2 * 4) { }
    const std::string name() const { return "PawnAttackBase"; }
    size_t maxActive() const { return 20; };
  protected:
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
  private:
    int index(const Player player,
	      const Square king, const Square pawn,
	      const bool has_pawn_effect,
	      PieceMask effect_mask) const
    {
      int x = pawn.x();
      const int y = (player == BLACK ? pawn.y() : 10 - pawn.y());
      int king_x = king.x();
      const int king_y = (player == BLACK ? king.y() : 10 - king.y());
      if (player == BLACK && king_x > 5)
      {
	king_x = 10 - king_x;
	x = 10 - x;
      }
      else if (player == WHITE && king_x >= 5)
      {
	king_x = 10 - king_x;
	x = 10 - x;
      }
      SupportState support = NO_SUPPORT;
      if (effect_mask.any())
      {
	PieceMask mask = effect_mask;
	mask.clearBit<ROOK>();
	mask.clearBit<BISHOP>();
	mask.clearBit<KING>();
	effect_mask.clearBit<KING>();
	if (mask.any())
	{
	  support = MINOR;
	}
	else if (!effect_mask.any())
	{
	  support = KING_ONLY;
	}
	else
	{
	  support = MAJOR_ONLY;
	}
      }
      return (king_y - 1) +
	9 * (king_x - 1 +
	     5 * (y - 1 +
		  9 * (x - 1 +
		       5 * ((has_pawn_effect ? 1 : 0) +
			    2 * support))));
    }
  };

  class SilverAdvance26 : public FeaturesOneNonUniq
  {
    enum { DIM = 1 };
  public:
    SilverAdvance26() : FeaturesOneNonUniq(DIM) { }
    void featuresOneNonUniq
    (const NumEffectState &state, IndexCacheI<MaxActiveWithDuplication> &features) const;
    const std::string name() const { return "SilverAdvance26"; };
  };

  class Promotion37 : public FeaturesOneNonUniq
  {
    enum { DIM = PTYPE_SIZE };
  public:
    Promotion37() : FeaturesOneNonUniq(DIM) { }
    void featuresOneNonUniq
    (const NumEffectState &state, IndexCacheI<MaxActiveWithDuplication> &features) const;
    const std::string name() const { return "Promotion37"; };
    size_t maxActive() const { return DIM*2; };
  };
}

#endif /* GPSSSHOGI_SAMPLE_EVAL_LIB_MINORPIECE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
