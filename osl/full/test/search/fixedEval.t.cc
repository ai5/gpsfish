// fixedEval.t.cc
#include "osl/search/fixedEval.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
using namespace osl;
using namespace osl::search;

BOOST_AUTO_TEST_CASE(FixedEvalTestShow)
{
  if (! OslConfig::verbose())
    return;
  std::cerr << "max\t" << eval::EvalTraits<BLACK>::MAX_VALUE << "\n";
  std::cerr << "foul\t" << FixedEval::winByFoul(BLACK) << "\n";
  std::cerr << "checkmate\t" << FixedEval::winByCheckmate(BLACK) << "\n";
  std::cerr << "window max\t" << FixedEval::windowMax(BLACK) << "\n";
  std::cerr << "win th\t" << FixedEval::winThreshold(BLACK) << "\n";
  std::cerr << "-inf\t" << FixedEval::minusInfty(BLACK) << "\n";
  std::cerr << "threat\t" << FixedEval::threatmatePenalty(BLACK) << "\n";
}
static bool isEven(int val)
{
  return (val % 2) == 0;
}
BOOST_AUTO_TEST_CASE(FixedEvalTestConstant) {
  BOOST_CHECK(FixedEval::winByFoul(BLACK) > 0);
  BOOST_CHECK(FixedEval::winByFoul(WHITE) < 0);
  BOOST_CHECK(isEven(FixedEval::winByFoul(BLACK)));
  BOOST_CHECK(isEven(FixedEval::winByFoul(WHITE)));

  BOOST_CHECK_EQUAL(FixedEval::winByFoul(BLACK), FixedEval::winByLoop(BLACK));
  BOOST_CHECK_EQUAL(FixedEval::winByFoul(WHITE), FixedEval::winByLoop(WHITE));

  BOOST_CHECK(FixedEval::winByCheckmate(BLACK) > 0);
  BOOST_CHECK(FixedEval::winByCheckmate(WHITE) < 0);
  BOOST_CHECK(isEven(FixedEval::winByCheckmate(BLACK)));
  BOOST_CHECK(isEven(FixedEval::winByCheckmate(WHITE)));

  BOOST_CHECK(FixedEval::isWinValue(BLACK, FixedEval::winByCheckmate(BLACK)));
  BOOST_CHECK(FixedEval::isWinValue(WHITE, FixedEval::winByCheckmate(WHITE)));

  BOOST_CHECK(FixedEval::brinkmatePenalty(BLACK, 0) < 0);
  BOOST_CHECK(FixedEval::brinkmatePenalty(WHITE, 0) > 0);
  BOOST_CHECK(isEven(FixedEval::brinkmatePenalty(BLACK, 2)));
  BOOST_CHECK(isEven(FixedEval::brinkmatePenalty(WHITE, 3)));
  BOOST_CHECK(FixedEval::brinkmatePenalty(BLACK, 0) 
	      > FixedEval::brinkmatePenalty(BLACK, 1000));
  BOOST_CHECK(FixedEval::brinkmatePenalty(WHITE, 0) 
	      < FixedEval::brinkmatePenalty(WHITE, 1000));
  BOOST_CHECK(! FixedEval::isWinValue
	      (WHITE, FixedEval::brinkmatePenalty(BLACK, 4000)));

  BOOST_CHECK(FixedEval::threatmatePenalty(BLACK) < 0);
  BOOST_CHECK(FixedEval::threatmatePenalty(WHITE) > 0);
  BOOST_CHECK(isEven(FixedEval::threatmatePenalty(BLACK)));
  BOOST_CHECK(isEven(FixedEval::threatmatePenalty(WHITE)));

  BOOST_CHECK(FixedEval::minusInfty(BLACK) < 0);
  BOOST_CHECK(FixedEval::minusInfty(WHITE) > 0);
  BOOST_CHECK(isEven(FixedEval::minusInfty(BLACK)));
  BOOST_CHECK(isEven(FixedEval::minusInfty(WHITE)));

  BOOST_CHECK(eval::isConsistentValue(FixedEval::minusInfty(BLACK)));
  BOOST_CHECK(eval::isConsistentValue(FixedEval::minusInfty(WHITE)));

  BOOST_CHECK(FixedEval::winThreshold(BLACK) > 0);
  BOOST_CHECK(FixedEval::winThreshold(WHITE) < 0);
  BOOST_CHECK(! isEven(FixedEval::winThreshold(BLACK)));
  BOOST_CHECK(! isEven(FixedEval::winThreshold(WHITE)));

  BOOST_CHECK(FixedEval::windowMax(BLACK) > 0);
  BOOST_CHECK(FixedEval::windowMax(WHITE) < 0);
  BOOST_CHECK(isEven(FixedEval::windowMax(BLACK)));
  BOOST_CHECK(isEven(FixedEval::windowMax(WHITE)));

  BOOST_CHECK(FixedEval::winByCheckmate(BLACK) < FixedEval::winByFoul(BLACK));
  BOOST_CHECK(FixedEval::winByCheckmate(WHITE) > FixedEval::winByFoul(WHITE));

  BOOST_CHECK(FixedEval::threatmatePenalty(BLACK) < FixedEval::winByCheckmate(BLACK));
  BOOST_CHECK(FixedEval::threatmatePenalty(WHITE) > FixedEval::winByCheckmate(WHITE));
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
