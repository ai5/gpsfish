/* fixedDepthSearcher.t.cc
 */
#include "osl/checkmate/fixedDepthSolverExt.h"
#include "osl/checkmate/fixedDepthSearcher.tcc"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::checkmate;

BOOST_AUTO_TEST_CASE(CheckmateFixedDepthSearchTestZero)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  * -KE-OU\n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  * +TO-FU\n"
			   "P4 *  *  *  *  *  * +RY *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n").initialState());
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(0, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess() && !pdp.isCheckmateFail());
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  * -KE * \n"
			   "P2 *  *  *  *  *  *  * -OU * \n"
			   "P3 *  *  *  *  *  * +FU+TO-FU\n"
			   "P4 *  *  *  *  *  * +RY *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00FU\n"
			   "P-00AL\n"
			   "-\n").initialState());
    FixedDepthSearcher searcher(state);
    Move lastMove=Move(Square(2,4),Square(2,3),PPAWN,
		       PTYPE_EMPTY,false,BLACK);
    ProofDisproof pdp=searcher.hasEscapeMove<BLACK>(lastMove,0);
    BOOST_CHECK(!pdp.isCheckmateSuccess() && !pdp.isCheckmateFail());
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  * -KE-OU\n"
			   "P2 *  *  *  *  *  *  *  * +FU\n"
			   "P3 *  *  *  *  *  * +FU+TO-FU\n"
			   "P4 *  *  *  *  *  * +RY *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00FU\n"
			   "P-00AL\n"
			   "-\n").initialState());
    FixedDepthSearcher searcher(state);
    Move lastMove=Move(Square(1,2),PAWN,BLACK);
    ProofDisproof pdp=searcher.hasEscapeMove<BLACK>(lastMove,0);
    BOOST_CHECK_EQUAL(ProofDisproof::PawnCheckmate(),pdp);
  }
}

BOOST_AUTO_TEST_CASE(CheckmateFixedDepthSearchTestOne)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  * -KE-OU\n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  * +TO-FU\n"
			   "P4 *  *  *  *  *  * +RY *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n").initialState());
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(1, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess() && !pdp.isCheckmateFail());
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  * -KE * \n"
			   "P2 *  *  *  *  *  *  * -OU * \n"
			   "P3 *  *  *  *  *  * +TO * -FU\n"
			   "P4 *  *  *  *  *  * +RY *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00FU\n"
			   "P-00AL\n"
			   "-\n").initialState());
    FixedDepthSearcher searcher(state);
    Move lastMove=Move(Square(2,4),Square(3,3),PPAWN,
		       PTYPE_EMPTY,false,BLACK);
    ProofDisproof pdp=searcher.hasEscapeMove<BLACK>(lastMove,1);
    BOOST_CHECK(!pdp.isCheckmateSuccess() && !pdp.isCheckmateFail());
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  * -FU-FU\n"
			   "P3 *  *  *  *  *  *  * -KE-OU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  * +KE+KE\n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00FU\n"
			   "P-00AL\n"
			   "+\n").initialState());
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(1, best_move);
    BOOST_CHECK_EQUAL(ProofDisproof::PawnCheckmate(),pdp);
  }
}

BOOST_AUTO_TEST_CASE(CheckmateFixedDepthSearchTestThree)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  * -OU * -KE-KY\n"
			   "P2 *  * +HI *  * -GI * -KI * \n"
			   "P3 *  * -KE+GI-FU-FU-FU-FU * \n"
			   "P4-FU * -FU+OU * +FU * -RY-FU\n"
			   "P5 * -FU *  * +KI *  *  *  * \n"
			   "P6+FU * +FU * +FU *  *  * +FU\n"
			   "P7 * +FU * +FU *  *  *  *  * \n"
			   "P8 *  * -KI *  * -UM *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00KA00KI\n"
			   "P-00GI00GI00FU00FU00FU\n"
			   "+\n").initialState());
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(2, best_move);
    BOOST_CHECK(pdp.isCheckmateSuccess());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE+GI *  *  *  *  * -KY\n"
			   "P2-OU-KE-GI * -KI *  *  *  * \n"
			   "P3 * -FU+GI *  *  * +TO * -FU\n"
			   "P4+FU * -FU-GI-HI *  *  *  * \n"
			   "P5 *  *  *  *  *  * -KA *  * \n"
			   "P6 *  * +FU+FU-FU *  *  *  * \n"
			   "P7+KE+FU+KA+KI *  *  *  * +FU\n"
			   "P8 *  * +OU *  * +KI *  *  * \n"
			   "P9+KY *  *  * +FU *  * +KE+KY\n"
			   "P+00HI00FU00FU00FU00FU00FU\n"
			   "P-00KI00FU00FU\n"
			   "+\n").initialState());
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(3, best_move);
    BOOST_CHECK_EQUAL(ProofDisproof::Checkmate(), pdp);
    BOOST_CHECK(best_move == Move(Square(9,4),Square(9,3),PPAWN,PTYPE_EMPTY,true,BLACK) ||
		   best_move == Move(Square(7,1),Square(8,2),PSILVER,KNIGHT,true,BLACK));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  * -OU *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  * +FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00KI00KI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(3, best_move);
    BOOST_CHECK_EQUAL(ProofDisproof::Checkmate(), pdp);
    BOOST_CHECK_EQUAL(Move(Square(3,3),GOLD,BLACK), best_move);
  }  
}

BOOST_AUTO_TEST_CASE(CheckmateFixedDepthSearchTestProofPieces)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  * -OU *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  * +FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00KI00KI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    FixedDepthSolverExt searcher(state);
    Move best_move;
    PieceStand pieces;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(3, best_move, pieces);
    BOOST_CHECK_EQUAL(ProofDisproof::Checkmate(), pdp);
    BOOST_CHECK_EQUAL(Move(Square(3,3),GOLD,BLACK), best_move);
    BOOST_CHECK_EQUAL(PieceStand(0,0,0,0,2,0,0,0), pieces);
  }  
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  * -OU * \n"
			   "P2 *  *  *  *  *  * -FU-FU-FU\n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  * +RY *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8+KI+KI+KI+KI *  *  *  *  * \n"
			   "P9+HI * +OU *  *  *  *  *  * \n"
			   "P-00FU\n"
			   "P+00AL\n"
			   "+\n").initialState());
    FixedDepthSolverExt searcher(state);
    Move best_move;
    PieceStand pieces;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(2, best_move, pieces);
    BOOST_CHECK_EQUAL(ProofDisproof::Checkmate(), pdp);
    BOOST_CHECK_EQUAL(Move(Square(4,4),Square(4,1),PROOK,PTYPE_EMPTY,
			      false,BLACK), 
			 best_move);
    BOOST_CHECK_EQUAL(PieceStand(0,4,4,4,0,2,0,0), pieces);
  }  
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  * -OU-GI\n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  * -FU-KE-FU\n"
			   "P5 *  *  *  *  * +KE *  *  * \n"
			   "P6+KA *  *  *  *  *  *  *  * \n"
			   "P7+KA *  *  *  *  *  *  *  * \n"
			   "P8+GI+GI+GI *  *  *  *  *  * \n"
			   "P9+HI+OU *  *  *  *  *  *  * \n"
			   "P-00FU\n"
			   "P+00AL\n"
			   "+\n").initialState());
    FixedDepthSolverExt searcher(state);
    Move best_move;
    PieceStand pieces;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(3, best_move, pieces);
    BOOST_CHECK_EQUAL(ProofDisproof::Checkmate(), pdp);
    BOOST_CHECK_EQUAL(Move(Square(3,3),GOLD,BLACK), best_move);
    BOOST_CHECK_EQUAL(PieceStand(0,0,1,0,2,0,0,0), pieces);
  }  
}

BOOST_AUTO_TEST_CASE(CheckmateFixedDepthSearchTestSuicide)
{
  {
    // 逃げるつもりが逃げていない手は生成しない
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * +KI+KI-OU\n"
			   "P2 *  *  *  *  *  *  * -KI * \n"
			   "P3 *  *  *  *  *  *  *  * +FU\n"
			   "P4 *  *  *  *  * +KA *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "-\n").initialState());
    FixedDepthSearcher searcher(state);
    Move lastMove=Move(Square(2,1),GOLD,BLACK);
    ProofDisproof pdp=searcher.hasEscapeMove<BLACK>(lastMove,/*0*/1);
    BOOST_CHECK_EQUAL(ProofDisproof::NoEscape(),pdp);
  }
  {
    // 逃げるつもりが逃げていない手は生成しない
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  * +KI+HI-OU * \n"
			   "P2 *  *  *  *  *  * -FU-FU-FU\n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "-\n").initialState());
    FixedDepthSearcher searcher(state);
    Move lastMove=Move(Square(3,1),ROOK,BLACK);
    ProofDisproof pdp=searcher.hasEscapeMove<BLACK>(lastMove,0);
    BOOST_CHECK_EQUAL(ProofDisproof::NoEscape(),pdp);
  }
  {
    // openな自殺手
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  * -OU-KA\n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  * +KI *  * \n"
			   "P4 *  *  *  *  * +OU *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(0, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess() && !pdp.isCheckmateFail());
    pdp=searcher.hasCheckmateMove<BLACK>(1, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess() && !pdp.isCheckmateFail());
  }
  {
    // 王が動く自殺手
    NumEffectState state(CsaString(
			   "P1 *  *  * -KY * -KY * -OU * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  * +OU *  *  *  * \n"
			   "P5 *  *  * +KA *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(0, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess() && !pdp.isCheckmateFail());
    pdp=searcher.hasCheckmateMove<BLACK>(1, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess() && !pdp.isCheckmateFail());
  }
}

BOOST_AUTO_TEST_CASE(CheckmateFixedDepthSearchTestCheckInCheck)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  * -OU *  *  *  * \n"
			   "P2 *  *  *  *  * +GI *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6-HI-HI-KA-KA *  *  *  * +KY\n"
			   "P7-FU-GI * +GI *  *  *  * +KY\n"
			   "P8 * -FU *  *  *  *  *  * +KY\n"
			   "P9+OU * +FU+FU+FU+FU+FU * +KY\n"
			   "P+00KI00KI00GI00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
			   "P-00KI00KI00KE00KE00KE00KE\n"
			   "-\n").initialState());
    // 98金の一手詰ではあるけど
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<WHITE>(0, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess() && !pdp.isCheckmateFail());

    pdp=searcher.hasCheckmateMove<WHITE>(1, best_move);
    BOOST_CHECK(pdp.isCheckmateFail());
  }
}

static int checkmate_count;
static int checkmate_counts[20];
static void testCheckFile(std::string filename)
{
  filename = OslConfig::testPublicFile(filename);
  
  checkmate_count++;
  RecordMinimal record=CsaFileMinimal(filename).load();

  NumEffectState state(record.initial_state);
  FixedDepthSearcher searcher(state);
  Move best_move;
  ProofDisproof pdp;
  for(int i=0;i<=8;i++){
    if(state.turn()==BLACK)
      pdp=searcher.hasCheckmateMove<BLACK>(i, best_move);
    else
      pdp=searcher.hasCheckmateMove<WHITE>(i, best_move);
    BOOST_CHECK(!pdp.isCheckmateFail());
    if(pdp.isCheckmateSuccess()){
      checkmate_counts[i]++;
    }
  }
}
BOOST_AUTO_TEST_CASE(CheckmateFixedDepthSearchTestCheckFiles)
{
  std::ifstream ifs(OslConfig::testPublicFile("short-checkmate-problems/FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=500;
  if (OslConfig::inUnitTestShort())
    count=2;
  std::string filename;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong())
    progress.reset(new boost::progress_display(count, std::cerr));
  while((ifs >> filename) && filename != "" && (++i<count)) {
    if (progress)
      ++(*progress);
    testCheckFile("short-checkmate-problems/" + filename);
  }
  if(OslConfig::verbose()){
    std::cerr << std::endl << "checkmate_count=" << checkmate_count << std::endl;
    for(i=0;i<=8;i++){
      std::cerr  << "checkmate_counts[" << i << "]=" << checkmate_counts[i] << std::endl;
    }
  }
}

static int no_checkmate_count;
static int no_checkmate_counts[20];
static void testNoCheckFile(std::string filename)
{
  filename = OslConfig::testPublicFile(filename);

  no_checkmate_count++;
  RecordMinimal record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initial_state);
  FixedDepthSearcher searcher(state);
  Move best_move;
  ProofDisproof pdp;
  for(int i=0;i<=8;i++){
    if(state.turn()==BLACK)
      pdp=searcher.hasCheckmateMove<BLACK>(i, best_move);
    else
      pdp=searcher.hasCheckmateMove<WHITE>(i, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess());
    if(pdp.isCheckmateFail()){
      no_checkmate_counts[i]++;
    }
  }
}
BOOST_AUTO_TEST_CASE(CheckmateFixedDepthSearchTestNoCheckFiles)
{
  std::ifstream ifs(OslConfig::testPublicFile("nocheckmate-problems/FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=500;
  if (OslConfig::inUnitTestShort())
    count=2;
  std::string filename;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong())
    progress.reset(new boost::progress_display(count, std::cerr));
  while((ifs >> filename) && filename != "" && (++i<count)) {
    if (progress)
      ++(*progress);
    testNoCheckFile("nocheckmate-problems/" + filename);
  }
  if(OslConfig::verbose()){
    std::cerr << std::endl << "no_checkmate_count=" << no_checkmate_count << std::endl;
    for(i=0;i<=8;i++){
      std::cerr  << "no_checkmate_counts[" << i << "]=" << no_checkmate_counts[i] << std::endl;
    }
  }
}

BOOST_AUTO_TEST_CASE(CheckmateFixedDepthSearchTestNoCheckmate)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY * -HI-FU *  * -KA-KE-KY\n"
			   "P2 *  *  *  * +GI-OU-KI *  * \n"
			   "P3 *  *  * +TO * -KI-GI-FU * \n"
			   "P4-FU * -FU * -FU-FU-FU * -FU\n"
			   "P5 *  *  * +GI *  *  * +FU+FU\n"
			   "P6+FU * +FU * +FU+GI *  *  * \n"
			   "P7 * +FU * +KI * +FU *  *  * \n"
			   "P8 *  * +KI+KA *  *  * +HI * \n"
			   "P9+KY+KE+OU *  *  *  * +KE+KY\n"
			   "P+00KE00FU\n"
			   "P-00FU\n"
			   "+\n").initialState());
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(0, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess());

    // r3738までは+6886KAで詰みと誤認していた
    pdp=searcher.hasCheckmateMove<BLACK>(2, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  *  * -KE-KY\n"
			   "P2 *  *  *  *  *  * +NK+KI * \n"
			   "P3 *  * -KA-FU-GI-KI * -FU-OU\n"
			   "P4-FU * -FU * -HI * -FU * -FU\n"
			   "P5 * -FU * +FU-FU *  * +FU * \n"
			   "P6+FU * +FU * +GI+FU+FU * +FU\n"
			   "P7 * +FU+GI+KI *  *  *  *  * \n"
			   "P8 * +OU+KI+KA *  *  * +HI * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00GI00FU00FU\n"
			   "+\n").initialState());
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(0, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess());

    pdp=searcher.hasCheckmateMove<BLACK>(2, best_move); // +4645FUは-3435FUから-2324FUで不詰
    BOOST_CHECK(!pdp.isCheckmateSuccess());
  }
  {
    NumEffectState state(CsaString(
			   "P1 * -KE *  *  *  * -OU * -KY\n"
			   "P2+NG *  * -GI *  * -KI *  * \n"
			   "P3-KY * -KA-FU * -KI+NK * -FU\n"
			   "P4 * -FU-FU * -FU-FU-FU-FU * \n"
			   "P5-FU *  * +FU *  *  *  * +FU\n"
			   "P6 *  * +FU * +FU * +FU+FU * \n"
			   "P7+FU+FU+GI+KI * +FU *  *  * \n"
			   "P8 * +OU+KI *  * +GI * +HI * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00HI00KA00KE\n"
			   "+\n").initialState());
    FixedDepthSearcher searcher(state);
    Move best_move;
    ProofDisproof pdp=searcher.hasCheckmateMove<BLACK>(0, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess());

    // r3636までは+0061HIで詰みと誤認していた
    pdp=searcher.hasCheckmateMove<BLACK>(2, best_move);
    BOOST_CHECK(!pdp.isCheckmateSuccess());
  }
}

BOOST_AUTO_TEST_CASE(CheckmateFixedDepthSearchTestNoPromote)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  * +GI * \n"
			   "P2 *  *  *  *  *  *  * -KE-KY\n"
			   "P3 *  *  *  *  * +UM+KI-OU-FU\n"
			   "P4 *  *  *  *  *  *  *  * -KE\n"
			   "P5 *  *  *  *  *  *  *  * +GI\n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU+TO *  * -HI *  * \n"
			   "P+00FU\n"
			   "P-00AL\n"
			   "-\n").initialState());
    FixedDepthSearcher searcher(state);
    const Move m33ki(Square(4,4), Square(3,3), GOLD, PTYPE_EMPTY, false, BLACK);
    ProofDisproof pdp=searcher.hasEscapeMove<BLACK>(m33ki, 3);
    BOOST_CHECK(!pdp.isCheckmateSuccess());
    // 33同飛車不成なら不詰、成ると詰み
  }
}



// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
