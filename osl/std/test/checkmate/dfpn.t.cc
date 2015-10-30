/* dfpn.t.cc
 */
#include "osl/checkmate/dfpn.h"
#include "osl/oslConfig.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>

#include <string>
#include <fstream>
#include <iostream>

using namespace osl;
using namespace osl::checkmate;
typedef DfpnTable table_t;

static osl::CArray<int,2> numSolved = {{ 0 }};

BOOST_AUTO_TEST_CASE(DfpnTestPawnCheckmate)
{
  {
    NumEffectState state=CsaString(
      "P1 *  *  *  *  *  * +KI * -KY\n"
      "P2 *  *  *  *  *  * +GI * -OU\n"
      "P3 *  *  *  *  *  * +RY *  * \n"
      "P4 *  *  *  *  *  *  * +KE * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  *  *  *  *  * -HI * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  * +OU *  *  *  *  *  * \n"
      "P+00FU\n"
      "P-00AL\n"
      "-\n").initialState();
    table_t table(BLACK);
    Dfpn searcher;
    searcher.setTable(&table);
    const PathEncoding path(WHITE);
    const Move lastMove = Move(Square(2,4),KNIGHT,BLACK);;
    ProofDisproof proofDisproof
      =searcher.hasEscapeMove(state,HashKey(state),path,1500,lastMove);
    BOOST_CHECK(!proofDisproof.isCheckmateSuccess());
  }
  {
    NumEffectState state=CsaString(
      "P1 *  *  *  *  *  *  * -KE-OU\n"
      "P2 *  *  *  *  *  *  *  * +FU\n"
      "P3 *  *  *  *  *  *  * +TO-FU\n"
      "P4 *  *  *  *  *  * +RY *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  *  *  *  *  *  *  * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  * +OU *  *  *  *  *  * \n"
      "P+00FU\n"
      "P-00AL\n"
      "-\n").initialState();
    table_t table(BLACK);
    Dfpn searcher;
    searcher.setTable(&table);
    const PathEncoding path(WHITE);
    const Move lastMove = Move(Square(1,2),PAWN,BLACK);;
    ProofDisproof proofDisproof
      =searcher.hasEscapeMove(state,HashKey(state),path,1500,lastMove);
    BOOST_CHECK_EQUAL(ProofDisproof::PawnCheckmate(),proofDisproof); // bug id 1
  }
  {
    NumEffectState state=CsaString(
      "P1 *  *  *  *  *  *  * -KE-OU\n"
      "P2 *  *  *  *  *  *  *  *  * \n"
      "P3 *  *  *  *  *  *  * +TO-FU\n"
      "P4 *  *  *  *  *  * +RY *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  *  *  *  *  *  *  * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  * +OU *  *  *  *  *  * \n"
      "P+00FU\n"
      "P-00AL\n"
      "+\n").initialState();
    table_t table(BLACK);
    const PathEncoding path(BLACK);
    Dfpn searcher; searcher.setTable(&table);
    Move bestMove;
    ProofDisproof pdp
      =searcher.hasCheckmateMove(state,HashKey(state),path,1500,bestMove);
    BOOST_CHECK(pdp.isCheckmateFail());
  }
  {
    NumEffectState state=CsaString(
      "P1 *  *  *  *  *  *  * -KE * \n"
      "P2 *  *  *  *  *  *  * -OU * \n"
      "P3 *  *  *  *  *  *  * -FU-FU\n"
      "P4 *  *  *  *  *  * +RY+FU * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  *  *  *  *  *  *  * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  * +OU *  *  *  *  *  * \n"
      "P-00AL\n"
      "+\n").initialState();
    table_t table(BLACK);
    const PathEncoding path(BLACK);
    Dfpn searcher; searcher.setTable(&table);
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state,HashKey(state),path,3000,checkMove);
    if (! proofDisproof.isCheckmateSuccess())
    {
      std::cerr << proofDisproof << "\n";
    }
    BOOST_CHECK(proofDisproof.isCheckmateSuccess());
    const Move bestMove=Move(Square(2,4),Square(2,3),PAWN,PAWN,false,BLACK);
    BOOST_CHECK_EQUAL(bestMove, checkMove);
  }
  {
    NumEffectState state=CsaString(
      "P1 *  *  *  *  *  *  * -KE * \n"
      "P2 *  *  *  *  *  *  * -OU * \n"
      "P3 *  *  *  *  *  *  * +TO-FU\n"
      "P4 *  *  *  *  *  * +RY *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  *  *  *  *  *  *  * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  * +OU *  *  *  *  *  * \n"
      "P+00FU\n"
      "P-00AL\n"
      "-\n").initialState();
    table_t table(BLACK);
    const PathEncoding path(WHITE);
    Dfpn searcher; searcher.setTable(&table);
    Move lastMove=Move(Square(2,4),Square(2,3),PPAWN,PAWN,true,BLACK);
    ProofDisproof pdp
      =searcher.hasEscapeMove(state,HashKey(state),path,1500,lastMove);
    if (pdp != ProofDisproof::PawnCheckmate())
    {
      BOOST_CHECK_EQUAL(ProofDisproof::LoopDetection(),
			   pdp);
    }
    BOOST_CHECK(pdp.isCheckmateFail());
  }
  {
    NumEffectState state=CsaString(
      "P1 *  *  *  *  *  *  * -KE-OU\n"
      "P2 *  *  *  *  *  *  *  *  * \n"
      "P3 *  *  *  *  *  *  * +TO-FU\n"
      "P4 *  *  *  *  *  * +RY *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  *  *  *  *  *  *  * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  * +OU *  *  *  *  *  * \n"
      "P+00FU\n"
      "P-00AL\n"
      "+\n").initialState();
    const PathEncoding path(BLACK);
    table_t table(BLACK);
    Dfpn searcher; searcher.setTable(&table);
    Move checkMove=Move::INVALID();
    ProofDisproof pdp
      =searcher.hasCheckmateMove(state,HashKey(state),path,1000,checkMove);
    BOOST_CHECK(pdp.isCheckmateFail());
  }
  {
    NumEffectState state(CsaString(
      "P1 *  *  *  *  *  *  *  *  * \n"
      "P2 *  *  *  *  *  *  *  *  * \n"
      "P3 *  *  *  *  *  *  *  * -KY\n"
      "P4 *  *  *  *  *  * +KA * -OU \n"
      "P5 *  *  *  *  * +KI * +KY-FU\n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  *  *  *  *  * +GI * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  * +OU *  *  *  *  *  * \n"
      "P+00FU\n"
      "P-00AL\n"
      "+\n").initialState());
    const PathEncoding path(BLACK);
    table_t table(BLACK);
    Dfpn searcher; searcher.setTable(&table);
    Move checkMove=Move::INVALID();
    ProofDisproof pdp
      =searcher.hasCheckmateMove(state,HashKey(state),path,1000,checkMove);
    BOOST_CHECK(pdp.isCheckmateSuccess());
    const Move bestMove=Move(Square(3,4),Square(2,3),BISHOP,PTYPE_EMPTY,false,BLACK);
    BOOST_CHECK_EQUAL(bestMove, checkMove);

  }
}

BOOST_AUTO_TEST_CASE(DfpnTestNoCheck)
{
  {
    NumEffectState state=CsaString(
"P1 *  *  *  * -KI *  * -KE-KY\n"
"P2 * -OU *  *  *  *  * -HI * \n"
"P3 * -FU * -KI-FU-FU-GI * -FU\n"
"P4+FU * +FU-FU *  *  *  *  * \n"
"P5-FU *  *  *  *  *  * -FU * \n"
"P6 * -UM *  * +UM *  *  * +FU\n"
"P7 *  *  * +FU+FU+FU+FU+FU * \n"
"P8 *  *  * +KI *  *  * +HI * \n"
"P9+KY *  *  * +OU * +GI+KE+KY\n"
"P+00FU\n"
"P-00FU\n"
"P-00FU\n"
"P+00KE\n"
"P-00KE\n"
"P-00GI\n"
"P-00GI\n"
"P-00KI\n"
"P+00KY\n"
"-\n").initialState();
    table_t table(WHITE);
    const PathEncoding path(WHITE);
    Dfpn searcher; searcher.setTable(&table);
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state,HashKey(state),path,1000,checkMove);
    BOOST_CHECK(proofDisproof.isUnknown());
    proofDisproof
      =searcher.hasCheckmateMove(state,HashKey(state),path,100000,checkMove);
    BOOST_CHECK(! proofDisproof.isCheckmateSuccess());
  }
  {
    NumEffectState state=CsaString(
"P1-KY-KE * -KI *  *  * -KE-KY\n"
"P2 * -OU-GI * -KI * -HI *  * \n"
"P3 * -FU *  * -FU-GI-KA-FU-FU\n"
"P4-FU * -FU-FU * -FU-FU *  * \n"
"P5 *  *  *  *  *  *  * +FU * \n"
"P6+FU * +FU * +FU+GI+FU * +FU\n"
"P7 * +FU * +FU * +FU *  *  * \n"
"P8 * +KA+OU * +KI+GI * +HI * \n"
"P9+KY+KE * +KI *  *  * +KE+KY\n"
"+\n").initialState();
    const PathEncoding path(BLACK);
    table_t table(BLACK);
    Dfpn searcher; searcher.setTable(&table);
    Move checkMove=Move::INVALID();
    const ProofDisproof result
      = searcher.hasCheckmateMove(state,HashKey(state),path,1000,checkMove);
    BOOST_CHECK(result.isCheckmateFail());
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  * -FU *  *  *  *  *\n"
			   "P2-OU+FU *  *  *  *  *  *  *\n"
			   "P3 * +KE+OU *  *  *  *  *  *\n"
			   "P4 *  *  *  *  *  *  *  *  *\n"
			   "P5 *  *  *  *  *  *  *  *  *\n"
			   "P6-FU *  *  *  *  *  *  *  *\n"
			   "P7 *  * +GI *  *  *  *  *  *\n"
			   "P8 *  *  *  *  *  *  *  *  *\n"
			   "P9 *  *  *  *  *  *  *  *  *\n"
			   "P+00KY\n"
			   "P-00HI00HI00KA00KA00KI00KI00KI00KI00GI00GI00GI00KE00KE00KE00KY00KY00KY00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    const PathEncoding path(BLACK);
    table_t table(BLACK);
    Dfpn searcher; searcher.setTable(&table);
    Move checkMove=Move::INVALID();
    const ProofDisproof result
      = searcher.hasCheckmateMove(state,HashKey(state),path,8000,checkMove);
    if (! result.isCheckmateFail())
      std::cerr << result << " " << checkMove << "\n";
    BOOST_CHECK(result.isCheckmateFail());
  }
}

BOOST_AUTO_TEST_CASE(DfpnTestLimit)
{
  NumEffectState state0=CsaString(
"P1-KY-KE * -KI *  *  * -KE-KY\n"
"P2 * -OU-GI * -KI * -HI *  * \n"
"P3 * -FU *  * -FU-GI-KA-FU-FU\n"
"P4-FU * -FU-FU * -FU-FU *  * \n"
"P5 *  *  *  *  *  *  * +FU * \n"
"P6+FU * +FU * +FU+GI+FU * +FU\n"
"P7 * +FU * +FU * +FU *  *  * \n"
"P8 * +KA+OU * +KI+GI * +HI * \n"
"P9+KY+KE * +KI *  *  * +KE+KY\n"
"+\n").initialState();

  NumEffectState state1=CsaString(
"P1+NY+TO *  *  *  * -OU-KE-KY\n"
"P2 *  *  *  *  * -GI-KI *  *\n"
"P3 * +RY *  * +UM * -KI-FU-FU\n"
"P4 *  * +FU-FU *  *  *  *  *\n"
"P5 *  * -KE * +FU *  * +FU *\n"
"P6-KE *  * +FU+GI-FU *  * +FU\n"
"P7 *  * -UM *  *  *  *  *  *\n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 * +OU * -GI *  *  *  * -NG\n"
"P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
"P-00KI00KY00FU00FU\n"
"P-00AL\n"
"+\n").initialState();
  const PathEncoding path(BLACK);

  table_t table(BLACK);
  Dfpn searcher; searcher.setTable(&table);
  /* すぐに詰まないことがわかる局面を読む */
  {
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state0,HashKey(state0),path,1000,checkMove);
    BOOST_CHECK(proofDisproof.isCheckmateFail());
  }
  /* 詰みのチェックに時間がかかる局面を読む */
  {
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state1,HashKey(state1),path,1000,checkMove);
    BOOST_CHECK(proofDisproof.isUnknown());
  }
  /* すぐに詰まないことがわかる局面を読む */
  {
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state0,HashKey(state0),path,1000,checkMove);
    BOOST_CHECK(proofDisproof.isCheckmateFail());
  }
  /* 詰みのチェックに時間がかかる局面を読む */
  {
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state1,HashKey(state1),path,1000,checkMove);
    BOOST_CHECK(proofDisproof.isUnknown());
  }
}


template<bool isCheck,bool isDefense>
static void testFile(const std::string& filename)
{
  static table_t tableBlack(BLACK);
  static table_t tableWhite(WHITE);

  RecordMinimal record=CsaFileMinimal(filename).load();
  auto moves=record.moves;

  NumEffectState state(record.initial_state);
  ProofDisproof proofDisproof;
  Move checkMove=Move::INVALID();
  const Player attacker = ((state.turn()==BLACK) ^ isDefense) ? BLACK:WHITE;
  table_t& table = (attacker == BLACK) ? tableBlack : tableWhite;
  const PathEncoding path(state.turn());
  table.clear();
  Dfpn searcher; searcher.setTable(&table);
  if (isDefense)
    proofDisproof
      =searcher.hasEscapeMove(state,HashKey(state),path,100000,checkMove);
  else
    proofDisproof
      =searcher.hasCheckmateMove(state,HashKey(state),path,100000,checkMove);

  if (OslConfig::verbose())
  {
    if (proofDisproof.isFinal())
      std::cerr << " | ";
    else 
      std::cerr << "proof=" << proofDisproof.proof()
		<< ", disproof=" << proofDisproof.disproof() << " ";
  }
  if (proofDisproof.isFinal())
  {
    ++numSolved[isCheck];
    if (isCheck)
    {
      BOOST_CHECK(proofDisproof.isCheckmateSuccess());
      if ((! moves.empty()) && (checkMove!=moves[0]))
      {
	if (OslConfig::verbose())
	  std::cerr << "different solution in " << filename;
#if 0
	std::cerr << "checkMove=" << checkMove << ",correct answer=" << moves[0] << std::endl;
	std::cerr << sstate;
#endif
      }
    }
    else
    {
      BOOST_CHECK(proofDisproof.isCheckmateFail());
    }
  }
  if (OslConfig::verbose())
    std::cerr << "\n";
}

BOOST_AUTO_TEST_CASE(DfpnTestFilesCheck)
{
  std::ifstream ifs(OslConfig::testPublicFile("checkmate-problems/FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=OslConfig::inUnitTestShort() ? 40 : 4000;
  std::string filename;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong() && ! OslConfig::verbose())
    progress.reset(new boost::progress_display(count, std::cerr));
  while((ifs >> filename) && filename != "" && (++i<count)) {
    if (OslConfig::verbose())
      std::cerr << "testFilesCheck: " << filename << " ";
    if (progress)
      ++(*progress);
    testFile<true,false>(OslConfig::testPublicFile("checkmate-problems/" + filename));
  }
  const double ratio = (double)numSolved[true]/i;
  if (OslConfig::verbose())
    std::cerr << numSolved[true] << " / " << i << " = " 
	      << ratio << "\n";
  BOOST_CHECK(ratio >= 0.9);
}

BOOST_AUTO_TEST_CASE(DfpnTestFilesNoCheck)
{
  std::ifstream ifs(OslConfig::testPublicFile("nocheckmate-problems/FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=OslConfig::inUnitTestShort() ? 40 : 4000;
  std::string filename;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong() && ! OslConfig::verbose())
    progress.reset(new boost::progress_display(count, std::cerr));
  while((ifs >> filename) && filename != "" && (++i<count)) {
    if (progress)
      ++(*progress);
    if (OslConfig::verbose())
      std::cerr << "testFilesNoCheck: " << filename << " ";
    testFile<false,false>(OslConfig::testPublicFile("nocheckmate-problems/" + filename));
  }
  const double ratio = (double)numSolved[false]/i;
  std::cerr << numSolved[false] << " / " << i << " = " 
	    << ratio << "\n";
  BOOST_CHECK(ratio >= 0.9);
}

BOOST_AUTO_TEST_CASE(DfpnTestNoPromote)
{
  {
    NumEffectState state=CsaString(
      "P1 *  *  *  *  *  *  * -KE * \n"
      "P2 *  *  *  *  *  *  * -OU * \n"
      "P3 *  *  *  *  *  *  * -KE-FU\n"
      "P4 *  *  *  *  *  * +RY+FU * \n"
      "P5 *  *  *  *  *  * -GI * +OU\n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  *  *  *  *  *  *  * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  *  *  *  *  *  *  *  * \n"
      "P+00FU\n"
      "P-00AL\n"
      "+\n").initialState();
    table_t table(BLACK);
    Dfpn searcher;
    searcher.setTable(&table);
    const PathEncoding path(BLACK);
    Move best_move;
    ProofDisproof pdp
      =searcher.hasCheckmateMove(state,HashKey(state),path,1500,best_move);
    BOOST_CHECK(pdp.isCheckmateSuccess());
    // -2423TOは打歩詰, -2423FUは詰み
  }
}



// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
