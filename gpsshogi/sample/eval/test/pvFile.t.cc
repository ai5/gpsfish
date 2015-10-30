/* pvFile.t.cc
 */
#include "pvFile.h"

#include <boost/test/unit_test.hpp>

using namespace gpsshogi;
using namespace osl;

BOOST_AUTO_TEST_CASE(PvFileTestConstruct)
{
  {
    gpsshogi::PVFileWriter pw("pvFile-test.gz");
  }
  {
    gpsshogi::PVFileReader pr("pvFile-test.gz");
  }
}

BOOST_AUTO_TEST_CASE(PvFileTestRW)
{
  PVVector pv;
  pv.push_back(Move(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK));
  pv.push_back(Move(Square(3,3),Square(3,4),PAWN,PTYPE_EMPTY,false,WHITE));
  {
    gpsshogi::PVFileWriter pw("pvFile-test.gz");
  }
  {
    int c = gpsshogi::PVFileReader::countPosition("pvFile-test.gz");
    BOOST_CHECK_EQUAL(0, c);
  }
  {
    gpsshogi::PVFileWriter pw("pvFile-test.gz");
    pw.newPosition(3, 4);
    pw.addPv(pv);
  }
  {
    gpsshogi::PVFileReader pr("pvFile-test.gz");
    int r, p;
    pr.newPosition(r, p);
    BOOST_CHECK_EQUAL(3, r);
    BOOST_CHECK_EQUAL(4, p);

    PVVector pv2;
    pr.readPv(pv2);
    BOOST_CHECK_EQUAL(pv.size(), pv2.size());
    BOOST_CHECK(std::equal(pv.begin(), pv.end(), pv2.begin()));
  }
  {
    int c = gpsshogi::PVFileReader::countPosition("pvFile-test.gz");
    BOOST_CHECK_EQUAL(1, c);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
