#ifndef _MINI_BOARD_H
#define _MINI_BOARD_H
#include "osl/book/compactBoard.h"
#include "osl/simpleState.h"
#include "boost/dynamic_bitset.hpp"
#include <string>
#include <vector>

namespace osl
{
  namespace book
  {
    /**
     * Square, Owner: 9 bits. For GOLD.
     *   1: Owner 0:Black; 1:WHITE
     *   2345: Square x
     *   6789: Square y
     */
    class OSquare
    {
    public:
      static const size_t total_bits;
      OSquare() : value(0) {}
      OSquare(const Piece& p)
      {
	const Square pos = p.square();
	const int bitPos = OPiece::position2Bits(pos); // 8 bits
        int owner = 0;
        if (p.owner() == BLACK)
          owner = 0;
        else
          owner = 1;
	value = owner << 8 | bitPos; // 9 bits
      }
      OSquare(const int i)
      {
	value = i;
      }
      Square getSquare() const
      {
        return OPiece::bits2Square(value);
      }
      Player getOwner() const
      {
        const int owner = value >> 8 & 1;
        if (owner == 0)
          return BLACK;
        else
          return WHITE;
      }
      operator int() const { return value; }
    protected:
      int value;
    };

    /**
     * Square, Owner, Promoted : 10 bits. 
     * For PAWN, LANCE, KNIGHT, SILVER, BISHOP and ROOK.
     *   1: Promote 0:unpromoted; 1:promoted
     *   2: Owner 0:Black; 1:WHITE
     *   3456: Square x
     *   789A: Square y
     */
    class OPSquare : public OSquare
    {
    public:
      static const size_t total_bits;
      OPSquare() : OSquare() {}
      OPSquare(const Piece& p)
        : OSquare(p)
      {
        int is_promoted = 0;
        if (p.isPromoted())
          is_promoted = 1;
	value = is_promoted << 9 | value; // 10 bits
      }
      OPSquare(const int i)
        : OSquare(i) {}
      bool isPromoted() const
      {
        const int is_promoted = value >> 9 & 1;
        if (is_promoted == 0)
          return false;
        else
          return true;
      }
    };

    /**
     * More compact board than CompactBoard. 400 bits.
     *   PAWNs        1 - 180
     *   LANCEs     181 - 220
     *   KNIGHTs    221 - 260
     *   SILVERs    261 - 300
     *   BISHOPs    301 - 320
     *   ROOKs      321 - 340
     *   GOLDs      341 - 376
     *   Black KING 377 - 384
     *   White KING 385 - 392
     *   not used   393 - 399
     *   turn       400 (0: Black; 1:White)
     */
    class MiniBoard
    {
    public:
      static const size_t total_bits; 
      MiniBoard() {}
      explicit MiniBoard(const SimpleState& state);
      SimpleState getState() const;
      boost::dynamic_bitset<> toBits() const;
      std::string toBase64() const;
    private:
      typedef std::vector<OPSquare> PawnArray;   // 10 bits x 18 = 180
      typedef std::vector<OPSquare> LanceArray;  // 10      x  4 =  40
      typedef std::vector<OPSquare> KnightArray; // 10      x  4 =  40
      typedef std::vector<OPSquare> SilverArray; // 10      x  4 =  40
      typedef std::vector<OPSquare> BishopArray; // 10      x  2 =  20
      typedef std::vector<OPSquare> RookArray;   // 10      x  2 =  20
      typedef std::vector<OSquare>  GoldArray;   //  9      x  4 =  36
      typedef osl::CArray<char, 2>    KingArray;   //  8      x  2 =  16 
                                                       // ------------------
                                                       //                392
      PawnArray   pawn_pieces;
      LanceArray  lance_pieces;
      KnightArray knight_pieces;
      SilverArray silver_pieces;
      BishopArray bishop_pieces;
      RookArray   rook_pieces;
      GoldArray   gold_pieces;
      KingArray   king_pieces;
      Player turn;

      /// Converts a base64 string to MiniBoard
      /// @return 0 (success); non-zero (failure)
      friend int fromBase64(const std::string& base64, MiniBoard& mb);
    };
  }
}

#endif // _MINI_BOARD_H
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
