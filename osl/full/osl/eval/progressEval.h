/* progressEval.h
 */
#ifndef EVAL_PROGRESSEVAL_H
#define EVAL_PROGRESSEVAL_H

// #define EXPERIMENTAL_EVAL

#include "osl/effect_util/pin.h"
#include "osl/eval/pieceEval.h"
#include "osl/eval/minorPieceBonus.h"

#include "osl/eval/ppair/piecePairPieceEval.h"
#include "osl/eval/endgame/attackDefense.h"
#include "osl/eval/mobilityTable.h"
#include "osl/progress/effect5x3.h"
#include "osl/progress/effect5x3d.h"
#include "osl/progress.h"
#include "osl/mobility/rookMobility.h"
#include "osl/mobility/bishopMobility.h"
#include "osl/mobility/lanceMobility.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace eval
  {
    struct ProgressDebugInfo
    {
      int eval, opening, endgame;
      int progress, progress_bonus;
      int progress_independent_bonus, progress_dependent_bonus;
      int minor_piece_bonus;

      // break down of progress bonus
      int black_danger, black_defense, white_danger, white_defense;
      // progress independent bonus
      int mobility_bonus, two_rook_bonus, knight_check_bonus, rook_rank_bonus,
	enter_king_bonus, middle_king_bonus, silver_penalty, gold_penalty;
      // progress dependent bonus
      int king8_attack_bonus, pin_bonus;

      MinorPieceDebugInfo minor_piece_bonus_info;
    };

    /**
     * 序盤と終盤の内分を使う評価関数.
     */
    template <class OpeningEval>
    class ProgressEvalGeneral
    {
    public:
      typedef OpeningEval opening_eval_t;
      typedef endgame::AttackDefense endgame_eval_t;
      typedef progress::Effect5x3WithBonus progress_t;
      typedef progress::Effect5x3d defense_t;
    private:
      opening_eval_t opening_eval;
      endgame_eval_t endgame_eval;
      progress_t current_progress;
      defense_t defense_effect;
      MinorPieceBonus minor_piece_bonus;
      // Records pinned pieces of a player, not the opponent's
      // (owner of pieces == player)
      mutable CArray<PieceMask, 2> pin_mask;
      CArray2d<int, 2, SHORT8_DIRECTION_MAX+1> can_check_pieces;
      int progress_independent_bonus;
      int progress_dependent_bonus;
      int major_pieces;
      CArray<int,2> attack_bonus; // index is king (i.e., defense)
      int rook_mobility, bishop_mobility, lance_mobility;
      enum{
	INVALID=EvalTraits<BLACK>::MAX_VALUE+1,
	  };
      mutable int cache;

      static CArray<int, PTYPEO_SIZE> capture_values;

      template<Player P, Ptype PTYPE, Direction Dir, Direction LongDir>
      void initializeCheckPieceDir(const NumEffectState &state, int count);
      template <Player P, Ptype PTYPE>
      void initializeCheckPiece(const NumEffectState &state);
      static void setUpInternal(const char *filename=0);
    public:
      ProgressEvalGeneral(const NumEffectState& state);
      void changeTurn() {}
      static bool initialized() 
      { 
	return opening_eval_t::initialized();
      } 
      static bool setUp(const char *filename)
      {
	if (! opening_eval_t::setUp())
	  return false;
	setUpInternal(filename);
	return true;
      }
      static bool setUp()
      {
	if (! opening_eval_t::setUp())
	  return false;
	setUpInternal();
	return true;
      }

      /** roundup は 2^n であること */
      static const int ROUND_UP = 64;
      /** 危険度ペナルティの16倍 */
      static int attackDefenseBonusT16(Progress16 black, Progress16 white,
				       Progress16 black_defense,
				       Progress16 white_defense) 
      {
	return 
	  (white.value() * 2 - white_defense.value()
	   - black.value() * 2 + black_defense.value())
	  * 3200 / 2;
      }
      static int composeValue(int value_opening, int value_endgame, 
			      Progress16 progress16,
			      Progress16 black,
			      Progress16 white,
			      Progress16 black_defense,
			      Progress16 white_defense,
			      int minor_piece_bonus,
			      int progress_independent_bonus,
			      int progress_dependent_bonus)
      {
	static_assert(((PtypeEvalTraits<BISHOP>::val * 2 + PtypeEvalTraits<PAWN>::val) %  16) == 0, "");
	static_assert((PtypeEvalTraits<PAWN>::val % 32) == 0, "");
	assert(progress16.isValid());
	assert(black.isValid());
	assert(white.isValid());
	/*
	  value_opening * (16 - progress16) + value_endgame * progress16
	  + bonus * (white_progress - black_progress) * prorgress16 / 16 */
	int result = value_opening*16 + 
	  progress16.value() * (value_endgame-value_opening +
				(attackDefenseBonusT16(black, white,
						       black_defense,
						       white_defense))
				/ 16);

	result += progress_independent_bonus;
	result += minor_piece_bonus;
	result += progress_dependent_bonus;
	result &= ~(ROUND_UP-1);
	assert(result % 2 == 0);
	return result;
      }
      const Progress16 progress16() const { return current_progress.progress16(); }
      const Progress16 progress16bonus(Player p) const {
	return current_progress.progress16bonus(p);
      }
      void invalidateCache(){ 
	cache=INVALID;
      }
      int value() const
      {
	if(cache==INVALID)
	  cache = composeValue(
	    openingValue(), 
	    endgame_eval.value(), 
	    progress16(),
	    current_progress.progress16bonus(BLACK),
	    current_progress.progress16bonus(WHITE),
	    defense_effect.progress16(BLACK),
	    defense_effect.progress16(WHITE),
	    minor_piece_bonus.value(
	      progress16(),
	      progress16bonus(BLACK),
	      progress16bonus(WHITE)),
	    progress_independent_bonus,
	    progress_dependent_bonus);

	return cache;
      }
      const Progress32 progress32() const { 
	return Progress32(current_progress.progress16(BLACK).value()
			  + current_progress.progress16(WHITE).value()); 
      }
      static void setValues(const SimpleState&, Progress16 progress16,
			    container::PieceValues&);
      static void setValues(const SimpleState& s, container::PieceValues& o);

      int expect(const NumEffectState& state, Move move) const;
      Move suggestMove(const NumEffectState&) const 
      {
	return Move();
      }
      void update(const NumEffectState& new_state, Move last_move);
      template <Player p>
      int calculateAttackBonusEach(const NumEffectState& state) const;
      template <Player Attack, Direction Dir>
      int calculateAttackBonusOne(const NumEffectState& state) const;
      int calculateKnightCheck(const NumEffectState& state) const;
      template <osl::Player P>
      int calculateKnightCheckEach(const NumEffectState& state) const;
      template <Player p>
      int calculateEnterKingBonus(const NumEffectState& state) const ;
      template <Player p>
      int calculateMiddleKingBonus(const NumEffectState& state) const;
      int calculateRookRankBonus(const NumEffectState& state) const;

    public:
      static int infty()
      {
	assert(endgame_eval_t::infty() <= opening_eval_t::infty());
	// 序盤を使用
	return composeValue(opening_eval_t::infty(), 0, Progress16(0),
			    Progress16(0), Progress16(0), Progress16(0),
			    Progress16(0), 0, 0, 0);
      }

      static int captureValue(PtypeO ptypeO)
      {
	return capture_values[ptypeOIndex(ptypeO)];
      }
      static int seeScale()
      {
	return captureValue(newPtypeO(WHITE,PAWN))
	  / PieceEval::captureValue(newPtypeO(WHITE,PAWN));
      }

      const PieceMask pins(Player player) const { 
	return pin_mask[player]; 
      }
      // for debug
      int minorPieceValue() const { 
	return minor_piece_bonus.value(progress16(),
				       progress16bonus(BLACK),
				       progress16bonus(WHITE)); 
      }
      int openingValue() const { return opening_eval.value(); }
      int endgameValue() const { return endgame_eval.value(); }
      ProgressDebugInfo debugInfo(const NumEffectState& state) const;

      // public for debugging purpose only
      // also updates pin_mask
      int calculatePinBonus(const NumEffectState& state) const;
      int calculateMobilityBonus() const;
      static int calculateMobilityBonusRook(const NumEffectState& state);
      static int calculateMobilityBonusBishop(const NumEffectState& state);
      static int calculateMobilityBonusLance(const NumEffectState& state);
      int calculateAttackRooks(const NumEffectState& state) const;
      int calculateAttackBonus(const NumEffectState& state) const;
      int calculateSilverPenalty(const NumEffectState& state) const;
      int calculateGoldPenalty(const NumEffectState& state) const;

      int attackDefenseBonus() const
      {
	return progress16().value()
	  * attackDefenseBonusT16(current_progress.progress16bonus(BLACK),
				  current_progress.progress16bonus(WHITE),
				  defense_effect.progress16(BLACK),
				  defense_effect.progress16(WHITE))
	  / 16;
      }
      int attackBonusScale(int val, Player attack) const 
      {
	return val * current_progress.progress16(alt(attack)).value() / 16 * 4;
      }
      void debug() const {}
      enum { AdjustableDimension
	     = PTYPE_SIZE + endgame::KingPieceTable::EffectiveDimension*2 };
      static void resetWeights(const int *weight);
    };

    typedef PiecePairPieceEval progress_eval_opening_t;
    class ProgressEval : public ProgressEvalGeneral<progress_eval_opening_t> 
    {
    public:
      explicit ProgressEval(const NumEffectState& state) 
	: ProgressEvalGeneral<progress_eval_opening_t>(state)
      {
      }
      ProgressEval() 
	: ProgressEvalGeneral<progress_eval_opening_t>(NumEffectState())
      {
      }
      static PtypeEvalTable Piece_Value;
    };
  } // namespace eval
  using eval::ProgressEval;
} // namespace osl

#endif /* EVAL_PROGRESSEVAL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
