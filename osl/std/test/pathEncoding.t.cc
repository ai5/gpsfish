/* pathEncoding.t.cc
 */
#include "osl/pathEncoding.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <unordered_set>
using namespace osl;

BOOST_AUTO_TEST_CASE(PathEncodingTestPass)
{
  PathEncoding b(BLACK);
  PathEncoding b2 = b;
  b2.pushMove(Move::PASS(BLACK));
  BOOST_CHECK(b != b2);

  PathEncoding w(WHITE);
  PathEncoding w2 = w;
  w2.pushMove(Move::PASS(WHITE));
  BOOST_CHECK(w != w2);
}

BOOST_AUTO_TEST_CASE(PathEncodingTestTurn)
{
  PathEncoding b(BLACK);
  BOOST_CHECK_EQUAL(BLACK, b.turn());

  PathEncoding w(WHITE);
  BOOST_CHECK_EQUAL(WHITE, w.turn());
}

BOOST_AUTO_TEST_CASE(PathEncodingTestDag)
{
  const Move m76fu = Move(Square(7,7), Square(7,6), PAWN, 
			     PTYPE_EMPTY, false, BLACK);
  const Move m26fu = Move(Square(2,7), Square(2,6), PAWN, 
			     PTYPE_EMPTY, false, BLACK);
  const Move m34fu = Move(Square(3,3), Square(3,4), PAWN, 
			     PTYPE_EMPTY, false, WHITE);
  PathEncoding path1, path2;
  BOOST_CHECK_EQUAL(BLACK, path1.turn());

  PathEncoding path76fu(path1, m76fu);
  BOOST_CHECK(Path_Encoding_Table.get(0, m76fu));
  BOOST_CHECK(path1 != path76fu);
  path1.pushMove(m76fu);
  BOOST_CHECK_EQUAL(WHITE, path1.turn());
  BOOST_CHECK_EQUAL(WHITE, path76fu.turn());
  BOOST_CHECK_EQUAL(path1, path76fu);

  BOOST_CHECK(Path_Encoding_Table.get(1, m34fu));
  path1.pushMove(m34fu);
  BOOST_CHECK_EQUAL(BLACK, path1.turn());

  BOOST_CHECK(Path_Encoding_Table.get(2, m26fu));
  path1.pushMove(m26fu);
  BOOST_CHECK_EQUAL(WHITE, path1.turn());
  BOOST_CHECK_EQUAL(3, path1.getDepth());
  
  path2.pushMove(m26fu);
  BOOST_CHECK(path1 != path2);
  path2.pushMove(m34fu);
  BOOST_CHECK(path1 != path2);
  path2.pushMove(m76fu);
  BOOST_CHECK_EQUAL(WHITE, path1.turn());
  BOOST_CHECK(path1 != path2);
  BOOST_CHECK_EQUAL(path1.getDepth(), path2.getDepth());
}

BOOST_AUTO_TEST_CASE(PathEncodingTestPop)
{
  const Move m76fu = Move(Square(7,7), Square(7,6), PAWN, 
			     PTYPE_EMPTY, false, BLACK);
  const Move m34fu = Move(Square(3,3), Square(3,4), PAWN, 
			     PTYPE_EMPTY, false, WHITE);
  PathEncoding path1, path2;
  BOOST_CHECK(path1 == path2);
  BOOST_CHECK_EQUAL(BLACK, path1.turn());

  path1.pushMove(m76fu);
  BOOST_CHECK_EQUAL(WHITE, path1.turn());
  BOOST_CHECK_EQUAL(1, path1.getDepth());
  BOOST_CHECK(path1 != path2);

  path1.popMove(m76fu);
  BOOST_CHECK_EQUAL(BLACK, path1.turn());
  BOOST_CHECK(path1 == path2);
  BOOST_CHECK_EQUAL(0, path1.getDepth());
  
  path1.pushMove(m76fu);
  BOOST_CHECK_EQUAL(WHITE, path1.turn());

  path2.pushMove(m76fu);
  BOOST_CHECK_EQUAL(WHITE, path2.turn());
  BOOST_CHECK(path1 == path2);

  path1.pushMove(m34fu);
  BOOST_CHECK(path1 != path2);
  BOOST_CHECK_EQUAL(WHITE, path2.turn());

  path1.popMove(m34fu);
  BOOST_CHECK(path1 == path2);
  BOOST_CHECK_EQUAL(WHITE, path2.turn());
}

BOOST_AUTO_TEST_CASE(PathEncodingTestUniq)
{
  typedef std::unordered_set<unsigned int> set_t;
  set_t highs, lows;
  size_t deja_high = 0, deja_low = 0;
  for (size_t i=0; i<PathEncodingTable::MaxEncodingLength; ++i)
  {
    if (OslConfig::inUnitTest() < 2 && (i % 7))
      continue;
    if (i % 16==0)
      std::cerr << '.';
    for (size_t j=0; j<Square::SIZE; ++j)
    {
      for (int k=0; k<PTYPE_SIZE; ++k)
      {
	const unsigned long long value
	  = Path_Encoding_Table.get(i, Square::nth(j), (Ptype)(k+PTYPE_MIN));
	const unsigned int h = value >> 32;
	const unsigned int l = (unsigned int)value;
	BOOST_CHECK(l);
	BOOST_CHECK(h);
	BOOST_CHECK((l % 2) == 0);
	const bool uh = highs.insert(h).second;
	const bool ul = lows.insert(l).second;
	if (! uh)
	{
	  ++deja_high;
	}
	if (! ul)
	{
	  ++deja_low;
	}
	BOOST_CHECK(uh || ul);
      }
    }
  }
  if (OslConfig::verbose())
    std::cerr << " dl " << deja_low << " dh " << deja_high << "\n";
  BOOST_CHECK(deja_low + deja_high< 387); // 374
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
