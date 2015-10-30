/* pairDifferenceTest.cc
 */
#include "recursiveDoUndoMove.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/state/hashEffectState.h"
#include "osl/ppair/pairDifference.h"
#include "osl/ppair/indexList.h"
#include "osl/record/record.h"
#include "osl/record/csaString.h"
#include "osl/record/csaRecord.h"
#include "osl/record/csa.h"
#include "osl/oslConfig.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <fstream>

class pairDifferenceTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(pairDifferenceTest);
  // CPPUNIT_TEST(testLoad);
  CPPUNIT_TEST(testConsistent);
  CPPUNIT_TEST_SUITE_END();
public:
  void testLoad();
  void testConsistent();
};

CPPUNIT_TEST_SUITE_REGISTRATION(pairDifferenceTest);

using namespace osl;
using namespace osl::ppair;
using namespace osl::eval::ppair;

void pairDifferenceTest::testLoad()
{
  const std::string filename = OslConfig::home() + "/data/sibling.pair";
  CPPUNIT_ASSERT(PiecePairRawEval::setUp(filename.c_str()));
}

void pairDifferenceTest::testConsistent()
{
  const std::string filename = OslConfig::home() + "/data/sibling.pair";
  CPPUNIT_ASSERT(PiecePairRawEval::setUp(filename.c_str()));

  extern bool isShortTest;
  std::ifstream ifs("./kifdat/FILES");
  std::string fileName;
  for(int i=0;i<(isShortTest ? 200 : 50) && (ifs >> fileName) ; i++){
    if(fileName == "") 
      break;
    fileName = "./kifdat/" + fileName;

    Record rec=CsaFile(fileName).getRecord();
    HashEffectState eState(rec.getInitialState());
    osl::vector<osl::Move> moves=rec.getMoves();
    PiecePairRawEval eval(eState);
    for(unsigned int i=0;i<moves.size();i++)
    {
      const Move move=moves[i];
      IndexList added, removed;
      PairDifference::diffWithMove(eState, move, added, removed);
      const int nextValue = PairDifference::applyDifference
	(eval.value(), added, removed); // raw value?
      ApplyMoveOfTurn::doMove(eState, move);
      eval.update(eState, move);
      CPPUNIT_ASSERT(eState.isConsistent(true));
      CPPUNIT_ASSERT_EQUAL(eval.value(), nextValue); // raw value?
      const size_t diffElements = added.size() + removed.size();
      if (move.capturePtype() == PTYPE_EMPTY)
	CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(80u), diffElements);
      else
	CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(158u), diffElements);
    }
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
