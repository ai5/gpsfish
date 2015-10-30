// historyToTable.t.cc
#include "osl/game_playing/historyToTable.h"
#include "osl/game_playing/gameState.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/csa.h"
#include "osl/hashKey.h"
#include "osl/sennichite.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::game_playing;

const int BLACK_WIN = 1000;
const int DRAW = 0;
const int WHITE_WIN = -1000;
const int limit = search::SearchTable::HistorySpecialDepth;

BOOST_AUTO_TEST_CASE(HistoryToTableTestBestMoveInWin)
{
  const char *horizon_drop_string = 
    "P1-KY *  * -KI *  *  * -KE-KY\n"
    "P2 * -OU * -RY-KI *  * -KA * \n"
    "P3 * +TO * -FU * -FU * -FU * \n"
    "P4+HI * -GI-GI-FU *  *  * -FU\n"
    "P5+FU *  *  *  *  *  *  *  * \n"
    "P6 *  *  * +FU *  *  *  *  * \n"
    "P7 *  * -KA+GI+FU+FU+FU+FU+FU\n"
    "P8 *  *  *  * +KI+GI+KI+OU * \n"
    "P9+KY *  *  *  *  *  * +KE+KY\n"
    "P+00FU00FU\n"
    "P-00AL\n"
    "-\n"
    "-7483GI\n"
    "+0084FU\n"
    "-8374GI\n"
    "+8483TO\n";
  CsaString horizon_drop(horizon_drop_string);
  GameState state(horizon_drop.initialState());
  const auto& moves = horizon_drop.load().moves;
  for (size_t i=0; i<moves.size(); ++i)
  {
    BOOST_CHECK(state.state().isValidMove(moves[i]));
    BOOST_CHECK(state.pushMove(moves[i]).isNormal());
  }

  SimpleHashTable table;
  HistoryToTable::adjustTable(state, table, BLACK_WIN, DRAW, WHITE_WIN);
  
  const SimpleHashRecord *record = table.find(HashKey(state.state()));
  BOOST_CHECK(record);
  BOOST_CHECK(record->hasLowerBound(limit));
  BOOST_CHECK(record->hasUpperBound(limit));
  BOOST_CHECK_EQUAL(WHITE_WIN, record->lowerBound());
  BOOST_CHECK_EQUAL(WHITE_WIN, record->upperBound());
  BOOST_CHECK(record->bestMove().move().isValid());
  BOOST_CHECK_EQUAL(Move(Square(7,4),Square(8,3),
			    SILVER,PPAWN,false,WHITE),
		       record->bestMove().move());
}

BOOST_AUTO_TEST_CASE(HistoryToTableTestBestMoveInLose)
{
  const char *horizon_drop_string = 
    "P1-KY *  * -KI *  *  * -KE-KY\n"
    "P2 *  *  * -RY-KI *  * -KA * \n"
    "P3 * -OU * -FU * -FU * -FU * \n"
    "P4+HI * -GI-GI-FU *  *  * -FU\n"
    "P5+FU *  *  *  *  *  *  *  * \n"
    "P6 *  *  * +FU *  *  *  *  * \n"
    "P7 *  * -KA+GI+FU+FU+FU+FU+FU\n"
    "P8 *  *  *  * +KI+GI+KI+OU * \n"
    "P9+KY *  *  *  *  *  * +KE+KY\n"
    "P+00FU00FU\n"
    "P-00AL\n"
    "+\n"
    "+0084FU\n"
    "-8382OU\n"
    "+8483TO\n"
    "-8283OU\n";
  CsaString horizon_drop(horizon_drop_string);
  GameState state(horizon_drop.initialState());
  const auto& moves = horizon_drop.load().moves;
  for (size_t i=0; i<moves.size(); ++i)
  {
    BOOST_CHECK(state.state().isValidMove(moves[i]));
    BOOST_CHECK(state.pushMove(moves[i]).isNormal());
  }

  SimpleHashTable table;
  HistoryToTable::adjustTable(state, table, BLACK_WIN, DRAW, WHITE_WIN);
  
  const SimpleHashRecord *record = table.find(HashKey(state.state()));
  BOOST_CHECK(record);
  BOOST_CHECK(record->hasLowerBound(limit));
  BOOST_CHECK(record->hasUpperBound(limit));
  BOOST_CHECK_EQUAL(WHITE_WIN, record->lowerBound());
  BOOST_CHECK_EQUAL(WHITE_WIN, record->upperBound());
  BOOST_CHECK(record->bestMove().move().isInvalid());
}

BOOST_AUTO_TEST_CASE(HistoryToTableTestCurrentState)
{
  GameState state((SimpleState(HIRATE)));
  SimpleHashTable table;
  HistoryToTable::adjustTable(state, table, BLACK_WIN, DRAW, WHITE_WIN);

  const SimpleHashRecord *record = table.find(HashKey(state.state()));
  BOOST_CHECK_EQUAL((const SimpleHashRecord*)0, record);
}

BOOST_AUTO_TEST_CASE(HistoryToTableTestDominance)
{
  GameState state(CsaString(
		    "P1-KY *  *  *  *  *  * +NY * \n"
		    "P2 * -OU-KI-KI *  *  *  * +RY\n"
		    "P3 * -GI-KE+KI *  *  *  * +HI\n"
		    "P4 *  * -FU-KY-FU *  * -FU * \n"
		    "P5-FU-FU * -KE * -FU *  *  * \n"
		    "P6 *  * +FU-FU+FU * -FU *  * \n"
		    "P7+FU+FU *  *  *  *  *  *  * \n"
		    "P8+KY+GI+GI-UM *  *  *  *  * \n"
		    "P9+OU+KE *  *  *  *  * +KE * \n"
		    "P-00FU00FU00FU00FU00FU00FU00GI00KA\n"
		    "P+00KI\n"
		    "+\n").initialState());
  SimpleHashTable table;
  HistoryToTable::adjustTable(state, table, BLACK_WIN, DRAW, WHITE_WIN);
  
  {
    // black have more pawn
    NumEffectState dominance_state(CsaString(
				      "P1-KY *  *  *  *  *  * +NY * \n"
				      "P2 * -OU-KI-KI *  *  *  * +RY\n"
				      "P3 * -GI-KE+KI *  *  *  * +HI\n"
				      "P4 *  * -FU-KY-FU *  * -FU * \n"
				      "P5-FU-FU * -KE * -FU *  *  * \n"
				      "P6 *  * +FU-FU+FU * -FU *  * \n"
				      "P7+FU+FU *  *  *  *  *  *  * \n"
				      "P8+KY+GI+GI-UM *  *  *  *  * \n"
				      "P9+OU+KE *  *  *  *  * +KE * \n"
				      "P-00FU00FU00FU00FU00FU00GI00KA\n"
				      "P+00FU00KI\n"
				      "+\n").initialState());
    const SimpleHashRecord *record = table.find(HashKey(dominance_state));
    BOOST_CHECK(record);
    BOOST_CHECK(record->hasLowerBound(limit));
    BOOST_CHECK(record->hasUpperBound(limit));
    BOOST_CHECK_EQUAL(BLACK_WIN, record->lowerBound());
    BOOST_CHECK_EQUAL(BLACK_WIN, record->upperBound());
  }

  {
    // white have more gold
    NumEffectState dominance_state(CsaString(
				      "P1-KY *  *  *  *  *  * +NY * \n"
				      "P2 * -OU-KI-KI *  *  *  * +RY\n"
				      "P3 * -GI-KE+KI *  *  *  * +HI\n"
				      "P4 *  * -FU-KY-FU *  * -FU * \n"
				      "P5-FU-FU * -KE * -FU *  *  * \n"
				      "P6 *  * +FU-FU+FU * -FU *  * \n"
				      "P7+FU+FU *  *  *  *  *  *  * \n"
				      "P8+KY+GI+GI-UM *  *  *  *  * \n"
				      "P9+OU+KE *  *  *  *  * +KE * \n"
				      "P-00FU00FU00FU00FU00FU00FU00KI00GI00KA\n"
				      "+\n").initialState());
    const SimpleHashRecord *record = table.find(HashKey(dominance_state));
    BOOST_CHECK(record);
    BOOST_CHECK(record->hasLowerBound(limit));
    BOOST_CHECK(record->hasUpperBound(limit));
    BOOST_CHECK_EQUAL(WHITE_WIN, record->lowerBound());
    BOOST_CHECK_EQUAL(WHITE_WIN, record->upperBound());
  }

}

BOOST_AUTO_TEST_CASE(HistoryToTableTestLose)
{
  const char *oute_string = 
    "P1-KY *  *  *  *  *  *  *  * \n"
    "P2 * -KY+TO *  * +NK * +HI * \n"
    "P3 *  *  * -FU-KI * -KE * -FU\n"
    "P4-FU * +NK *  * -OU *  *  * \n"
    "P5 *  *  *  *  *  * -GI *  * \n"
    "P6 *  *  *  *  *  *  * -FU * \n"
    "P7 * +FU * +FU+FU+OU+FU * +FU\n"
    "P8 * +GI+KI+GI *  *  *  * +GI\n"
    "P9 *  *  *  *  *  *  * +KE+KY\n"
    "P+00HI00KA00KA00KI00KI00KY00FU00FU00FU00FU00FU00FU00FU00FU\n"
    "-\n"
    "-3546GI\n"
    "+4736OU\n"
    "-4635GI\n"
    "+3647OU\n";

  CsaString oute_sennnichite(oute_string);
  GameState state(oute_sennnichite.initialState());
  const auto& moves = oute_sennnichite.load().moves;
  for (size_t i=0; i<moves.size(); ++i)
  {
    BOOST_CHECK(state.state().isValidMove(moves[i]));
    BOOST_CHECK(state.pushMove(moves[i]).isNormal());
  }

  SimpleHashTable table;
  HistoryToTable::adjustTable(state, table, BLACK_WIN, DRAW, WHITE_WIN);

  HashKey key = HashKey(state.state());
  key = key.newHashWithMove(Move(Square(3,5),Square(4,6), SILVER, PTYPE_EMPTY, false, WHITE));
  const SimpleHashRecord *record = table.find(key);
  BOOST_CHECK(record);
  BOOST_CHECK(record->hasLowerBound(limit));
  BOOST_CHECK(record->hasUpperBound(limit));
  BOOST_CHECK_EQUAL(BLACK_WIN, record->lowerBound());
  BOOST_CHECK_EQUAL(BLACK_WIN, record->upperBound());
  BOOST_CHECK(record->bestMove().move().isInvalid());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
