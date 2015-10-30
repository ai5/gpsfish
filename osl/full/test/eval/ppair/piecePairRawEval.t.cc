// piecePairRawEval.t.cc
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/state/historyState.h"
#include "../consistencyTest.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
using namespace osl;
using namespace osl::eval;
using namespace osl::eval::ppair;

BOOST_AUTO_TEST_CASE(PiecePairRawEvalTestLoad)
{
  BOOST_CHECK(PiecePairRawEval::setUp());
}

BOOST_AUTO_TEST_CASE(PiecePairRawEvalTestEval)
{
  HistoryState state;
  const int start_value = PiecePairRawEval(state).value();
  int value1, value12, value2;
  const Move m1 = Move(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK);
  {
    DoUndoMoveLock lock1(state, m1);
    BOOST_CHECK(state.isConsistent());

    value1 = PiecePairRawEval(state).value();
    const Move m12 = Move(Square(3,3),Square(3,4),PAWN,PTYPE_EMPTY,false,WHITE);
    {
      DoUndoMoveLock lock12(state, m12);
      BOOST_CHECK(state.isConsistent());
      value12 = PiecePairRawEval(state).value();
      if (OslConfig::verbose())
	std::cerr << "76FU34FU " << start_value 
		  << " ==> " << value1 
		  << " ==> " << value12 
		  << "\n";
    } //   state.undoMove(m12);
    BOOST_CHECK(state.isConsistent());
  } // state.undoMove(m1);
  BOOST_CHECK(state.isConsistent());

  const Move m2 = Move(Square(2,7),Square(2,6),PAWN,PTYPE_EMPTY,false,BLACK);
  {
    DoUndoMoveLock lock2(state, m2);
    BOOST_CHECK(state.isConsistent());
    value2 = PiecePairRawEval(state).value();
    if (OslConfig::verbose())
      std::cerr << "26FU " << start_value << " ==> " << value2 << "\n";
  } //  state.undoMove(m2);
  BOOST_CHECK(state.isConsistent());

  const Move m3 = Move(Square(8,7),Square(8,6),PAWN,PTYPE_EMPTY,false,BLACK);
  {
    DoUndoMoveLock lock2(state, m3);
    BOOST_CHECK(state.isConsistent());
    const int value3 = PiecePairRawEval(state).value();
    if (OslConfig::verbose())
      std::cerr << "86FU " << start_value << " ==> " << value3 << "\n";

    BOOST_CHECK(start_value < value1);
    BOOST_CHECK(value1 > value12);
    BOOST_CHECK(start_value <= value2);
    BOOST_CHECK(value3 < value1);
    BOOST_CHECK(value3 < value2);
  }
#if 0
  NumEffectState state=CsaString(
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
  PiecePairEval eval(state);
  std::cerr << "value is " << eval.value() << "\n";

  state=CsaString(
    "P1-KY-KE-GI-KI *  * -HI-KE-KY\n"
    "P2 * -HI *  * -KA-OU *  *  * \n"
    "P3-FU * -FU-FU-FU * -GI-FU-FU\n"
    "P4 *  *  *  *  * -FU *  *  * \n"
    "P5 * -FU * +KA *  *  *  *  * \n"
    "P6 *  * +FU *  *  * +FU * +FU\n"
    "P7+FU+FU * +FU+FU+FU+KE *  * \n"
    "P8 * +GI+KI *  *  * +GI *  * \n"
    "P9+KY+KE *  * +OU+KI *  * +KY\n"
    "P+00FU\n"
    "P+00FU\n"
    "P+00KI\n"
    "+\n").initialState();
  PiecePairEval eval2(state);
  std::cerr << "value is " << eval2.value() << "\n";
#endif
}

BOOST_AUTO_TEST_CASE(PiecePairRawEvalTestConsistentUpdate)
{
  consistencyTestUpdate<PiecePairRawEval>();
}

BOOST_AUTO_TEST_CASE(PiecePairRawEvalTestConsistentExpect)
{
  consistencyTestExpect<PiecePairRawEval>();
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
