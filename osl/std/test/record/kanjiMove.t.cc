#include "osl/record/kanjiMove.h"
#include "osl/record/kanjiCode.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::record;

BOOST_AUTO_TEST_CASE(KanjiMoveTestMove)
{
  const static std::string str_b26fu     = K_BLACK_SIGN K_R2 K_K6 K_PAWN;
  const static std::string str_w84fu     = K_WHITE_SIGN K_R8 K_K4 K_PAWN;
  const static std::string str_b25fu     = K_BLACK_SIGN K_R2 K_K5 K_PAWN;
  const static std::string str_w85fu     = K_WHITE_SIGN K_R8 K_K5 K_PAWN;
  const static std::string str_b24fu     = K_BLACK_SIGN K_R2 K_K4 K_PAWN;
  const static std::string str_wdofu     = K_WHITE_SIGN K_ONAZI K_SPACE K_PAWN;
  const static std::string str_b58kimigi = K_BLACK_SIGN K_R5 K_K8 K_GOLD K_MIGI;
  
  KanjiMove kmove;
  NumEffectState state((SimpleState(HIRATE)));
  Move last_move;

  
  Move m26fu(Square(2,7), Square(2,6), PAWN, PTYPE_EMPTY, false, BLACK);
  BOOST_CHECK_EQUAL(m26fu, kmove.strToMove(str_b26fu, state, last_move));
  last_move = m26fu;
  state.makeMove(last_move);

  Move m84fu(Square(8,3), Square(8,4), PAWN, PTYPE_EMPTY, false, WHITE);
  BOOST_CHECK_EQUAL(m84fu, kmove.strToMove(str_w84fu, state, last_move));
  last_move = m84fu;
  state.makeMove(last_move);

  Move m25fu(Square(2,6), Square(2,5), PAWN, PTYPE_EMPTY, false, BLACK);
  BOOST_CHECK_EQUAL(m25fu, kmove.strToMove(str_b25fu, state, last_move));
  last_move = m25fu;
  state.makeMove(last_move);

  Move m85fu(Square(8,4), Square(8,5), PAWN, PTYPE_EMPTY, false, WHITE);
  BOOST_CHECK_EQUAL(m85fu, kmove.strToMove(str_w85fu, state, last_move));
  last_move = m85fu;
  state.makeMove(last_move);

  Move m24fu(Square(2,5), Square(2,4), PAWN, PTYPE_EMPTY, false, BLACK);
  BOOST_CHECK_EQUAL(m24fu, kmove.strToMove(str_b24fu, state, last_move));
  last_move = m24fu;
  state.makeMove(last_move);

  Move mdofu(Square(2,3), Square(2,4), PAWN, PAWN, false, WHITE);
  BOOST_CHECK_EQUAL(mdofu, kmove.strToMove(str_wdofu, state, last_move));
  last_move = mdofu;
  state.makeMove(last_move);

  Move m58ki(Square(4,9), Square(5,8), GOLD, PTYPE_EMPTY, false, BLACK);
  BOOST_CHECK_EQUAL(m58ki, kmove.strToMove(str_b58kimigi, state, last_move));
  last_move = m58ki;
  state.makeMove(last_move);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
