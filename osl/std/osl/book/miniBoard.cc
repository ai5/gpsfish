#include "osl/record/record.h"
#include "osl/book/miniBoard.h"
#include "osl/misc/base64.h"
#include <boost/dynamic_bitset.hpp>
#include <iostream>
#include <algorithm>
#include <sstream>

namespace osl
{
  namespace book
  {

    struct oposition_sort
    { 
      bool operator()(const OSquare& l, const OSquare& r)
      {
	// need to special case pieces on stand
	if (l.getSquare() == Square::STAND() || r.getSquare() == Square::STAND())
	{
	  if (l.getSquare() == Square::STAND() && r.getSquare() != Square::STAND())
	    return true;
	  else if (l.getSquare() != Square::STAND() && r.getSquare() == Square::STAND())
	    return false;
	  else
	    {
	      if (l.getOwner() != r.getOwner())
	        return l.getOwner() == WHITE;
	      return true;
	    }
	}
	else
	{
	  if (l.getSquare().x() < r.getSquare().x())
	    return true;
	  else if (l.getSquare().x() > r.getSquare().x())
	    return false;
	  else
	    {
	      if (l.getSquare().y() <= r.getSquare().y())
	        return true;
	      else
	        return false;
	    }
	}
      }
    };

    const size_t MiniBoard::total_bits  = 400; 
    const size_t OSquare::total_bits  =   9;
    const size_t OPSquare::total_bits =  10;

    MiniBoard::MiniBoard(const SimpleState& state)
    {
      pawn_pieces.reserve(18);
      lance_pieces.reserve(4);
      knight_pieces.reserve(4);
      silver_pieces.reserve(4);
      bishop_pieces.reserve(2);
      rook_pieces.reserve(2);
      gold_pieces.reserve(4);

      for (int i = 0; i < 40; ++i)
      {
	if(!state.usedMask().test(i)) continue;
        const Piece p = state.pieceOf(i);
        switch (unpromote(p.ptype()))
        {
        case PAWN:
          pawn_pieces.push_back(OPSquare(p));
          break;
        case LANCE:
          lance_pieces.push_back(OPSquare(p));
          break;
        case KNIGHT:
          knight_pieces.push_back(OPSquare(p));
          break;
        case SILVER:
          silver_pieces.push_back(OPSquare(p));
          break;
        case BISHOP:
          bishop_pieces.push_back(OPSquare(p));
          break;
        case ROOK:
          rook_pieces.push_back(OPSquare(p));
          break;
        case GOLD:
          gold_pieces.push_back(OSquare(p));
          break;
        case KING:
          if (p.owner() == BLACK)
            king_pieces[0] = static_cast<char>(OPiece::position2Bits(p.square()));
          else
            king_pieces[1] = static_cast<char>(OPiece::position2Bits(p.square()));
          break;
        default:
          assert(false);
        }
      }
      turn = state.turn();

      std::sort(pawn_pieces.begin(),   pawn_pieces.end(),   oposition_sort());
      std::sort(lance_pieces.begin(),  lance_pieces.end(),  oposition_sort());
      std::sort(knight_pieces.begin(), knight_pieces.end(), oposition_sort());
      std::sort(silver_pieces.begin(), silver_pieces.end(), oposition_sort());
      std::sort(bishop_pieces.begin(), bishop_pieces.end(), oposition_sort());
      std::sort(rook_pieces.begin(),   rook_pieces.end(),   oposition_sort());
      std::sort(gold_pieces.begin(),   gold_pieces.end(),   oposition_sort());
    }

    SimpleState
    MiniBoard::getState() const
    {
      SimpleState state;
      state.init();

      for (PawnArray::const_iterator p = pawn_pieces.begin();
           p != pawn_pieces.end(); ++p)
      {
        Ptype ptype = PAWN;
        if (p->isPromoted())
          ptype = promote(ptype);
	state.setPiece(p->getOwner(), p->getSquare(), ptype);
      }
      for (LanceArray::const_iterator p = lance_pieces.begin();
           p != lance_pieces.end(); ++p)
      {
        Ptype ptype = LANCE;
        if (p->isPromoted())
          ptype = promote(ptype);
	state.setPiece(p->getOwner(), p->getSquare(), ptype);
      }
      for (KnightArray::const_iterator p = knight_pieces.begin();
           p != knight_pieces.end(); ++p)
      {
        Ptype ptype = KNIGHT;
        if (p->isPromoted())
          ptype = promote(ptype);
	state.setPiece(p->getOwner(), p->getSquare(), ptype);
      }
      for (SilverArray::const_iterator p = silver_pieces.begin();
           p != silver_pieces.end(); ++p)
      {
        Ptype ptype = SILVER;
        if (p->isPromoted())
          ptype = promote(ptype);
	state.setPiece(p->getOwner(), p->getSquare(), ptype);
      }
      for (BishopArray::const_iterator p = bishop_pieces.begin();
           p != bishop_pieces.end(); ++p)
      {
        Ptype ptype = BISHOP;
        if (p->isPromoted())
          ptype = promote(ptype);
	state.setPiece(p->getOwner(), p->getSquare(), ptype);
      }
      for (RookArray::const_iterator p = rook_pieces.begin();
           p != rook_pieces.end(); ++p)
      {
        Ptype ptype = ROOK;
        if (p->isPromoted())
          ptype = promote(ptype);
	state.setPiece(p->getOwner(), p->getSquare(), ptype);
      }
      for (GoldArray::const_iterator p = gold_pieces.begin();
           p != gold_pieces.end(); ++p)
      {
	state.setPiece(p->getOwner(), p->getSquare(), GOLD);
      }
      state.setPiece(BLACK, OPiece::bits2Square(king_pieces[0]), KING);
      state.setPiece(WHITE, OPiece::bits2Square(king_pieces[1]), KING);
      state.setTurn(turn);

      return state;
    }

    boost::dynamic_bitset<> 
    MiniBoard::toBits() const
    {
      boost::dynamic_bitset<> bits(total_bits);

      for (PawnArray::const_iterator p = pawn_pieces.begin();
           p != pawn_pieces.end(); ++p)
      {
        const int value = static_cast<int>(*p);
        const boost::dynamic_bitset<> mask(total_bits, static_cast<unsigned long>(value));
        bits = bits << OPSquare::total_bits | mask;
      }
      for (LanceArray::const_iterator p = lance_pieces.begin();
           p != lance_pieces.end(); ++p)
      {
        const int value = static_cast<int>(*p);
        const boost::dynamic_bitset<> mask(total_bits, static_cast<unsigned long>(value));
        bits = bits << OPSquare::total_bits | mask;
      }
      for (KnightArray::const_iterator p = knight_pieces.begin();
           p != knight_pieces.end(); ++p)
      {
        const int value = static_cast<int>(*p);
        const boost::dynamic_bitset<> mask(total_bits, static_cast<unsigned long>(value));
        bits = bits << OPSquare::total_bits | mask;
      }
      for (SilverArray::const_iterator p = silver_pieces.begin();
           p != silver_pieces.end(); ++p)
      {
        const int value = static_cast<int>(*p);
        const boost::dynamic_bitset<> mask(total_bits, static_cast<unsigned long>(value));
        bits = bits << OPSquare::total_bits | mask;
      }
      for (BishopArray::const_iterator p = bishop_pieces.begin();
           p != bishop_pieces.end(); ++p)
      {
        const int value = static_cast<int>(*p);
        const boost::dynamic_bitset<> mask(total_bits, static_cast<unsigned long>(value));
        bits = bits << OPSquare::total_bits | mask;
      }
      for (RookArray::const_iterator p = rook_pieces.begin();
           p != rook_pieces.end(); ++p)
      {
        const int value = static_cast<int>(*p);
        const boost::dynamic_bitset<> mask(total_bits, static_cast<unsigned long>(value));
        bits = bits << OPSquare::total_bits | mask;
      }
      for (GoldArray::const_iterator p = gold_pieces.begin();
           p != gold_pieces.end(); ++p)
      {
        const int value = static_cast<int>(*p);
        const boost::dynamic_bitset<> mask(total_bits, static_cast<unsigned long>(value));
        bits = bits << OSquare::total_bits | mask;
      }
      for (KingArray::const_iterator p = king_pieces.begin();
           p != king_pieces.end(); ++p)
      {
        const char value = static_cast<char>(*p);
        const boost::dynamic_bitset<> mask(total_bits, static_cast<unsigned long>(value));
        bits = bits << 8 | mask;
      }
      
      unsigned long value = 0;
      if (turn == BLACK)
        value = 0;
      else
        value = 1;
      const boost::dynamic_bitset<> mask(total_bits, static_cast<unsigned long>(value));
      bits = bits << 8 | mask;

      return bits;
    }

    std::string 
    MiniBoard::toBase64() const
    {
      const boost::dynamic_bitset<> bits = toBits();
      return misc::base64Encode(bits);
    }

    int fromBase64(const std::string& base64, MiniBoard& mb)
    {
      const boost::dynamic_bitset<> bits = misc::base64Decode(base64);
      if (bits.size() == 0)
        return 1;
      assert(bits.size() == MiniBoard::total_bits);

      for (int i=0; i<18; ++i)
      {
        const boost::dynamic_bitset<> mask(MiniBoard::total_bits, 1023ul);
        const unsigned long value = 
          (bits >> (MiniBoard::total_bits - OPSquare::total_bits*(i+1)) 
           & mask).to_ulong();
        const OPSquare p(static_cast<int>(value));
        mb.pawn_pieces.push_back(p);
      }
      for (int i=0; i<4; ++i)
      {
        const boost::dynamic_bitset<> mask(MiniBoard::total_bits, 1023ul);
        const unsigned long value = 
          (bits >> (MiniBoard::total_bits - OPSquare::total_bits*(i+19)) 
           & mask).to_ulong();
        const OPSquare p(static_cast<int>(value));
        mb.lance_pieces.push_back(p);
      }
      for (int i=0; i<4; ++i)
      {
        const boost::dynamic_bitset<> mask(MiniBoard::total_bits, 1023ul);
        const unsigned long value = 
          (bits >> (MiniBoard::total_bits - OPSquare::total_bits*(i+23)) 
           & mask).to_ulong();
        const OPSquare p(static_cast<int>(value));
        mb.knight_pieces.push_back(p);
      }
      for (int i=0; i<4; ++i)
      {
        const boost::dynamic_bitset<> mask(MiniBoard::total_bits, 1023ul);
        const unsigned long value = 
          (bits >> (MiniBoard::total_bits - OPSquare::total_bits*(i+27)) 
           & mask).to_ulong();
        const OPSquare p(static_cast<int>(value));
        mb.silver_pieces.push_back(p);
      }
      for (int i=0; i<2; ++i)
      {
        const boost::dynamic_bitset<> mask(MiniBoard::total_bits, 1023ul);
        const unsigned long value = 
          (bits >> (MiniBoard::total_bits - OPSquare::total_bits*(i+31)) 
           & mask).to_ulong();
        const OPSquare p(static_cast<int>(value));
        mb.bishop_pieces.push_back(p);
      }
      for (int i=0; i<2; ++i)
      {
        const boost::dynamic_bitset<> mask(MiniBoard::total_bits, 1023ul);
        const unsigned long value = 
          (bits >> (MiniBoard::total_bits - OPSquare::total_bits*(i+33)) 
           & mask).to_ulong();
        const OPSquare p(static_cast<int>(value));
        mb.rook_pieces.push_back(p);
      }
      for (int i=0; i<4; ++i)
      {
        const boost::dynamic_bitset<> mask(MiniBoard::total_bits, 511ul);
        const unsigned long value = 
          (bits >> (MiniBoard::total_bits - OPSquare::total_bits*34 - OSquare::total_bits*(i+1)) & mask).to_ulong();
        const OSquare p(static_cast<int>(value));
        mb.gold_pieces.push_back(p);
      }
      for (int i=0; i<2; ++i)
      {
        const boost::dynamic_bitset<> mask(MiniBoard::total_bits, 255ul);
        const unsigned long value = 
          (bits >> (MiniBoard::total_bits - OPSquare::total_bits*34 - OSquare::total_bits*4 - 8*(i+1)) & mask).to_ulong();
        mb.king_pieces[i] = static_cast<char>(value);
      }
      const boost::dynamic_bitset<> mask(MiniBoard::total_bits, 255ul);
      const unsigned long value = (bits & mask).to_ulong();
      if ((value&1) == 0)
        mb.turn = BLACK;
      else
        mb.turn = WHITE;
      return 0;
    }
  } // namespace record
} // namespace osl

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
