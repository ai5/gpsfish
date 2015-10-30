// dominanceCheck.t.cc
#include "osl/search/dominanceCheck.h"
#include "osl/csa.h"
#include "osl/numEffectState.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::search;

static void
confirmDetectionInLast(HashKey key, const std::vector<Move>& moves,
		       DominanceCheck::Result result)
{
  HashKeyStack history;
  for (size_t i=0; i<moves.size(); ++i)
  {
    BOOST_CHECK_EQUAL(DominanceCheck::NORMAL,
			 DominanceCheck::detect(history, key));
    history.push(key);
    key = key.newHashWithMove(moves[i]);
  }
  BOOST_CHECK_EQUAL(result, DominanceCheck::detect(history, key));
}

BOOST_AUTO_TEST_CASE(SearchDominanceCheckLoseBlack)
{
  const char *test_record_str = 
    "P1 *  *  *  *  *  *  * -KY-OU \n"
    "P2 *  *  *  *  *  *  * -KY+TO\n"
    "P3 *  *  *  *  *  *  * -KY * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  * +KE * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9+OU *  *  *  *  *  *  *  * \n"
    "P+00FU\n"
    "P-00AL\n"
    "-\n"
    "-1112OU\n"
    "+0013FU\n"
    "-1211OU\n"
    "+1312TO\n";
  CsaString test_record(test_record_str);
  
  NumEffectState state(test_record.initialState());
  const std::vector<Move>& moves = test_record.load().moves;
  confirmDetectionInLast(HashKey(state), moves, DominanceCheck::LOSE);
}

BOOST_AUTO_TEST_CASE(SearchDominanceCheckLoseWhite)
{
  const char *test_record_str = 
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 * -KE *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 * +KE *  *  *  *  *  *  * \n"
    "P8-TO+KY *  *  *  *  *  *  * \n"
    "P9+OU+KY *  *  *  *  *  *  * \n"
    "P+00FU\n"
    "P-00AL\n"
    "+\n"
    "+9998OU\n"
    "-0097FU\n"
    "+9899OU\n"
    "-9798TO\n";
  CsaString test_record(test_record_str);
  
  NumEffectState state(test_record.initialState());
  const std::vector<Move>& moves = test_record.load().moves;
  confirmDetectionInLast(HashKey(state), moves, DominanceCheck::LOSE);
}

BOOST_AUTO_TEST_CASE(SearchDominanceCheckWinWhite)
{
  const char *test_record_str = 
    "P1 *  *  *  *  *  *  * -KY-OU \n"
    "P2 *  *  *  *  *  *  * -KY * \n"
    "P3 *  *  *  *  *  *  * -KY+FU\n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  * +KE * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9+OU *  *  *  *  *  *  *  * \n"
    "P+00FU\n"
    "P-00AL\n"
    "+\n"
    "+1312TO\n"
    "-1112OU\n"
    "+0013FU\n"
    "-1211OU\n";
  CsaString test_record(test_record_str);
  
  NumEffectState state(test_record.initialState());
  const std::vector<Move>& moves = test_record.load().moves;
  confirmDetectionInLast(HashKey(state), moves, DominanceCheck::WIN);
}

BOOST_AUTO_TEST_CASE(SearchDominanceCheckWinBlack)
{
  const char *test_record_str = 
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 * -KE *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7-FU+KE *  *  *  *  *  *  * \n"
    "P8 * +KY *  *  *  *  *  *  * \n"
    "P9+OU+KY *  *  *  *  *  *  * \n"
    "P+00FU\n"
    "P-00AL\n"
    "-\n"
    "-9798TO\n"
    "+9998OU\n"
    "-0097FU\n"
    "+9899OU\n";
  CsaString test_record(test_record_str);
  
  NumEffectState state(test_record.initialState());
  const std::vector<Move>& moves = test_record.load().moves;
  confirmDetectionInLast(HashKey(state), moves, DominanceCheck::WIN);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
