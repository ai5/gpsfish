#include "osl/book/miniBoardChar50.h"
#include "osl/book/compactBoard.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>
using namespace osl;
using namespace osl::book;

BOOST_AUTO_TEST_CASE(MiniBoardChar50TestConstruct)
{
  const SimpleState state(HIRATE);
  MiniBoardChar50 mb(state);
  const SimpleState retb(mb.toSimpleState(BLACK)), retw(mb.toSimpleState(WHITE));
  BOOST_CHECK(retb.isConsistent());
  BOOST_CHECK(retw.isConsistent());

  BOOST_CHECK_EQUAL(CompactBoard(state), CompactBoard(retb));
  BOOST_CHECK_EQUAL(CompactBoard(state.rotate180()), CompactBoard(retw));
}

BOOST_AUTO_TEST_CASE(MiniBoardChar50TestFile)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string filename;
  for (int i=0; i<(OslConfig::inUnitTestShort() ? 10 : 900) && (ifs >> filename); ++i)
  {
    if ((i % 100) == 0)
      std::cerr << '.';
    if (filename == "") 
      break;
    filename = OslConfig::testCsaFile(filename);

    const auto record=CsaFileMinimal(filename).load();
    NumEffectState state(record.initialState());
    for (auto move:record.moves)
    {
      state.makeMove(move);
      MiniBoardChar50 mb(state);
      BOOST_CHECK_EQUAL(static_cast<const SimpleState&>(state), mb.toSimpleState(state.turn()));
    }
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

