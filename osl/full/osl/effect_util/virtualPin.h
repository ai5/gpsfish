/* virtualPin.h
 */
#ifndef _VIRTUALPIN_H
#define _VIRTUALPIN_H

#include "osl/numEffectState.h"
namespace osl
{
  namespace effect_util
  {
    class VirtualPin
    {
    private:
      template <Direction DIR>
      static bool findDirection(const SimpleState& state, Square target,
				Player defense, const PieceMask& remove)
      {
	const Offset diff = Board_Table.getOffset(defense, DIR);
	Piece p;
	for (p=state.nextPiece(target, diff);;p=state.nextPiece(p.square(), diff)) {
	  if (! p.isPiece())
	    return false;
	  if (! remove.test(p.number()))
	    break;
	}
	assert(p.isPiece());
	if (p.owner() == defense)
	  return false;
	return (Ptype_Table.getMoveMask(p.ptype())
		& DirectionTraits<DirectionTraits<DIR>::longDir>::mask);
      }
    public:
      /** remove が全て動くと defenseの玉に攻め方の効きが発生するか。*/
      static bool find(const NumEffectState& state, Player defense, const PieceMask& remove);
      static bool find(const NumEffectState& state, Player defense, Square target)
      {
	return find(state, defense, state.effectSetAt(target));
      }
    };
  }
  using effect_util::VirtualPin;
}

#endif /* _VIRTUALPIN_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
