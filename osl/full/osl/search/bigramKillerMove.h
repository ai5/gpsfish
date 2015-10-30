/* bigramKillerMove.h
 */
#ifndef _BIGRAMKILLERMOVETABLE_H
#define _BIGRAMKILLERMOVETABLE_H

#include "osl/search/lRUMoves.h"
#include "osl/numEffectState.h"
#include <cstddef>
#include <cassert>
namespace osl
{
  namespace search
  {
    /**
     * 相手の指手に基づくkiller move
     */
    class BigramKillerMove
    {
    private:
      CArray2d<LRUMoves,Square::SIZE,PTYPEO_SIZE> killer_moves;
    public:
      BigramKillerMove();
      ~BigramKillerMove();
      void clear();
      void setMove(Move key, Move value)
      {
	if (value.isPass())
	  return;
	if (key.to() == value.to())
	  return;		// takeback は読みそう
	assert(value.isValid());
	assert(key.player() != value.player());
	killer_moves[key.to().index()][ptypeOIndex(key.ptypeO())].setMove(value);
      }
      const LRUMoves& operator[](Move key) const
      {
	return killer_moves[key.to().index()][ptypeOIndex(key.ptypeO())];
      }
      void getMove(const NumEffectState& state, Move last_move,
		   MoveVector& moves) const;
      void dump() const;
    };
  } // namespace search
} // namespace osl

#endif /* _BIGRAMKILLERMOVETABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
