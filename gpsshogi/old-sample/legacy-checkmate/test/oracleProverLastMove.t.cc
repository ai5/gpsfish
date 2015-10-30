/* oracleProverLastMove.t.cc
 */
#include "osl/checkmate/oracleProver.h"
#include "osl/checkmate/oraclePoolLastMove.h"
#include "osl/checkmate/dualCheckmateSearcher.h"
#include "osl/checkmate/analyzer/checkTableAnalyzer.h"
#include "osl/checkmate/checkmateRecorder.h"
#include "osl/record/csaString.h"
#include "osl/state/hashEffectState.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <fstream>
class OracleProverLastMoveTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(OracleProverLastMoveTest);
  CPPUNIT_TEST(testCheckmate);
  CPPUNIT_TEST_SUITE_END();
public:
  void testCheckmate();
};
CPPUNIT_TEST_SUITE_REGISTRATION(OracleProverLastMoveTest);

using namespace osl;
using namespace osl::checkmate;
extern bool isShortTest;

const size_t limit = 400000;
static bool isCheckmate(const char *orginal_str, Move last_move,
			const char *similar_str, Move last_move_similar)
{
  HashEffectState original(CsaString(orginal_str).getInitialState());

  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(original.turn());
  Move check_move;
  PathEncoding path(original.turn());
  AttackOracleAges oracle_age;
  const bool win = 
    checker.isWinningState(limit, original, original.getHash(),
			   path, check_move, oracle_age, last_move);
  CPPUNIT_ASSERT(win);
  const CheckHashRecord *record = table.find(original.getHash());
  CPPUNIT_ASSERT(record);
  CPPUNIT_ASSERT(record->proofDisproof().isCheckmateSuccess());

  HashEffectState similar(CsaString(similar_str).getInitialState());
  Move best_move;
  const bool result 
    = checker.isWinningStateByOracleLastMove(similar, similar.getHash(),
					     path, best_move, 
					     last_move_similar,
					     oracle_age.proof_last_move);
  if (! result)
  {
    CheckTableAnalyzer analyzer(table.getTwinTable());
    std::ofstream os("oracleProverLastMove.t.log");
    analyzer.showProofTree(record,similar.getHash(),path,true,os);
  }

  return result;
}

void OracleProverLastMoveTest::testCheckmate()
{
  CPPUNIT_ASSERT(isCheckmate(
		   "P1 *  *  *  *  *  *  * -KE-OU\n"
		   "P2 *  *  *  *  *  *  * -KA-KY\n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  * -KA  *  *  *  * \n"
		   "P7 *  * +GI *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 *  * +OU *  *  *  *  *  * \n"
		   "P-00KI\n"
		   "P+00AL\n"
		   "-\n",
		   Move(Square(7,8),Square(7,7),SILVER,GOLD,
			   false,BLACK),
		   "P1 *  *  *  *  *  *  * -KE-OU\n"
		   "P2 *  *  *  *  *  *  * -KA-KY\n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  * -KA  *  *  *  * \n"
		   "P7 *  * +GI *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 *  * +OU *  *  *  *  *  * \n"
		   "P-00KI\n"
		   "P+00AL\n"
		   "-\n",
		   Move(Square(7,8),Square(7,7),SILVER,ROOK,
			   false,BLACK)
		   ));		// 違うものを取っても詰
  CPPUNIT_ASSERT(! isCheckmate(
		   "P1 *  *  *  *  *  *  * -KE-OU\n"
		   "P2 *  *  *  *  *  *  * -KA-KY\n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  * -KA  *  *  *  * \n"
		   "P7 *  * +GI *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 *  * +OU *  *  *  *  *  * \n"
		   "P-00KI\n"
		   "P+00AL\n"
		   "-\n",
		   Move(Square(7,8),Square(7,7),SILVER,GOLD,
			   false,BLACK),
		   "P1 *  *  *  *  *  *  * -KE-OU\n"
		   "P2 *  *  *  *  *  *  * -KA-KY\n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  * -KA  *  *  *  * \n"
		   "P7 *  * +KI *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 *  * +OU *  *  *  *  *  * \n"
		   "P-00KI\n"
		   "P+00AL\n"
		   "-\n",
		   Move(Square(7,8),Square(7,7),GOLD,ROOK,
			   false,BLACK)
		   ));		// 金は後ろに効いている
  CPPUNIT_ASSERT(! isCheckmate(
		   "P1 *  *  *  *  *  *  * -KE-OU\n"
		   "P2 *  *  *  *  *  *  * -KA-KY\n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  * -KA  *  *  *  * \n"
		   "P7 *  * +GI *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 *  * +OU *  *  *  *  *  * \n"
		   "P-00KI\n"
		   "P+00AL\n"
		   "-\n",
		   Move(Square(7,8),Square(7,7),SILVER,GOLD,
			   false,BLACK),
		   "P1 *  *  *  *  *  *  * -KE-OU\n"
		   "P2 *  *  *  *  *  *  * -KA-KY\n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  * -KA  *  *  *  * \n"
		   "P7 * +KI *  *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 *  * +OU *  *  *  *  *  * \n"
		   "P-00KI\n"
		   "P+00AL\n"
		   "-\n",
		   Move(Square(7,8),Square(8,7),GOLD,KNIGHT,
			   false,BLACK)
		   ));		// 違う駒だとだめ
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
