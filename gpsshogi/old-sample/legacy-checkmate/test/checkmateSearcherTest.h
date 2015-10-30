/* checkmateSearcherTest.h
 */
#ifndef _CHECKMATESEARCHERTEST_H
#define _CHECKMATESEARCHERTEST_H

#ifndef CHECKMATE_DEBUG
#  define CHECKMATE_DEBUG
#endif

#include "osl/checkmate/checkmateSearcher.h"
#include "osl/checkmate/checkmateSearcher.tcc"
#include "osl/checkmate/oracleProver.tcc"
#include "osl/checkmate/oracleDisprover.tcc"
#include "osl/checkmate/analyzer/checkTableAnalyzer.h"

#include "osl/record/csaRecord.h"
#include "osl/record/csaString.h"

#include "osl/state/hashEffectState.h"
#include "osl/oslConfig.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <fstream>
#include <iostream>

using namespace osl;
using namespace osl::checkmate;
extern bool isShortTest;

/**
 * 様々な CheckmateSearcher をテストするためのテンプレート
 */
template <class Searcher>
class CheckmateSearcherTest 
{
public:
  typedef HashEffectState state_t;
  typedef typename Searcher::table_t table_t;
  void testNoCheck();
  void testPawnCheckmate();
  void test30c();
  void testFilesCheck();
  void testFilesNoCheck();
  void testLimit();
private:
  static CArray<int,2> numSolved;
  template<bool isCheck,bool isDefense>
  void testFile(const std::string& filename);
};

static const int total_node_limit = CHECKMATE_DEFAULT_TOTAL_NODE_LIMIT;

template <class Searcher>
osl::CArray<int,2> CheckmateSearcherTest<Searcher>::numSolved = {{ 0 }};

template <class Searcher>
void CheckmateSearcherTest<Searcher>::testPawnCheckmate()
{
  {
    SimpleState sstate=CsaString(
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
      "-\n").getInitialState();
    state_t state(sstate);
    table_t table(BLACK);
    Searcher searcher(BLACK,table,total_node_limit);
    searcher.setVerbose(!isShortTest);
    const PathEncoding path(WHITE);
    const Move lastMove = Move(Square(1,2),PAWN,BLACK);;
    ProofDisproof proofDisproof
      =searcher.hasEscapeMove(state,state.getHash(),path,1500,lastMove);
    CPPUNIT_ASSERT_EQUAL(ProofDisproof::PawnCheckmate(),proofDisproof); // bug id 1
  }
  {
    SimpleState sstate=CsaString(
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
      "+\n").getInitialState();
    state_t state(sstate);
    table_t table(BLACK);
    const PathEncoding path(BLACK);
    Searcher searcher(BLACK,table,total_node_limit);
    searcher.setVerbose(!isShortTest);
    Move bestMove;
    ProofDisproof pdp
      =searcher.hasCheckmateMove(state,state.getHash(),path,1500,bestMove);
    const HashKey& key = state.getHash();
    const CheckHashRecord *record = table.find(key);
    CheckTableAnalyzer analyzer(table.getTwinTable());
    CPPUNIT_ASSERT(analyzer.proofOrDisproofTreeSize(record,key,path,false));
    CPPUNIT_ASSERT(pdp.isCheckmateFail());
    CPPUNIT_ASSERT_EQUAL(ProofDisproof::PawnCheckmate(),
			 record->bestResultInSolved);
  }
  {
    SimpleState sstate=CsaString(
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
      "+\n").getInitialState();
    state_t state(sstate);
    table_t table(BLACK);
    CheckTableAnalyzer analyzer(table.getTwinTable());
    const PathEncoding path(BLACK);
    Searcher searcher(BLACK,table,total_node_limit);
    searcher.setVerbose(!isShortTest);
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state,state.getHash(),path,3000,checkMove);
    if (! proofDisproof.isCheckmateSuccess())
    {
      std::cerr << proofDisproof << "\n";
      CheckTableAnalyzer analyzer(table.getTwinTable());
      const CheckHashRecord *record = table.find(state.getHash());
      std::ofstream os("CheckmateSearcherTest.log");
      analyzer.showTree(record, os, 100, true, true);
    }
    CPPUNIT_ASSERT(proofDisproof.isCheckmateSuccess());
    const Move bestMove=Move(Square(2,4),Square(2,3),PAWN,PAWN,false,BLACK);
    CPPUNIT_ASSERT_EQUAL(bestMove, checkMove);
  }
  {
    SimpleState sstate=CsaString(
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
      "-\n").getInitialState();
    state_t state(sstate);
    table_t table(BLACK);
    CheckTableAnalyzer analyzer(table.getTwinTable());
    const PathEncoding path(WHITE);
    Searcher searcher(BLACK,table,total_node_limit);
    searcher.setVerbose(!isShortTest);
    Move lastMove=Move(Square(2,4),Square(2,3),PPAWN,PAWN,true,BLACK);
    ProofDisproof pdp
      =searcher.hasEscapeMove(state,state.getHash(),path,1500,lastMove);
    CPPUNIT_ASSERT(pdp.isCheckmateFail());
    if (pdp != ProofDisproof::PawnCheckmate())
    {
      const HashKey& key = state.getHash();
      const CheckHashRecord *record = table.find(key);
      CPPUNIT_ASSERT(analyzer.proofOrDisproofTreeSize(record,key,path,true));
      const TwinEntry *loop = record->findLoop(path, table.getTwinTable());
      CPPUNIT_ASSERT(loop && loop->move.record);
      CPPUNIT_ASSERT_EQUAL(ProofDisproof::PawnCheckmate(),
			   loop->move.record->bestResultInSolved);
    }
  }
  {
    SimpleState sstate=CsaString(
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
      "+\n").getInitialState();
    state_t state(sstate);
    const PathEncoding path(BLACK);
    table_t table(BLACK);
    CheckTableAnalyzer analyzer(table.getTwinTable());
    Searcher searcher(BLACK,table,total_node_limit);
    searcher.setVerbose(!isShortTest);
    Move checkMove=Move::INVALID();
    ProofDisproof pdp
      =searcher.hasCheckmateMove(state,state.getHash(),path,1000,checkMove);
    const HashKey& key = state.getHash();
    const CheckHashRecord *record = table.find(key);
    CPPUNIT_ASSERT(analyzer.proofOrDisproofTreeSize(record,key,path,false));
    CPPUNIT_ASSERT_EQUAL(ProofDisproof::PawnCheckmate(),
			 record->bestResultInSolved);
    CPPUNIT_ASSERT(pdp.isCheckmateFail());
  }
}

template <class Searcher>
void CheckmateSearcherTest<Searcher>::testNoCheck()
{
  {
    SimpleState sstate=CsaString(
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
"-\n").getInitialState();
    state_t state(sstate);
    table_t table(WHITE);
    CheckTableAnalyzer analyzer(table.getTwinTable());
    const PathEncoding path(WHITE);
    Searcher searcher(WHITE,table,total_node_limit);
    searcher.setVerbose(!isShortTest);
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state,state.getHash(),path,1000,checkMove);
    CPPUNIT_ASSERT(proofDisproof.isUnknown());
    proofDisproof
      =searcher.hasCheckmateMove(state,state.getHash(),path,100000,checkMove);
    CPPUNIT_ASSERT(! proofDisproof.isCheckmateSuccess());
  }
  {
    SimpleState sstate=CsaString(
"P1-KY-KE * -KI *  *  * -KE-KY\n"
"P2 * -OU-GI * -KI * -HI *  * \n"
"P3 * -FU *  * -FU-GI-KA-FU-FU\n"
"P4-FU * -FU-FU * -FU-FU *  * \n"
"P5 *  *  *  *  *  *  * +FU * \n"
"P6+FU * +FU * +FU+GI+FU * +FU\n"
"P7 * +FU * +FU * +FU *  *  * \n"
"P8 * +KA+OU * +KI+GI * +HI * \n"
"P9+KY+KE * +KI *  *  * +KE+KY\n"
"+\n").getInitialState();
    state_t state(sstate);
    const PathEncoding path(BLACK);
    table_t table(BLACK);
    CheckTableAnalyzer analyzer(table.getTwinTable());
    Searcher searcher(BLACK,table,total_node_limit);
    searcher.setVerbose(!isShortTest);
    Move checkMove=Move::INVALID();
    const ProofDisproof result
      = searcher.hasCheckmateMove(state,state.getHash(),path,1000,checkMove);
    CPPUNIT_ASSERT(result.isCheckmateFail());
  }
  {
    state_t state(CsaString(
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
			   "+\n").getInitialState());
    const PathEncoding path(BLACK);
    table_t table(BLACK);
    CheckTableAnalyzer analyzer(table.getTwinTable());
    Searcher searcher(BLACK,table,total_node_limit);
    searcher.setVerbose(!isShortTest);
    Move checkMove=Move::INVALID();
    const ProofDisproof result
      = searcher.hasCheckmateMove(state,state.getHash(),path,8000,checkMove);
    if (! result.isCheckmateFail())
      std::cerr << "false checkmate " << typeid(*this).name() << " " << checkMove << "\n";
    CPPUNIT_ASSERT(result.isCheckmateFail());
  }
}

/**
 * 
 */
template <class Searcher>
void CheckmateSearcherTest<Searcher>::testLimit()
{
  SimpleState sstate0=CsaString(
"P1-KY-KE * -KI *  *  * -KE-KY\n"
"P2 * -OU-GI * -KI * -HI *  * \n"
"P3 * -FU *  * -FU-GI-KA-FU-FU\n"
"P4-FU * -FU-FU * -FU-FU *  * \n"
"P5 *  *  *  *  *  *  * +FU * \n"
"P6+FU * +FU * +FU+GI+FU * +FU\n"
"P7 * +FU * +FU * +FU *  *  * \n"
"P8 * +KA+OU * +KI+GI * +HI * \n"
"P9+KY+KE * +KI *  *  * +KE+KY\n"
"+\n").getInitialState();
  state_t state0(sstate0);


  SimpleState sstate1=CsaString(
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
"+\n").getInitialState();
  state_t state1(sstate1);
  const PathEncoding path(BLACK);

  table_t table(BLACK);
  CheckTableAnalyzer analyzer(table.getTwinTable());
  Searcher searcher(BLACK,table,10);
  searcher.setVerbose(!isShortTest);
  /* すぐに詰まないことがわかる局面を読む */
  {
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state0,state0.getHash(),path,1000,checkMove);
    CPPUNIT_ASSERT(proofDisproof.isCheckmateFail());
  }
  /* 詰みのチェックに時間がかかる局面を読む */
  {
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state1,state1.getHash(),path,1000,checkMove);
    CPPUNIT_ASSERT(proofDisproof.isUnknown());
  }
  /* すぐに詰まないことがわかる局面を読む */
  {
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state0,state0.getHash(),path,1000,checkMove);
    CPPUNIT_ASSERT(proofDisproof.isCheckmateFail());
  }
  /* 詰みのチェックに時間がかかる局面を読む */
  {
    Move checkMove=Move::INVALID();
    ProofDisproof proofDisproof
      =searcher.hasCheckmateMove(state1,state1.getHash(),path,1000,checkMove);
    CPPUNIT_ASSERT(proofDisproof.isUnknown());
  }
}


template <class Searcher>
template<bool isCheck,bool isDefense>
void CheckmateSearcherTest<Searcher>::testFile(const std::string& filename)
{
  static table_t tableBlack(BLACK);
  static table_t tableWhite(WHITE);

  Record rec=CsaFile(filename).getRecord();
  vector<osl::Move> moves=rec.getMoves();
  SimpleState sstate=rec.getInitialState();

  state_t state(sstate);
  ProofDisproof proofDisproof;
  Move checkMove=Move::INVALID();
  const Player attacker = ((state.turn()==BLACK) ^ isDefense) ? BLACK:WHITE;
  table_t& table = (attacker == BLACK) ? tableBlack : tableWhite;
  const PathEncoding path(state.turn());
  table.clear();
  table.clearTwins();
  Searcher searcher(attacker,table,total_node_limit);
  if (isDefense)
    proofDisproof
      =searcher.hasEscapeMove(state,state.getHash(),path,100000,checkMove);
  else
    proofDisproof
      =searcher.hasCheckmateMove(state,state.getHash(),path,100000,checkMove);

  if (! isShortTest)
  {
    if (proofDisproof.isFinal())
      std::cerr << " | ";
    else 
      std::cerr << "proof=" << proofDisproof.proof()
		<< ", disproof=" << proofDisproof.disproof() << " ";
  }
  if (proofDisproof.isFinal())
  {
    CheckTableAnalyzer analyzer(table.getTwinTable());
    ++numSolved[isCheck];
    if (isCheck)
    {
      CPPUNIT_ASSERT(proofDisproof.isCheckmateSuccess());
      if ((! moves.empty()) && (checkMove!=moves[0]))
      {
	if (! isShortTest)
	  std::cerr << "different solution in " << filename << "\n";
#if 0
	std::cerr << "checkMove=" << checkMove << ",correct answer=" << moves[0] << std::endl;
	std::cerr << sstate;
#endif
      }
      const HashKey& key = state.getHash();
      const CheckHashRecord *record = table.find(key);
      const size_t proofSize = analyzer.proofTreeSize(record, key, path, ! isDefense);
      if (!isShortTest)
	std::cerr << proofSize << "\n";
    }
    else
    {
      CPPUNIT_ASSERT(proofDisproof.isCheckmateFail());
      CheckTableAnalyzer analyzer(table.getTwinTable());
      const HashKey& key = state.getHash();
      const CheckHashRecord *record = table.find(key);
      const size_t proofSize = analyzer.proofOrDisproofTreeSize(record, key, path, isDefense);
      if (!isShortTest)
	std::cerr << proofSize << "\n";
    }
  }
}

template <class Searcher>
void CheckmateSearcherTest<Searcher>::test30c()
{
  if (! isShortTest)
    std::cerr << "testFilesCheck: 30c.csa" << std::endl;
  testFile<true,true>(OslConfig::testFile("ProblemProgress2/checkmate/30c.csa"));
}

template <class Searcher>
void CheckmateSearcherTest<Searcher>::testFilesCheck()
{
  std::ifstream ifs(OslConfig::testFile("kisenCheckProblems/FILES"));
  CPPUNIT_ASSERT(ifs);
  int i=0;
  int count=400;
  if (isShortTest)
    count=40;
  std::string filename;
  while((ifs >> filename) && (++i<count)) {
    if(filename == "") 
      break;
    if (! isShortTest)
      std::cerr << "testFilesCheck: " << filename << " ";
    testFile<true,false>(OslConfig::testFile("kisenCheckProblems/" + filename));
  }
  const double ratio = (double)numSolved[true]/i;
  if (! isShortTest)
    std::cerr << numSolved[true] << " / " << i << " = " 
	      << ratio << "\n";
  CPPUNIT_ASSERT(ratio >= 0.9);
}

template <class Searcher>
void CheckmateSearcherTest<Searcher>::testFilesNoCheck()
{
  // CheckmateRecorder::DepthTracer::maxVerboseLogDepth=5;
  std::ifstream ifs(OslConfig::testFile("kisenNoCheckProblems/FILES"));
  CPPUNIT_ASSERT(ifs);
  int i=0;
  int count=400;
  if (isShortTest)
      count=40;
  std::string filename;
  while((ifs >> filename) && (++i<count)) {
    if(filename == "") 
	break;
    if (! isShortTest)
      std::cerr << "testFilesNoCheck: " << filename << " ";
    testFile<false,false>(OslConfig::testFile("kisenNoCheckProblems/" + filename));
  }
  const double ratio = (double)numSolved[false]/i;
  std::cerr << numSolved[false] << " / " << i << " = " 
	    << ratio << "\n";
  CPPUNIT_ASSERT(ratio >= 0.9);
}



#endif /* _CHECKMATESEARCHERTEST_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
