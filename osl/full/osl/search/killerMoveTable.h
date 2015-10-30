/* killerMoveTable.h
 */
#ifndef OSL_KILLERMOVETABLE_H
#define OSL_KILLERMOVETABLE_H

#include "osl/search/lRUMoves.h"
#include "osl/numEffectState.h"
#include <cstddef>
#include <cassert>
namespace osl
{
  namespace search
  {
    /**
     * 単純なkiller move
     *
     * 深さごとの最善手を登録する．テーブルに登録されていない局面で，
     * 「自分が何を指そうが相手からこう指されると困る」と言うときに有
     * 効．最善の判定はいい加減で，その深さで最後にbestMoveとして発見されたmove
     */
    class KillerMoveTable
    {
    public:
      static const int KillerMoveMax = 64;
    private:
      CArray<LRUMoves,KillerMoveMax> killer_moves;
    public:
      KillerMoveTable();
      ~KillerMoveTable();
      void clear();
      void setMove(size_t depth, const Move& move)
      {
	assert(move.isValid());
	killer_moves[depth].setMove(move);
      }
      void getMove(const NumEffectState& state, size_t depth,
		   MoveVector& out) const
      {
	const LRUMoves& moves = killer_moves[depth];
	for (size_t i=0; i<moves.size(); ++i)
	{
	  Move m;
	  {
	    m = moves[i];
	  }
	  if (! m.isNormal())
	    return;
	  if (state.isAlmostValidMove<false>(m))
	    out.push_back(m);
	}
      }
    };
  } // namespace search
} // namespace osl

#endif /* OSL_KILLERMOVETABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
