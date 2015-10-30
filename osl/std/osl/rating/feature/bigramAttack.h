/* bigramAttack.h
 */
#ifndef _BIGRAMATTACK_H
#define _BIGRAMATTACK_H

#include "osl/rating/feature.h"

namespace osl
{
  namespace rating
  {
    class BigramAttack : public Feature
    {
      int property;
      bool same, focus_x;
    public:
      static const std::string name(int x1, int y1, int x2, int y2, int king_index, bool s, bool f);
      BigramAttack(int x1, int y1, int x2, int y2, int king_index, bool s, bool f) 
	: Feature(name(x1,y1,x2,y2,king_index,s,f)), property((((x1+2)*5+y1+2)*25 + (x2+2)*5+y2+2)*5 + king_index), same(s), focus_x(f)
      {
      }
      // [0,4]
      static int indexKing(Player attack, Square king, bool focus_x)
      {
	int x = focus_x ? king.x() : king.y();
	if (! focus_x && attack == WHITE)
	  x = 10 - x;
	if (x <= 3)
	  return x-1;
	if (x >= 7)
	  return x-5;
	return 2;
      }
      static int indexOfMove(Square king, Move move)
      {
	int x_diff = move.to().x() - king.x();
	if (abs(x_diff) > 2)
	  return -1;
	x_diff += 2;
	assert(x_diff >= 0 && x_diff <= 4);
	int y_diff = move.to().y() - king.y();
	if (abs(y_diff) > 2)
	  return -1;
	if (move.player() == WHITE)
	  y_diff = -y_diff;
	y_diff += 2;
	assert(y_diff >= 0 && y_diff <= 4);
	return x_diff * 5 + y_diff;
      }
      static int index(const NumEffectState& state, Move move, const RatingEnv& env, bool same, bool focus_x) 
      {
	if (! env.history.hasLastMove(same+1))
	  return -1;
	const Move prev = env.history.lastMove(same+1);
	if (! prev.isNormal())
	  return -1;
	const Square king = state.kingSquare(alt(state.turn()));
	const int index1 = indexOfMove(king, prev);
	if (index1 < 0)
	  return -1;
	const int index2 = indexOfMove(king, move);
	if (index2 < 0)
	  return -1;
	return (index1 * 25 + index2) * 5 + indexKing(move.player(), king, focus_x);
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	int index = this->index(state, move, env, same, focus_x);
	return index == property;
      }
    };
  }
}

#endif /* _BIGRAMATTACK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
