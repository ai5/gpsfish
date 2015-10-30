/* proofNumberTable.t.cc
 */
#include "osl/checkmate/proofNumberTable.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::checkmate;

BOOST_AUTO_TEST_CASE(ProofNumberTableTestCountLiberty)
{
  const ProofNumberTable& table = Proof_Number_Table;
  {
    unsigned int liberty = 0xff;
    BOOST_CHECK_EQUAL((uint8_t)7, table.countLiberty(PAWN, U, liberty).liberty);
    BOOST_CHECK_EQUAL((uint8_t)6, table.countLiberty(LANCE, U, liberty).liberty);
    BOOST_CHECK_EQUAL((uint8_t)5, table.countLiberty(SILVER, U, liberty).liberty);
    BOOST_CHECK_EQUAL((uint8_t)6, table.countLiberty(SILVER, UL, liberty).liberty);
    BOOST_CHECK_EQUAL((uint8_t)3, table.countLiberty(GOLD, U, liberty).liberty);
    BOOST_CHECK_EQUAL((uint8_t)2, table.countLiberty(PROOK, U, liberty).liberty);
    BOOST_CHECK_EQUAL((uint8_t)5, table.countLiberty(GOLD, D, liberty).liberty);
    // 王手ではない	
    BOOST_CHECK_EQUAL((uint8_t)7, table.countLiberty(PAWN, D, liberty).liberty);
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * -OU *  * \n"
			   "P2 *  *  *  *  *  *  * -FU * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 * +FU *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    const Move m42ki(Square(4,2),GOLD,BLACK);
    const Move m42gi(Square(4,2),SILVER,BLACK);

    BOOST_CHECK_EQUAL(3, table.countLiberty(state, m42gi));
    BOOST_CHECK_EQUAL(2, table.countLiberty(state, m42ki));

    const Move m68ki(Square(6,8),GOLD,  WHITE);
    const Move m68gi(Square(6,8),SILVER,WHITE);

    BOOST_CHECK_EQUAL(3, table.countLiberty(state, m68gi));
    BOOST_CHECK_EQUAL(2, table.countLiberty(state, m68ki));

    const Move m71hi(Square(7,1),ROOK,BLACK);
    const Move m53ka(Square(5,3),BISHOP,BLACK);

    BOOST_CHECK_EQUAL(3, table.countLiberty(state, m71hi));
    BOOST_CHECK_EQUAL(4, table.countLiberty(state, m53ka));

    const Move m39hi(Square(3,9),ROOK,  WHITE);
    const Move m57ka(Square(5,7),BISHOP,WHITE);

    BOOST_CHECK_EQUAL(3, table.countLiberty(state, m39hi));
    BOOST_CHECK_EQUAL(4, table.countLiberty(state, m57ka));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * -OU *  * \n"
			   "P2 *  *  *  *  *  *  * -FU * \n"
			   "P3 *  *  *  *  * +GI *  *  * \n"
			   "P4 *  *  * -FU+HI *  *  *  * \n"
			   "P5 *  * -KA+FU *  * +KA *  * \n"
			   "P6 *  *  *  * -HI+FU *  *  * \n"
			   "P7 *  *  * -GI *  *  *  *  * \n"
			   "P8 * +FU *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    const Move m42ng(Square(4,3),Square(4,2),PSILVER,PTYPE_EMPTY,true,BLACK);
    const Move m42gi(Square(4,3),Square(4,2),SILVER,PTYPE_EMPTY,false,BLACK);

    BOOST_CHECK_EQUAL(2, table.countLiberty(state, m42gi)); // 本当は3
    BOOST_CHECK_EQUAL(2, table.countLiberty(state, m42ng));

    const Move m68ng(Square(6,7),Square(6,8),PSILVER,PTYPE_EMPTY,true,WHITE);
    const Move m68gi(Square(6,7),Square(6,8),SILVER,PTYPE_EMPTY,false,WHITE);

    BOOST_CHECK_EQUAL(2, table.countLiberty(state, m68gi));
    BOOST_CHECK_EQUAL(2, table.countLiberty(state, m68ng));

    const Move m51ry(Square(5,4),Square(5,1),PROOK,PTYPE_EMPTY,true,BLACK);
    const Move m53um(Square(3,5),Square(5,3),PBISHOP,PTYPE_EMPTY,true,BLACK);

    BOOST_CHECK_EQUAL(2, table.countLiberty(state, m51ry));
    BOOST_CHECK_EQUAL(3, table.countLiberty(state, m53um));

    const Move m59ry(Square(5,6),Square(5,9),PROOK,PTYPE_EMPTY,true,WHITE);
    const Move m57um(Square(7,5),Square(5,7),PBISHOP,PTYPE_EMPTY,false,WHITE);

    BOOST_CHECK_EQUAL(2, table.countLiberty(state, m59ry));
    BOOST_CHECK_EQUAL(3, table.countLiberty(state, m57um));
  }
  {
    // 空き王手
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * -OU *  * +GI+HI\n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    const Move m31ng(Square(2,2),Square(3,1),PSILVER,PTYPE_EMPTY,true,BLACK);
    const Move m13gi(Square(2,2),Square(2,3),SILVER,PTYPE_EMPTY,false,BLACK);

    BOOST_CHECK_EQUAL(7, table.countLiberty(state, m31ng));
    BOOST_CHECK_EQUAL(7, table.countLiberty(state, m13gi));
  }
  {
    // 空き王手 2マス離れ
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * -OU * +KA * +HI\n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    const Move m54um(Square(3,2),Square(5,4),PBISHOP,PTYPE_EMPTY,true,BLACK);
    const Move m65um(Square(3,2),Square(6,5),PBISHOP,PTYPE_EMPTY,true,BLACK);

    BOOST_CHECK_EQUAL(4, table.countLiberty(state, m54um));
    BOOST_CHECK_EQUAL(5, table.countLiberty(state, m65um)); // XXX:今のliberty-1
  }
  {
    // 空き王手 2マス離れ 無関係
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * -OU *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  * +KY *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  * +KA\n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    const Move m32ky(Square(3,4),Square(3,2),LANCE,PTYPE_EMPTY,false,BLACK);
    const Move m32ny(Square(3,4),Square(3,2),PLANCE,PTYPE_EMPTY,true,BLACK);

    BOOST_CHECK_EQUAL(8, table.countLiberty(state, m32ky));
    BOOST_CHECK_EQUAL(6, table.countLiberty(state, m32ny));
  }
  {
    // 追加利き
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * -OU *  *  *  * \n"
			   "P3 *  *  *  * -FU *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  * +HI *  *  *  * \n"
			   "P6 *  *  *  * +KY *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    const Move m53ry(Square(5,5),Square(5,3),PROOK,PAWN,true,BLACK);

    BOOST_CHECK_EQUAL(2, table.countLiberty(state, m53ry));
  }
}

BOOST_AUTO_TEST_CASE(ProofNumberTableTestLibertyAfterAllDrop)
{
  const ProofNumberTable& table = Proof_Number_Table;
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * -OU *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  * +FU *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(6, table.libertyAfterAllDrop(state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * -OU *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  * +FU *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00KI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(3, table.libertyAfterAllDrop(state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * -OU *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  * +FU *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00KI00GI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(3, table.libertyAfterAllDrop(state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * -OU *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  * +FU *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00GI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(5, table.libertyAfterAllDrop(state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * -OU *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  * +FU *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P+00HI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(4, table.libertyAfterAllDrop(state));
  }
}

BOOST_AUTO_TEST_CASE(ProofNumberTableTestLibertyAfterAllMove)
{
  const ProofNumberTable& table = Proof_Number_Table;
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * -OU *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  * +FU *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(6, table.libertyAfterAllMove(state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * -OU *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  * +FU *  *  *  * \n"
			   "P5 *  *  * +KE *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(3, table.libertyAfterAllMove(state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  * -OU *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  * +FU *  *  *  * \n"
			   "P6 *  *  * +KE *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    BOOST_CHECK(3 < table.libertyAfterAllMove(state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  * -OU *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  * +KI+FU *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(3, table.libertyAfterAllMove(state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  * -OU-FU *  * +HI *  * \n"
			   "P3 * -FU * +FU *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(1, table.libertyAfterAllMove(state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  * -OU-FU *  * +HI *  * \n"
			   "P6 * -FU * +FU *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  * +OU *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(3, table.libertyAfterAllMove(state));
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

