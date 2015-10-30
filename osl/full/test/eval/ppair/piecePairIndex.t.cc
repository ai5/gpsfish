/* piecePairIndexTest.cc
 */
#include "osl/eval/ppair/piecePairIndex.h"
#include <boost/test/unit_test.hpp>
#include <bitset>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::eval;
using namespace osl::eval::ppair;

typedef std::bitset<PiecePairIndex::maxPairIndex> set_t;
set_t visited;
static void visitRelation(size_t index)
{
  size_t i1, i2;
  PiecePairIndex::meltIndex(index, i1, i2);
  const size_t i = PiecePairIndex::canonicalIndexOf(i1, i2);

  if (visited[i])
  {
    std::cerr << "oops " << i << "\n";
    size_t i1, i2;
    PiecePairIndex::meltIndex(i, i1, i2);
    std::cerr << i1 << " " << i2 << "\n";
    Square pos;
    PtypeO ptypeo;
    PiecePairIndex::meltIndex(i1, pos, ptypeo);
    std::cerr << pos << " " << ptypeo << "\n";
    PiecePairIndex::meltIndex(i2, pos, ptypeo);
    std::cerr << pos << " " << ptypeo << "\n";
  }
  BOOST_CHECK(! visited[i]);
  visited.flip(i);
}

BOOST_AUTO_TEST_CASE(PiecePairIndexTestForEachRelation)
{
  PiecePairIndex::forEachRelation(visitRelation);
  size_t numCanons = 0;
  for (size_t i=0; i<PiecePairIndex::maxPairIndex; ++i)
  {
    size_t i1, i2;
    Square pos1, pos2;
    PtypeO ptypeo1=PTYPEO_MIN, ptypeo2=PTYPEO_MIN;

    PiecePairIndex::meltIndex(i, i1, i2);
    if (PiecePairIndex::canonicalIndexOf(i1, i2) != i)
      goto confirm_not_visited;

    PiecePairIndex::meltIndex(i1, pos1, ptypeo1);
    if (! pos1.isValid())
      goto confirm_not_visited;
    if (! isPiece(ptypeo1))
      goto confirm_not_visited;
    PiecePairIndex::meltIndex(i2, pos2, ptypeo2);
    if (! pos2.isValid())
      goto confirm_not_visited;
    if (! isPiece(ptypeo2))
      goto confirm_not_visited;
    if ((pos1 == pos2)
	&& pos1.isOnBoard()
	&& (ptypeo1 != ptypeo2))
      goto confirm_not_visited;
    if (pos1.isPieceStand() && isPromoted(ptypeo1))
      goto confirm_not_visited;
    if (pos2.isPieceStand() && isPromoted(ptypeo2))
      goto confirm_not_visited;

    // must be visited
    if (! visited[i])
    {
      std::cerr << "must visit " << i1 << " " << i2 << "\n"
		<< pos1 << " " << ptypeo1 << " " << pos2 << " " << ptypeo2
		<< "\n";
    }
    ++numCanons;
    continue;
  confirm_not_visited:
    if (visited[i])
    {
      std::cerr << "must not visit " << i1 << " " << i2 << "\n"
		<< pos1 << " " << ptypeo1 << " " << pos2 << " " << ptypeo2
		<< "\n";
    }
  }
  BOOST_CHECK_EQUAL((size_t)2578852, numCanons);
  BOOST_CHECK_EQUAL(numCanons, visited.count());
}


BOOST_AUTO_TEST_CASE(PiecePairIndexTestUniqIndex)
{
  set_t s;
  s.flip(PiecePairIndex::positionIndexOf(Square::STAND()));
  for (int x=1; x<=9; ++x)
  {
    for (int y=1; y<=9; ++y)
    {
      const Square pos1 = Square(x,y);
      const unsigned int index = PiecePairIndex::positionIndexOf(pos1);
      BOOST_CHECK(index < PiecePairIndex::maxSquareIndex);
      BOOST_CHECK(! s.test(index));
      s.flip(index);
    }
  }
  s.reset();
  for (int p=PPAWN; p<=ROOK; ++p)
  {
    const Ptype ptype = static_cast<Ptype>(p);
    const unsigned int indexBlack = PiecePairIndex::ptypeOIndexOf(newPtypeO(BLACK,ptype));
    const unsigned int indexWhite = PiecePairIndex::ptypeOIndexOf(newPtypeO(WHITE,ptype));
    
    BOOST_CHECK(indexBlack < PiecePairIndex::maxPtypeOIndex);
    BOOST_CHECK(! s.test(indexBlack));
    s.flip(indexBlack);
    BOOST_CHECK(indexWhite < PiecePairIndex::maxPtypeOIndex);
    BOOST_CHECK(! s.test(indexWhite));
    s.flip(indexWhite);
  }
}

/** 本当はmember 変数にするべきだが手抜き*/
static set_t dejavu;
static void testPieces(Piece p1, Piece p2)
{
  const unsigned int index12 = PiecePairIndex::indexOf(p1, p2);
  // std::cerr << index12 << " " << p1 << " " << p2 << "\n";
  BOOST_CHECK(index12 < PiecePairIndex::maxPairIndex);
  BOOST_CHECK(! dejavu.test(index12)
		 || (std::cerr << p1 << p2, 0));
  dejavu.flip(index12);
  if ((p1.square() == p2.square())
      && (p1.ptype() == p2.ptype())
      && (p1.owner() == p2.owner()))
    return;
  
  const unsigned int index21 = PiecePairIndex::indexOf(p2, p1);
  // std::cerr << index21 << " " << p2 << "\n";
  BOOST_CHECK(index21 < PiecePairIndex::maxPairIndex);
  BOOST_CHECK(! dejavu.test(index21)
		 || (std::cerr << p1 << p2, 0));
  dejavu.flip(index21);
}

BOOST_AUTO_TEST_CASE(PiecePairIndexTestIndex)
{
  for (int x=1; x<=9; ++x)
  {
    std::cerr << x << " ";
    for (int y=1; y<=9; ++y)
    {
      const Square pos1 = Square(x,y);
      for (int ptype=PPAWN; ptype<=PTYPE_MAX; ++ptype)
      {
	const Piece p1 = Piece(BLACK, static_cast<Ptype>(ptype), 0/*?*/, pos1);
	const unsigned int index = PiecePairIndex::indexOf(p1);
	BOOST_CHECK(index < PiecePairIndex::maxPieceIndex
		       || (std::cerr << p1 << "\n" << index << "\n" << PiecePairIndex::maxPieceIndex << "\n", 0));
	{
	  PtypeO ptypeo;
	  Square pos;
	  PiecePairIndex::meltIndex(index, pos, ptypeo);
	  BOOST_CHECK_EQUAL(newPtypeO(BLACK, static_cast<Ptype>(ptype)),
			       ptypeo);
	  BOOST_CHECK_EQUAL(pos1, pos);
	}
	const Piece p1w = Piece(WHITE, static_cast<Ptype>(ptype), 0/*?*/, pos1);
	BOOST_CHECK(PiecePairIndex::indexOf(p1w) < PiecePairIndex::maxPieceIndex);
	{
	  PtypeO ptypeo;
	  Square pos;
	  PiecePairIndex::meltIndex(PiecePairIndex::indexOf(p1w),
					pos, ptypeo);
	  BOOST_CHECK_EQUAL(newPtypeO(WHITE, static_cast<Ptype>(ptype)),
			       ptypeo);
	  BOOST_CHECK_EQUAL(pos1, pos);
	}

	testPieces(p1, p1);
	testPieces(p1, p1w);
	testPieces(p1w, p1w);

	for (int x2=x; x2<=9; ++x2)
	{
	  for (int y2=y; y2<=9; ++y2)
	  {
	    Square pos2 = Square(x2,y2);
	    if (pos1 == pos2)
	      continue;
	    
	    for (int ptype2=PPAWN; ptype2<=PTYPE_MAX; ++ptype2)
	    {
	      const Piece p2 = Piece(BLACK, static_cast<Ptype>(ptype2), 0/*?*/, pos2);
	      const unsigned int index2 = PiecePairIndex::indexOf(p2);
	      BOOST_CHECK(index2 < PiecePairIndex::maxPieceIndex
			     || (std::cerr << p1 << "\n" << index << "\n" << PiecePairIndex::maxPieceIndex << "\n", 0));
	      const Piece p2w = Piece(WHITE, static_cast<Ptype>(ptype2), 0/*?*/, pos2);
	      BOOST_CHECK(PiecePairIndex::indexOf(p2w) < PiecePairIndex::maxPieceIndex);

	      testPieces(p1, p2);
	      testPieces(p1, p2w);
	      testPieces(p1w, p2);
	      testPieces(p1w, p2w);
	    }
	  }
	}

	for (int ptype2=KING; ptype2<=PTYPE_MAX; ++ptype2)
	{
	  const Piece p2 = Piece(BLACK, static_cast<Ptype>(ptype2), 0/*?*/, Square::STAND());
	  const unsigned int index2 = PiecePairIndex::indexOf(p2);
	  BOOST_CHECK(index2 < PiecePairIndex::maxPieceIndex
			 || (std::cerr << p1 << "\n" << index << "\n" << PiecePairIndex::maxPieceIndex << "\n", 0));
	  const Piece p2w = Piece(WHITE, static_cast<Ptype>(ptype2), 0/*?*/, Square::STAND());
	  BOOST_CHECK(PiecePairIndex::indexOf(p2w) < PiecePairIndex::maxPieceIndex);

	  testPieces(p1, p2);
	  testPieces(p1, p2w);
	  testPieces(p1w, p2);
	  testPieces(p1w, p2w);
	}
      }	// ptype
    } // x
  } // y

  // 持駒同士は最後に
  for (int ptype2=KING; ptype2<=PTYPE_MAX; ++ptype2)
  {
    const Piece p2 = Piece(BLACK, static_cast<Ptype>(ptype2), 0/*?*/, Square::STAND());
    const unsigned int index2 = PiecePairIndex::indexOf(p2);
    BOOST_CHECK(index2 < PiecePairIndex::maxPieceIndex);
    const Piece p2w = Piece(WHITE, static_cast<Ptype>(ptype2), 0/*?*/, Square::STAND());
    BOOST_CHECK(PiecePairIndex::indexOf(p2w) < PiecePairIndex::maxPieceIndex);
    testPieces(p2, p2);
    testPieces(p2, p2w);
    testPieces(p2w, p2w);
  }
#if 0
  std::cerr << PiecePairIndex::maxPieceIndex << "\n";
  std::cerr << PiecePairIndex::maxPairIndex << "\n";
#endif
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
