#ifndef _COMPACT_BOARD_H
#define _COMPACT_BOARD_H
#include "osl/simpleState.h"
#include <vector>
#include <string>

namespace osl
{
  namespace book
  {
    class OPiece
    {
    public:
      OPiece(Piece p)
      {
	const Square pos = p.square();
	const int bitPos = position2Bits(pos);
	value = (static_cast<int>(p.owner()) << 20 |
		 static_cast<int>(p.ptype()) << 16 | bitPos);
      }
      OPiece(int i)
      {
	value = i;
      }
      Square square() const
      {
        return bits2Square(value);
      }
      Ptype ptype() const
      {
	return static_cast<Ptype>((value >> 16) & 0xf);
      }
      Player owner() const
      {
	return static_cast<Player>(value >> 20);
      }
      operator int() const { return value; }

      /** Converts a position to an integer (bits) */
      static int position2Bits(const Square& pos);
      /** Converts an integer (bits) to Square */
      static Square bits2Square(const int bit_position);
    private:
      int value;
    };

    class CompactBoard;
    /**
     * 局面を比較する. 
     * 将棋としての局面（手番や持ち駒を含む）を比較する. 
     * NumEffectState等と異なり駒番号は考慮されない.
     */
    bool operator==(const CompactBoard&, const CompactBoard&);
    std::ostream& operator<<(std::ostream& os, const CompactBoard& c);
    std::istream& operator>>(std::istream& os, CompactBoard& c);
    /**
     * SimpleStateよりcompactな局面の表現 
     */
    class CompactBoard
    {
    public:
      CompactBoard() {}
      explicit CompactBoard(const SimpleState& state);
      SimpleState state() const;
      const std::vector<OPiece>& pieces() const {return piece_vector;};
      Player turn() const {return player_to_move;}

      friend std::ostream& operator<<(std::ostream& os, const CompactBoard& c);
      friend std::istream& operator>>(std::istream& os, CompactBoard& c);
      friend bool operator==(const CompactBoard&, const CompactBoard&);
    private:
      std::vector<OPiece> piece_vector;
      Player player_to_move;
    };
    int readInt(std::istream& is);
    void writeInt(std::ostream& os, int n);
  }
}

#endif // _COMPACT_BOARD_H
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
