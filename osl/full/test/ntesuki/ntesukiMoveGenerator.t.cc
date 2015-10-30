#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "osl/state/numEffectState.tcc"
#include "osl/record/csaString.h"
#include "osl/record/csaRecord.h"

#include <boost/test/unit_test.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

using namespace osl;
using namespace osl::ntesuki;

class NtesukiMoveGeneratorTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(NtesukiMoveGeneratorTest);
  CPPUNIT_TEST(testGetAttackMoves);
  CPPUNIT_TEST_SUITE_END();
public:
  void testGetAttackMoves();
};

//-----------------------------------------------------------------------------
// Attack
//-----------------------------------------------------------------------------

void NtesukiMoveGeneratorTest::
testGetAttackMoves()
{
  NtesukiMoveGenerator gam(OslConfig::verbose());
  {
    SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  * +FU * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
    NumEffectState nState(state);
  
    NtesukiMoveList moves;
    osl::Square lastTo = Square::STAND();
    
    gam.generate<BLACK>(nState, moves);
    BOOST_CHECK_EQUAL(size_t(85), moves.size());
  }
  {
    SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  * +FU * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  * +HI * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
    NumEffectState nState(state);
    NtesukiMoveGenerator gam(OslConfig::verbose());
    
    NtesukiMoveList moves;
    osl::Square lastTo = Square::STAND();
    
    gam.generate<BLACK>(nState, moves);
    BOOST_CHECK_EQUAL((size_t)91, moves.size());
  }
  {
    SimpleState state = CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  * +FU * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  * -HI * +HI * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
    NumEffectState nState(state);
    NtesukiMoveGenerator gam(OslConfig::verbose());
    
    NtesukiMoveList moves;
    osl::Square lastTo = Square::STAND();
    
    gam.generate<BLACK>(nState, moves);
    BOOST_CHECK_EQUAL((size_t)6, moves.size());
  }
  {
    SimpleState state		      =	CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  * +FU  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
    NumEffectState nState(state);
    NtesukiMoveGenerator gam(OslConfig::verbose());
    
    NtesukiMoveList moves;
      
    gam.generate<BLACK>(nState, moves);
    BOOST_CHECK_EQUAL((size_t)86, moves.size());
  }
}

CPPUNIT_TEST_SUITE_REGISTRATION(NtesukiMoveGeneratorTest);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
