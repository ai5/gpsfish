/* newProgress.h */
#ifndef PROGRESS_EXPERIMENTAL_NEW_PROGRESS_H
#define PROGRESS_EXPERIMENTAL_NEW_PROGRESS_H

#include "osl/numEffectState.h"
#include "osl/eval/midgame.h"
#include "osl/container.h"
#include <cstdlib>
namespace osl
{
  namespace progress
  {
    template <int N>
    class ProgressN
    {
      int progress;
    public:
      explicit ProgressN(int value=0) : progress(value)
      {
	assert(isValid());
      }
      int value() const { return progress; }
      bool isValid() const { 
	return (progress >= 0) && (progress < N);
      }
    };
    template <int N>
    inline bool operator==(ProgressN<N> l, ProgressN<N> r)
    {
      return l.value() == r.value();
    }
    template <int N>
    inline bool operator!=(ProgressN<N> l, ProgressN<N> r)
    {
      return ! (l == r);
    }
    template <int N>
    inline bool operator<(ProgressN<N> l, ProgressN<N> r)
    {
      return l.value() < r.value();
    }
    typedef ProgressN<16> Progress16;
    typedef ProgressN<32> Progress32;

    namespace ml
    {
      struct NewProgressDebugInfo
      {
        enum Feature
        {
          ATTACK_5X3,
          DEFENSE_5X3,
          ATTACK5X5,
          STAND,
          EFFECT5X5,
          KING_RELATIVE_ATTACK,
          KING_RELATIVE_DEFENSE,
          NON_PAWN_ATTACKED_PAIR,
          FEATURE_LIMIT
        };
        CArray<int, FEATURE_LIMIT> black_values;
        CArray<int, FEATURE_LIMIT> white_values;
      };

      struct NewProgressData
      {
	CArray<MultiInt,2> non_pawn_ptype_attacked_pair_eval;
	MultiInt promotion37_eval;
	CArray<int, 2> progresses, attack5x5_progresses, stand_progresses,
	  effect_progresses, defenses;
	CArray<int, 2> rook, bishop, gold, silver, promoted,
	  king_relative_attack, king_relative_defense, non_pawn_ptype_attacked_pair;
	int pawn_facing, promotion37, piecestand7;
      };
      class NewProgress : private NewProgressData
      {
      public:
	enum { ProgressScale = 2 };
      private:
	static bool initialized_flag;
	static CArray<int, Piece::SIZE> stand_weight;
	static CArray<int, 1125> attack5x5_weight;
	static CArray<int, 5625> attack5x5_x_weight;
	static CArray<int, 10125> attack5x5_y_weight;
	static CArray<int, 75> effectstate_weight;
	static CArray<int, 81*15*10> attack_relative;
	static CArray<int, 81*15*10> defense_relative;
	static CArray<int, 4284> king_relative_weight;
	static CArray<int, 262144> attacked_ptype_pair_weight;
	static CArray<int, 10> pawn_facing_weight;
	static CArray<int, 16> promotion37_weight;
	static CArray<int, 56> piecestand7_weight;
	static int max_progress;
	void updatePieceKingRelativeBonus(const NumEffectState &state);
	void updateNonPawnAttackedPtypePair(const NumEffectState& state);
	template <Player Owner>
	void updateNonPawnAttackedPtypePairOne(const NumEffectState& state);
	void updatePawnFacing(const NumEffectState& state);
	template <Player Attack>	
	void promotion37One(const NumEffectState& state, int rank);
	void updatePromotion37(const NumEffectState& state);
	void updatePieceStand7(const NumEffectState& state);
	template <Player P>
	static void progressOne(const NumEffectState &state,
				int &attack, int &defense);
	template <Player P>
	void updateAttack5x5PiecesAndState(const NumEffectState &state);
	template <Player P>
	void updateAttack5x5Pieces(PieceMask, const NumEffectState&);
	template <Player P>
	int attack5x5Value(const NumEffectState &state) const;
	template <Player P>
	static int index(Square king, Square target)
	{
	  const int x_diff = std::abs(king.x() - target.x()); // [0, 4]
	  const int y_diff = (P == BLACK ? king.y() - target.y() :
			      target.y() - king.y()) + 2; // [-2, 2] + 2
	  return x_diff * 5 + y_diff;
	}
	template <Player P>
	static int indexX(Square king, Square target)
	{
	  int target_x = (king.x() > 5 ? 10 - king.x() : king.x()); // [1, 5]
	  int x_diff = king.x() - target.x(); // [-4, 4]
	  if (P == BLACK && king.x() >= 6)
	  {
	    x_diff = -x_diff;
	  }
	  else if (P == WHITE && king.x() >= 5)
	  {
	    x_diff = -x_diff;
	  }
	  const int y_diff = (P == BLACK ? king.y() - target.y() :
			      target.y() - king.y()) + 2; // [-2, 2] + 2
	  return ((x_diff + 4) * 5 + y_diff) * 5 + target_x - 1;
	}
	template <Player P>
	static int indexY(Square king, Square target)
	{
	  const int x_diff = std::abs(king.x() - target.x()); // [0, 4]
	  const int y_diff = (P == BLACK ? king.y() - target.y() :
			      target.y() - king.y()) + 2; // [-2, 2] + 2
	  const int king_y = (P == BLACK ? king.y() : 10 - king.y()); // [1, 9]
	  return (x_diff * 5 + y_diff) * 9 + king_y - 1;
	}
	static int index5x5(int rook, int bishop, int gold, int silver,
			    int promoted)
	{
	  assert(0 <= promoted && promoted <= 4);
	  return promoted + 5 * (silver + 5 * (gold + 5 * (bishop + 3 * rook)));
	}
	static int index5x5x(int rook, int bishop, int gold, int silver,
			     int promoted, int king_x)
	{
	  assert(0 <= promoted && promoted <= 4);
	  return king_x - 1 +
	    5 * (promoted + 5 * (silver + 5 * (gold + 5 * (bishop + 3 * rook))));
	}
	static int index5x5y(int rook, int bishop, int gold, int silver,
			     int promoted, int king_y)
	{
	  assert(0 <= promoted && promoted <= 4);
	  return king_y - 1 +
	    9 * (promoted + 5 * (silver + 5 * (gold + 5 * (bishop + 3 * rook))));
	}
	template <Player P>
	static int indexPerEffect(Square king, Square target,
				  int count)
	{
	  const int x_diff = std::abs(king.x() - target.x()); // [0, 4]
	  const int y_diff = (P == BLACK ? king.y() - target.y() :
			      target.y() - king.y()) + 2; // [-2, 2] + 2
	  return x_diff * 5 + y_diff + std::min(8, count) * 25;
	}

	template <Player P>
	static int indexPerEffectY(Square king, Square target,
				   int count)
	{
	  const int king_y = (P == BLACK ? king.y() : 10 - king.y());
	  const int x_diff = std::abs(king.x() - target.x()); // [0, 4]
	  const int y_diff = (P == BLACK ? king.y() - target.y() :
			      target.y() - king.y()) + 2; // [-2, 2] + 2
	  return king_y - 1 + 9 * (x_diff * 5 + y_diff + std::min(8, count) * 25);
	}
	template <Player P>
	static int indexPerEffectX(Square king, Square target,
				   int count)
	{
	  const int king_x = (king.x() > 5 ? 10 - king.x() : king.x());
	  int x_diff = king.x() - target.x(); // [-4, 4]
	  if ((P == BLACK && (king.x() > 5)) ||
	      (P == WHITE && (king.x() >= 5)))
	    x_diff = -x_diff;
	  const int y_diff = (P == BLACK ? king.y() - target.y() :
			      target.y() - king.y()) + 2; // [-2, 2] + 2
	  return king_x - 1 + 5 * (x_diff + 4 +
				   9 * (y_diff + 5 *  std::min(8, count)));
	}
	template <Player P>
	static int indexRelative(const Square king,
				 const Ptype ptype, const Square pos)
	{
	  const int x = std::abs(pos.x() - king.x());
	  const int y = (king.y() - pos.y()) *
	    (P == osl::BLACK ? 1 : -1) + 8;
	  return (ptype - osl::PTYPE_PIECE_MIN) * 17 * 9 + (x * 17 + y);
	}
	static int indexRelative(const Player player, const Square king,
				 const Piece piece)
	{
	  if (player == BLACK)
	  {
	    return indexRelative<BLACK>(king, piece.ptype(),
					piece.square());
	  }
	  else
	  {
	    return indexRelative<WHITE>(king, piece.ptype(),
					piece.square());
	  }
	}
      public:
	NewProgress(const NumEffectState &state);
	int progress() const
	{
	  return
	    std::max(std::min(progresses[0] + progresses[1] +
			      attack5x5_progresses[0] +
			      attack5x5_progresses[1] +
			      stand_progresses[0] + stand_progresses[1] +
			      effect_progresses[0] + effect_progresses[1] +
			      defenses[0] + defenses[1] +
			      king_relative_attack[0] +
			      king_relative_attack[1] +
			      king_relative_defense[0] +
			      king_relative_defense[1] +
			      non_pawn_ptype_attacked_pair[0] +
			      non_pawn_ptype_attacked_pair[1] +
			      pawn_facing + promotion37 + piecestand7,
			      max_progress-ProgressScale), 0) / ProgressScale;
	}
	static int maxProgress() { return max_progress / ProgressScale; }
	template<Player P>
	void updateSub(const NumEffectState &new_state, Move last_move);
	void update(const NumEffectState &new_state, Move last_move){
	  if(new_state.turn()==BLACK)
	    updateSub<WHITE>(new_state,last_move);
	  else
	    updateSub<BLACK>(new_state,last_move);
	}
        NewProgressDebugInfo debugInfo() const;
      private:
	template<Player P>
	void updateMain(const NumEffectState &new_state, Move last_move);
      public:
	const Progress16 progress16() const
	{
	  return Progress16(16 * progress() / maxProgress());
	}
	const Progress16 progress16(Player p) const
	{
	  assert(maxProgress() > 0);
	  return Progress16(
	    16 * std::max(
	      std::min(progresses[playerToIndex(alt(p))] +
		       attack5x5_progresses[playerToIndex(alt(p))] +
		       stand_progresses[playerToIndex(alt(p))] +
		       effect_progresses[playerToIndex(alt(p))] +
		       defenses[playerToIndex(alt(p))] +
		       king_relative_attack[playerToIndex(alt(p))] +
		       king_relative_defense[playerToIndex(p)] +
		       non_pawn_ptype_attacked_pair[p],
		       max_progress-ProgressScale), 0)
	    / ProgressScale / maxProgress());
	}
	// p == attack player, alt(p) == king owner
	const Progress16 progressAttack(Player p) const
	{
	  assert(maxProgress() > 0);
	  return Progress16(
	    8 * std::max(
	      std::min(progresses[alt(p)] +
		       attack5x5_progresses[alt(p)] +
		       stand_progresses[alt(p)] +
		       effect_progresses[alt(p)] +
		       king_relative_attack[alt(p)],
		       max_progress-ProgressScale), -max_progress+ProgressScale)
	    / ProgressScale / maxProgress() + 8);
	}
	// p == king owner (defense player)
	const Progress16 progressDefense(Player p) const
	{
	  assert(maxProgress() > 0);
	  return Progress16(
	    8 * std::max(
	      std::min(defenses[alt(p)] +
		       king_relative_defense[p] +
		       non_pawn_ptype_attacked_pair[p],
		       max_progress-ProgressScale),
	      -max_progress + ProgressScale)
	    / ProgressScale / maxProgress() + 8);
	}
	static bool initialized()
	{
	  return initialized_flag;
	}
	static bool setUp(const char *filename);
	static bool setUp();
	static std::string defaultFilename();
	const NewProgressData rawData() const { return *this; }
      };
      bool operator==(const NewProgressData& l, const NewProgressData& r);
      inline bool operator==(const NewProgress& l, const NewProgress& r) 
      {
	return l.rawData() == r.rawData();
      }
    }
    using ml::NewProgress;
  }
  using progress::Progress16;
  using progress::Progress32;
  using progress::NewProgress;
}

#endif // PROGRESS_EXPERIMENTAL_NEW_PROGRESS_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
