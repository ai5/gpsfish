/* openMidEndingEval.h
 */

#ifndef EVAL_ML_OPENMIDENDINGEVAL_H
#define EVAL_ML_OPENMIDENDINGEVAL_H

#include "osl/eval/weights.h"
#include "osl/eval/ptypeEval.h"
#include "osl/eval/evalStagePair.h"
#include "osl/eval/evalTraits.h"
#include "osl/eval/ptypeEval.h"
#include "osl/numEffectState.h"
#include "osl/progress.h"
#include "osl/bits/align16New.h"
#include "osl/oslConfig.h"
#include <cstring>

#define USE_TEST_PROGRESS
// NewProgressが学習可能な場合に定義 (現在はosl側に変更はないので常に定義)
#define LEARN_TEST_PROGRESS

namespace osl
{
  namespace eval
  {
    namespace ml
    {
      using namespace osl::progress::ml;
      class OpenMidEndingPtypeTable : public PtypeEvalTable
      {
      public:
	OpenMidEndingPtypeTable();
      };

      struct OpenMidEndingEvalDebugInfo
      {
        enum StageFeature
        {
          KING_PIECE_RELATIVE,
          PIECE_STAND,
          KING25_EFFECT_EACH,
          PTYPEX,
          PTYPEY,
          ROOK_MOBILITY,
          BISHOP_MOBILITY,
          LANCE_MOBILITY,
          ROOK_EFFECT,
          BISHOP_EFFECT,
          PIECE_STAND_COMBINATION,
          PIECE_STAND_TURN,
          ROOK_PAWN,
          PAWN_DROP,
          PIECE_STAND_Y,
          KNIGHT_CHECK,
          PAWN_ADVANCE,
          PAWN_PTYPEO,
          PROMOTED_MINOR_PIECE,
          KING_PIECE_RELATIVE_NOSUPPORT,
          NON_PAWN_ATTACKED,
          NON_PAWN_ATTACKED_PTYPE,
          PTYPE_YY,
          KING3PIECES,
          BISHOP_HEAD,
          KNIGHT_HEAD,
          ROOK_PROMOTE_DEFENSE,
          PTYPE_COUNT,
          LANCE_EFFECT_PIECE,
          PTYPE_Y_PAWN_Y,
          BISHOP_AND_KING,
          PIECE_FORK_TURN,
          ROOK_SILVER_KNIGHT,
          BISHOP_SILVER_KNIGHT,
          KING25_EFFECT_SUPPORTED,
          KING_ROOK_BISHOP,
          KING_X_BLOCKED3,
          GOLD_RETREAT,
          SILVER_RETREAT,
          ALL_GOLD,
          ALL_MAJOR,
          KING25_EFFECT_DEFENSE,
          ANAGUMA_EMPTY,
          NO_PAWN_ON_STAND,
          NON_PAWN_PIECE_STAND,
          PIN_PTYPE_ALL,
          KING_MOBILITY,
          GOLD_AND_SILVER_NEAR_KING,
          PTYPE_COMBINATION,
          KING25_BOTH_SIDE,
          KING25_MOBILITY,
          BISHOP_STAND_FILE5,
          MAJOR_CHECK_WITH_CAPTURE,
          SILVER_ADVANCE26,
          KING25_EFFECT3,
          BISHOP_BISHOP_PIECE,
          ROOK_ROOK,
          ROOK_ROOK_PIECE,
          KING25_EFFECT_COUNT_COMBINATION,
          NON_PAWN_ATTACKED_PTYPE_PAIR,
          ATTACK_MAJORS_IN_BASE,
          STAGE_FEATURE_LIMIT
        };
        enum ProgressIndependentFeature
        {
          PIECE,
          BISHOP_EXCHANGE_SILVER_KING, // recalculated_value
          ENTER_KING_DEFENSE,
          KING25_EFFECT_ATTACK, // end recalculated_value
          PIECE_PAIR,
          PIECE_PAIR_KING,
          PROGRESS_INDEPENDENT_FEATURE_LIMIT
        };
        int value;
        int progress;
        CArray<int, PROGRESS_INDEPENDENT_FEATURE_LIMIT> progress_independent_values;
        CArray<MultiInt, STAGE_FEATURE_LIMIT> stage_values;

	static const char *name(ProgressIndependentFeature);
	static const char *name(StageFeature);
      };

      class OpenMidEndingEval
#if OSL_WORDSIZE == 32
	: public misc::Align16New
#endif
      {
      private:
	enum { INVALID=EvalTraits<BLACK>::MAX_VALUE+1 };
	enum {
	  /** one should attack king after when he captured almost all pieces */
	  ProgressIndependentValueLimit = 4000
	};
	enum LoadStatus { Zero=0, Loaded, Random };
	static volatile LoadStatus initialized_flag;
	static Weights piece_pair_weights;
	typedef osl::progress::ml::NewProgress progress_t;
	progress_t progress;
	MultiIntPair kingx_blocked, king25_effect_each;
	MultiIntPair king25_both_side,king_rook_bishop;
	MultiIntPair piece_stand_turn, non_pawn_attacked,
	  non_pawn_attacked_ptype, piece_fork_turn;
	MultiInt ptypey, ptypex, king_table_value;
	MultiInt piece_stand_value, recalculated_stage_value, pawn_advance;
	MultiInt rook_mobility, bishop_mobility, lance_mobility;
	MultiInt knight_advance, pawn_drop, promoted_minor_piece, rook_pawn,
	  rook_effect, bishop_effect, bishop_head, nosupport, ptype_yy, king3pieces;
	MultiInt rook_promote_defense;
	MultiInt piece_stand_combination, piece_stand_y, knight_check,
	  knight_head, pawn_ptypeo, ptype_count_value, lance_effect_piece,
	  ptype_y_pawn_y, bishop_and_king, rook_silver_knight, bishop_silver_knight;
	CArray<BoardMask, 2> knight_fork_squares;
	CArray<PieceMask, 2> effect25; // index: owner of king
	CArray<PieceMask, 2> effect25_supported; // index: owner of king
	CArray<PieceMask, 2> effected_mask;
	CArray<PieceMask, 2> effected_mask_for_attacked;
	CArray<PieceMask, 40> attacked_mask;
	CArray<int, 5>  black_vertical, white_vertical,
	  black_king_vertical, white_king_vertical;
	// flat
	CArray<int,2> piece_pair_king_value;
	CArray<int, 2> non_pawn_stand_count;
	CArray2d<int, 2, 3> gs_near_king_count;
	CArray2d<int, 2, PTYPE_SIZE> ptype_count, ptype_board_count;
	CArray<std::pair<Square,int>, 2> knight_drop, silver_drop, bishop_drop, rook_drop;
	CArray2d<int, 2, 9> pawns;
	int progress_independent_value, // should be renamed to piece
	  recalculated_value, piece_pair_value;
	int black_pawn_count;
	int black_major_count, black_gold_count;
	int black_attack_effect, black_attack_piece,
	  white_attack_effect, white_attack_piece,
	  black_attack_supported_piece, white_attack_supported_piece;
	int black_defense_effect, black_defense_piece,
	  white_defense_effect, white_defense_piece;
	mutable int cache;
	Player turn;
	unsigned int ptypeo_mask;
	CArray<bool, 2> can_check; // king is defense
	bool use_progress_independent_value_limit;
	static const int ROUND_UP = 2;
	static int roundUp(int v)
	{
	  return v & (~(ROUND_UP-1)); 
	}
	void updateGoldSilverNearKing(const NumEffectState &state)
	{
	  const CArray<Square,2> kings = {{ 
	      state.kingSquare(BLACK),
	      state.kingSquare(WHITE),
	    }};
	  gs_near_king_count.fill(0);
	  for (int i = PtypeTraits<GOLD>::indexMin;
	       i < PtypeTraits<GOLD>::indexLimit; ++i)
	  {
	    const Piece p = state.pieceOf(i);
	    if (p.isOnBoard())
	    {
	      const Square pos = p.square();
	      const int y_diff = std::abs(pos.y() - kings[p.owner()].y());
	      const int x_diff = std::abs(pos.x() - kings[p.owner()].x());
	      if (y_diff <= 2 && x_diff <= 3)
	      {
		++gs_near_king_count[p.owner()][std::max(x_diff, y_diff) - 1];
	      }
	    }
	  }
	  for (int i = PtypeTraits<SILVER>::indexMin;
	       i < PtypeTraits<SILVER>::indexLimit; ++i)
	  {
	    const Piece p = state.pieceOf(i);
	    if (p.isOnBoard())
	    {
	      const Square pos = p.square();
	      const int y_diff = std::abs(pos.y() - kings[p.owner()].y());
	      const int x_diff = std::abs(pos.x() - kings[p.owner()].x());
	      if (y_diff <= 2 && x_diff <= 3)
	      {
		++gs_near_king_count[p.owner()][std::max(x_diff, y_diff) - 1];
	      }
	    }
	  }
	}
      public:
	explicit OpenMidEndingEval
	(const NumEffectState &state=NumEffectState(),
	 bool limit_progress_independent_value=! OslConfig::hasByoyomi());
	OpenMidEndingEval& operator=(const OpenMidEndingEval& src)
	{
	  if (this != &src)
	    memcpy(this, &src, sizeof(OpenMidEndingEval));
	  return *this;
	}
	void changeTurn() { }
	static bool initialized()
	{
	  return initialized_flag;
	}
	static bool setUp(const char *filename);
	static bool setUp();
	static std::string defaultFilename();
	int progressIndependentValue() const 
	{
	  return progress_independent_value + recalculated_value + piece_pair_value
	    + piece_pair_king_value[BLACK] + piece_pair_king_value[WHITE];
	}
	void debug() const;
	MultiInt stageValue() const 
	{
	  return king_table_value + piece_stand_value +
	    king25_effect_each[BLACK] + king25_effect_each[WHITE] +
	    ptypex + ptypey + rook_mobility + bishop_mobility + lance_mobility +
	    rook_effect + bishop_effect +
	    piece_stand_combination + piece_stand_turn[turn] +
	    rook_pawn + pawn_drop + piece_stand_y + knight_check +
	    pawn_advance + pawn_ptypeo + promoted_minor_piece +
	    nosupport +
	    non_pawn_attacked[turn] + non_pawn_attacked_ptype[turn] +
	    ptype_yy + king3pieces + bishop_head + knight_head
	    + rook_promote_defense +
	    ptype_count_value + lance_effect_piece + ptype_y_pawn_y +
	    bishop_and_king + piece_fork_turn[turn] + rook_silver_knight + bishop_silver_knight +
	    recalculated_stage_value;
	}
	int openingValue() const 
	{
	  return stageValue()[0];
	}
	int midgameValue() const 
	{
	  return stageValue()[1];
	}
	int midgame2Value() const 
	{
	  return stageValue()[2];
	}
	int endgameValue() const 
	{
	  return stageValue()[EndgameIndex];
	}
	void invalidateCache() { cache=INVALID; }
	static int progressIndependentValueAdjusted(int value, int progress,
						    int progress_max)
	{
	  if (value > ProgressIndependentValueLimit) {
	    int diff = value - ProgressIndependentValueLimit;
	    value = ProgressIndependentValueLimit
	      + diff * progress/progress_max;
	  }
	  else if (value < -ProgressIndependentValueLimit) {
	    int diff = value + ProgressIndependentValueLimit;
	    value = -ProgressIndependentValueLimit
	      + diff * progress/progress_max;
	  }
	  return value;
	}
	int composeOpenMidEndgame() const
	{
	  const int progress_max = NewProgress::maxProgress(), c = progress_max/2;
	  const int progress = this->progress.progress();
	  int progress_independent = use_progress_independent_value_limit
	    ? progressIndependentValueAdjusted
	    (progressIndependentValue(), progress, progress_max)
	    : progressIndependentValue();
	  int sum = progress_independent * progress_max;
	  if (progress < c) 
	  {
	    sum += openingValue() * 2*(c - progress);
	    sum += midgameValue() * 2*progress;
	  }
	  else 
	  {
	    sum += midgameValue() * 2*(progress_max - progress);
	    sum += endgameValue() * 2*(progress - c);
	  }
	  return sum;
	}
#ifdef EVAL_QUAD
	int composeOpenMid2Endgame() const
	{
	  const int progress_max = NewProgress::maxProgress();
	  const int progress = this->progress.progress();
	  const int c0 = progress_max/3, c1 = c0*2;
#ifndef NDEBUG
	  const int w2 = progress_max - c1;
#endif
	  assert(c0 == w2);
	  int progress_independent = use_progress_independent_value_limit
	    ? progressIndependentValueAdjusted
	    (progressIndependentValue(), progress, progress_max)
	    : progressIndependentValue();
	  int sum = progress_independent * c0;
	  const MultiInt stage_sum = stageValue();
	  if (progress < c0) 
	  {
	    sum += stage_sum[0] * (c0 - progress);
	    sum += stage_sum[1] * progress;
	  }
	  else if (progress < c1) 
	  {
	    sum += stage_sum[1] * (c1 - progress);
	    sum += stage_sum[2] * (progress-c0);
	  }
	  else 
	  {
	    sum += stage_sum[2] * (progress_max - progress);
	    sum += stage_sum[3] * (progress - c1);
	  }
	  return sum;
	}
#endif
	int value() const
	{
	  if (cache==INVALID) 
	  {
#ifdef EVAL_QUAD
	    cache = roundUp(composeOpenMid2Endgame());
#else
	    cache = roundUp(composeOpenMidEndgame());
#endif
	  }
	  return cache;
	}
	const Move suggestMove(const NumEffectState& state) const
	{
	  assert(turn == state.turn());
	  Move suggest;
	  int best_value = 0;
	  if (! rook_drop[turn].first.isPieceStand()) {
	    assert(state.hasPieceOnStand(turn, ROOK));
	    suggest = Move(rook_drop[turn].first, ROOK, turn);
	    best_value = rook_drop[turn].second;
	  }
	  assert(best_value >= 0);
	  if (bishop_drop[turn].second > best_value) {
	    assert(! bishop_drop[turn].first.isPieceStand());
	    assert(state.hasPieceOnStand(turn, BISHOP));
	    suggest = Move(bishop_drop[turn].first, BISHOP, turn);
	    best_value = bishop_drop[turn].second;
	  }
	  if (silver_drop[turn].second > best_value) {
	    assert(! silver_drop[turn].first.isPieceStand());
	    assert(state.hasPieceOnStand(turn, SILVER));
	    suggest = Move(silver_drop[turn].first, SILVER, turn);
	    best_value = silver_drop[turn].second;
	  }
	  if (knight_drop[turn].second > best_value
	      && state.hasPieceOnStand(turn, KNIGHT)) {
	    assert(! knight_drop[turn].first.isPieceStand());
	    suggest = Move(knight_drop[turn].first, KNIGHT, turn);
	    best_value = knight_drop[turn].second;
	  }
	  return suggest;
	}
	int expect(const NumEffectState &state, Move move) const;
	template<Player P>
	void updateSub(const NumEffectState &new_state, Move last_move);
	void update(const NumEffectState &new_state, Move last_move);
	const Progress32 progress32() const
	{ 
	  return Progress32(progress.progress16(BLACK).value()
			    + progress.progress16(WHITE).value()); 
	}
	const Progress16 progress16() const { return progress.progress16(); }
	int progressValue() const { return progress.progress(); }
	int progressMax() const { return progress.maxProgress(); }
      public:
	static int infty()
	{
#ifdef EVAL_QUAD
	  assert(NewProgress::maxProgress() % 3 == 0);
	  return 57984 * (NewProgress::maxProgress()/3);
#else
	  return 57984 * NewProgress::maxProgress();
#endif
	}
	static int captureValue(PtypeO ptypeO) {
	  assert(isValidPtypeO(ptypeO));
	  return roundUp((-PieceEval::value(ptypeO) +
			  PieceEval::value(captured(ptypeO))) * seeScale());
	}
	static int seeScale() {
#ifdef EVAL_QUAD
	  assert(NewProgress::maxProgress() % 3 == 0);
	  return (NewProgress::maxProgress()/3);
#else
	  return NewProgress::maxProgress();
#endif
	}

	OpenMidEndingEvalDebugInfo debugInfo(const NumEffectState &state);
	static void setRandom();
	static void resetWeights(const int *w, size_t length);
	static OpenMidEndingPtypeTable Piece_Value;
	bool progressIndependentValueLimit() const {
	  return use_progress_independent_value_limit;
	}
      private:
	template <class Reader>
	static void doResetWeights(Reader& reader);
      };
    }
    using ml::OpenMidEndingEval;
  }
  using eval::OpenMidEndingEval;
}

#endif // EVAL_ML_OPENMIDENDINGEVAL_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
