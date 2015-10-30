/* profDisproofPices.t.cc
 */
#include "osl/ntesuki/ntesukiSearcher.h"
#include "osl/state/hashEffectState.h"
#include "osl/record/csaString.h"
#include "osl/record/csaRecord.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "osl/ntesuki/ntesukiTable.h"
#include "osl/checkmate/proofDisproof.h"
#include <boost/test/unit_test.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

class ProofDisproofPieces : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ProofDisproofPieces);
  CPPUNIT_TEST(test_proof0tesuki0);
  CPPUNIT_TEST(test_proof0tesuki1);
  CPPUNIT_TEST(test_proof0tesuki2);
  CPPUNIT_TEST(test_proof1tesuki0);
  CPPUNIT_TEST(test_proof1tesuki1);
  CPPUNIT_TEST(test_proof1tesuki2);
  CPPUNIT_TEST(test_proof1tesuki3);
  CPPUNIT_TEST(test_proofKatagyoku0);
  CPPUNIT_TEST(test_disproof0tesuki0);
  CPPUNIT_TEST(test_disproof0tesuki1);
  CPPUNIT_TEST(test_disproof1tesuki0);
  CPPUNIT_TEST(test_disproof1tesuki1);
  CPPUNIT_TEST(test_disproof1tesuki1_1);
  CPPUNIT_TEST(test_disproof1tesuki1_2);
  CPPUNIT_TEST(test_pdp_pieces0);
  CPPUNIT_TEST_SUITE_END();
public:
  void test_proof0tesuki0();
  void test_proof0tesuki1();
  void test_proof0tesuki2();
  void test_proof1tesuki0();
  void test_proof1tesuki1();
  void test_proof1tesuki2();
  void test_proof1tesuki3();
  void test_proofKatagyoku0();

  void test_disproof0tesuki0();
  void test_disproof0tesuki1();
  void test_disproof1tesuki0();
  void test_disproof1tesuki1();
  void test_disproof1tesuki1_1();
  void test_disproof1tesuki1_2();

  void test_pdp_pieces0();
};
CPPUNIT_TEST_SUITE_REGISTRATION(ProofDisproofPieces);

using namespace osl;
using namespace osl::ntesuki;
const size_t TABLE_LIMIT = 1000000;
const size_t READ_LIMIT = 2000000;

typedef  NtesukiMoveGenerator movegen_t;

static volatile int stop_flag = false;

void ProofDisproofPieces::test_proof0tesuki0()
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
  NtesukiSearcher searcher(nState, gam.get(),  TABLE_LIMIT, &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(0, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(),  0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(GOLD);

  BOOST_CHECK_EQUAL(correct, ans);
}

void ProofDisproofPieces::test_proof1tesuki0()
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
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(GOLD);

  BOOST_CHECK_EQUAL(correct, ans);
}

void ProofDisproofPieces::test_proof1tesuki1()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  * -KI * \n"
    "P4 *  *  *  *  *  *  * +FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU+KY *  *  *  * \n"
    "P8+HI+GI+KI+GI+GI *  *  *  * \n"
    "P9+HI+KI+OU+KI+GI *  *  *  * \n"
    "P-00AL\n"
    "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;

  BOOST_CHECK_EQUAL(correct, ans);
}

void ProofDisproofPieces::test_proof1tesuki2()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  * -KI * \n"
    "P4 *  *  *  *  *  *  * +FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU *  *  *  *  * \n"
    "P8+HI+KY+KI *  *  *  *  *  * \n"
    "P9+OU+KI+HI+KI *  *  *  *  * \n"
    "P+00GI00GI00GI00GI\n"
    "P-00AL\n"
    "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT,
			   &stop_flag, OslConfig::verbose(), 2,
			   osl::ntesuki::NtesukiRecord::pn_iw,
			   osl::ntesuki::NtesukiRecord::no_ps,
			   osl::ntesuki::NtesukiRecord::no_is,
			   2);

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(SILVER, 4);

  BOOST_CHECK_EQUAL(correct, ans);
}

void ProofDisproofPieces::test_proof0tesuki1()
{
/* 当初使っていた例は，後手に歩が一枚多いと，別の(より早い)手順を
   見つける可能性があるため，変更する必要がある．
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
    "P+00FU00FU00FU00FU00GI00KY00KA\n"
    "P-00FU00KI00KI\n"
    "-\n"
    );

 */
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
    "P+00FU00FU00FU00FU00GI00KY00KA\n"
    "P-00FU00KI00KI\n"
    "-\n"
    );
    SimpleState state = csa_str.initialState();
    const osl::vector<Move> moves = csa_str.load().moves();
    
    NumEffectState nState(state);
    boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
      gam(new movegen_t());
    NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT,
			     &stop_flag, OslConfig::verbose(), 1);
    
    const int ntesuki_num = searcher.searchSlow(osl::WHITE, READ_LIMIT);
    BOOST_CHECK_EQUAL(0, ntesuki_num);
    
    HashKey key (state);
    NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
    PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

    PieceStand correct;
    correct.add(PAWN, 1);
    correct.add(GOLD, 2);

    BOOST_CHECK_EQUAL(correct, ans);
  }
}

void ProofDisproofPieces::test_proof0tesuki2()
{
  /* from search tree of
   *  probles/100/4/1.csa
   *  by ntesuki_searcher 2153
   */
  {
  SimpleState state=CsaString(
   "P1-KY-OU *  *  *  *  *  * -KY\n"
   "P2 * +KI *  *  *  *  *  *  * \n"
   "P3 *  * +GI * -GI * -KE * -FU\n"
   "P4-FU * -FU-FU-FU-KA-FU *  * \n"
   "P5 * +KE *  *  * -FU *  *  * \n"
   "P6+FU-FU *  * +FU *  *  *  * \n"
   "P7 *  * +FU+FU * +FU+FU * +FU\n"
   "P8 *  * +KI+GI *  *  *  *  * \n"
   "P9+KY * +OU * +KI * -RY+KE+KY\n"
   "P+00KE\n"
   "P-00KA00KI00GI00FU00FU00FU00HI\n"
   "-\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(BLACK, READ_LIMIT);
  BOOST_CHECK_EQUAL(0, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  BOOST_CHECK_EQUAL(correct, ans);
  }
  {
  SimpleState state=CsaString(
   "P1-KY-OU *  *  *  *  *  * -KY\n"
   "P2 *  *  *  *  *  *  *  *  * \n"
   "P3 *  * +GI * -GI * -KE * -FU\n"
   "P4-FU * -FU-FU-FU-KA-FU *  * \n"
   "P5 * +KE *  *  * -FU *  *  * \n"
   "P6+FU-FU *  * +FU *  *  *  * \n"
   "P7 *  * +FU+FU * +FU+FU * +FU\n"
   "P8 *  * +KI+GI *  *  *  *  * \n"
   "P9+KY * +OU * +KI * -RY+KE+KY\n"
   "P+00KI\n"
   "P-00KA00KI00GI00FU00FU00FU00HI00KE\n"
   "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(BLACK, READ_LIMIT);
  BOOST_CHECK_EQUAL(0, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(GOLD, 1);
  BOOST_CHECK_EQUAL(correct, ans);
  }
  {
  SimpleState state=CsaString(
   "P1-KY-OU *  *  *  *  *  * -KY\n"
   "P2 *  *  *  *  *  *  *  *  * \n"
   "P3 *  * +GI * -GI * -KE * -FU\n"
   "P4-FU * -FU-FU-FU-KA-FU *  * \n"
   "P5 * +KE *  *  * -FU *  *  * \n"
   "P6+FU-FU *  * +FU *  *  *  * \n"
   "P7 *  * +FU+FU * +FU+FU * +FU\n"
   "P8 *  * +KI+GI *  *  *  *  * \n"
   "P9+KY * +OU * +KI * -RY+KE+KY\n"
   "P+00KI00KE\n"
   "P-00KA00KI00GI00FU00FU00FU00HI\n"
   "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(BLACK, READ_LIMIT);
  BOOST_CHECK_EQUAL(0, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(GOLD, 1);
  BOOST_CHECK_EQUAL(correct, ans);
  }
  {
  SimpleState state=CsaString(
   "P1-KY *  *  *  *  *  *  * -KY\n"
   "P2 * -OU *  *  *  *  *  *  * \n"
   "P3 *  * +GI * -GI * -KE * -FU\n"
   "P4-FU * -FU-FU-FU-KA-FU *  * \n"
   "P5 * +KE *  *  * -FU *  *  * \n"
   "P6+FU-FU *  * +FU *  *  *  * \n"
   "P7 *  * +FU+FU * +FU+FU * +FU\n"
   "P8 *  * +KI+GI *  *  *  *  * \n"
   "P9+KY * +OU * +KI * -RY+KE+KY\n"
   "P+00KI00KE\n"
   "P-00KA00KI00GI00FU00FU00FU00HI\n"
   "-\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(BLACK, READ_LIMIT);
  BOOST_CHECK_EQUAL(0, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(GOLD, 1);
  BOOST_CHECK_EQUAL(correct, ans);
  }

  {
  SimpleState state=CsaString(
   "P1-KY *  *  *  *  *  *  * -KY\n"
   "P2 * -OU *  *  *  *  *  *  * \n"
   "P3 *  *  *  * -GI * -KE * -FU\n"
   "P4-FU * -FU-FU-FU-KA-FU *  * \n"
   "P5 * +KE *  *  * -FU *  *  * \n"
   "P6+FU-FU *  * +FU *  *  *  * \n"
   "P7 *  * +FU+FU * +FU+FU * +FU\n"
   "P8 *  * +KI+GI *  *  *  *  * \n"
   "P9+KY * +OU * +KI * -RY+KE+KY\n"
   "P+00GI00KE00KI\n"
   "P-00KA00KI00GI00FU00FU00FU00HI\n"
   "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(0, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(SILVER, 1);
  correct.add(GOLD, 1);
  //correct.add(KNIGHT, 1);
  BOOST_CHECK_EQUAL(correct, ans);
  }

  {
  SimpleState state=CsaString(
   "P1-KY+HI *  *  *  *  *  * -KY\n"
   "P2 * -KI *  *  *  *  *  *  * \n"
   "P3 * -OU *  * -GI * -KE * -FU\n"
   "P4-FU * -FU-FU-FU-KA-FU *  * \n"
   "P5 * +KE *  *  * -FU *  *  * \n"
   "P6+FU-FU *  * +FU *  *  *  * \n"
   "P7 *  * +FU+FU * +FU+FU * +FU\n"
   "P8 *  * +KI+GI *  *  *  *  * \n"
   "P9+KY * +OU * +KI * -RY+KE+KY\n"
   "P+00GI00KE\n"
   "P-00KA00KI00GI00FU00FU00FU\n"
   "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(0, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(SILVER, 1);
  //correct.add(KNIGHT, 1);
  //althogh opponent does not have any KNIGHTs,
  //we checkmate with unescapable moves
  //a gold will be captured

  BOOST_CHECK_EQUAL(correct, ans);
  }
}


void ProofDisproofPieces::test_proof1tesuki3()
{
  /* from search tree of
   *  probles/100/4/1.csa
   *  by ntesuki_searcher 2168
   *   assertion failed in ../include/osl/ntesuki/ntesukiRecord.tcc line 367
   *   PP is (stand 2 1 2 1 1 0 0)
   *    = 00HI00HI00KA00KI00KI00GI00KE
   *   where 00KA is exessive
   */
  {
  SimpleState state=CsaString(
    "P1-KY-OU *  *  *  *  *  * -KY\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 * +TO *  * -GI * -KE * -FU\n"
    "P4-FU * -FU-FU-FU-KA-FU *  * \n"
    "P5 * -GI *  *  * -FU *  *  * \n"
    "P6+FU-FU *  * +FU *  *  *  * \n"
    "P7 *  * +FU+FU * +FU+FU * +FU\n"
    "P8 * +KI * +GI *  *  *  *  * \n"
    "P9+KY+KE * +OU-KI *  * +KE+KY\n"
    "P+00HI00HI00KA00KI00KI00GI00KE\n"
    "P-00FU00FU\n"
    "+\n"
    ).initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag,
			   OslConfig::verbose(), 2,
			   osl::ntesuki::NtesukiRecord::pn_iw,
			   osl::ntesuki::NtesukiRecord::no_ps,
			   osl::ntesuki::NtesukiRecord::no_is,
			   2);

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(ROOK, 2);
  correct.add(BISHOP, 1);
  correct.add(GOLD, 2);
  correct.add(SILVER, 1);
  correct.add(KNIGHT, 1);

  BOOST_CHECK_EQUAL(correct, ans);
  }
  {
  SimpleState state=CsaString(
    "P1-KY-OU *  *  *  *  *  * -KY\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 * +TO *  * -GI * -KE * -FU\n"
    "P4-FU * -FU-FU-FU-KA-FU *  * \n"
    "P5 * -GI *  *  * -FU *  *  * \n"
    "P6+FU-FU *  * +FU *  *  *  * \n"
    "P7 *  * +FU+FU * +FU+FU * +FU\n"
    "P8 * +KI * +GI * -KI *  *  * \n"
    "P9+KY+KE+OU-UM *  *  * +KE+KY\n"
    "P+00HI00HI00KI00KI00GI00KE\n"
    "P-00FU00FU\n"
    "+\n"
    ).initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag,
			   OslConfig::verbose(),2,
			   osl::ntesuki::NtesukiRecord::pn_iw,
			   osl::ntesuki::NtesukiRecord::no_ps,
			   osl::ntesuki::NtesukiRecord::no_is,
			   2);

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(ROOK, 2);
  correct.add(GOLD, 2);
  correct.add(SILVER, 1);
  correct.add(KNIGHT, 1);

  BOOST_CHECK_EQUAL(correct, ans);
  }
  {
  SimpleState state=CsaString(
    "P1-KY-OU *  *  *  *  *  * -KY\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 * +TO *  * -GI * -KE * -FU\n"
    "P4-FU * -FU-FU-FU-KA-FU *  * \n"
    "P5 * -GI *  *  * -FU *  *  * \n"
    "P6+FU-FU *  * +FU *  *  *  * \n"
    "P7 *  * +FU+FU * +FU+FU * +FU\n"
    "P8 * +KI * +GI-KA-KI *  *  * \n"
    "P9+KY+KE-HI+OU *  *  * +KE+KY\n"
    "P+00HI00KI00KI00GI00KE\n"
    "P-00FU00FU\n"
    "+\n"
    ).initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag,
			   OslConfig::verbose(),2,
			   osl::ntesuki::NtesukiRecord::pn_iw,
			   osl::ntesuki::NtesukiRecord::no_ps,
			   osl::ntesuki::NtesukiRecord::no_is,
			   2);

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(ROOK, 1);
  correct.add(GOLD, 2);
  correct.add(SILVER, 1);
  correct.add(KNIGHT, 1);

  BOOST_CHECK_EQUAL(correct, ans);
  }
}

/* 勝浦修「必至のかけ方」創元社より
 */
void ProofDisproofPieces::test_proofKatagyoku0()
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
  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(ntesuki_num);

  PieceStand correct;
  correct.add(GOLD);
  correct.add(GOLD);

  BOOST_CHECK_EQUAL(correct, ans);
}

void ProofDisproofPieces::test_disproof0tesuki0()
{
  SimpleState state=CsaString(
    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
    "P2 * -HI *  *  *  *  * -KA * \n"
    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
    "P8 * +KA *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "+\n").initialState();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);
  NtesukiSearcher searcher(nState, gam.get(),  TABLE_LIMIT, &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(-1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(),  0);
  PieceStand ans = record->getPDPieces<BLACK>(0);

  PieceStand correct;
  //correct.add(PAWN,0);

  BOOST_CHECK_EQUAL(correct, ans);
}

void ProofDisproofPieces::test_disproof0tesuki1()
{
  SimpleState state=CsaString(
    "P1 *  *  * +HI-FU *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  * +KI\n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6-FU-FU-FU-FU * -FU-FU-FU-FU\n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
    "P9+OU *  *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").initialState();
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NumEffectState nState(state);
  NtesukiSearcher searcher(nState, gam.get(),  TABLE_LIMIT, &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(-1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(),  0);
  PieceStand ans = record->getPDPieces<BLACK>(0);

  PieceStand correct;
  correct.add(ROOK, 1);
  correct.add(BISHOP, 2);
  correct.add(GOLD,3);
  correct.add(SILVER,4);
  correct.add(KNIGHT,4);
  correct.add(LANCE,4);
  //correct.add(PAWN,0);

  BOOST_CHECK_EQUAL(correct, ans);
}

void ProofDisproofPieces::test_disproof1tesuki0()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  * -KE * \n"
    "P4 *  *  *  *  *  *  * +FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU *  *  *  *  *  *  * \n"
    "P8+KI+GI *  *  *  *  *  *  * \n"
    "P9+OU+KI *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(-1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(1);

  PieceStand correct;
  correct.add(ROOK, 2);
  correct.add(BISHOP, 2);
  correct.add(GOLD,2);
  correct.add(SILVER,3);
  correct.add(KNIGHT,3);
  correct.add(LANCE,4);
  correct.add(PAWN,15);

  BOOST_CHECK_EQUAL(correct, ans);
}

void ProofDisproofPieces::test_disproof1tesuki1()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  * -OU-KI * \n"
    "P2 *  *  *  *  *  *  * +KI * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU *  *  *  *  *  *  * \n"
    "P8+KI+GI *  *  *  *  *  *  * \n"
    "P9+OU+KI *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(BLACK, READ_LIMIT);
  BOOST_CHECK_EQUAL(-1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(1);

  PieceStand correct;
  correct.add(ROOK, 2);
  correct.add(BISHOP, 2);
  correct.add(GOLD,0);
  correct.add(SILVER,3);
  correct.add(KNIGHT,4);
  correct.add(LANCE,4);
  correct.add(PAWN,16);


  BOOST_CHECK_EQUAL(correct, ans);
}

void ProofDisproofPieces::test_disproof1tesuki1_1()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  * -OU *  * \n"
    "P2 *  *  *  *  *  *  * -KI * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU *  *  *  *  *  *  * \n"
    "P8+KI+GI *  *  *  *  *  *  * \n"
    "P9+OU+KI *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(-1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(1);

  PieceStand correct;
  correct.add(ROOK, 2);
  correct.add(BISHOP, 2);
  correct.add(GOLD,1);
  correct.add(SILVER,3);
  correct.add(KNIGHT,4);
  correct.add(LANCE,4);
  correct.add(PAWN,16);


  BOOST_CHECK_EQUAL(correct, ans);
}

void ProofDisproofPieces::test_disproof1tesuki1_2()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -KI * \n"
    "P2 *  *  *  *  *  *  * -OU * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU *  *  *  *  *  *  * \n"
    "P8+KI+GI *  *  *  *  *  *  * \n"
    "P9+OU+KI *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(nState.turn(), READ_LIMIT);
  BOOST_CHECK_EQUAL(-1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<BLACK>(1);

  PieceStand correct;
  correct.add(ROOK, 2);
  correct.add(BISHOP, 2);
  correct.add(GOLD,1);
  correct.add(SILVER,3);
  correct.add(KNIGHT,4);
  correct.add(LANCE,4);
  correct.add(PAWN,16);


  BOOST_CHECK_EQUAL(correct, ans);
}

void ProofDisproofPieces::test_pdp_pieces0()
{
  /* from the search tree of
   *  problems/100/4/012.csa
   */
  {//proof
  SimpleState state=CsaString(
    "P1-KY-KE-OU *  *  *  * -KE-KY\n"
    "P2 *  *  * -KI+NG *  *  *  * \n"
    "P3 * -FU * -FU-FU *  * -FU-FU\n"
    "P4-FU * -FU-KA *  *  *  *  * \n"
    "P5 * -KE * -GI * -FU * +FU * \n"
    "P6+FU+OU+FU *  *  *  *  *  * \n"
    "P7 * +FU * +FU+FU+FU *  * +FU\n"
    "P8 *  *  * +GI * +GI *  *  * \n"
    "P9+KY *  *  * +KI-RY * +KE+KY\n"
    "P+00HI00KA00KI\n"
    "P-00KI00FU00FU\n"
    "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(WHITE, READ_LIMIT);
  BOOST_CHECK_EQUAL(0, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<WHITE>(0);

  PieceStand correct;
  correct.add(GOLD,1);
  correct.add(PAWN,2);

  BOOST_CHECK_EQUAL(correct, ans);
  }
  {//disproof
  SimpleState state=CsaString(
    "P1-KY-KE-OU *  *  *  * -KE-KY\n"
    "P2 *  *  * -KI+NG *  *  *  * \n"
    "P3 * -FU * -FU-FU *  * -FU-FU\n"
    "P4-FU * -FU-KA *  *  *  *  * \n"
    "P5 * -KE * -GI * -FU * +FU * \n"
    "P6+FU+OU+FU *  *  *  *  *  * \n"
    "P7 * +FU * +FU+FU+FU *  * +FU\n"
    "P8 *  *  * +GI * +GI *  *  * \n"
    "P9+KY *  *  * +KI-RY * +KE+KY\n"
    "P+00HI00KA00KI00KI\n"
    "P-00FU00FU\n"
    "+\n").initialState();
  NumEffectState nState(state);
  boost::scoped_ptr<osl::ntesuki::NtesukiMoveGenerator>
    gam(new movegen_t());
  NtesukiSearcher searcher(nState, gam.get(), TABLE_LIMIT, &stop_flag, OslConfig::verbose(), 1);

  const int ntesuki_num = searcher.searchSlow(WHITE, READ_LIMIT);
  BOOST_CHECK_EQUAL(-1, ntesuki_num);

  HashKey key (state);
  NtesukiRecord *record = searcher.getTable().allocateRoot(key, PieceStand(), 0);
  PieceStand ans = record->getPDPieces<WHITE>(0);

  PieceStand correct;
  correct.add(ROOK, 1);
  correct.add(BISHOP, 1);
  correct.add(GOLD,2);
  correct.add(SILVER,0);
  correct.add(KNIGHT,0);
  correct.add(LANCE,0);
  correct.add(PAWN,0);

  BOOST_CHECK_EQUAL(correct, ans);
  }
}
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
