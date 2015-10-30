#include "osl/search/searchRecorder.h"
#include "osl/moveLogProb.h"
#include <boost/test/unit_test.hpp>
using namespace osl;

BOOST_AUTO_TEST_CASE(SearchRecorderTestWrite)
{
  const bool verbose = OslConfig::verbose();

  SearchRecorder recorder("SearchRecorderTest.log");
  MoveLogProb m(Move(Square(6,5),Square(2,1),PBISHOP,KNIGHT,true,BLACK),
		7777777);
  recorder.tryMove(m, 111, 11111);
  recorder.recordValue(m, 222, true, 22222);
  recorder.recordTopLevelLowFail(m, 333);
  recorder.recordTopLevelHighFail(m, 444);
  recorder.startSearch(555);
  recorder.finishSearch(m.move(), 1.0, verbose);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
