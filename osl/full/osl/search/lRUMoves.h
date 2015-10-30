/* lRUMoves.h
 */
#ifndef OSL_SEARCH_LRUMOVES_H
#define OSL_SEARCH_LRUMOVES_H

#include "osl/basic_type.h"
#include "osl/container.h"
#ifdef OSL_SMP
#  include "osl/misc/lightMutex.h"
#endif

namespace osl
{
  namespace search
  {
    class LRUMoves
    {
      typedef CArray<Move, 2> moves_t;
      moves_t moves;
#ifdef OSL_SMP
      typedef osl::misc::LightMutex Mutex;
      mutable Mutex mutex;
#endif
    public:
      LRUMoves() {}
      LRUMoves(const LRUMoves& src)
	: moves(src.moves)
      {
      }
      LRUMoves& operator=(const LRUMoves& src)
      {
	if (this != &src)
	  moves = src.moves;
	return *this;
      }

      void clear()
      {
#ifdef OSL_SMP
	SCOPED_LOCK(lk,mutex);
#endif
	moves.fill(Move::INVALID());
      }
      void setMove(Move best_move)
      {
#ifdef OSL_SMP
	SCOPED_LOCK(lk,mutex);
#endif
	if (best_move.isNormal() && moves[0] != best_move)
	{
	  moves[1] = moves[0];
	  moves[0] = best_move;
	}
      }
      const Move operator[](size_t i) const
      {
#ifdef OSL_USE_RACE_DETECTOR
	SCOPED_LOCK(lk,mutex);
#endif
	return moves[i];
      }
      static size_t size() { return moves_t::size(); }
    };
  }
} // namespace osl

#endif /* OSL_SEARCH_LRUMOVES_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
