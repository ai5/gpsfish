/* pvVector.h
 */
#ifndef GPSSHOGI_PVVECTOR_H
#define GPSSHOGI_PVVECTOR_H

#include "osl/container.h"

namespace gpsshogi
{
  enum { PvMaxDepth = 48 };
  struct PVVector : public osl::FixedCapacityVector<osl::Move,PvMaxDepth> 
  {
    int pieceValue() const;
    int pieceValueOfTurn() const;
  };
  std::ostream& operator<<(std::ostream&, const PVVector&);
}

#endif /* GPSSHOGI_PVVECTOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
