/* effect5x3.h
 */
#ifndef PROGRESS_EFFECT5X3_H
#define PROGRESS_EFFECT5X3_H

#include "osl/progress.h"
#include "osl/numEffectState.h"
#include "osl/bits/centering5x3.h"
#include "osl/container.h"
#include <algorithm>
namespace osl
{
  namespace progress
  {
    /**
     * 玉の周囲5x3の領域の利きの数と持駒から計算した進行度.
     * 5x3領域は盤面内になるように補正する．
     * 黒と白の両方の進行度を持つ．
     * 持駒の重みは
     * PAWN 1
     * LANCE 4
     * KNIGHT,SILVER,GOLD 8
     * BISHOP,ROOK 12
     * 利きの数は8の重みを持つ．
     */
    struct Effect5x3
    {
    public:
      static void updateStand(int& old_stand, Move last_move);
      static int makeProgressAll(Player defense, const NumEffectState& state,
				 Square king);
      static int makeProgressArea(Player attack, const NumEffectState& state,
				  Square king);
    public:
      static int makeProgressStand(Player attack, const NumEffectState& state);
      /**
       * 王の位置を指定したprogressの計算.
       * @param defense - こちらの玉に注目したprogress
       * @param state - 盤面
       * @param king - 玉の位置がここにあるとする．
       * 一般には，盤面から玉の位置は特定できるが，差分計算の途中では
       * 一致しないとして呼び出すことがある．
       */
      static int makeProgress(Player defense, const NumEffectState& state){
	return makeProgressAll(defense,state,state.kingSquare(defense));
      }
      static int makeProgress(const NumEffectState& state)
      {
	return makeProgress(BLACK, state) + makeProgress(WHITE, state);
      }
      /**
       * 0-15 の値を返す
       */
      static const Progress16 progress16(int progress)
      {
	assert(progress >= 0);
	const int rank = progress / 8; // 適当
	return Progress16(std::min(rank, 15));
      }
      /**
       * 0-15 の値を返す.
       * プレイヤ個人毎
       */
      static const Progress16 progress16each(int progress)
      {
	assert(progress >= 0);
	const int rank = progress / 8; // 調整中
	return Progress16(std::min(rank, 15));
      }
    protected:
      CArray<int,2> progresses, stand_progresses, area_progresses;
    public:
      explicit Effect5x3(const NumEffectState& state);
      void changeTurn() {}
      int progress(Player p) const { return progresses[p]; }
      const Progress16 progress16() const
      {
	return progress16(progresses[0] + progresses[1]);
      }
      const Progress16 progress16(Player p) const
      {
	return progress16each(progress(p));
      }
      // 必要なもの
      Effect5x3 expect(const NumEffectState& state, Move move) const;
      void update(const NumEffectState& new_state, Move last_move);
    };
    struct Effect5x3WithBonus : public Effect5x3
    {
      explicit Effect5x3WithBonus(const NumEffectState& state);
      template <Player Attack>
      static int makeProgressAreaBonus(const NumEffectState& state,
				       Square king);
      template <Player Attack, bool AlwaysPromotable, bool AlwaysNotPromotable>
      static int makeProgressAreaBonus(const NumEffectState& state, 
				       Square king, Square center);
      const Progress16 progress16bonus(Player p) const
      {
	return progress16each(progress_bonuses[p] +
			      stand_progresses[p]);
      }
      void update(const NumEffectState& new_state, Move last_move);
      Effect5x3WithBonus expect(const NumEffectState& state, Move move) const;
      // Public for testing
      int countEffectPieces(const NumEffectState &state, Player attack) const;
      // p is defense player
      const PieceMask effect5x3Mask(Player p) const { return effect_mask[p]; }
    private:
      void updateProgressBonuses(const NumEffectState& state, bool black=true, bool white=true);
      static int attackEffect3(const NumEffectState& state, Player attack, Square target);
      template <Player Defense>
      static PieceMask makeEffectMask(const NumEffectState& state);
      void updateStand(Player pl, Move m) { Effect5x3::updateStand(stand_progresses[pl], m); }

      CArray<int,2> progress_bonuses;
      CArray<PieceMask, 2> effect_mask;
    };
  } // namespace progress
} // namespace osl

#endif /* PROGRESS_EFFECT5X3_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
