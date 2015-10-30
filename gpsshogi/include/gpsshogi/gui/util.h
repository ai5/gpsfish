#ifndef GPSSHOGI_GUI_UTIL_H
#define GPSSHOGI_GUI_UTIL_H
#include "osl/numEffectState.h"
#include <qstring.h>

namespace gpsshogi
{
  namespace gui
  {
    class Util
    {
    public:
      static QString getKanjiPiece(osl::Ptype p);
      static QString moveToString(osl::Move m);
      static QString moveToString(osl::Move m, const osl::NumEffectState&,
				  osl::Move last_move = osl::Move());
      static int compare(osl::Move m1, osl::Move m2);
    };
  }
}

#endif // GPSSHOGI_GUI_UTIL_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
