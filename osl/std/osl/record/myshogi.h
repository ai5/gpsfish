/* myshogi.h
 */
#ifndef OSL_RECORD_MYSHOGI_H
#define OSL_RECORD_MYSHOGI_H

#include "osl/numEffectState.h"
#include <string>
namespace osl
{
  namespace record
  {
    namespace myshogi
    {
      std::string show(const NumEffectState& state);
      std::string show(const NumEffectState& state,
		       Move last_move,
		       const NumEffectState& prev,
		       bool add_csa_move=false);
      std::string show(Player);
      std::string show(Ptype);
      std::string show(Square);
      std::string show(Piece);
    }
  }
}

#endif /* OSL_RECORD_MYSHOGI_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
