/* kingEval.h
 */
#ifndef _KINGEVAL_H
#define _KINGEVAL_H

#include "eval/eval.h"
#include "eval/pieceFeature.h"
#include "eval/indexCache.h"
namespace osl
{
  namespace checkmate
  {
    class King8Info;
  }
}

namespace gpsshogi
{
  using namespace osl;

  class PieceKingRelativeFeature : public FeaturesOneNonUniq
  {
  public:
    enum { ONE_DIM = 2142 };
    PieceKingRelativeFeature() : FeaturesOneNonUniq(ONE_DIM * 2) { }
    static int index(const Player player, const Square king,
		     const Ptype ptype, const Square pos)
    {
      const int x = std::abs(pos.x() - king.x());
      const int y = (king.y() - pos.y()) *
	(player == osl::BLACK ? 1 : -1) + 8;
      return (ptype - osl::PTYPE_PIECE_MIN) * 17 * 9 + (x * 17 + y);
    }

    static int index(const Player player, const Square king,
		     const Piece piece)
    {
      return index(player, king, piece.ptype(), piece.square());
    }
    size_t maxActive() const { return 160; }
    const std::string name() const { return "PieceKingRelativeBoth"; };
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    void featuresOne(const NumEffectState &state, features_one_t&out) const;

    template <int N>
    static CArray<int, N> evalWithUpdateMulti(const NumEffectState& state,
					      Move moved,
					      const CArray<int, N> &last_values,
					      const Weights& w);
    static MultiInt evalWithUpdateStages(const NumEffectState& state,
					 Move moved,
					 const MultiInt &last_values,
					 const MultiWeights& w);
  };
  class PieceKingRelativeStages : public EvalComponentStages
  {
  public:
    PieceKingRelativeStages() : EvalComponentStages(new PieceKingRelativeFeature) { }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;
  };
#ifdef USE_OLD_FEATURE
  class PieceKingRelativeBase : public EvalComponent
  {
  protected:
    virtual int distanceIndex(const osl::Player player,
			      const osl::Square king,
			      const osl::Square pos) const = 0;
    virtual int index(const osl::Ptype ptype,
		      int distanceIndex) const = 0;
    int index(const osl::Player player,
	      const osl::Square king,
	      const osl::Square pos,
	      const osl::Ptype ptype) const {
      return  index(ptype, distanceIndex(player, king, pos));
    }
    bool is_attack;
  public:
    PieceKingRelativeBase(int dim, bool attack=true)
      : EvalComponent(dim), is_attack(attack) { }
    int eval(const osl::NumEffectState &state) const;
    int evalWithUpdate(const osl::NumEffectState &state,
		       osl::Move moved, int last_value) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    int pieceValue(const osl::NumEffectState &state,
		   const osl::Piece piece) const;
    int pieceValue(const osl::Square king,const osl::Piece piece) const;
    size_t maxActive() const { return 38; }
    void showSummary(std::ostream &os) const;
    void showAll(std::ostream& os) const;
  };

  class PieceKingRelative : public PieceKingRelativeBase
  {
  protected:
    int distanceIndex(const osl::Player player,
		      const osl::Square king,
		      const osl::Square pos) const;
    int index(const osl::Ptype ptype,
	      int distanceIndex) const;
  public:
    PieceKingRelative(bool attack=true)
      : PieceKingRelativeBase((osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) *
			      (17 * 9), attack) { }
    virtual void showAll(std::ostream& os) const;
    const std::string name() const {
      return std::string("PieceKingRelative") +
	(is_attack ? "Attack" : "Defense");
    };
  };

  template <int DIM, bool is_x>
  class PieceKingRelativeAbsBase : public EvalComponent
  {
  protected:
    int index(const NumEffectState &state, const Piece piece) const
    {
      return index(state, piece.ptypeO(), piece.square());
    }
    int index(const NumEffectState &state,
		      const PtypeO ptypeO, const Square position) const;
    bool is_attack;
  public:
    PieceKingRelativeAbsBase(bool attack=true)
      :  EvalComponent(DIM), is_attack(attack) {}
    int eval(const osl::NumEffectState &state) const;
    int evalWithUpdate(const osl::NumEffectState &state,
		       osl::Move moved, int last_value) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    virtual const std::string name() const { return "PieceKingRelativeAbsBase"; }
    size_t maxActive () const { return 40; }
  };

  class PieceKingRelativeAbsX
    : public PieceKingRelativeAbsBase<((osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) *
				       5 * 9 * 17), true>
  {
  public:
    PieceKingRelativeAbsX(bool attack=true)
      : PieceKingRelativeAbsBase<((osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) *
				  5 * 9 * 17), true>(attack) { }
    const std::string name() const {
      return std::string("PieceKingRelativeAbsX") +
	(is_attack ? "Attack" : "Defense");
    }
  };

  class PieceKingRelativeAbsY
    : public PieceKingRelativeAbsBase<((osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) *
				       9 * 9 * 17), false>
  {
  public:
    PieceKingRelativeAbsY(bool attack=true)
      : PieceKingRelativeAbsBase<((osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) *
				  9 * 9 * 17), false>(attack) { }
    const std::string name() const {
      return std::string("PieceKingRelativeAbsY") +
	(is_attack ? "Attack" : "Defense");
    }
  };

  class PieceKingRelativeAbs : public EvalComponent
  {
    enum { DIM = (osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) *
	   9 * 9 * 9 * 9 };
  protected:
    int index(const NumEffectState &state, const Piece piece) const
    {
      return index(state, piece.ptypeO(), piece.square());
    }
    int index(const NumEffectState &state,
	      const PtypeO ptypeO, const Square position) const;
    bool is_attack;
  public:
    PieceKingRelativeAbs(bool attack=true)
      :  EvalComponent(DIM), is_attack(attack) {}
    int eval(const osl::NumEffectState &state) const;
    int evalWithUpdate(const osl::NumEffectState &state,
		       osl::Move moved, int last_value) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    virtual const std::string name() const {
      return (std::string("PieceKingRelativeAbs") +
	      (is_attack ? "Attack" : "Defense")); }
    size_t maxActive () const { return 40; }
  };

  class SimpleAttackKing : public EvalComponent
  {
    int distanceIndex(const osl::Square king,
		      const osl::Square pos) const;
    int index(const osl::Square king,
	      const osl::Square pos,
	      const osl::Ptype ptype) const;
    int index(const osl::Ptype ptype,
	      int distanceIndex) const;
    int pieceValue(const osl::NumEffectState &state,
		   const osl::Piece piece) const;
    bool is_attack;
  public:
    SimpleAttackKing(bool attack=true) : EvalComponent(
      (osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) * (8 - 1 + 1)),
					 is_attack(attack) { }
    int eval(const osl::NumEffectState &state) const;
    int evalWithUpdate(const osl::NumEffectState &state,
		       osl::Move moved, int last_value) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os) const;
    void showAll(std::ostream& os) const;
    const std::string name() const;
  };
#endif
  class King8EffectBase : public EvalComponent
  {
  public:
    enum EffectState
    {
      NOT_EMPTY = -1,
      NO_EFFECT = 0,
      LESS_EFFECT,
      MORE_EFFECT,
      MORE_EFFECT_KING_ONLY
    };
    King8EffectBase(int dim) : EvalComponent(dim) { }
    int eval(const osl::NumEffectState &state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    virtual void showSummary(std::ostream &os) const = 0;
  protected:
    virtual EffectState effectState(const NumEffectState &state,
				    const Player defense,
				    const Direction dir) const;
    virtual int index(Piece king,
		      const Direction dir,
		      EffectState state) const = 0;
    virtual bool isTarget(const osl::checkmate::King8Info &) const {
      return true; }
    virtual const std::string name() const { return "King8EffectBase"; };
  };

  class King8EffectEmptySquare : public King8EffectBase
  {
  public:
    King8EffectEmptySquare() : King8EffectBase(8 * 4) { }
    const std::string name() const { return "King8EffectEmptySquare"; };
    virtual void showSummary(std::ostream &os) const;
  protected:
    virtual int index(Piece king,
		      const Direction dir,
		      EffectState state) const;
  };

  class King8EffectEmptySquareY : public King8EffectBase
  {
  public:
    King8EffectEmptySquareY() : King8EffectBase(8 * 4 * 9) { }
    const std::string name() const { return "King8EffectEmptySquareY"; };
    void showAll(std::ostream &os) const;
  protected:
    virtual int index(Piece king,
		      const Direction dir,
		      EffectState state) const;
  };

  class King8EffectDefenseSquareY : public King8EffectEmptySquareY
  {
  public:
    King8EffectDefenseSquareY() : King8EffectEmptySquareY() { }
  protected:
    virtual EffectState effectState(const NumEffectState &state,
				    const Player defense,
				    const Direction dir) const;
    const std::string name() const { return "King8EffectDefenseSquareY"; };
  };

  class King8EffectEmptySquareDBlocked : public King8EffectEmptySquare
  {
  public:
    King8EffectEmptySquareDBlocked() : King8EffectEmptySquare() { }
    const std::string name() const {
      return "King8EffectEmptySquareDBlocked";
    };
    static bool isBlocked(const osl::checkmate::King8Info &);
  protected:
    virtual bool isTarget(const osl::checkmate::King8Info &info) const {
      return isBlocked(info);
    }
  };
  class King8EffectEmptySquareUBlocked : public King8EffectEmptySquare
  {
  public:
    King8EffectEmptySquareUBlocked() : King8EffectEmptySquare() { }
    const std::string name() const {
      return "King8EffectEmptySquareUBlocked";
    };
    static bool isBlocked(const osl::checkmate::King8Info &);
  protected:
    virtual bool isTarget(const osl::checkmate::King8Info &info) const {
      return isBlocked(info);
    }
  };
  class King8EffectEmptySquareLBlocked : public King8EffectEmptySquare
  {
  public:
    King8EffectEmptySquareLBlocked() : King8EffectEmptySquare() { }
    const std::string name() const {
      return "King8EffectEmptySquareLBlocked";
    };
    static bool isBlocked(const osl::checkmate::King8Info &);
  protected:
    virtual bool isTarget(const osl::checkmate::King8Info &info) const {
      return isBlocked(info);
    }
  };
  class King8EffectEmptySquareRBlocked : public King8EffectEmptySquare
  {
  public:
    King8EffectEmptySquareRBlocked() : King8EffectEmptySquare() { }
    const std::string name() const {
      return "King8EffectEmptySquareRBlocked";
    };
    static bool isBlocked(const osl::checkmate::King8Info &);
  protected:
    virtual bool isTarget(const osl::checkmate::King8Info &info) const {
      return isBlocked(info);
    }
  };

  class King8EffectDefenseSquare : public King8EffectEmptySquare
  {
  public:
    King8EffectDefenseSquare() : King8EffectEmptySquare() { }
  protected:
    virtual EffectState effectState(const NumEffectState &state,
			    const Player defense,
			    const Direction dir) const;
    const std::string name() const { return "King8EffectDefenseSquare"; };
  };

  class King8EffectDefenseSquareDBlocked : public King8EffectDefenseSquare
  {
  public:
    King8EffectDefenseSquareDBlocked() : King8EffectDefenseSquare() { }
    const std::string name() const {
      return "King8EffectDefenseSquareDBlocked";
    };
  protected:
    virtual bool isTarget(const osl::checkmate::King8Info &info) const {
      return King8EffectEmptySquareDBlocked::isBlocked(info);
    }
  };
  class King8EffectDefenseSquareUBlocked : public King8EffectDefenseSquare
  {
  public:
    King8EffectDefenseSquareUBlocked() : King8EffectDefenseSquare() { }
    const std::string name() const {
      return "King8EffectDefenseSquareUBlocked";
    };
  protected:
    virtual bool isTarget(const osl::checkmate::King8Info &info) const {
      return King8EffectEmptySquareUBlocked::isBlocked(info);
    }
  };
  class King8EffectDefenseSquareLBlocked : public King8EffectDefenseSquare
  {
  public:
    King8EffectDefenseSquareLBlocked() : King8EffectDefenseSquare() { }
    const std::string name() const {
      return "King8EffectDefenseSquareLBlocked";
    };
  protected:
    virtual bool isTarget(const osl::checkmate::King8Info &info) const {
      return King8EffectEmptySquareLBlocked::isBlocked(info);
    }
  };
  class King8EffectDefenseSquareRBlocked : public King8EffectDefenseSquare
  {
  public:
    King8EffectDefenseSquareRBlocked() : King8EffectDefenseSquare() { }
    const std::string name() const {
      return "King8EffectDefenseSquareRBlocked";
    };
  protected:
    virtual bool isTarget(const osl::checkmate::King8Info &info) const {
      return King8EffectEmptySquareRBlocked::isBlocked(info);
    }
  };

  struct King25EffectCommon
  {
    static void countEffectAndPieces(bool is_attack,
				     const osl::NumEffectState &state,
				     const osl::Player attack,
				     int &effect,
				     int &piece);
    static int index(int effect, int piece_count);
    static void featuresNonUniq(bool is_attack, const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset);
  };
  class King25EffectAttack : public EvalComponent, King25EffectCommon
  {
  public:
    King25EffectAttack() : EvalComponent(17 * 128) { }
    int eval(const osl::NumEffectState &state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    size_t maxActive() const { return 2; }
    void showAll(std::ostream &os) const;
    const std::string name() const {
      return "King25EffectAttack";
    }
  };
  class King25EffectDefense : public EvalComponentMulti, King25EffectCommon
  {
  public:
    King25EffectDefense() : EvalComponentMulti(17 * 128) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
      CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    size_t maxActiveOne() const { return 2; }
    void showAll(std::ostream &os, const MultiWeights& weights) const;
    const std::string name() const { return "King25EffectDefense"; }
  };

  struct King25EffectYCommon
  {
    static void countEffectAndPieces(bool is_attack,
				     const osl::NumEffectState &state,
				     const osl::Player attack,
				     int &effect,
				     int &piece,
				     int & king_y);
    static int index(int king_y, int effect, int piece_count);
    static void featuresNonUniq(bool is_attack, const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset);
  };
  class King25EffectYAttack : public EvalComponent, King25EffectYCommon
  {
  public:
    King25EffectYAttack() : EvalComponent(17 * 128 * 9) { }
    int eval(const osl::NumEffectState &state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    size_t maxActive() const { return 2; }
    void showAll(std::ostream &os) const;
    const std::string name() const { return "King25EffectYAttack"; }
  };
  class King25EffectYDefense : public EvalComponentMulti, King25EffectYCommon
  {
  public:
    King25EffectYDefense() : EvalComponentMulti(17 * 128 * 9) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
      CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    size_t maxActiveOne() const { return 2; }
    void showAll(std::ostream &os, const MultiWeights& weights) const;
    const std::string name() const {
      return "King25EffectYDefense";
    }
  };

  class King25EffectXY : public EvalComponent
  {
  private:
    void countEffectAndPieces(const osl::NumEffectState &state,
			      const osl::Player attack,
			      int &effect,
			      int &piece,
			      int &king_x,
			      int & king_y) const;
    int indexY(int king_y, int effect, int piece_count) const;
    int indexX(int king_x, int effect, int piece_count) const;
    bool is_attack;
    enum
    {
      DIM_X = 17 * 128 * 5,
      DIM_Y = 17 * 128 * 9,
    };
  public:
    King25EffectXY(bool attack=true) : EvalComponent(DIM_X + DIM_Y),
				       is_attack(attack) { }
    int eval(const osl::NumEffectState &state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    size_t maxActive() const { return 4; }
    void showAll(std::ostream &os) const;
    const std::string name() const {
      return std::string("King25EffectXY") +
	(is_attack ? "Attack" : "Defense");
    };
  };

  // With piece count (HI, KA, KI, GI)
  class King25Effect2 : public FeaturesOneNonUniq
  {
    int index(int effect, int piece_count, int stand_count) const
    {
      return (effect + 64 * piece_count) * 13 + stand_count;
    }
  public:
    King25Effect2() : FeaturesOneNonUniq(17 * 13 * 64) { }
    size_t maxActive() const { return 10; }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25Effect2"; };
    static void countEffectAndPieces(
      const osl::NumEffectState &state,
      const osl::Player attack,
      int &effect,
      int &piece,
      int &king_y)
    {
      const Player king_player = alt(attack);
      const Square king = state.kingSquare(king_player);
      if (king_player == BLACK)
	king_y = king.y();
      else
	king_y = 10 - king.y();
      const int min_x = std::max(1, king.x() - 2);
      const int max_x = std::min(9, king.x() + 2);
      const int min_y = std::max(1, king.y() - 2);
      const int max_y = std::min(9, king.y() + 2);

      PieceMask mask;
      int count = 0;
      for (int y = min_y; y <= max_y; ++y)
      {
	for (int x = min_x; x <= max_x; ++x)
	{
	  const Square target(x, y);
	  count += state.countEffect(attack, target);
	  mask = mask | state.effectSetAt(target);
	}
      }
      effect = std::min(63, count);
      mask = mask & state.piecesOnBoard(attack);
      piece = std::min(16,  mask.countBit());
    }
  };

  class King25EffectY2 : public FeaturesOneNonUniq
  {
    int index(int king_y, int effect, int piece_count, int stand_count) const
    {
      return ((effect + 64 * piece_count) * 13 + stand_count) * 9 + king_y - 1;
    }
  public:
    King25EffectY2() : FeaturesOneNonUniq(17 * 13 * 64 * 9) { }
    size_t maxActive() const { return 10; }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25EffectY2"; };
  };

  class King25EffectSupported : public FeaturesOneNonUniq
  {
    int index(int piece_count, int supported) const
    {
      return supported * 17 + piece_count;
    }
  public:
    King25EffectSupported() : FeaturesOneNonUniq(17 * 17) { }
    size_t maxActive() const { return 10; }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25EffectSupported"; };
    static void countEffectAndPieces(
      const osl::NumEffectState &state,
      const osl::Player attack,
      int &piece,
      int &supported,
      int &king_y)
    {
      const Player king_player = alt(attack);
      const Square king = state.kingSquare(king_player);
      if (king_player == BLACK)
	king_y = king.y();
      else
	king_y = 10 - king.y();
      const int min_x = std::max(1, king.x() - 2);
      const int max_x = std::min(9, king.x() + 2);
      const int min_y = std::max(1, king.y() - 2);
      const int max_y = std::min(9, king.y() + 2);

      PieceMask mask;
      for (int y = min_y; y <= max_y; ++y)
      {
	for (int x = min_x; x <= max_x; ++x)
	{
	  const Square target(x, y);
	  mask = mask | state.effectSetAt(target);
	}
      }
      mask = mask & state.piecesOnBoard(attack);
      piece = std::min(16,  mask.countBit());
      mask = mask & state.effectedMask(attack);
      supported = std::min(16, mask.countBit());
    }
  };

  class King25EffectSupportedY : public FeaturesOneNonUniq
  {
    int index(int piece_count, int supported, int y) const
    {
      return (supported * 17 + piece_count) * 9 + y - 1;
    }
  public:
    King25EffectSupportedY() : FeaturesOneNonUniq(17 * 17 * 9) { }
    size_t maxActive() const { return 10; }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25EffectSupportedY"; };
  };

  class King25Empty : public EvalComponent
  {
  protected:
    enum { DIM = 5 * 3 };
    int index(int rel_x, int rel_y) const;
    template <Player P>
    void stateOne(const NumEffectState &state,
		  CArray<int, DIM> &result) const;
    virtual bool isTarget(const NumEffectState &state,
			  const Square pos,
			  const Player defense) const;
  public:
    King25Empty() : EvalComponent(DIM) { }
    int eval(const osl::NumEffectState &state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os) const;
    virtual const std::string name() const { return "King25Empty"; }
  };

  class King25EmptyNoEffect : public King25Empty
  {
  protected:
    virtual bool isTarget(const NumEffectState &state,
			  const Square pos,
			  const Player defense) const;
  public:
    King25EmptyNoEffect() : King25Empty() { }
    virtual const std::string name() const { return "King25EmptyNoEffect"; }
  };
#ifdef USE_OLD_FEATURE
  class King25EmptyAbs : public EvalComponent
  {
  private:
    enum { DIM = 5 * 5 * 5 * 9 };
    int index(Square king,
	      Square target,
	      Player player) const;
    template <Player Defense>
    int evalOne(const osl::NumEffectState &state) const;
    template <Player Defense>
    void oneFeature(const osl::NumEffectState &state,
		   CArray<int, DIM> &features) const;
  public:
    King25EmptyAbs() : EvalComponent(DIM) { }
    int evalWithUpdate(
      const NumEffectState &state, osl::Move moved,
      int last_value) const;
    int eval(const osl::NumEffectState &state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    const std::string name() const { return "King25EmptyAbs"; }
  };
#endif
  class King25EffectEach : public EvalComponentMulti
  {
  public:
    enum EffectState
    {
      NO_ATTACK_DEFENSE_0,
      NO_ATTACK_DEFENSE_1,
      NO_ATTACK_DEFENSE_2,
      ATTACK_DIFF_N2,
      ATTACK_DIFF_N1,
      ATTACK_DIFF_0,
      ATTACK_DIFF_1,
      ATTACK_DIFF_2,
      STATE_MAX, // 8
    };
  private:
    // rel_y, abs_x, effect state, owner of piece
    enum { DIM = 5 * 3 * STATE_MAX * 3 };
    static int index(const NumEffectState &state, Square target,
	      Player defense);
    template <osl::Player Defense, typename Integer, class Weights>
    static Integer evalOne(const osl::NumEffectState &state, Integer initial, const Weights&);
    template <osl::Player Defense>
    void featureOne(const osl::NumEffectState &state,
		    CArray<int, DIM> &feature_count) const;
  public:
    King25EffectEach() : EvalComponentMulti(DIM) { }
    MultiInt eval(const NumEffectState& state, const MultiWeights& w,
      CArray<MultiInt,2>& saved_state) const;
    MultiInt evalWithUpdate(const NumEffectState& state, Move moved, MultiInt last_value, const MultiWeights& w,
      CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    static EffectState effectState(const NumEffectState &state,
				   Square target,
				   Player defense);
    const std::string name() const { return "King25EffectEach"; }
  };

  class King25EffectEachYFeatures : public FeaturesOneNonUniq
  {
  public:
    // rel_y, abs_x, effect state, owner of piece, king_y
    enum { DIM = 5 * 3 * King25EffectEach::STATE_MAX * 3 * 9 }; // 3240
    static int index(const NumEffectState &state, Square target,
		     Player defense)
    {
      const Square king = state.kingSquare(defense);
      const Piece piece = state.pieceAt(target);
      // [0, 2]
      const int rel_x = std::abs(king.x() - target.x());
      // [-2, +2]
      const int rel_y = (target.y() - king.y()) * (defense == BLACK ? 1 : -1);
      const int king_y = (defense == BLACK ? king.y() : 10 - king.y());
      int piece_owner;
      if (piece.isEmpty())
	piece_owner = 0;
      else if (piece.owner() == defense)
	piece_owner = 1;
      else
	piece_owner = 2;

      int val = (rel_y + 2 +
		 rel_x * 5 +
		 King25EffectEach::effectState(state, target, defense) * 5 * 3 +
		 piece_owner * 5 * 3 * 8) * 9 + king_y - 1;
      return val;
    }
    template <osl::Player Defense>
    static void featureOne(const osl::NumEffectState &state,
			   IndexCacheI<MaxActiveWithDuplication> &features);
  public:
    King25EffectEachYFeatures() : FeaturesOneNonUniq(DIM) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "King25EffectEachY"; }
    size_t maxActive() const { return 5*5*2; }
    void showAllOne(const Weights&,
		    int n,
		    std::ostream &os) const;
  };
  class King25EffectEachYStages : public EvalComponentStagesBW
  {
  public:
    King25EffectEachYStages() : EvalComponentStagesBW(new King25EffectEachYFeatures) { }

    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;

    void featureOneBlack(const NumEffectState &state, index_list_t& out) const
    {
      King25EffectEachYFeatures::featureOne<BLACK>(state, out);
    }
    void featureOneWhite(const NumEffectState &state, index_list_t& out) const
    {
      King25EffectEachYFeatures::featureOne<WHITE>(state, out);
    }
  };

  class King25EffectEachXFeatures : public FeaturesOneNonUniq
  {
  public:
    // rel_y, rel_x, effect state, owner of piece, king_x
    enum { DIM = 5 * 5 * King25EffectEach::STATE_MAX * 3 * 5 }; // 3000
    static int index(const NumEffectState &state, Square target,
		     Player defense)
    {
      const Square king = state.kingSquare(defense);
      const Piece piece = state.pieceAt(target);
      // [-2, 2]
      int rel_x = king.x() - target.x();
      // [-2, +2]
      const int rel_y = (target.y() - king.y()) * (defense == BLACK ? 1 : -1);
      const int king_x = (king.x() > 5 ? 10 - king.x() : king.x());
      if ((defense == BLACK && king.x() >= 6) ||
	  (defense == WHITE && king.x() >= 5))
      {
	rel_x = -rel_x;
      }
      int piece_owner;
      if (piece.isEmpty())
	piece_owner = 0;
      else if (piece.owner() == defense)
	piece_owner = 1;
      else
	piece_owner = 2;

      int val = (rel_y + 2 +
		 (rel_x + 2) * 5 +
		 King25EffectEach::effectState(state, target, defense) * 5 * 5 +
		 piece_owner * 5 * 5 * 8) * 5 + king_x - 1;
      return val;
    }
    template <osl::Player Defense>
    static void featureOne(const osl::NumEffectState &state,
			   IndexCacheI<MaxActiveWithDuplication> &features);
  public:
    King25EffectEachXFeatures() : FeaturesOneNonUniq(DIM) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "King25EffectEachX"; }
    size_t maxActive() const { return 5*5*2; }
  };
  class King25EffectEachXStages : public EvalComponentStagesBW
  {
  public:
    King25EffectEachXStages() : EvalComponentStagesBW(new King25EffectEachXFeatures) { }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;

    void featureOneBlack(const NumEffectState &state, index_list_t& out) const
    {
      King25EffectEachXFeatures::featureOne<BLACK>(state, out);
    }
    void featureOneWhite(const NumEffectState &state, index_list_t& out) const
    {
      King25EffectEachXFeatures::featureOne<WHITE>(state, out);
    }
  };

  class King25EffectEachXYFeatures : public FeaturesOneNonUniq
  {
  private:
    static int index(const NumEffectState &state, Square target,
		     Player defense)
    {
      const Square king = state.kingSquare(defense);
      const Piece piece = state.pieceAt(target);
      // [-2, 2]
      int rel_x = king.x() - target.x();
      // [-2, +2]
      const int rel_y = (target.y() - king.y()) * (defense == BLACK ? 1 : -1);
      const int king_x = (king.x() > 5 ? 10 - king.x() : king.x());
      const int king_y = (defense == BLACK ? king.y() : 10 - king.y());
      if ((defense == BLACK && king.x() >= 6) ||
	  (defense == WHITE && king.x() >= 5))
      {
	rel_x = -rel_x;
      }
      int piece_owner;
      if (piece.isEmpty())
	piece_owner = 0;
      else if (piece.owner() == defense)
	piece_owner = 1;
      else
	piece_owner = 2;

      int val = ((rel_y + 2 +
		  (rel_x + 2) * 5 +
		  King25EffectEach::effectState(state, target, defense) * 5 * 5 +
		  piece_owner * 5 * 5 * 8) * 9 + king_y - 1) * 5 + king_x - 1;
      return val;
    }
  public:
    // x, y, rel_x, rel_y, effect state, owner of piece
    King25EffectEachXYFeatures()
      : FeaturesOneNonUniq(5 * 9 * 5 * 5 * King25EffectEach::STATE_MAX * 3) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    size_t maxActive() const { return 5*5*2; }
    const std::string name() const { return "King25EffectEachXY"; }
    template <osl::Player Defense>
    static void featureOne(const osl::NumEffectState &state,
			   IndexCacheI<MaxActiveWithDuplication> &features);
  };

  class King25EffectEachXYStages : public EvalComponentStagesBW
  {
  public:
    King25EffectEachXYStages()
      : EvalComponentStagesBW(new King25EffectEachXYFeatures) { }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;
    void featureOneBlack(const NumEffectState &state, index_list_t&) const;
    void featureOneWhite(const NumEffectState &state, index_list_t&) const;
  };

  class KingXBlocked : public EvalComponentMulti
  {
  protected:
    virtual int index(const NumEffectState &state, Piece king, int diff) const;
    template <osl::Player P>
    static bool isBlocked(const NumEffectState &state,
			  int diff);
  public:
    KingXBlocked(int dim=10) : EvalComponentMulti(dim) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
      CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    const std::string name() const { return "KingXBlocked"; }
  };

  class KingXBlockedY : public KingXBlocked
  {
  private:
    virtual int index(const NumEffectState &state, Piece king, int diff) const;
  public:
    KingXBlockedY() : KingXBlocked(10 * 9) { }
    const std::string name() const { return "KingXBlockedY"; }
  };

  class KingXBothBlocked : public FeaturesOneNonUniq
  {
  public:
    KingXBothBlocked() : FeaturesOneNonUniq(5) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "KingXBothBlocked"; }
  };

  class KingXBothBlockedY : public FeaturesOneNonUniq
  {
  public:
    KingXBothBlockedY() : FeaturesOneNonUniq(9 * 5) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "KingXBothBlockedY"; }
  };

  class KingXBlocked3 : public FeaturesOneNonUniq
  {
  public:
    // king x, L or R, U, UR, R
    KingXBlocked3() : FeaturesOneNonUniq(5 * 2 * 2 * 2 * 2) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "KingXBothBlocked3"; }
  private:
    template <Player P>
    int index(const Square king, bool is_l,
	      bool u_blocked, bool opp_u_blocked, bool opp_blocked) const
    {
      int x = king.x();
      if (x >= 6)
      {
	x = 10 - x;
	if (P == BLACK)
	{
	  is_l = !is_l;
	}
      }
      else if (P == WHITE && x <= 4)
      {
	is_l = !is_l;
      }
      return x - 1 + 5 * ((is_l ? 1 : 0) + 2 * ((u_blocked ? 1 : 0) + 2 * ((opp_u_blocked ? 1  : 0) + 2 * (opp_blocked ? 1 : 0))));
    }
  };

  class KingXBlocked3Y : public FeaturesOneNonUniq
  {
  public:
    // king x, king y, L or R, U, UR, R
    KingXBlocked3Y() : FeaturesOneNonUniq(5 * 9 * 2 * 2 * 2 * 2) { }
    void featuresOneNonUniq(const NumEffectState &state,
			    IndexCacheI<MaxActiveWithDuplication>&diffs) const;
    const std::string name() const { return "KingXBothBlocked3Y"; }
  private:
    template <Player P>
    int index(const Square king, bool is_l,
	      bool u_blocked, bool opp_u_blocked, bool opp_blocked) const
    {
      int x = king.x();
      const int y = (P == BLACK ? king.y() : 10 - king.y());
      if (x >= 6)
      {
	x = 10 - x;
	if (P == BLACK)
	{
	  is_l = !is_l;
	}
      }
      else if (P == WHITE && x <= 4)
      {
	is_l = !is_l;
      }
      return x - 1 + 5 * (y - 1 + 9 * ((is_l ? 1 : 0) + 2 * ((u_blocked ? 1 : 0) + 2 * ((opp_u_blocked ? 1  : 0) + 2 * (opp_blocked ? 1 : 0)))));
    }
  };

  class AnagumaEmpty : public EvalComponentMulti
  {
  private:
    enum { DIM = 4 };
    static int index(const NumEffectState &state, Square king,
		     Square target);
    template <Player Defense>
    void featureOne(const NumEffectState &state,
		    CArray<int, DIM> &features) const;
  public:
    AnagumaEmpty() : EvalComponentMulti(DIM) { }
    MultiInt eval(const osl::NumEffectState &state, const MultiWeights& weights,
      CArray<MultiInt,2>& saved_state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void showSummary(std::ostream &os, const MultiWeights& weights) const;
    const std::string name() const { return "AnagumaEmpty"; }
  };

  class PieceKingRelativeNoSupport : public FeaturesOneNonUniq
  {
  private:
    enum { ONE_DIM = 2142 };
    int index(const Player player, const Square king,
	      const Ptype ptype, const Square pos) const
    {
      const int x = std::abs(pos.x() - king.x());
      const int y = (king.y() - pos.y()) *
	(player == osl::BLACK ? 1 : -1) + 8;
      return (ptype - osl::PTYPE_PIECE_MIN) * 17 * 9 + (x * 17 + y);
    }
    int index(const Player player, const Square king,
	      const Piece piece) const
    {
      return index(player, king, piece.ptype(), piece.square());
    }
  public:
    PieceKingRelativeNoSupport() : FeaturesOneNonUniq(ONE_DIM * 2) { }
    size_t maxActive() const { return 160; }
    const std::string name() const { return "PieceKingRelativeNoSupport"; } 

    void featuresOneNonUniq(
      const NumEffectState &,
      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
    void featuresOne(const NumEffectState &state, features_one_t&out) const;
  };

  class PtypeYYFeatures : public FeaturesOneNonUniq
  {
  public:
    // 32 (ptypeo) * 9 (piece y) * 9 (king y)
    enum { DIM = 2592 };
    PtypeYYFeatures() : FeaturesOneNonUniq(DIM) { }
    // 40 (piece size) * 2 (attack, defense) * 2 (opening, ending)
    size_t maxActive() const { return 160; }
    const std::string name() const { return "PtypeYY"; }
  protected:
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
  public:
    template <Player KingPlayer>
    static int index(const Piece p, const Square king) 
    {
      return index<KingPlayer>(p.ptypeO(), p.square(), king);
    }

    template <Player KingPlayer>
    static int index(const PtypeO ptypeO, const Square position,
		     const Square king) 
    {
      const int king_y = (KingPlayer == BLACK ? king.y() : 10 - king.y());
      const int piece_y = (KingPlayer == BLACK ? position.y() :
			   10 - position.y());
      return (king_y - 1) * 9 * 32 + (piece_y - 1) * 32 +
	(KingPlayer == BLACK ? ptypeO :
	 newPtypeO(alt(getOwner(ptypeO)), getPtype(ptypeO))) - PTYPEO_MIN;
    }
    template <int N>
    static CArray<int, N> evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const CArray<int, N> &last_values,
      const Weights& w);
    static MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      const MultiWeights& w);
  };
  class PtypeYYStages : public EvalComponentStages
  {
  public:
    PtypeYYStages() : EvalComponentStages(new PtypeYYFeatures) { }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;
  };

  class King3Pieces : public FeaturesOneNonUniq {
  public:
    // ptypeo * ptypeo * (H V D)
    King3Pieces() : FeaturesOneNonUniq(32 * 32 * 3) { }
    const std::string name() const { return "King3Pieces"; }
    size_t macActive() const { return 8; }
  protected:
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
  private:
    enum Direction
    {
      HORIZONTAL = 0,
      VERTICAL,
      DIAGONAL,
    };
    int index(const Player king, PtypeO p1, PtypeO p2,
	      Direction dir) const
    {
      if (king == WHITE)
      {
	if (isPiece(p1))
	  p1 = newPtypeO(alt(getOwner(p1)), getPtype(p1));
	if (isPiece(p2))
	  p2 = newPtypeO(alt(getOwner(p2)), getPtype(p2));
      }
      return ptypeOIndex(p1) * 32 + ptypeOIndex(p2) + 1024 * dir;
    }
    template <Player King>
    void featuresKing(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
  };

  class King3PiecesXY : public FeaturesOneNonUniq {
    // ptypeo * ptypeo * (H V D) * {X, Y}
    enum
    {
      X_DIM = 32 * 32 * 3 * 5,
      Y_DIM = 32 * 32 * 3 * 9,
      DIM = X_DIM + Y_DIM
    };
  public:
    King3PiecesXY() : FeaturesOneNonUniq(DIM) { }
    const std::string name() const { return "King3PiecesXY"; }
    size_t maxActive() const { return 16; }
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
    enum Direction
    {
      HORIZONTAL = 0,
      VERTICAL,
      DIAGONAL,
    };
    int indexY(const Player king,
	       const Square king_position,
	       PtypeO p1, PtypeO p2,
	       Direction dir) const
    {
      if (king == WHITE)
      {
	if (isPiece(p1))
	  p1 = newPtypeO(alt(getOwner(p1)), getPtype(p1));
	if (isPiece(p2))
	  p2 = newPtypeO(alt(getOwner(p2)), getPtype(p2));
      }
      const int king_y = (king == BLACK ? king_position.y() :
			  10 - king_position.y());
      return ptypeOIndex(p1) * 32 + ptypeOIndex(p2) + 1024 * dir
	+ (king_y - 1) * 32 * 32 * 3 + X_DIM;
    }
    int indexX(const Player king,
	       const Square king_position,
	       PtypeO p1, PtypeO p2,
	       Direction dir) const
    {
      if (king == WHITE)
      {
	if (isPiece(p1))
	  p1 = newPtypeO(alt(getOwner(p1)), getPtype(p1));
	if (isPiece(p2))
	  p2 = newPtypeO(alt(getOwner(p2)), getPtype(p2));
      }
      const int king_x = (king_position.x() > 5 ? 10 - king_position.x() :
			  king_position.x());
      if (dir == HORIZONTAL &&
	  ((king == BLACK && king_position.x() >= 6) ||
	   (king == WHITE && king_position.x() <= 4)))
      {
	PtypeO tmp = p1;
	p1 = p2; p2 = tmp;
      }
      return ptypeOIndex(p1) * 32 + ptypeOIndex(p2) + 1024 * dir
	+ (king_x - 1) * 32 * 32 * 3;
    }
  private:
    template <Player King>
    void featuresKing(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
  };
  
  class GoldAndSilverNearKing : public FeaturesOneNonUniq
  {
  public:
    // gold + silver, king_x, king_y, distance [1-3]
    GoldAndSilverNearKing() : FeaturesOneNonUniq(9 * 5 * 9 * 3) { }
    const std::string name() const { return "GoldAndSilverNearKing"; }
    size_t macActive() const { return 6; }
  protected:
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
  private:
    template <Player Defense>
    void featuresKing(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
    template <Player Defense, Ptype PTYPE>
    void countPiece(const NumEffectState &state,
		    CArray<int, 3> &out) const;
    template <Player Defense>
    int index(const Square king, int distance0, int count) const
    {
      int king_x = (king.x() > 5 ? 10 - king.x() : king.x());
      int king_y = (Defense == WHITE ? 10 - king.y() : king.y());
      return king_x - 1 + 5 * (king_y - 1 + 9 * (distance0 + 3 * count));
    }
  };

  class GoldAndSilverNearKingCombination : public FeaturesOneNonUniq
  {
  public:
    // gold + silver, king_x, king_y, count at distance 1, 2, 3
    GoldAndSilverNearKingCombination()
      : FeaturesOneNonUniq(9 * 5 * 6 * 6 * 6) { }
    const std::string name() const
    {
      return "GoldAndSilverNearKingCombination";
    }
    size_t macActive() const { return 2; }
  protected:
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
  private:
    template <Player Defense>
    void featuresKing(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
    template <Player Defense, Ptype PTYPE>
    void countPiece(const NumEffectState &state,
		    CArray<int, 3> &out) const;
    template <Player Defense>
    int index(const Square king, int count0, int count1, int count2) const
    {
      int king_x = (king.x() > 5 ? 10 - king.x() : king.x());
      int king_y = (Defense == WHITE ? 10 - king.y() : king.y());
      return king_x + 5 * (king_y + 9 * (count0 + 6 * (count1 + 6 * count2)));
    }
  };

  class GoldNearKing : public FeaturesOneNonUniq
  {
  public:
    // distance 1, 2, 3, 4
    GoldNearKing() : FeaturesOneNonUniq(5 * 5 * 5 * 5) { }
    const std::string name() const { return "GoldNearKing"; }
  private:
    int index(const CArray<int, 4> &golds) const
    {
      return golds[0] + 5 * (golds[1] + 5 * (golds[2] + 5 * golds[3]));
    }
    template <Player P>
    void countGold(const NumEffectState &state,
		   CArray<int, 4> &golds) const;
  protected:
    void featuresOneNonUniq(
      const NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
  };

  class KingMobility : public FeaturesOneNonUniq
  {
  private:
    template<Player P, Direction Dir>
    int index(Square king, Square end, int dir) const
    {
      const int king_x = (king.x() > 5 ? 10 - king.x() : king.x()) - 1;
      const int king_y = (P == BLACK ? king.y() : 10 - king.y()) - 1;
      const int mobility = ((Dir == L || Dir == R) ?
			    std::abs(end.x() - king.x()) :
			    std::abs(end.y() - king.y())) - 1;
      return king_x + 5 * (king_y + 9 * (dir + 8 * mobility));
    }
    template <Player P>
    void featuresKing(const NumEffectState &state,
		      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
  public:
    // Mobility, Dir, king_x, king_y
    KingMobility() : FeaturesOneNonUniq(9 * 8 * 5 * 9) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "KingMobility"; }
    size_t maxActive() const { return 16; }
  };

  class KingMobilitySum : public FeaturesOneNonUniq
  {
  private:
    template<Player P>
    int index(Square king, int mobility) const
    {
      const int king_x = (king.x() > 5 ? 10 - king.x() : king.x()) - 1;
      const int king_y = (P == BLACK ? king.y() : 10 - king.y()) - 1;
      return king_x + 5 * (king_y + 9 * mobility);
    }
    template <Player P>
    void featuresKing(const NumEffectState &state,
		      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
  public:
    // Mobility, king_x, king_y
    KingMobilitySum() : FeaturesOneNonUniq(65 * 5 * 9) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "KingMobilitySum"; }
    size_t maxActive() const { return 2; }
  };

  class King25BothSideFeatures : public FeaturesOneNonUniq
  {
  public:
    King25BothSideFeatures() : FeaturesOneNonUniq(32 * 32 * 8) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25BothSide"; }
    size_t maxActive() const { return 16; }
  private:
    static int index(int effect1, int effect2, int i)
    {
      assert(0 <= effect1 && effect1 < 32);
      assert(0 <= effect2 && effect2 < 32);
      return effect1 + 32 * (effect2 + 32 * i);
    }
  public:
    template <Player P>
    static void featuresKing(const NumEffectState &state,
			     IndexCacheI<MaxActiveWithDuplication> &diffs);
  private:
    template <Player Defense>
    static int effectVertical(const NumEffectState &state, int index)
    {
      Square king = state.kingSquare<Defense>();
      int result = 0;
      const int x = king.x() + (Defense == BLACK ? -2 + index : 2 - index);
      for (int y_diff = -2; y_diff <= 2; ++y_diff)
      {
	const int y = king.y() + y_diff;
	if (x < 1 || x > 9 || y < 1 || y > 9 ||
	    state.hasEffectAt<alt(Defense)>(Square(x, y)))
	{
	  result |= (1 << (Defense == BLACK ? y_diff + 2 : -y_diff + 2));
	}
      }
      return result;
    }
  };

  class King25BothSide : public EvalComponentStagesBW
  {
  public:
    King25BothSide() : EvalComponentStagesBW(new King25BothSideFeatures)
    {
    }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;

    void featureOneBlack(const NumEffectState &state, index_list_t& out) const
    {
      King25BothSideFeatures::featuresKing<BLACK>(state, out);
    }
    void featureOneWhite(const NumEffectState &state, index_list_t& out) const
    {
      King25BothSideFeatures::featuresKing<WHITE>(state, out);
    }
  };
  
  class King25BothSideYFeatures : public FeaturesOneNonUniq
  {
  public:
    King25BothSideYFeatures() : FeaturesOneNonUniq(32 * 32 * 8 * 9) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25BothSideY"; }
    size_t maxActive() const { return 16; }
  private:
    template <Player P>
    static int index(Square king, int effect1, int effect2, int i)
    {
      const int king_y = (P == BLACK ? king.y() : 10 - king.y());
      assert(0 <= effect1 && effect1 < 32);
      assert(0 <= effect2 && effect2 < 32);
      return king_y - 1 + 9 *(effect1 + 32 * (effect2 + 32 * i));
    }
  public:
    template <Player P>
    static void featuresKing(const NumEffectState &state,
		      IndexCacheI<MaxActiveWithDuplication> &diffs);
  private:
    template <Player Defense>
    static int effectVertical(const NumEffectState &state, int index)
    {
      Square king = state.kingSquare<Defense>();
      int result = 0;
      const int x = king.x() + (Defense == BLACK ? -2 + index : 2 - index);
      for (int y_diff = -2; y_diff <= 2; ++y_diff)
      {
	const int y = king.y() + y_diff;
	if (x < 1 || x > 9 || y < 1 || y > 9 ||
	    state.hasEffectAt<alt(Defense)>(Square(x, y)))
	{
	  result |= (1 << (Defense == BLACK ? y_diff + 2 : -y_diff + 2));
	}
      }
      return result;
    }
  };

  class King25BothSideY : public EvalComponentStagesBW
  {
  public:
    King25BothSideY() : EvalComponentStagesBW(new King25BothSideYFeatures)
    {
    }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;

    void featureOneBlack(const NumEffectState &state, index_list_t& out) const
    {
      King25BothSideYFeatures::featuresKing<BLACK>(state, out);
    }
    void featureOneWhite(const NumEffectState &state, index_list_t& out) const
    {
      King25BothSideYFeatures::featuresKing<WHITE>(state, out);
    }
  };

  class King25BothSideXFeatures : public FeaturesOneNonUniq
  {
  public:
    King25BothSideXFeatures() : FeaturesOneNonUniq(32 * 32 * 8 * 5) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25BothSideX"; }
    size_t maxActive() const { return 16; }
  private:
    template <Player P>
    static int index(Square king, int effect1, int effect2, int i, int j)
    {
      const int king_x = (king.x() >= 6 ? 10 - king.x() : king.x());
      if ((P == BLACK && king.x() > 5) ||
	  (P == WHITE && king.x() < 5))
      {
	const int tmp = effect1;
	effect1 = effect2;
	effect2 = tmp;
	const int tmp2 = i;
	i = 4 - j;
	j = 4 - tmp2;
      }
      if (i == 2)
	--j;
      const int combination = (i * 3 + j - 2);
      assert(0 <= effect1 && effect1 < 32);
      assert(0 <= effect2 && effect2 < 32);
      return king_x - 1 + 5 * (effect1 + 32 * (effect2 + 32 * combination));
    }
  public:
    template <Player P>
    static void featuresKing(const NumEffectState &state,
			     IndexCacheI<MaxActiveWithDuplication> &diffs);
  private:
    template <Player Defense>
    static int effectVertical(const NumEffectState &state, int index)
    {
      Square king = state.kingSquare<Defense>();
      int result = 0;
      const int x = king.x() + (Defense == BLACK ? -2 + index : 2 - index);
      for (int y_diff = -2; y_diff <= 2; ++y_diff)
      {
	const int y = king.y() + y_diff;
	if (x < 1 || x > 9 || y < 1 || y > 9 ||
	    state.hasEffectAt<alt(Defense)>(Square(x, y)))
	{
	  result |= (1 << (Defense == BLACK ? y_diff + 2 : -y_diff + 2));
	}
      }
      return result;
    }
  };

  class King25BothSideX : public EvalComponentStagesBW
  {
  public:
    King25BothSideX() : EvalComponentStagesBW(new King25BothSideXFeatures)
    {
    }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;

    void featureOneBlack(const NumEffectState &state, index_list_t& out) const
    {
      King25BothSideXFeatures::featuresKing<BLACK>(state, out);
    }
    void featureOneWhite(const NumEffectState &state, index_list_t& out) const
    {
      King25BothSideXFeatures::featuresKing<WHITE>(state, out);
    }
  };

  template <Ptype MajorPiece>
  class KingMobilityWithMajor : public FeaturesOneNonUniq
  {
  private:
    template<Player P, Direction Dir>
    int index(Square king, Square end, int dir) const
    {
      const int king_x = (king.x() > 5 ? 10 - king.x() : king.x()) - 1;
      const int king_y = (P == BLACK ? king.y() : 10 - king.y()) - 1;
      const int mobility = ((Dir == L || Dir == R) ?
			    std::abs(end.x() - king.x()) :
			    std::abs(end.y() - king.y())) - 1;
      return king_x + 5 * (king_y + 9 * (dir + 8 * mobility));
    }
    template <Player P>
    void featuresKing(const NumEffectState &state,
		      IndexCacheI<MaxActiveWithDuplication> &diffs) const;
  public:
    // Mobility, Dir, king_x, king_y
    KingMobilityWithMajor() : FeaturesOneNonUniq(9 * 8 * 5 * 9) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "KingMobilityWithMajor"; }
    size_t maxActive() const { return 16; }
  };

  class KingMobilityWithRook : public KingMobilityWithMajor<ROOK>
  {
  public:
    const std::string name() const { return "KingMobilityWithRook"; }
  };

  class KingMobilityWithBishop : public KingMobilityWithMajor<BISHOP>
  {
  public:
    const std::string name() const { return "KingMobilityWithBishop"; }
  };

  class King25KingMobilityFeatures : public FeaturesOneNonUniq
  {
  public:
    King25KingMobilityFeatures() : FeaturesOneNonUniq(32 * 32 * 4) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25KingMobility"; }
    size_t maxActive() const { return 8; }
  private:
    static int index(int effect1, int effect2, int i)
    {
      assert(0 <= effect1 && effect1 < 32);
      assert(0 <= effect2 && effect2 < 32);
      return effect1 + 32 * (effect2 + 32 * i);
    }
  public:
    template <Player P>
    static void featuresKing(const NumEffectState &state,
			     IndexCacheI<MaxActiveWithDuplication> &diffs);
  private:
    template <Player Defense>
    static int effectVertical(const NumEffectState &state, int index)
    {
      Square king = state.kingSquare<Defense>();
      int result = 0;
      const int x = king.x() + (Defense == BLACK ? -2 + index : 2 - index);
      for (int y_diff = -2; y_diff <= 2; ++y_diff)
      {
	const int y = king.y() + y_diff;
	if (x >= 1 && x <= 9 && y >= 1 && y <= 9)
	{
	  const Square pos(x, y);
	  const Piece piece = state.pieceAt(pos);
	  if (!state.hasEffectAt<alt(Defense)>(pos) &&
	      (piece.isEmpty() || piece.owner() != Defense))
	  result |= (1 << (Defense == BLACK ? y_diff + 2 : -y_diff + 2));
	}
      }
      return result;
    }
  };

  class King25KingMobility : public EvalComponentStagesBW
  {
  public:
    King25KingMobility() : EvalComponentStagesBW(new King25KingMobilityFeatures())
    {
    }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;
    void featureOneBlack(const NumEffectState &state, index_list_t& out) const
    {
      King25KingMobilityFeatures::featuresKing<BLACK>(state, out);
    }
    void featureOneWhite(const NumEffectState &state, index_list_t& out) const
    {
      King25KingMobilityFeatures::featuresKing<WHITE>(state, out);
    }
  };

  class King25KingMobilityYFeatures : public FeaturesOneNonUniq
  {
  public:
    King25KingMobilityYFeatures() : FeaturesOneNonUniq(32 * 32 * 4 * 9) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25KingMobilityY"; }
    size_t maxActive() const { return 8; }
  private:
    template <Player Defense>
    static int index(Square king, int effect1, int effect2, int i)
    {
      const int king_y = (Defense == BLACK ? king.y() : 10 - king.y());
      assert(0 <= effect1 && effect1 < 32);
      assert(0 <= effect2 && effect2 < 32);
      return king_y - 1 + 9 * (effect1 + 32 * (effect2 + 32 * i));
    }
  public:
    template <Player P>
    static void featuresKing(const NumEffectState &state,
		      IndexCacheI<MaxActiveWithDuplication> &diffs);
  private:
    template <Player Defense>
    static int effectVertical(const NumEffectState &state, int index)
    {
      Square king = state.kingSquare<Defense>();
      int result = 0;
      const int x = king.x() + (Defense == BLACK ? -2 + index : 2 - index);
      for (int y_diff = -2; y_diff <= 2; ++y_diff)
      {
	const int y = king.y() + y_diff;
	if (x >= 1 && x <= 9 && y >= 1 && y <= 9)
	{
	  const Square pos(x, y);
	  const Piece piece = state.pieceAt(pos);
	  if (!state.hasEffectAt<alt(Defense)>(pos) &&
	      (piece.isEmpty() || piece.owner() != Defense))
	  result |= (1 << (Defense == BLACK ? y_diff + 2 : -y_diff + 2));
	}
      }
      return result;
    }
  };

  class King25KingMobilityY : public EvalComponentStagesBW
  {
  public:
    King25KingMobilityY() : EvalComponentStagesBW(new King25KingMobilityYFeatures())
    {
    }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;
    void featureOneBlack(const NumEffectState &state, index_list_t& out) const
    {
      King25KingMobilityYFeatures::featuresKing<BLACK>(state, out);
    }
    void featureOneWhite(const NumEffectState &state, index_list_t& out) const
    {
      King25KingMobilityYFeatures::featuresKing<WHITE>(state, out);
    }
  };

  class King25KingMobilityXFeatures : public FeaturesOneNonUniq
  {
  public:
    King25KingMobilityXFeatures() : FeaturesOneNonUniq(32 * 32 * 4 * 5) { }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25KingMobilityX"; }
    size_t maxActive() const { return 8; }
  private:
    template <Player Defense>
    static int index(Square king, int effect1, int effect2, int i)
    {
      const int king_x = (king.x() > 5 ? 10 - king.x() : king.x());
      if ((Defense == BLACK && king.x() > 5) ||
	  (Defense == WHITE && king.x() < 5))
      {
	const int tmp = effect1;
	effect1 = effect2;
	effect2 = tmp;
	i = 3 - i;
      }
      assert(0 <= effect1 && effect1 < 32);
      assert(0 <= effect2 && effect2 < 32);
      return king_x - 1 + 5 * (effect1 + 32 * (effect2 + 32 * i));
    }
  public:
    template <Player P>
    static void featuresKing(const NumEffectState &state,
			     IndexCacheI<MaxActiveWithDuplication> &diffs);
  private:
    template <Player Defense>
    static int effectVertical(const NumEffectState &state, int index)
    {
      Square king = state.kingSquare<Defense>();
      int result = 0;
      const int x = king.x() + (Defense == BLACK ? -2 + index : 2 - index);
      for (int y_diff = -2; y_diff <= 2; ++y_diff)
      {
	const int y = king.y() + y_diff;
	if (x >= 1 && x <= 9 && y >= 1 && y <= 9)
	{
	  const Square pos(x, y);
	  const Piece piece = state.pieceAt(pos);
	  if (!state.hasEffectAt<alt(Defense)>(pos) &&
	      (piece.isEmpty() || piece.owner() != Defense))
	  result |= (1 << (Defense == BLACK ? y_diff + 2 : -y_diff + 2));
	}
      }
      return result;
    }
  };

  class King25KingMobilityX : public EvalComponentStagesBW
  {
  public:
    King25KingMobilityX() : EvalComponentStagesBW(new King25KingMobilityXFeatures())
    {
    }
    MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const;

    void featureOneBlack(const NumEffectState &state, index_list_t& out) const
    {
      King25KingMobilityXFeatures::featuresKing<BLACK>(state, out);
    }
    void featureOneWhite(const NumEffectState &state, index_list_t& out) const
    {
      King25KingMobilityXFeatures::featuresKing<WHITE>(state, out);
    }
  };
  
  class King25Effect3 : public FeaturesOneNonUniq
  {
    int index(int piece_count, bool with_knight,
	      int stand_count, bool with_knight_on_stand,
	      int attacked_count) const
    {
      return (piece_count + 10 *
	      ((with_knight ? 1 : 0) + 2 *
	       (stand_count + 10 * ((with_knight_on_stand ? 1 : 0) +
				    2 * attacked_count))));
    }
  public:
    // piece count, with knight, stand HI KA KI GI count, with knight on stand,
    // HI KA KI GI attacked not from attacking pieces
    King25Effect3() : FeaturesOneNonUniq(10 * 2 * 10 * 2 * 6) { }
    size_t maxActive() const { return 2; }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25Effect3"; };
    template <Player Attack>
    static void countPieces(
      const osl::NumEffectState &state,
      int &piece_count,
      bool &with_knight,
      int &stand_count,
      bool &stand_with_knight,
      int &attacked_count)
    {
      const Player Defense = alt(Attack);
      const Square king = state.kingSquare<Defense>();
      const int min_x = std::max(1, king.x() - 2);
      const int max_x = std::min(9, king.x() + 2);
      const int min_y = std::max(1, king.y() - 2);
      const int max_y = std::min(9, king.y() + 2);

      PieceMask mask;
      for (int y = min_y; y <= max_y; ++y)
      {
	for (int x = min_x; x <= max_x; ++x)
	{
	  const Square target(x, y);
	  mask = mask | state.effectSetAt(target);
	}
      }
      mask = mask & state.piecesOnBoard(Attack);
      const PieceMask promoted_mask = (mask & state.promotedPieces());
      with_knight = (mask & ~state.promotedPieces()).selectBit<KNIGHT>().any();
      mask.clearBit<KNIGHT>();
      mask.clearBit<LANCE>();
      mask.clearBit<PAWN>();
      mask = mask | promoted_mask;
      piece_count = std::min(9,  mask.countBit());
      stand_count = std::min(9,
			     state.countPiecesOnStand<ROOK>(Attack) +
			     state.countPiecesOnStand<BISHOP>(Attack) +
			     state.countPiecesOnStand<GOLD>(Attack) +
			     state.countPiecesOnStand<SILVER>(Attack));
      stand_with_knight = state.hasPieceOnStand<KNIGHT>(Attack);
      // mask without small pieces is better or mask with small pieces
      // is better?
      PieceMask attacked =
	state.effectedMask(Attack) & state.piecesOnBoard(Defense);
      attacked.clearBit<KNIGHT>();
      attacked.clearBit<LANCE>();
      attacked.clearBit<PAWN>();
      PieceMask attacking;
      while (attacked.any())
      {
	const Piece piece = state.pieceOf(attacked.takeOneBit());
	attacking = attacking | state.effectSetAt(piece.square());
      }
      attacking = (attacking & state.piecesOnBoard(Attack) & ~mask);
      attacked_count = std::min(5, attacking.countBit());
    }
  };
  class King25Effect3Y : public FeaturesOneNonUniq
  {
    int index(int piece_count, bool with_knight,
	      int stand_count, bool with_knight_on_stand,
	      int attacked_count, int king_y) const
    {
      return (piece_count + 10 *
	      ((with_knight ? 1 : 0) + 2 *
	       (stand_count + 10 * ((with_knight_on_stand ? 1 : 0) +
				    2 * attacked_count)))) * 9 + king_y - 1;
    }
  public:
    // piece count, with knight, stand HI KA KI GI count, with knight on stand,
    // HI KA KI GI attacked not from attacking pieces
    King25Effect3Y() : FeaturesOneNonUniq(10 * 2 * 10 * 2 * 6 * 9) { }
    size_t maxActive() const { return 2; }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25Effect3Y"; };
    template <Player Attack>
    static void countPieces(
      const osl::NumEffectState &state,
      int &piece_count,
      bool &with_knight,
      int &stand_count,
      bool &stand_with_knight,
      int &attacked_count)
    {
      const Player Defense = alt(Attack);
      const Square king = state.kingSquare<Defense>();
      const int min_x = std::max(1, king.x() - 2);
      const int max_x = std::min(9, king.x() + 2);
      const int min_y = std::max(1, king.y() - 2);
      const int max_y = std::min(9, king.y() + 2);

      PieceMask mask;
      for (int y = min_y; y <= max_y; ++y)
      {
	for (int x = min_x; x <= max_x; ++x)
	{
	  const Square target(x, y);
	  mask = mask | state.effectSetAt(target);
	}
      }
      mask = mask & state.piecesOnBoard(Attack);
      const PieceMask promoted_mask = (mask & state.promotedPieces());
      with_knight = (mask & ~state.promotedPieces()).selectBit<KNIGHT>().any();
      mask.clearBit<KNIGHT>();
      mask.clearBit<LANCE>();
      mask.clearBit<PAWN>();
      mask = mask | promoted_mask;
      piece_count = std::min(9,  mask.countBit());
      stand_count = std::min(9,
			     state.countPiecesOnStand<ROOK>(Attack) +
			     state.countPiecesOnStand<BISHOP>(Attack) +
			     state.countPiecesOnStand<GOLD>(Attack) +
			     state.countPiecesOnStand<SILVER>(Attack));
      stand_with_knight = state.hasPieceOnStand<KNIGHT>(Attack);
      // mask without small pieces is better or mask with small pieces
      // is better?
      PieceMask attacked =
	state.effectedMask(Attack) & state.piecesOnBoard(Defense);
      attacked.clearBit<KNIGHT>();
      attacked.clearBit<LANCE>();
      attacked.clearBit<PAWN>();
      PieceMask attacking;
      while (attacked.any())
      {
	const Piece piece = state.pieceOf(attacked.takeOneBit());
	attacking = attacking | state.effectSetAt(piece.square());
      }
      attacking = (attacking & state.piecesOnBoard(Attack) & ~mask);
      attacked_count = std::min(5, attacking.countBit());
    }
  };

  class King25EffectCountCombination : public FeaturesOneNonUniq
  {
    int index(int attack_count, int defense_count) const
    {
      return attack_count + 10 * defense_count;
    }
  public:
    King25EffectCountCombination() : FeaturesOneNonUniq(10 * 10) { }
    size_t maxActive() const { return 10; }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25EffectCountCombination"; };
    template <Player Attack>
    static void countPieces(
      const osl::NumEffectState &state,
      int &attack_count,
      int &defense_count)
    {
      const Player Defense = alt(Attack);
      const Square king = state.kingSquare<Defense>();
      const int min_x = std::max(1, king.x() - 2);
      const int max_x = std::min(9, king.x() + 2);
      const int min_y = std::max(1, king.y() - 2);
      const int max_y = std::min(9, king.y() + 2);

      PieceMask mask;
      for (int y = min_y; y <= max_y; ++y)
      {
	for (int x = min_x; x <= max_x; ++x)
	{
	  const Square target(x, y);
	  mask = mask | state.effectSetAt(target);
	}
      }
      PieceMask attack_mask = mask & state.piecesOnBoard(Attack);
      PieceMask defense_mask = mask & state.piecesOnBoard(Defense);
      const PieceMask attack_promoted_mask = (attack_mask & state.promotedPieces());
      attack_mask.clearBit<KNIGHT>();
      attack_mask.clearBit<LANCE>();
      attack_mask.clearBit<PAWN>();
      attack_mask = attack_mask | attack_promoted_mask;
      attack_count = std::min(9, attack_mask.countBit() +
			      state.countPiecesOnStand<ROOK>(Attack) +
			      state.countPiecesOnStand<BISHOP>(Attack) +
			      state.countPiecesOnStand<GOLD>(Attack) +
			      state.countPiecesOnStand<SILVER>(Attack));
      const PieceMask defense_promoted_mask = (defense_mask & state.promotedPieces());
      defense_mask.clearBit<KNIGHT>();
      defense_mask.clearBit<LANCE>();
      defense_mask.clearBit<PAWN>();
      defense_mask = defense_mask | defense_promoted_mask;
      defense_count = std::min(9, defense_mask.countBit() +
			       state.countPiecesOnStand<ROOK>(Defense) +
			       state.countPiecesOnStand<BISHOP>(Defense) +
			       state.countPiecesOnStand<GOLD>(Defense) +
			       state.countPiecesOnStand<SILVER>(Defense));
    }
    const std::string describe(size_t local_index) const;
  };

  class King25EffectCountCombinationY : public FeaturesOneNonUniq
  {
    int index(int attack_count, int defense_count, int y) const
    {
      return y - 1 + 9 * (attack_count + 10 * defense_count);
    }
  public:
    King25EffectCountCombinationY() : FeaturesOneNonUniq(10 * 10 * 9) { }
    size_t maxActive() const { return 100; }
    void featuresOneNonUniq(const NumEffectState &,
			    IndexCacheI<MaxActiveWithDuplication> &) const;
    const std::string name() const { return "King25EffectCountCombinationY"; };
  };

  class BishopExchangeSilverKing : public EvalComponent
  {
    static int indexKing(Square king) 
    {
      const int y = king.y();
      if (y >= 3)
	return -1;
      return (y-1)*9 + king.x()-1;
    }
    static int indexRook(Square rook) 
    {
      assert(rook.isOnBoard());
      const int y = rook.y();
      if (y >= 6)
	return -1;
      return (y-1)*9 + rook.x()-1;
    }
    static int indexSilver(Square silver) 
    {
      return (silver.y()-1)*9 + silver.x()-1;
    }
  public:
    enum { BISHOP_ONE_DIM = 18 * 81 * (45*2), DIM = BISHOP_ONE_DIM*3 };
    BishopExchangeSilverKing() : EvalComponent(DIM) { }
    size_t maxActive() const { return 4; }
    int eval(const NumEffectState& state) const;
    void featuresNonUniq(const NumEffectState& state, index_list_t& out, int offset) const;
    const std::string name() const { return "BishopExchangeSilverKing"; };
  private:
    void addOne(Player king_owner, const NumEffectState &state,
		IndexCacheI<MaxActiveWithDuplication> &diffs, int offset) const;
  };

  class EnterKingDefense : public EvalComponent
  {
  public:
    enum { DIM = (8+8+8+8)*3 };
    EnterKingDefense() : EvalComponent(DIM) { }
    int eval(const NumEffectState& state) const;
    void featuresNonUniq(const NumEffectState& state, index_list_t& out, int offset) const;
    const std::string name() const { return "EnterKingDefense"; };
  private:
    void addOne(Player king_owner, const NumEffectState &state,
		IndexCacheI<MaxActiveWithDuplication> &diffs, int offset) const;
  };

}


#endif /* _KINGEVAL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
