#include "osl/search/bigramKillerMove.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
using namespace osl;
using namespace osl::search;

BOOST_AUTO_TEST_CASE(BigramKillerMoveTestAdust)
{
  BigramKillerMove bigrams;
  const Move m68b(Square(2,8),Square(6,8),ROOK,PTYPE_EMPTY,false,BLACK);
  const Move m26w(Square(2,3),Square(2,6),ROOK,GOLD,false,WHITE);
  // 先手の飛車が動いたら，後手が26の金を取る
  bigrams.setMove(m68b, m26w);
  BOOST_CHECK_EQUAL(m26w, bigrams[m68b][0]);

  MoveVector moves;
  NumEffectState state(CsaString(
			 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			 "P2 *  *  *  *  *  *  * -KA * \n"
			 "P3-FU-FU-FU-FU-FU-FU-FU-HI-FU\n"
			 "P4 *  *  *  *  *  *  *  * -TO\n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  *  *  *  *  *  * +KI * \n"
			 "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
			 "P8 * +KA * +HI *  *  *  *  * \n"
			 "P9+KY+KE+GI+KI+OU * +GI+KE+KY\n"
			 "P+00FU\n"
			 "-\n").initialState());
  bigrams.getMove(state, m68b, moves);
  BOOST_CHECK_EQUAL((size_t)1, moves.size());
  BOOST_CHECK_EQUAL(m26w, *moves.begin());

  moves.clear();
  
  // 後手の飛車に歩を叩いてもkiller move が対応
  NumEffectState state2(CsaString(
			 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			 "P2 *  *  *  *  *  *  * -KA * \n"
			 "P3-FU-FU-FU-FU-FU-FU-FU * -FU\n"
			 "P4 *  *  *  *  *  *  * -HI-TO\n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  *  *  *  *  *  * +KI * \n"
			 "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
			 "P8 * +KA * +HI *  *  *  *  * \n"
			 "P9+KY+KE+GI+KI+OU * +GI+KE+KY\n"
			 "P-00FU\n"
			 "-\n").initialState());
  bigrams.getMove(state2, m68b, moves);
  BOOST_CHECK_EQUAL((size_t)1, moves.size());
  const Move m26w_from24(Square(2,4),Square(2,6),ROOK,GOLD,false,WHITE);
  BOOST_CHECK_EQUAL(m26w_from24, *moves.begin());

  moves.clear();
  
  // 後手の飛車が辺なところにいれば無視
  NumEffectState state3(CsaString(
			 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			 "P2 *  *  *  *  *  *  * -KA * \n"
			 "P3-FU-FU-FU-FU-FU-FU-FU * -FU\n"
			 "P4 *  *  * -HI *  *  *  * -TO\n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  *  *  *  *  *  * +KI * \n"
			 "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
			 "P8 * +KA * +HI *  *  *  *  * \n"
			 "P9+KY+KE+GI+KI+OU * +GI+KE+KY\n"
			 "P-00FU\n"
			 "-\n").initialState());
  bigrams.getMove(state3, m68b, moves);
  BOOST_CHECK(moves.empty());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
