/* pvHistory.h
 */
#ifndef _PVHISTORY_H
#define _PVHISTORY_H

#include "osl/search/moveWithComment.h"
#include "osl/container.h"

namespace osl
{
  namespace game_playing
  {
    struct PVHistory : public CArray<MoveWithComment,8>
    {
    };
  }
}

#endif /* _PVHISTORY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
