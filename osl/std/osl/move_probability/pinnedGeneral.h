/* pinnedGeneral.h
 */
#ifndef OSL_MOVE_PROBABILITY_PINNEDGENERAL_H
#define OSL_MOVE_PROBABILITY_PINNEDGENERAL_H
#include "osl/basic_type.h"

namespace osl
{
  namespace move_probability
  {
    struct PinnedGeneral
    {
      Piece general, covered;
      Square attack;
      PinnedGeneral(Piece g, Piece c, Square a) : general(g), covered(c), attack(a)
      {
      }
      PinnedGeneral() {}
    };
    inline bool operator==(const PinnedGeneral& l, const PinnedGeneral& r) 
    {
      return l.general == r.general && l.covered == r.covered && l.attack == r.attack;
    }
  }
}

#endif /* OSL_MOVE_PROBABILITY_PINNEDGENERAL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
