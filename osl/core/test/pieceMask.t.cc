#include "osl/bits/pieceMask.h"
#include <boost/test/unit_test.hpp>

using namespace osl;

BOOST_AUTO_TEST_CASE(PieceMaskTestCount2)
{
    PieceMask piece_mask;
    BOOST_CHECK_EQUAL(0, piece_mask.countBit2());
    piece_mask.set(0);
    BOOST_CHECK_EQUAL(1, piece_mask.countBit2());
    piece_mask.set(1);
    BOOST_CHECK_EQUAL(2, piece_mask.countBit2());
}

BOOST_AUTO_TEST_CASE(PieceMaskTestIsEmpty)
{
  {
    PieceMask piece_mask;
    BOOST_CHECK(piece_mask.none());
    piece_mask.setAll();
    BOOST_CHECK(!piece_mask.none());
    piece_mask.resetAll();
    BOOST_CHECK(piece_mask.none());
    piece_mask.set(10);
    BOOST_CHECK(!piece_mask.none());
    piece_mask.reset(10);
    BOOST_CHECK(piece_mask.none());
    piece_mask.set(37);
    BOOST_CHECK(!piece_mask.none());
    piece_mask.reset(37);
    BOOST_CHECK(piece_mask.none());
  }
  {
    BOOST_CHECK(PieceMask64(mask_t::makeDirect(0x0uLL)).none());
    BOOST_CHECK(!PieceMask64(mask_t::makeDirect(0x8uLL)).none());
    BOOST_CHECK(!PieceMask64(mask_t::makeDirect(0x800000000uLL)).none());
  }
}

BOOST_AUTO_TEST_CASE(PieceMaskTestSetGet)
{
  {
    PieceMask piece_mask;
    BOOST_CHECK(piece_mask.getMask(0).value()==0uLL);
    piece_mask.set(10);
    BOOST_CHECK(piece_mask.getMask(0).value()==(1uLL<<10));
    piece_mask.set(37);
    BOOST_CHECK(piece_mask.getMask(0).value() == ((1uLL<<10)|(1uLL<<37)));
  }
  {
    PieceMask piece_mask;
    piece_mask.setMask(0, mask_t::makeDirect(0xf00000000fuLL));
    BOOST_CHECK(piece_mask.test(3));
    BOOST_CHECK(!piece_mask.test(4));
    BOOST_CHECK(!piece_mask.test(35));
    BOOST_CHECK(piece_mask.test(36));
  }
}

BOOST_AUTO_TEST_CASE(PieceMaskTestMultiple)
{
  {
    PieceMask piece_mask;
    BOOST_CHECK(!piece_mask.hasMultipleBit());
    piece_mask.set(10);
    BOOST_CHECK(!piece_mask.hasMultipleBit());
    piece_mask.set(37);
    BOOST_CHECK(piece_mask.hasMultipleBit());
    piece_mask.reset(10);
    BOOST_CHECK(!piece_mask.hasMultipleBit());
    piece_mask.set(32);
    BOOST_CHECK(piece_mask.hasMultipleBit());
    piece_mask.set(12);
    BOOST_CHECK(piece_mask.hasMultipleBit());
    piece_mask.set(31);
    BOOST_CHECK(piece_mask.hasMultipleBit());
    piece_mask.reset(32);
    BOOST_CHECK(piece_mask.hasMultipleBit());
    piece_mask.reset(37);
    BOOST_CHECK(piece_mask.hasMultipleBit());
    piece_mask.reset(12);
    BOOST_CHECK(!piece_mask.hasMultipleBit());
  }
}

BOOST_AUTO_TEST_CASE(PieceMaskTestSelectBit)
{
  PieceMask m;
  for (int i=0; i<Piece::SIZE; ++i) {
    m.set(i);
  }

  BOOST_CHECK_EQUAL(m.selectBit<PAWN>(),   mask_t::makeDirect(PtypeFuns<PAWN>::indexMask));
  BOOST_CHECK_EQUAL(m.selectBit<LANCE>(),  mask_t::makeDirect(PtypeFuns<LANCE>::indexMask));
  BOOST_CHECK_EQUAL(m.selectBit<KNIGHT>(), mask_t::makeDirect(PtypeFuns<KNIGHT>::indexMask));
  BOOST_CHECK_EQUAL(m.selectBit<SILVER>(), mask_t::makeDirect(PtypeFuns<SILVER>::indexMask));
  BOOST_CHECK_EQUAL(m.selectBit<GOLD>(),   mask_t::makeDirect(PtypeFuns<GOLD>::indexMask));
  BOOST_CHECK_EQUAL(m.selectBit<BISHOP>(), mask_t::makeDirect(PtypeFuns<BISHOP>::indexMask));
  BOOST_CHECK_EQUAL(m.selectBit<ROOK>(),   mask_t::makeDirect(PtypeFuns<ROOK>::indexMask));
}

BOOST_AUTO_TEST_CASE(PieceMaskTestClearBit)
{
  PieceMask m;
  for (int i=0; i<Piece::SIZE; ++i) {
    m.set(i);
  }
  m.clearBit<KNIGHT>();
  BOOST_CHECK(m.selectBit<KNIGHT>().none());
  BOOST_CHECK(~(m.selectBit<PAWN>()).none());
  BOOST_CHECK(~(m.selectBit<LANCE>()).none());
  BOOST_CHECK(~(m.selectBit<SILVER>()).none());
  BOOST_CHECK(~(m.selectBit<GOLD>()).none());
  BOOST_CHECK(~(m.selectBit<BISHOP>()).none());
  BOOST_CHECK(~(m.selectBit<ROOK>()).none());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
