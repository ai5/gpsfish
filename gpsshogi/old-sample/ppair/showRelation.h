/* showRelation.h
 */
#ifndef _SHOWRELATION_H
#define _SHOWRELATION_H

#include "osl/record/kanjiPrint.h"
#include "osl/ptype.h"
#include "osl/position.h"
#include <iostream>

namespace osl
{
  void showPiece(PtypeO ptypeo, Square pos)
  {
    const Player player = getOwner(ptypeo);
    const Ptype ptype = getPtype(ptypeo);

    std::cout << ((player == BLACK) ? "ве" : "вд")
	      << pos.x() << pos.y()
	      << record::StandardCharacters().kanji(ptype);
  }
}

#endif /* _SHOWRELATION_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
