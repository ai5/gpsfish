/* pieceTable.h
 */
#ifndef OSL_PIECE_TABLE_H
#define OSL_PIECE_TABLE_H

#include "osl/basic_type.h"
#include "osl/container.h"
namespace osl
{
  class PieceTable
  {
  private:
    CArray<Ptype, Piece::SIZE> ptypes;
    template<Ptype T>
    void initPtype();
  public:
    PieceTable();
    Ptype getPtypeOf(int num) const{
      assert(validNumber(num));
      return ptypes[num];
    }
    static bool validNumber(int num) {
      return 0<=num && num<=39;
    }
  };

  extern const PieceTable Piece_Table;
}

#endif /* OSL_PIECE_TABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
