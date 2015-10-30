#include "osl/bits/pieceStand.h"
#include "osl/bits/ptypeTable.h"
#include "osl/numEffectState.h"
#include "osl/random.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>

using namespace osl;

namespace 
{
  bool isMax(PieceStand l, PieceStand r, PieceStand m)
  {
    for (unsigned int i=0; i<PieceStand::order.size(); ++i)
    {
      const Ptype ptype = PieceStand::order[i];
      if (std::max(l.get(ptype), r.get(ptype)) != m.get(ptype))
	return false;
    }
    return true;
  }

  void testMaxRecursive(PieceStand stand, unsigned int ptype_index)
  {
    using osl::random;
    if (ptype_index < PieceStand::order.size())
    {
      const Ptype ptype = PieceStand::order[ptype_index];
      const int max_pieces
	= Ptype_Table.getIndexLimit(ptype) - Ptype_Table.getIndexMin(ptype);
      for (int i=0; i<max_pieces; ++i)
      {
	PieceStand stand_copy(stand);
	stand_copy.add(ptype, i);
	testMaxRecursive(stand_copy, ptype_index+1);
      }
      return;
    }
    for (unsigned int i=0; i<(OslConfig::inUnitTest() < 2 ? 8 : 16); ++i)
    {
      const PieceStand target(random() % 19, random() % 5, random() % 5,
			      random() % 5, random() % 5, random() % 3, 
			      random() % 3, random() % 3);
      const PieceStand ml = stand.max(target);
      const PieceStand ml2 = stand.max2(target);
      BOOST_CHECK(isMax(stand, target, ml));
      BOOST_CHECK(isMax(stand, target, ml2));
    
      const PieceStand mr = target.max(stand);
      const PieceStand mr2 = target.max2(stand);
      BOOST_CHECK(isMax(stand, target, mr));
      BOOST_CHECK(isMax(stand, target, mr2));
    }
    // SLOOOW!!
    // testMaxRecursive2(stand, empty_stand, 0);
  }

  void testMaxRecursive2(PieceStand l, PieceStand r, unsigned int ptype_index)
  {
    if (ptype_index < PieceStand::order.size())
    {
      const Ptype ptype = PieceStand::order[ptype_index];
      const int max_pieces
	= Ptype_Table.getIndexLimit(ptype) - Ptype_Table.getIndexMin(ptype);
      for (int i=0; i<max_pieces; ++i)
      {
	PieceStand stand_copy(r);
	stand_copy.add(ptype, i);
	testMaxRecursive2(l, stand_copy, ptype_index+1);
      }
      return;
    }
    const PieceStand m = l.max(r);
    BOOST_CHECK(isMax(l, r, m));
  }
}

BOOST_AUTO_TEST_CASE(PieceStandTestMax)
{
  const PieceStand l(12,1,2,3,4,1,0,0);
  const PieceStand r(8,3,0,4,2,0,2,0);
  const PieceStand m = l.max(r);
  BOOST_CHECK(isMax(l, r, m));

  PieceStand empty_stand;
  testMaxRecursive(empty_stand, 0);
}


BOOST_AUTO_TEST_CASE(PieceStandTestCarry)
{
  unsigned int flag=0;
  flag |= (1<<30);
  flag |= (1<<27);
  flag |= (1<<23);
  flag |= (1<<17);
  flag |= (1<<13);
  flag |= (1<<9);
  flag |= (1<<5);
  flag |= (1<<2);
  BOOST_CHECK_EQUAL(flag, PieceStand::carryMask);

  PieceStand pieces;
  pieces.carriesOn();
  for (int i=KING; i<=PTYPE_MAX; ++i)
    BOOST_CHECK_EQUAL(0u, pieces.get((Ptype)i));
  pieces.carriesOff();
  for (int i=KING; i<=PTYPE_MAX; ++i)
    BOOST_CHECK_EQUAL(0u, pieces.get((Ptype)i));
}


BOOST_AUTO_TEST_CASE(PieceStandTestSet)
{
  PieceStand pieces;
  for (int i=KING; i<=PTYPE_MAX; ++i)
  {
    const Ptype pi = (Ptype)i;
    const unsigned int num =	// 持駒になる最大の個数
      Ptype_Table.getIndexLimit(pi) - Ptype_Table.getIndexMin(pi);
    BOOST_CHECK_EQUAL(0u, pieces.get(pi));
    pieces.add(pi, num);
    for (int j=KING; j<=PTYPE_MAX; ++j)
    {
      const Ptype pj = (Ptype)j;
      if (i==j)
	BOOST_CHECK_EQUAL(num, pieces.get(pj));
      else
	BOOST_CHECK_EQUAL(0u, pieces.get(pj));
    }
    pieces.sub(pi, num);
    BOOST_CHECK_EQUAL(0u, pieces.get(pi));
  }
}

BOOST_AUTO_TEST_CASE(PieceStandTestSuperior)
{
  PieceStand p1, p2;
  BOOST_CHECK(p1.isSuperiorOrEqualTo(p2));
  BOOST_CHECK(p2.isSuperiorOrEqualTo(p1));
  
  p1.add(PAWN, 3);

  BOOST_CHECK(p1.isSuperiorOrEqualTo(p2));
  BOOST_CHECK(! p2.isSuperiorOrEqualTo(p1));

  BOOST_CHECK_EQUAL(3u, p1.get(PAWN));

  p2.add(GOLD, 1);
  
  BOOST_CHECK(! p1.isSuperiorOrEqualTo(p2));
  BOOST_CHECK(! p2.isSuperiorOrEqualTo(p1));

  p1.add(GOLD, 1);

  BOOST_CHECK(p1.isSuperiorOrEqualTo(p2));
  BOOST_CHECK(! p2.isSuperiorOrEqualTo(p1));
}

BOOST_AUTO_TEST_CASE(PieceStandTestAddSubAtmostOnePiece) 
{
  PieceStand p1;
  PieceStand   pPawn(1,0,0,0,0,0,0,0);
  PieceStand  pLance(0,1,0,0,0,0,0,0);
  PieceStand pKnight(0,0,1,0,0,0,0,0);
  PieceStand pSilver(0,0,0,1,0,0,0,0);
  PieceStand   pGold(0,0,0,0,1,0,0,0);
  PieceStand pBishop(0,0,0,0,0,1,0,0);
  PieceStand   pRook(0,0,0,0,0,0,1,0);
  PieceStand   pKing(0,0,0,0,0,0,0,1);

  p1.addAtmostOnePiece(pPawn);
  p1.addAtmostOnePiece(pKnight);
  p1.addAtmostOnePiece(pGold);
  p1.addAtmostOnePiece(pRook);

  BOOST_CHECK_EQUAL(PieceStand(1,0,1,0,1,0,1,0),p1);

  p1.addAtmostOnePiece(pPawn);
  p1.addAtmostOnePiece(pLance);
  p1.addAtmostOnePiece(pKnight);
  p1.addAtmostOnePiece(pSilver);

  BOOST_CHECK_EQUAL(PieceStand(2,1,2,1,1,0,1,0),p1);

  p1.addAtmostOnePiece(pPawn);
  p1.addAtmostOnePiece(pLance);
  p1.addAtmostOnePiece(pGold);
  p1.addAtmostOnePiece(pBishop);

  BOOST_CHECK_EQUAL(PieceStand(3,2,2,1,2,1,1,0),p1);

  p1.addAtmostOnePiece(pPawn);
  p1.addAtmostOnePiece(pLance);
  p1.addAtmostOnePiece(pKnight);
  p1.addAtmostOnePiece(pSilver);
  p1.addAtmostOnePiece(pGold);
  p1.addAtmostOnePiece(pBishop);
  p1.addAtmostOnePiece(pRook);
  p1.addAtmostOnePiece(pKing);

  BOOST_CHECK_EQUAL(PieceStand(4,3,3,2,3,2,2,1),p1);

  p1.subAtmostOnePiece(pPawn);
  p1.subAtmostOnePiece(pKnight);
  p1.subAtmostOnePiece(pGold);
  p1.subAtmostOnePiece(pRook);

  BOOST_CHECK_EQUAL(PieceStand(3,3,2,2,2,2,1,1),p1);

  p1.subAtmostOnePiece(pPawn);
  p1.subAtmostOnePiece(pLance);
  p1.subAtmostOnePiece(pKnight);
  p1.subAtmostOnePiece(pSilver);

  BOOST_CHECK_EQUAL(PieceStand(2,2,1,1,2,2,1,1),p1);

  p1.subAtmostOnePiece(pPawn);
  p1.subAtmostOnePiece(pLance);
  p1.subAtmostOnePiece(pGold);
  p1.subAtmostOnePiece(pBishop);

  BOOST_CHECK_EQUAL(PieceStand(1,1,1,1,1,1,1,1),p1);

  p1.subAtmostOnePiece(pPawn);
  p1.subAtmostOnePiece(pLance);
  p1.subAtmostOnePiece(pKnight);
  p1.subAtmostOnePiece(pSilver);
  p1.subAtmostOnePiece(pGold);
  p1.subAtmostOnePiece(pBishop);
  p1.subAtmostOnePiece(pRook);
  p1.subAtmostOnePiece(pKing);

  BOOST_CHECK_EQUAL(PieceStand(0,0,0,0,0,0,0,0),p1);

}

BOOST_AUTO_TEST_CASE(PieceStandTestNextPrev) 
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  std::cerr << OslConfig::testCsaFile("FILES") << std::endl;
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0;i<(OslConfig::inUnitTest() < 2 ? 10 : 900) && (ifs >> file_name) ; i++)
  {
    if ((i % 100) == 0)
      std::cerr << '.';
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const RecordMinimal record=CsaFileMinimal(file_name).load();
    auto& moves=record.moves;

    auto state = record.initial_state;
    PieceStand black(BLACK, state);
    PieceStand white(WHITE, state);
    
    for (unsigned int i=0; i<moves.size(); i++)
    {
      const Move m = moves[i];
      state.makeMove(m);

      PieceStand black_new(BLACK, state);
      PieceStand white_new(WHITE, state);

      BOOST_CHECK_EQUAL(black_new, black.nextStand(BLACK, m));
      BOOST_CHECK_EQUAL(white_new, white.nextStand(WHITE, m));

      BOOST_CHECK_EQUAL(black, black_new.previousStand(BLACK, m));
      BOOST_CHECK_EQUAL(white, white_new.previousStand(WHITE, m));

      black = black_new;
      white = white_new;
    }
  }
}

BOOST_AUTO_TEST_CASE(PieceStandTestIONumbers)
{
  PieceStand src;
  for (unsigned int i=0; i<PieceStand::order.size(); ++i)
  {
    const Ptype ptype = PieceStand::order[i];
    src.add(ptype, osl::random() % 3);
  }
  std::stringstream ss;
  PieceStandIO::writeNumbers(ss, src);
  PieceStand dst;
  PieceStandIO::readNumbers(ss, dst);
    
  BOOST_CHECK_EQUAL(src, dst);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
