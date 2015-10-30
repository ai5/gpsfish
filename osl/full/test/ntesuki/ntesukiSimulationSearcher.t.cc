#include "osl/ntesuki/ntesukiSimulationSearcher.h"
#include "osl/ntesuki/ntesukiSimulationSearcherProof.tcc"
#include "osl/ntesuki/ntesukiSimulationSearcherDisproof.tcc"
#include "osl/ntesuki/ntesukiSearcher.h"
#include "osl/ntesuki/ntesukiTable.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "osl/record/csaString.h"

#include <boost/test/unit_test.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace osl;

class NtesukiSimulationSearcherTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(NtesukiSimulationSearcherTest);
  CPPUNIT_TEST(test0Tesuki0);
  CPPUNIT_TEST(test0Tesuki1);
  CPPUNIT_TEST(test0Tesuki2);
  CPPUNIT_TEST(test1Tesuki0);
  //CPPUNIT_TEST(test0Tesuki0Fail);
  //CPPUNIT_TEST(test1Tesuki0Fail);
  CPPUNIT_TEST_SUITE_END();
public:
  void test0Tesuki0();
  void test0Tesuki1();
  void test0Tesuki2();
  void test1Tesuki0();
  void test1Tesuki1();
  void test0Tesuki0Fail();
  void test1Tesuki0Fail();
};


using namespace osl;
using namespace osl::ntesuki;

typedef  NtesukiMoveGenerator movegen_t;

static volatile int stop_flag = false;

static void compare_simulation(SimpleState& sstate_orig,
			       SimpleState& sstate,
			       const int ntesuki_correct,
			       const bool correct)
{
  PathEncoding path;
  HashEffectState state_orig(sstate_orig);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());

  NtesukiSearcher searcher(state_orig, gam.get(),
			   100000, &stop_flag, OslConfig::verbose(), ntesuki_correct + 1);
  NtesukiTable& table = searcher.getTable();

  int ntesuki_num = 0;
  TRY_DFPN;
  ntesuki_num = searcher.searchSlow(state_orig.turn());
  CATCH_DFPN;
  BOOST_CHECK_EQUAL(ntesuki_correct,ntesuki_num);

  const NtesukiRecord* record_orig = table.allocateRoot(HashKey(state_orig),
						      PieceStand(WHITE, state_orig), 0);
  
  NumEffectState state(sstate);

  NtesukiSimulationSearcher simulator(state, gam.get(), path,
				      table, osl::ntesuki::NtesukiRecord::no_is,
				      OslConfig::verbose());
  NtesukiRecord* record = table.allocateRoot(HashKey(state),
					 PieceStand(WHITE, state), 0);
  NtesukiRecord::state = &state;
  TRY_DFPN;
  bool result = simulator.startFromAttackProof<BLACK>(record, record_orig, ntesuki_num, Move::INVALID());
  BOOST_CHECK(correct == result);
  CATCH_DFPN;
}

void NtesukiSimulationSearcherTest::
test0Tesuki0()
{
  SimpleState sstate_orig = CsaString(
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
  SimpleState sstate = CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3+FU+FU *  *  *  *  * +FU * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
  compare_simulation(sstate_orig, sstate, 0, true);
}

void NtesukiSimulationSearcherTest::
test0Tesuki1()
{
  SimpleState sstate_orig = CsaString(
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
  SimpleState sstate = CsaString(
    "P1 *  *  *  *  *  * -KI-OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3+FU+FU *  *  *  *  * +FU * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
  compare_simulation(sstate_orig, sstate, 0, false);
}

void NtesukiSimulationSearcherTest::
test0Tesuki2()
{
  SimpleState sstate_orig = CsaString(
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
  SimpleState sstate = CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3+FU+FU *  *  *  *  * +FU * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").initialState();
  compare_simulation(sstate_orig, sstate, 0, false);
}

void NtesukiSimulationSearcherTest::
test1Tesuki0()
{
  if (isShortTest) return;
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());

  PathEncoding path;
  SimpleState sstate_orig = CsaString(
    "P1-KY-KE *  *  * -OU * -KE-KY\n"
    "P2 *  *  *  * -KE-KI *  *  * \n"
    "P3 *  * -KA * +UM * +FU *  * \n"
    "P4-FU * -FU-FU-GI * -FU-FU-FU\n"
    "P5 *  *  *  * -KI *  *  *  * \n"
    "P6+FU+GI+FU+FU *  *  *  * +FU\n"
    "P7+OU+FU *  *  *  *  *  *  * \n"
    "P8 *  * -NG *  *  *  *  *  * \n"
    "P9+KY+KE *  *  * -HI *  * +KY\n"
    "P+00FU00FU00FU00GI00HI\n"
    "P-00FU00FU00FU00KI00KI\n"
    "+\n").initialState();
  /*
   * +0031HI
   * -4131OU
   * +0032GI
   * -3122OU
   * +5342UM
   * %TORYO
   */

  NumEffectState state_orig(sstate_orig);
  NtesukiSearcher searcher(state_orig, gam.get(),
			   10000000, &stop_flag, OslConfig::verbose());
  NtesukiTable& table = searcher.getTable();

  const int ntesuki_num = searcher.searchSlow(state_orig.turn(), 20000000);
  BOOST_CHECK_EQUAL(1, ntesuki_num);
  const NtesukiRecord* record_orig = table.allocateRoot(HashKey(state_orig),
						      PieceStand(WHITE, state_orig), 0);

  {
    //handed +00HI to -00HI
    SimpleState sstate = CsaString(
      "P1-KY-KE *  *  * -OU * -KE-KY\n"
      "P2 *  *  *  * -KE-KI *  *  * \n"
      "P3 *  * -KA * +UM * +FU *  * \n"
      "P4-FU * -FU-FU-GI * -FU-FU-FU\n"
      "P5 *  *  *  * -KI *  *  *  * \n"
      "P6+FU+GI+FU+FU *  *  *  * +FU\n"
      "P7+OU+FU *  *  *  *  *  *  * \n"
      "P8 *  * -NG *  *  *  *  *  * \n"
      "P9+KY+KE *  *  * -HI *  * +KY\n"
      "P+00FU00FU00FU00GI\n"
      "P-00FU00FU00FU00KI00KI00HI\n"
      "+\n").initialState();
    NumEffectState state(sstate);

    NtesukiSimulationSearcher simulator(state, gam.get(), path,
					table, osl::ntesuki::NtesukiRecord::no_is, OslConfig::verbose());
    NtesukiRecord* record = table.allocateRoot(HashKey(state),
					     PieceStand(WHITE, state), 0);
  
    bool result = simulator.startFromAttackProof<BLACK>(record, record_orig,
							ntesuki_num, Move::INVALID());
    BOOST_CHECK_EQUAL(false, result);
  }
  {
    //moved -42KI to -31KI
    SimpleState sstate = CsaString(
      "P1-KY-KE *  *  * -OU * -KE-KY\n"
      "P2 *  *  *  * -KE * -KI *  * \n"
      "P3 *  * -KA * +UM * +FU *  * \n"
      "P4-FU * -FU-FU-GI * -FU-FU-FU\n"
      "P5 *  *  *  * -KI *  *  *  * \n"
      "P6+FU+GI+FU+FU *  *  *  * +FU\n"
      "P7+OU+FU *  *  *  *  *  *  * \n"
      "P8 *  * -NG *  *  *  *  *  * \n"
      "P9+KY+KE *  *  * -HI *  * +KY\n"
      "P+00FU00FU00FU00GI00HI\n"
      "P-00FU00FU00FU00KI00KI\n"
      "+\n").initialState();
    NumEffectState state(sstate);

    NtesukiSimulationSearcher simulator(state, gam.get(), path,
					table, osl::ntesuki::NtesukiRecord::no_is, OslConfig::verbose());
  
    NtesukiRecord* record = table.allocateRoot(HashKey(state),
					     PieceStand(WHITE, state), 0);

    bool result = simulator.startFromAttackProof<BLACK>(record, record_orig,
							ntesuki_num, Move::INVALID());
    BOOST_CHECK_EQUAL(false, result);
  }
  //moved -91KY to -92KY
  //move +99KY to +98KY
}

void NtesukiSimulationSearcherTest::
test1Tesuki1()
{
  if (isShortTest) return;
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());

  PathEncoding path;
  SimpleState sstate_orig = CsaString(
    "P1-KY *  * -KI *  *  *  * -KY\n"
    "P2-OU * -GI *  *  *  *  *  * \n"
    "P3-FU-FU-KE-FU+RY *  *  *  * \n"
    "P4 *  * +KE * -FU+FU+GI-FU-FU\n"
    "P5+FU * -FU * -KI * +GI-KE * \n"
    "P6 *  *  *  *  *  *  *  * +FU\n"
    "P7 * -NG * +FU * -TO-UM *  * \n"
    "P8 *  *  * +KI+KI *  * -RY * \n"
    "P9+KY+KE+OU * +FU *  *  * +KY\n"
    "P+00KA\n"
    "P-00FU00FU00FU00FU00FU\n"
    "+\n").initialState();
  NumEffectState state_orig(sstate_orig);
  NtesukiSearcher searcher(state_orig, gam.get(),
			   10000000, &stop_flag, OslConfig::verbose());
  NtesukiTable& table = searcher.getTable();

  const int ntesuki_num = searcher.searchSlow(state_orig.turn(), 20000000);
  BOOST_CHECK_EQUAL(1, ntesuki_num);
  const NtesukiRecord* record_orig = table.allocateRoot(HashKey(state_orig),
						      PieceStand(WHITE, state_orig), 0);

  {
    //handed +00KA to -00KA
    SimpleState sstate = CsaString(
      "P1-KY *  * -KI *  *  *  * -KY\n"
      "P2-OU * -GI *  *  *  *  *  * \n"
      "P3-FU-FU-KE-FU+RY *  *  *  * \n"
      "P4 *  * +KE * -FU+FU+GI-FU-FU\n"
      "P5+FU * -FU * -KI * +GI-KE * \n"
      "P6 *  *  *  *  *  *  *  * +FU\n"
      "P7 * -NG * +FU * -TO-UM *  * \n"
      "P8 *  *  * +KI+KI *  * -RY * \n"
      "P9+KY+KE+OU * +FU *  *  * +KY\n"
      "P-00FU00FU00FU00FU00FU00KA\n"
      "+\n").initialState();
    NumEffectState state(sstate);

    NtesukiSimulationSearcher simulator(state, gam.get(), path,
					table, osl::ntesuki::NtesukiRecord::no_is, OslConfig::verbose());
    NtesukiRecord* record = table.allocateRoot(HashKey(state),
					     PieceStand(WHITE, state), 0);
    bool result = simulator.startFromAttackProof<BLACK>(record, record_orig,
							ntesuki_num, Move::INVALID());
    BOOST_CHECK_EQUAL(false, result);
  }
  {
    //moved -61KI to -71KI
    SimpleState sstate = CsaString(
      "P1-KY * -KI *  *  *  *  * -KY\n"
      "P2-OU * -GI *  *  *  *  *  * \n"
      "P3-FU-FU-KE-FU+RY *  *  *  * \n"
      "P4 *  * +KE * -FU+FU+GI-FU-FU\n"
      "P5+FU * -FU * -KI * +GI-KE * \n"
      "P6 *  *  *  *  *  *  *  * +FU\n"
      "P7 * -NG * +FU * -TO-UM *  * \n"
      "P8 *  *  * +KI+KI *  * -RY * \n"
      "P9+KY+KE+OU * +FU *  *  * +KY\n"
      "P+00KA\n"
      "P-00FU00FU00FU00FU00FU\n"
      "+\n").initialState();
    NumEffectState state(sstate);

    NtesukiSimulationSearcher simulator(state, gam.get(), path,
					table, osl::ntesuki::NtesukiRecord::no_is, OslConfig::verbose());
    NtesukiRecord* record = table.allocateRoot(HashKey(state),
					     PieceStand(WHITE, state), 0);  
    bool result = simulator.startFromAttackProof<BLACK>(record, record_orig,
							ntesuki_num, Move::INVALID());
    BOOST_CHECK_EQUAL(false, result);
  }
  {
    //moved -11KY to -12KY
    SimpleState sstate = CsaString(
      "P1-KY *  * -KI *  *  *  *  * \n"
      "P2-OU * -GI *  *  *  *  * -KY\n"
      "P3-FU-FU-KE-FU+RY *  *  *  * \n"
      "P4 *  * +KE * -FU+FU+GI-FU-FU\n"
      "P5+FU * -FU * -KI * +GI-KE * \n"
      "P6 *  *  *  *  *  *  *  * +FU\n"
      "P7 * -NG * +FU * -TO-UM *  * \n"
      "P8 *  *  * +KI+KI *  * -RY+KY\n"
      "P9+KY+KE+OU * +FU *  *  *  * \n"
      "P+00KA\n"
      "P-00FU00FU00FU00FU00FU\n"
      "+\n").initialState();
    NumEffectState state(sstate);
    
    NtesukiSimulationSearcher simulator(state, gam.get(), path,
					table, osl::ntesuki::NtesukiRecord::no_is, OslConfig::verbose());
    NtesukiRecord* record = table.allocateRoot(HashKey(state),
					     PieceStand(WHITE, state), 0);    
    bool result = simulator.startFromAttackProof<BLACK>(record, record_orig,
							ntesuki_num, Move::INVALID());
    BOOST_CHECK_EQUAL(true, result);
  }
  {
    //move +19KY to +18KY
    SimpleState sstate = CsaString(
      "P1-KY *  * -KI *  *  *  * -KY\n"
      "P2-OU * -GI *  *  *  *  *  * \n"
      "P3-FU-FU-KE-FU+RY *  *  *  * \n"
      "P4 *  * +KE * -FU+FU+GI-FU-FU\n"
      "P5+FU * -FU * -KI * +GI-KE * \n"
      "P6 *  *  *  *  *  *  *  * +FU\n"
      "P7 * -NG * +FU * -TO-UM *  * \n"
      "P8 *  *  * +KI+KI *  * -RY+KY\n"
      "P9+KY+KE+OU * +FU *  *  *  * \n"
      "P+00KA\n"
      "P-00FU00FU00FU00FU00FU\n"
      "+\n").initialState();
    NumEffectState state(sstate);
    
    NtesukiSimulationSearcher simulator(state, gam.get(), path,
					table, osl::ntesuki::NtesukiRecord::no_is, OslConfig::verbose());
    NtesukiRecord* record = table.allocateRoot(HashKey(state),
					     PieceStand(WHITE, state), 0);    
    bool result = simulator.startFromAttackProof<BLACK>(record, record_orig,
							ntesuki_num, Move::INVALID());
    BOOST_CHECK_EQUAL(true, result);
  }
}

void NtesukiSimulationSearcherTest::
test0Tesuki0Fail()
{
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  SimpleState sstate_orig = CsaString(
    "P1-KY *  * -KI *  *  *  * -KY\n"
    "P2-OU * -GI *  *  *  *  *  * \n"
    "P3-FU-FU-KE-FU+RY *  *  *  * \n"
    "P4 *  * +KE * -FU+FU+GI-FU-FU\n"
    "P5+FU * -FU * -KI * +GI-KE * \n"
    "P6 *  *  *  *  *  *  *  * +FU\n"
    "P7 * -NG * +FU * -TO-UM *  * \n"
    "P8 *  *  * +KI+KI *  * -RY * \n"
    "P9+KY+KE+OU * +FU *  *  * +KY\n"
    "P-00FU00FU00FU00FU00FU00KA\n"
    "+\n").initialState();
  NumEffectState state_orig(sstate_orig);
  NtesukiSearcher searcher(state_orig, gam.get(),
			   100000, &stop_flag, OslConfig::verbose(), 1);
  NtesukiTable& table = searcher.getTable();

  const int ntesuki_num = searcher.searchSlow(state_orig.turn());
  BOOST_CHECK_EQUAL(-1, ntesuki_num);
  const NtesukiRecord* record_orig = table.allocateRoot(HashKey(state_orig),
						      PieceStand(WHITE, state_orig), 0);
    
  PathEncoding path(state_orig.turn());
    
  {
    SimpleState sstate = CsaString(
    "P1-KY *  * -KI *  *  *  * -KY\n"
    "P2-OU * -GI *  *  *  *  *  * \n"
    "P3-FU-FU-KE-FU+RY *  *  *  * \n"
    "P4 *  * +KE * -FU+FU+GI-FU-FU\n"
    "P5+FU * -FU * -KI * +GI-KE * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 * -NG * +FU * -TO-UM * +FU\n"
    "P8 *  *  * +KI+KI *  * -RY * \n"
    "P9+KY+KE+OU * +FU *  *  * +KY\n"
    "P-00FU00FU00FU00FU00FU00KA\n"
    "+\n").initialState();
    NumEffectState state(sstate);

    NtesukiSimulationSearcher simulator(state, gam.get(), path,
					table, osl::ntesuki::NtesukiRecord::no_is, OslConfig::verbose());
    NtesukiRecord* record = table.allocateRoot(HashKey(state),
					     PieceStand(WHITE, state), 0);  
    bool result = simulator.startFromAttackDisproof<BLACK>(record, record_orig,
							   0, Move::INVALID());
    BOOST_CHECK(result);
  }

  {
    SimpleState sstate = CsaString(
    "P1-KY *  * -KI *  *  *  * -KY\n"
    "P2-OU * -GI *  *  *  *  *  * \n"
    "P3-FU-FU-KE-FU+RY *  *  *  * \n"
    "P4 *  * +KE * -FU+FU+GI-FU-FU\n"
    "P5+FU * -FU * -KI * +GI-KE * \n"
    "P6 *  *  *  *  *  *  *  * +FU\n"
    "P7 * -NG * +FU * -TO-UM *  * \n"
    "P8 *  *  * +KI+KI *  * -RY * \n"
    "P9+KY+KE+OU * +FU *  *  * +KY\n"
    "P+00KA\n"
    "P-00FU00FU00FU00FU00FU\n"
    "+\n").initialState();
    NumEffectState state(sstate);

    NtesukiSimulationSearcher simulator(state, gam.get(), path,
					table, osl::ntesuki::NtesukiRecord::no_is, OslConfig::verbose());
    NtesukiRecord* record = table.allocateRoot(HashKey(state),
					     PieceStand(WHITE, state), 0);  
    bool result = simulator.startFromAttackDisproof<BLACK>(record, record_orig,
							   0, Move::INVALID());
    BOOST_CHECK(!result);
  }
}

void NtesukiSimulationSearcherTest::
test1Tesuki0Fail()
{
  if (isShortTest) return;
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  SimpleState sstate_orig = CsaString(
    "P1-KY *  * -KI *  *  *  * -KY\n"
    "P2-OU * -GI *  *  *  *  *  * \n"
    "P3-FU-FU-KE-FU+RY *  *  *  * \n"
    "P4 *  * +KE * -FU+FU+GI-FU-FU\n"
    "P5+FU * -FU * -KI * +GI-KE * \n"
    "P6 *  *  *  *  *  *  *  * +FU\n"
    "P7 * -NG * +FU * -TO-UM *  * \n"
    "P8 *  *  * +KI+KI *  * -RY * \n"
    "P9+KY+KE+OU * +FU *  *  * +KY\n"
    "P-00FU00FU00FU00FU00FU00KA\n"
    "+\n").initialState();
  NumEffectState state_orig(sstate_orig);
  NtesukiSearcher searcher(state_orig, gam.get(),
			   100000, &stop_flag, OslConfig::verbose());
  NtesukiTable& table = searcher.getTable();

  const int ntesuki_num = searcher.searchSlow(state_orig.turn());
  BOOST_CHECK_EQUAL(-1, ntesuki_num);
  const NtesukiRecord* record_orig = table.allocateRoot(HashKey(state_orig),
						      PieceStand(WHITE, state_orig), 0);
    
  PathEncoding path(state_orig.turn());
    
  {
    SimpleState sstate = CsaString(
    "P1-KY *  * -KI *  *  *  * -KY\n"
    "P2-OU * -GI *  *  *  *  *  * \n"
    "P3-FU-FU-KE-FU+RY *  *  *  * \n"
    "P4 *  * +KE * -FU+FU+GI-FU-FU\n"
    "P5+FU * -FU * -KI * +GI-KE * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 * -NG * +FU * -TO-UM * +FU\n"
    "P8 *  *  * +KI+KI *  * -RY * \n"
    "P9+KY+KE+OU * +FU *  *  * +KY\n"
    "P-00FU00FU00FU00FU00FU00KA\n"
    "+\n").initialState();
    NumEffectState state(sstate);

    NtesukiSimulationSearcher simulator(state, gam.get(), path,
					table, osl::ntesuki::NtesukiRecord::no_is, OslConfig::verbose());
    NtesukiRecord* record = table.allocateRoot(HashKey(state),
					     PieceStand(WHITE, state), 0);  
    bool result = simulator.startFromAttackDisproof<BLACK>(record, record_orig,
							   1, Move::INVALID());
    BOOST_CHECK(result);
  }

  {
    SimpleState sstate = CsaString(
    "P1-KY *  * -KI *  *  *  * -KY\n"
    "P2-OU * -GI *  *  *  *  *  * \n"
    "P3-FU-FU-KE-FU+RY *  *  *  * \n"
    "P4 *  * +KE * -FU+FU+GI-FU-FU\n"
    "P5+FU * -FU * -KI * +GI-KE * \n"
    "P6 *  *  *  *  *  *  *  * +FU\n"
    "P7 * -NG * +FU * -TO-UM *  * \n"
    "P8 *  *  * +KI+KI *  * -RY * \n"
    "P9+KY+KE+OU * +FU *  *  * +KY\n"
    "P+00KA\n"
    "P-00FU00FU00FU00FU00FU\n"
    "+\n").initialState();
    NumEffectState state(sstate);

    NtesukiSimulationSearcher simulator(state, gam.get(), path,
					table, osl::ntesuki::NtesukiRecord::no_is, OslConfig::verbose());
  
    NtesukiRecord* record = table.allocateRoot(HashKey(state),
					     PieceStand(WHITE, state), 0);
    bool result = simulator.startFromAttackDisproof<BLACK>(record, record_orig,
							   1, Move::INVALID());
    BOOST_CHECK(!result);
  }
}
CPPUNIT_TEST_SUITE_REGISTRATION(NtesukiSimulationSearcherTest);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
