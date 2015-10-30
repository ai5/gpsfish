/* searchInfo.h
 */
#ifndef OSL_RECORD_SEARCHINFO_H
#define OSL_RECORD_SEARCHINFO_H

#include "osl/basic_type.h"
#include <vector>
#include <string>

namespace osl
{
  namespace record
  {
    struct SearchInfo
    {
      int value;
      std::vector<Move> moves;
      SearchInfo() : value(0)
      {
      }

      bool isValid() const {return !moves.empty();}
    };
  }
} // namespace osl

#endif /* OSL_RECORD_SEARCHINFO_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
