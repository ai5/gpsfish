/* firstMoveThreatmate.tcc
 */
#ifndef SEARCH_FIRSTMOVETHREATMATE_H
#define SEARCH_FIRSTMOVETHREATMATE_H
#include "osl/basic_type.h"

namespace osl
{
  namespace search
  {
    /**
     * 初手に対して詰めろ探索をするかどうかを判定
     */
    struct FirstMoveThreatmate
    {
      static bool isMember(Move last_move, Square king)
      {
	const Ptype captured = last_move.capturePtype();
	const Square to = last_move.to();
	return ((captured != PTYPE_EMPTY)
		|| (isMajor(last_move.ptype()))
		|| (abs(to.x() - king.x()) + abs(to.y() - king.y()) < 8));
      }
    };
  }
}

#endif /* SEARCH_FIRSTMOVETHREATMATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
