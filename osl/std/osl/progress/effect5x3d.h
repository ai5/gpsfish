/* effect5x3.h
 */
#ifndef PROGRESS_EFFECT5X3D_H
#define PROGRESS_EFFECT5X3D_H

#include "osl/progress.h"
#include "osl/numEffectState.h"
#include <algorithm>
namespace osl
{
  namespace progress
  {
    /**
     * 玉の周囲5x3の領域の利きの数ら計算した自玉のまわりの利きの数。
     * 5x3領域は盤面内になるように補正する．
     * 黒と白の両方の進行度を持つ．
     * Effect5x3 との違い持駒の重みなし
     */
    struct Effect5x3d
    {
      /**
       * 王の位置を指定したprogressの計算.
       * @param defense - こちらの玉に注目したprogress
       * @param state - 盤面
       * @param king - 玉の位置がここにあるとする．
       * 一般には，盤面から玉の位置は特定できるが，差分計算の途中では
       * 一致しないとして呼び出すことがある．
       */
      static int makeProgress(Player defense, const NumEffectState& state,
			      Square king);
      static int makeProgress(Player defense, const NumEffectState& state){
	return makeProgress(defense,state,state.kingSquare(defense));
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
	const int rank = progress / 8 - 16; // 適当
	return Progress16(std::max(std::min(rank, 15), 0));
      }
      /**
       * 0-15 の値を返す.
       * プレイヤ個人毎
       */
      static const Progress16 progress16each(int progress)
      {
	assert(progress >= 0);
	const int rank = progress / 8 - 8; // 調整中
	return Progress16(std::max(std::min(rank, 15), 0));
      }
    private:
      CArray<int,2> progresses;
    public:
      explicit Effect5x3d(const NumEffectState& state) 
      {
	progresses[BLACK]=makeProgress(BLACK, state);
	progresses[WHITE]=makeProgress(WHITE, state);
      }
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
      void update(const NumEffectState& new_state, Move last_move);
    };
  } // namespace progress
} // namespace osl

#endif /* PROGRESS_EFFECT5X3D_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
