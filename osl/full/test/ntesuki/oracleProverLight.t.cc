#include "osl/ntesuki/oracleProverLight.h"
#include "osl/ntesuki/ntesukiSearcher.h"
#include "osl/ntesuki/ntesukiTable.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "osl/record/csaString.h"

#include <boost/test/unit_test.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace osl;

class OracleProverLightTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(OracleProverLightTest);
  CPPUNIT_TEST(test0Tesuki0);
  CPPUNIT_TEST(test0Tesuki1);
  CPPUNIT_TEST(test0Tesuki2);
  CPPUNIT_TEST(test1Tesuki0);
  //CPPUNIT_TEST(test1Tesuki1);
  CPPUNIT_TEST_SUITE_END();
public:
  void test0Tesuki0();
  void test0Tesuki1();
  void test0Tesuki2();
  void test1Tesuki0();
  void test1Tesuki1();
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
    mg(new movegen_t());

  NtesukiSearcher searcher(state_orig, mg.get(),
			   100000, &stop_flag, OslConfig::verbose(), ntesuki_correct + 1);

  const int ntesuki_num = searcher.searchSlow(state_orig.turn());
  BOOST_CHECK_EQUAL(ntesuki_correct, ntesuki_num);
  
  NumEffectState state(sstate);

  OracleProverLight simulator(state, mg.get(), path,
			      searcher.getTable());

  NtesukiRecord *record = searcher.getTable().allocateRoot(HashKey(state), PieceStand(), 0);
  NtesukiRecord *record_orig = searcher.getTable().allocateRoot(HashKey(state_orig), PieceStand(), 0);
  const NtesukiMove& m = record_orig->getBestMove<BLACK>(ntesuki_num);
  if (m.isImmediateCheckmate()) return;

  bool result = simulator.startFromAttack<BLACK>(record, record_orig, ntesuki_num);
  BOOST_CHECK(correct == result);
}

void OracleProverLightTest::
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

void OracleProverLightTest::
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

void OracleProverLightTest::
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

void OracleProverLightTest::
test1Tesuki0()
{
  if (isShortTest) return;
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    mg(new movegen_t());

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
  NtesukiSearcher searcher(state_orig, mg.get(),
			   10000000, &stop_flag, OslConfig::verbose());
  const int ntesuki_num = searcher.searchSlow(state_orig.turn(), 20000000);
  BOOST_CHECK_EQUAL(1, ntesuki_num);

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

    OracleProverLight simulator(state, mg.get(), path,
				searcher.getTable());
  
    bool result = simulator.startFromAttack<BLACK>(searcher.getTable().
						   allocateRoot(HashKey(state), PieceStand(), 0),
						   searcher.getTable().
						   allocateRoot(HashKey(state_orig), PieceStand(), 0),
						   ntesuki_num);
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

    OracleProverLight simulator(state, mg.get(), path,
				searcher.getTable());
  
    bool result = simulator.startFromAttack<BLACK>(searcher.getTable().
						   allocateRoot(HashKey(state), PieceStand(), 0),
						   searcher.getTable().
						   allocateRoot(HashKey(state_orig), PieceStand(), 0),
						   ntesuki_num);
    BOOST_CHECK_EQUAL(false, result);
  }
  //moved -91KY to -92KY
  //move +99KY to +98KY
}

void OracleProverLightTest::
test1Tesuki1()
{
  if (isShortTest) return;
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    mg(new movegen_t());

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
  NtesukiSearcher searcher(state_orig, mg.get(),
			   10000000, &stop_flag, OslConfig::verbose());
  const int ntesuki_num = searcher.searchSlow(state_orig.turn(), 20000000);
  BOOST_CHECK_EQUAL(1, ntesuki_num);

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

    OracleProverLight simulator(state, mg.get(), path,
				searcher.getTable());
  
    bool result = simulator.startFromAttack<BLACK>(searcher.getTable().
						   allocateRoot(HashKey(state), PieceStand(), 0),
						   searcher.getTable().
						   allocateRoot(HashKey(state_orig), PieceStand(), 0),
						   ntesuki_num);
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

    OracleProverLight simulator(state, mg.get(), path,
				searcher.getTable());
  
    bool result = simulator.startFromAttack<BLACK>(searcher.getTable().
						   allocateRoot(HashKey(state), PieceStand(), 0),
						   searcher.getTable().
						   allocateRoot(HashKey(state_orig), PieceStand(), 0),
						   ntesuki_num);
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
    
    OracleProverLight simulator(state, mg.get(), path,
				searcher.getTable());
    
    bool result = simulator.startFromAttack<BLACK>(searcher.getTable().
						   allocateRoot(HashKey(state), PieceStand(), 0),
						   searcher.getTable().
						   allocateRoot(HashKey(state_orig), PieceStand(), 0),
						   ntesuki_num);
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
    
    OracleProverLight simulator(state, mg.get(), path,
				searcher.getTable());
    
    bool result = simulator.startFromAttack<BLACK>(searcher.getTable().
						   allocateRoot(HashKey(state), PieceStand(), 0),
						   searcher.getTable().
						   allocateRoot(HashKey(state_orig), PieceStand(), 0),
						   ntesuki_num);
    BOOST_CHECK_EQUAL(true, result);
  }
}

CPPUNIT_TEST_SUITE_REGISTRATION(OracleProverLightTest);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
