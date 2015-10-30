#include "osl/repetitionCounter.h"
#include "osl/csa.h"
#include "osl/move_classifier/check_.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace osl;

BOOST_AUTO_TEST_CASE(RepetitionCounterTestClone)
{
  using namespace osl::move_classifier;
  const char *stateString = 
    "P1-KY *  *  *  *  *  * +NY * \n"
    "P2 * -OU-KI-KI *  *  *  * +RY\n"
    "P3 * -GI-KE+KI *  *  *  * +HI\n"
    "P4 *  * -FU-KY-FU *  * -FU * \n"
    "P5-FU-FU * -KE * -FU *  *  * \n"
    "P6 *  * +FU-FU+FU * -FU *  * \n"
    "P7+FU+FU *  *  *  *  *  *  * \n"
    "P8+KY+GI+GI-UM *  *  *  *  * \n"
    "P9+OU+KE *  *  *  *  * +KE * \n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00GI\n"
    "P+00KI\n"
    "P-00KA\n"
    "+\n";
  NumEffectState state((CsaString(stateString).initialState()));
  const Move m = Move(Square(7,1),GOLD,BLACK);

  RepetitionCounter counter;
  counter.push(state, m);

  state.makeMove(m);

  const Move m2 = Move(Square(7,9),SILVER,WHITE);
  RepetitionCounter copy(counter);
  BOOST_CHECK(copy.isConsistent());
  copy.push(state,m2);
  BOOST_CHECK(copy.isConsistent());

  copy.pop();
  BOOST_CHECK(copy.isConsistent());
}

BOOST_AUTO_TEST_CASE(RepetitionCounterTestEmpty)
{
  using namespace osl::move_classifier;
  const char *stateString = 
    "P1-KY *  *  *  *  *  * +NY * \n"
    "P2 * -OU-KI-KI *  *  *  * +RY\n"
    "P3 * -GI-KE+KI *  *  *  * +HI\n"
    "P4 *  * -FU-KY-FU *  * -FU * \n"
    "P5-FU-FU * -KE * -FU *  *  * \n"
    "P6 *  * +FU-FU+FU * -FU *  * \n"
    "P7+FU+FU *  *  *  *  *  *  * \n"
    "P8+KY+GI+GI-UM *  *  *  *  * \n"
    "P9+OU+KE *  *  *  *  * +KE * \n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00GI\n"
    "P+00KI\n"
    "P-00KA\n"
    "+\n";
  NumEffectState state((CsaString(stateString).initialState()));
  const Move m = Move(Square(7,1),GOLD,BLACK);
  const HashKey new_key = HashKey(state).newHashWithMove(m);

  RepetitionCounter counter;
  BOOST_CHECK_EQUAL(-1, counter.getFirstMove(new_key));
  BOOST_CHECK_EQUAL(-1, counter.getLastMove(new_key));
  BOOST_CHECK_EQUAL(0u, counter.countRepetition(new_key));
  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), counter.isSennichite(state, m));
  BOOST_CHECK_EQUAL(0, counter.checkCount(BLACK));
  BOOST_CHECK_EQUAL(0, counter.checkCount(WHITE));

  counter.push(state, m);

  BOOST_CHECK_EQUAL(0, counter.getFirstMove(new_key));
  BOOST_CHECK_EQUAL(0, counter.getLastMove(new_key));
  BOOST_CHECK_EQUAL(1u, counter.countRepetition(new_key));
  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), counter.isSennichite(state, m));
  BOOST_CHECK_EQUAL(0, counter.checkCount(BLACK));
  BOOST_CHECK_EQUAL(0, counter.checkCount(WHITE));
}

BOOST_AUTO_TEST_CASE(RepetitionCounterTestDraw)
{
  const char *test_record_str = 
    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
    "P2 * -HI *  *  *  *  * -KA * \n"
    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
    "P8 * +KA *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "+\n"
    "+3948GI\n"
    "-7162GI\n"
    "+4839GI\n"
    "-6271GI\n"
    "+3948GI\n"
    "-7162GI\n"
    "+4839GI\n"
    "-6271GI\n"
    "+3948GI\n"
    "-7162GI\n"
    "+4839GI\n"
    "-6271GI\n";
  CsaString test_record(test_record_str);
  
  NumEffectState state(test_record.initialState());
  const auto& moves = test_record.load().moves;

  HashKey key(state);
  RepetitionCounter counter(state);
  for (int i=0; i<(int)moves.size(); ++i)
  {
    key = HashKey(state);
    BOOST_CHECK_EQUAL(i % 4, counter.getFirstMove(key));
    BOOST_CHECK_EQUAL(i, counter.getLastMove(key));
    BOOST_CHECK_EQUAL((i / 4u)+1, counter.countRepetition(key));
    BOOST_CHECK_EQUAL(0, counter.checkCount(BLACK));
    BOOST_CHECK_EQUAL(0, counter.checkCount(WHITE));
    const Move m = moves[i];
    if (i <(int)moves.size() -1)
      BOOST_CHECK_EQUAL(Sennichite::NORMAL(),
			   counter.isSennichite(state, m));
    else
      BOOST_CHECK_EQUAL(Sennichite::DRAW(), counter.isSennichite(state, m));
    counter.push(state, m);
    state.makeMove(m);
  }
}

BOOST_AUTO_TEST_CASE(RepetitionCounterTestLose)
{
  const char *test_record_str = 
    "P1 *  *  *  *  * +UM *  * -KY\n"
    "P2 *  *  *  *  *  *  * -KE-OU\n"
    "P3 *  *  *  *  *  * +FU * -FU\n"
    "P4 *  *  *  *  *  *  * +FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9+OU *  *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n"
    "+4123UM\n"
    "-1221OU\n"
    "+2332UM\n"
    "-2112OU\n"
    "+3223UM\n"
    "-1221OU\n"
    "+2332UM\n"
    "-2112OU\n"
    "+3223UM\n"
    "-1221OU\n"
    "+2332UM\n"
    "-2112OU\n"
    "+3223UM\n";
  CsaString test_record(test_record_str);
  
  NumEffectState state(test_record.initialState());
  const std::vector<Move>& moves = test_record.load().moves;

  HashKey key = HashKey(state);
  RepetitionCounter counter(state);
  for (int i=0; i<(int)moves.size(); ++i)
  {
    key = HashKey(state);
    BOOST_CHECK_EQUAL(i, counter.getLastMove(key));
    BOOST_CHECK_EQUAL((i+1)/2, counter.checkCount(BLACK));
    BOOST_CHECK_EQUAL(0, counter.checkCount(WHITE));
    const Move m = moves[i];
    if (i <(int)moves.size() -1)
      BOOST_CHECK_EQUAL(Sennichite::NORMAL(), counter.isSennichite(state, m));
    else
      BOOST_CHECK_EQUAL(Sennichite::BLACK_LOSE(),
			   counter.isSennichite(state, m));
    counter.push(state, m);
    state.makeMove(m);
  }
}

BOOST_AUTO_TEST_CASE(RepetitionCounterTestLose2)
{
  const char *test_record_str = 
    "P1 *  *  *  *  *  *  *  * -KY\n"
    "P2 *  *  *  *  *  * +UM-KE-OU\n"
    "P3 *  *  *  *  *  * +FU * -FU\n"
    "P4 *  *  *  *  *  *  * +FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9+OU *  *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n"
    "+3223UM\n"
    "-1221OU\n"
    "+2332UM\n"
    "-2112OU\n"
    "+3223UM\n"
    "-1221OU\n"
    "+2332UM\n"
    "-2112OU\n"
    "+3223UM\n"
    "-1221OU\n"
    "+2332UM\n"
    "-2112OU\n";
  CsaString test_record(test_record_str);
  
  NumEffectState state(test_record.initialState());
  const std::vector<Move>& moves = test_record.load().moves;

  HashKey key = HashKey(state);
  RepetitionCounter counter(state);
  for (int i=0; i<(int)moves.size(); ++i)
  {
    key = HashKey(state);
    BOOST_CHECK_EQUAL(i, counter.getLastMove(key));
    BOOST_CHECK_EQUAL((i+1)/2, counter.checkCount(BLACK));
    BOOST_CHECK_EQUAL(0, counter.checkCount(WHITE));
    const Move m = moves[i];
    if (i <(int)moves.size() -1)
      BOOST_CHECK_EQUAL(Sennichite::NORMAL(), counter.isSennichite(state, m));
    else
      BOOST_CHECK_EQUAL(Sennichite::BLACK_LOSE(),
			   counter.isSennichite(state, m));
    counter.push(state, m);
    state.makeMove(m);
  }
}

static void testFile(const std::string& filename)
{
  typedef RepetitionCounter::list_t list_t;

  auto record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initialState());
  const auto& moves=record.moves;
  std::vector<SimpleState> states;
  states.push_back(state);
  RepetitionCounter counter(state);
  for (size_t i=0;i<moves.size();i++)
  {
    const Move move=moves[i];
    const HashKey key = HashKey(state).newHashWithMove(move);
    const Sennichite sennichite = counter.isSennichite(state, move);
    if (counter.countRepetition(key) < 3)
    {
      BOOST_CHECK_EQUAL(Sennichite::NORMAL(), sennichite);
    }

    counter.push(state, move);
    BOOST_CHECK(counter.isConsistent());
    const RepetitionCounter copy(counter);
    counter.pop();
    counter.push(state, move);
    BOOST_CHECK(RepetitionCounter::maybeEqual(copy,counter));
    state.makeMove(move);
    states.push_back(state);
    const int repeated = counter.countRepetition(key);
    if (repeated > 1)
    {
      const list_t& l = counter.getRepetitions(key);
      assert(l.size() > 1);
      list_t::const_iterator p=l.begin();
      const SimpleState& s0 = states.at(*p);
      ++p;
      for (; p!= l.end(); ++p)
      {
	BOOST_CHECK(s0 == states.at(*p));
      }
      if (repeated >= 4)
      {
	if (sennichite.isDraw())
	{
	  if (OslConfig::verbose())
	    std::cerr << "DRAW\n";
	}
	else
	{
	  std::cerr << "王手がらみの4回目\n";
	  counter.printMatches(key);
	}
	BOOST_CHECK(! sennichite.isNormal());
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(RepetitionCounterTestCsaFiles)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=1000;
  if (OslConfig::inUnitTest() < 2)
      count=10;
  std::string filename;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTest() >= 2)
    progress.reset(new boost::progress_display(count, std::cerr));
  while((ifs >> filename) && filename != "" && (++i<count)) {
    if (progress)
      ++(*progress);
    testFile(OslConfig::testCsaFile(filename));
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
