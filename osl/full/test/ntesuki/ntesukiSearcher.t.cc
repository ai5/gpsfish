/* ntesukiSeacher.cc
 */
#include "osl/ntesuki/ntesukiSearcher.h"
#include "osl/state/hashEffectState.h"
#include "osl/record/csaString.h"
#include "osl/record/csaRecord.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "osl/ntesuki/ntesukiTable.h"
#include "osl/checkmate/proofDisproof.h"
#include "osl/apply_move/applyMove.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

class NtesukiSearcherTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(NtesukiSearcherTest);
  CPPUNIT_TEST(test0tesuki0);
  CPPUNIT_TEST(test0tesuki0op);
  CPPUNIT_TEST(test0tesuki2);
  CPPUNIT_TEST(test0tesuki1);
  CPPUNIT_TEST(test1tesuki0);
  CPPUNIT_TEST(testKatagyoku0);
  CPPUNIT_TEST(testRecognizeAttackBack0);
  CPPUNIT_TEST(testRecognizeAttackBack1);
  CPPUNIT_TEST(test1tesukiTest01);
  CPPUNIT_TEST(test1tesukiTest13);
  CPPUNIT_TEST(test1tesukiTest22);
  CPPUNIT_TEST(test1tesukiTest25);
  CPPUNIT_TEST(test1tesukiTest29);
  CPPUNIT_TEST(test1tesukiTest56);
  CPPUNIT_TEST(test1tesukiTest76);
  CPPUNIT_TEST(test1tesukiTest94);
  
#if 0
  CPPUNIT_TEST(test2tesuki0);
  CPPUNIT_TEST(test2tesuki1);
#endif

  CPPUNIT_TEST_SUITE_END();
public:
  void test0tesuki0();
  void test0tesuki0op();
  void test0tesuki1();
  void test0tesuki2();
  void test1tesuki0();
  void test2tesuki0();
  void test2tesuki1();
  void testRecognizeAttackBack0();
  void testRecognizeAttackBack1();
  void test1tesukiTest01();
  void test1tesukiTest13();
  void test1tesukiTest22();
  void test1tesukiTest25();
  void test1tesukiTest29();
  void test1tesukiTest56();
  void test1tesukiTest76();
  void test1tesukiTest94();
  void test3tesuki0();
  void test3tesuki1();
  void testKatagyoku0();
};
CPPUNIT_TEST_SUITE_REGISTRATION(NtesukiSearcherTest);

using namespace osl;
using namespace osl::ntesuki;
const size_t TABLE_LIMIT = 5000000;
const size_t READ_LIMIT = 10000000;

typedef  NtesukiMoveGenerator movegen_t;

volatile int stop_flag = false;

void NtesukiSearcherTest::test0tesuki0()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  * +FU * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);
  NtesukiSearcher searcher(nState, gam.get(),  TABLE_LIMIT,
			   &stop_flag, !isShortTest, 1);

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(0, ntesuki_num);
}

void NtesukiSearcherTest::test0tesuki0op()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  * -FU *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00FU00FU00FU00FU00FU00FU00FU00FU\n"
    "P+00FU00FU00FU00FU00FU00FU00FU00FU\n"
    "P+00FU\n"
    "P+00KY00KY00KY00KY00KE00KE00KE00KE\n"
    "P+00GI00GI00GI00GI00KI00KI00KI\n"
    "P+00KA00KA00HI00HI\n"
    "P-00KI\n"
    "-\n").initialState();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);
  NtesukiSearcher searcher(nState, gam.get(),  TABLE_LIMIT,
			   &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(0, ntesuki_num);
}

void NtesukiSearcherTest::test0tesuki1()
{
  SimpleState state=CsaString(
    "P1-KY-KE-GI-KI-OU-KI+UM-KE-KY\n"
    "P2 * -HI *  * +KA *  *  *  * \n"
    "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
    "P4 *  *  *  *  *  * -FU *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  * +FU *  *  *  *  *  * \n"
    "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
    "P8 *  *  *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "P+00GI\n"
    "+\n").initialState();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT,
			   &stop_flag,OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(nState.turn());
  BOOST_CHECK_EQUAL(0, ntesuki_num);
}

void NtesukiSearcherTest::test0tesuki2()
{
  for (int i = 17; i >= 0; i--)
  {
    CsaString csa_str(
     "P1-KY *  * +TO-KI-OU *  *  * \n"
     "P2 *  * +RY-GI *  * -KI *  * \n"
     "P3-FU * -FU-FU *  *  * -GI-FU\n"
     "P4 *  *  *  * -KE-FU *  *  * \n"
     "P5 *  *  *  *  * -KE-FU *  * \n"
     "P6 * +FU+FU *  *  *  *  * +FU\n"
     "P7+FU+OU * +FU * +FU+GI *  * \n"
     "P8 *  *  *  * -RY *  *  *  * \n"
     "P9+KY+KE+KE *  *  * -UM * +KY\n"
     "P+00FU00FU00FU00GI00KY00KA\n"
     "P-00FU00FU00KI00KI\n"
     "-\n"
     "-0088KI\n"
     "+8796OU\n"
     "-0095KI\n"
     "+9695OU\n"
     "-9394FU\n"
     "+9596OU\n"
     "-9495FU\n"
     "+9685OU\n"
     "-5855RY\n"
     "+0075KY\n"
     "-0084FU\n"
     "+8584OU\n"
     "-5564RY\n"
     "+8483OU\n"
     "-6494RY\n"
     "+8382OU\n"
     "-9492RY\n"
   );
    SimpleState state = csa_str.initialState();
    const osl::vector<Move> moves = csa_str.load().moves();
    
    for (int j = 0; j < i; j++)
    {
      ApplyMoveOfTurn::doMove(state, moves[j]);
    }

    NumEffectState nState(state);
    boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
      gam(new movegen_t());
    NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT,
			     &stop_flag, OslConfig::verbose(), 1);
    
    if (OslConfig::verbose())
    {
      std::cerr << "trying" << i << "\n"
		<< state;
    }
    const int ntesuki_num = searcher.searchSlow(osl::WHITE, READ_LIMIT);
    if (OslConfig::verbose())
    {
      std::cerr << "\tresult\t" << ntesuki_num
		<< std::endl;
    }
    BOOST_CHECK_EQUAL(0, ntesuki_num);
  }
}

void NtesukiSearcherTest::test1tesuki0()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  * +FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU+KY *  *  *  * \n"
    "P8+HI+GI+KI+GI+GI *  *  *  * \n"
    "P9+HI+KI+OU+KI+GI *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT,
			   &stop_flag, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);
}

void NtesukiSearcherTest::test2tesuki0()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  * +FU * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 * +FU+FU+FU *  *  *  *  * \n"
    "P8 * +GI+KI+GI *  *  *  *  * \n"
    "P9 * +KI+OU+KI *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT,
			   &stop_flag, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(nState.turn());
  BOOST_CHECK_EQUAL(2, ntesuki_num);
}

void NtesukiSearcherTest::test2tesuki1()
{
  SimpleState state=CsaString(
    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
    "P2 * -HI *  *  *  *  * +UM * \n"
    "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
    "P4 *  *  *  *  *  * -FU *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  * +FU *  *  *  *  *  * \n"
    "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
    "P8 *  *  *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "P+00KA\n"
    "+\n").initialState();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT,
			   &stop_flag, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(nState.turn());
  BOOST_CHECK_EQUAL(2, ntesuki_num);
}

void NtesukiSearcherTest::testRecognizeAttackBack0()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  * +FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  * -FU *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag,
			   OslConfig::verbose(), 2,
			   osl::ntesuki::NtesukiRecord::pn_iw,
			   osl::ntesuki::NtesukiRecord::pn_ps,
			   osl::ntesuki::NtesukiRecord::tonshi_is,
			   0);

  const int ntesuki_num = searcher.searchSlow(nState.turn());
  BOOST_CHECK_EQUAL(-1, ntesuki_num);
}

void NtesukiSearcherTest::testRecognizeAttackBack1()
{
  CsaFile file(OslConfig::testFile("grimbergen_testset/test12.csa"));
  SimpleState state = file.initialState ();
  const osl::vector<Move> moves=file.load().moves();
  NumEffectState nState(state);

  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT,
			   &stop_flag, OslConfig::verbose(), 2,
			   osl::ntesuki::NtesukiRecord::pn_iw,
			   osl::ntesuki::NtesukiRecord::pn_ps,
			   osl::ntesuki::NtesukiRecord::tonshi_is,
			   0);

  NtesukiSearcher::delay_non_pass = true;
  NtesukiSearcher::ptt_invalid_defense = true;
  //NtesukiSearcher::ptt_siblings_fail = true;
  NtesukiSearcher::ptt_siblings_success = true;
  NtesukiSearcher::delay_interpose = true;
  NtesukiSearcher::delay_nopromote = true;
  NtesukiRecord::use_dominance = true;
  NtesukiRecord::fixed_search_depth = 4;

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  if (!searcher.exceedReadNodeLimit())
  {
    BOOST_CHECK_EQUAL(1, ntesuki_num);
  }

  const NtesukiTable& table = searcher.getTable();
  const HashKey key = HashKey(state).
    newHashWithMove(Move(Square(6,2), BISHOP, BLACK));
  const NtesukiRecord *record = table.find(key);
  if (record)
  {
    BOOST_CHECK(!record->getValue<BLACK>(1).isCheckmateSuccess());
  }
  else
  {
    std::cerr << "node not searched\n";
  }
}

/* see if can solve some problems
 */
static void
test_state(const char *csa_filename)
{
  CsaFile file(csa_filename);
  SimpleState init_state = file.initialState ();
  Player winner = init_state.turn();
  const osl::vector<Move> moves=file.load().moves();
  
  std::cerr << "for " << csa_filename << "\n";
  for (int i = moves.size(); i >= 0; i--)
  {
    std::cerr << "trying " << i << "...";
    SimpleState state(init_state);
    for (int j = 0; j < i; j++)
    {
      ApplyMoveOfTurn::doMove(state, moves[j]);
    }
    boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
      gam(new movegen_t());
    NumEffectState nState(state);
    NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT,
			     &stop_flag, OslConfig::verbose(), 2,
			     NtesukiRecord::pn_iw,
			     NtesukiRecord::no_ps,
			     NtesukiRecord::no_is,
			     2 /* tsumero cost */,
			     1 /* tsumero estimates */);
    NtesukiSearcher::delay_non_pass = true;
    NtesukiSearcher::ptt_invalid_defense = true;
    //NtesukiSearcher::ptt_siblings_fail = true;
    //NtesukiSearcher::ptt_siblings_success = true;
    NtesukiSearcher::delay_interpose = true;
    NtesukiSearcher::delay_nopromote = true;
    NtesukiRecord::use_dominance = true;
    NtesukiRecord::fixed_search_depth = 4;

    const int ntesuki_num = searcher.searchSlow(winner, READ_LIMIT);
    std::cerr << ntesuki_num << "\n";
    BOOST_CHECK(ntesuki_num >= 0);
  }
}


void NtesukiSearcherTest::
test1tesukiTest01()
{
  if (isShortTest) return;
  test_state(OslConfig::testFile("grimbergen_testset/test1.csa"));
}

void NtesukiSearcherTest::
test1tesukiTest22()
{
  if (isShortTest) return;
  CsaFile file(OslConfig::testFile("grimbergen_testset/test22.csa"));
  SimpleState state = file.initialState ();
  const osl::vector<Move> moves=file.load().moves();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);

  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());
  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);
}

void NtesukiSearcherTest::
test1tesukiTest13()
{
  if (isShortTest) return;
  CsaFile file(OslConfig::testFile("grimbergen_testset/test13.csa"));
  SimpleState state = file.initialState ();
  const osl::vector<Move> moves=file.load().moves();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);

  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());
  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);
}

void NtesukiSearcherTest::
test1tesukiTest25()
{
  if (isShortTest) return;
  CsaFile file(OslConfig::testFile("grimbergen_testset/test25.csa"));
  SimpleState state = file.initialState ();
  const osl::vector<Move> moves=file.load().moves();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);

  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());
  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);
}

void NtesukiSearcherTest::
test1tesukiTest29()
{
  if (isShortTest) return;
  CsaFile file(OslConfig::testFile("grimbergen_testset/test29.csa"));
  SimpleState state = file.initialState ();
  const osl::vector<Move> moves=file.load().moves();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);

  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());
  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);
}

void NtesukiSearcherTest::
test1tesukiTest56()
{
  if (isShortTest) return;
  CsaFile file(OslConfig::testFile("grimbergen_testset/test56.csa"));
  SimpleState state = file.initialState ();
  const osl::vector<Move> moves=file.load().moves();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);

  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());
  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);
}

void NtesukiSearcherTest::
test1tesukiTest76()
{
  if (isShortTest) return;
  CsaFile file(OslConfig::testFile("grimbergen_testset/test76.csa"));
  SimpleState state = file.initialState ();
  const osl::vector<Move> moves=file.load().moves();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);

  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());
  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);
}

void NtesukiSearcherTest::
test1tesukiTest94()
{
  if (isShortTest) return;
  CsaFile file(OslConfig::testFile("grimbergen_testset/test94.csa"));
  SimpleState state = file.initialState ();
  const osl::vector<Move> moves=file.load().moves();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);

  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());
  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);
}


/* 勝浦修「必至のかけ方」創元社より
 */
void NtesukiSearcherTest::testKatagyoku0()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  * -OU * -KE-KY\n"
    "P2 *  *  * +TO * -GI *  *  * \n"
    "P3 *  *  *  *  * -FU * -FU-FU\n"
    "P4 *  *  *  *  *  * -FU *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU *  *  *  *  *  *  * \n"
    "P8+KY+KI *  *  *  *  *  *  * \n"
    "P9+OU+KI *  *  *  *  *  *  * \n"
    "P+00KI00KI\n"
    "P-00AL\n"
    "+\n").initialState();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT,
			   &stop_flag, OslConfig::verbose());

  const int ntesuki_num =
    searcher.searchSlow(nState.turn());

  BOOST_CHECK_EQUAL(1, ntesuki_num);
}
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
