#ifndef _PIECE_VALUES_H
#define _PIECE_VALUES_H
#include "osl/container.h"
#include <iosfwd>

namespace osl
{  
  class NumEffectState;
  namespace container
  {
    /**
     * 駒番号->intの配列. 局面に対する駒の価値など
     */
    class PieceValues : public CArray<int,Piece::SIZE>
    {
    public:
      PieceValues();
      ~PieceValues();
      
      int sum() const;
      void showValues(std::ostream&, const NumEffectState&) const;
    };
  } // namespace container
  using container::PieceValues;
} // namespace osl
#endif // _PIECE_VALUES_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
