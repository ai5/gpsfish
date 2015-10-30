#include "osl/record/ki2.h"
#include "osl/record/kanjiCode.h"

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <iterator>

using namespace osl;
using namespace osl::record;

// reference: http://www.shogi.or.jp/faq/kihuhyouki.html

BOOST_AUTO_TEST_CASE(Ki2TestShow)
{
  {
    const NumEffectState state;
    const Move move(Square(7,7), Square(7,6), PAWN, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R7 K_K6 K_PAWN), 
			 ki2::show(move, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  * +TO *  * -OU * \n"
			   "P2 *  * +KI *  *  *  *  *  * \n"
			   "P3+KI *  * +TO *  *  * +OU * \n"
			   "P4 *  *  *  * +TO *  *  *  * \n"
			   "P5 *  *  * +GI *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  * +GI *  * +GI *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m82u(Square(9,3), Square(8,2), GOLD, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m82y(Square(7,2), Square(8,2), GOLD, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K2 K_GOLD K_UE), 
			 ki2::show(m82u, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K2 K_GOLD K_YORU), 
			 ki2::show(m82y, state));
    const Move m52h(Square(5,1), Square(5,2), PPAWN, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m52u(Square(6,3), Square(5,2), PPAWN, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K2 K_PPAWN K_HIKU), 
			 ki2::show(m52h, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K2 K_PPAWN K_UE), 
			 ki2::show(m52u, state));
    const Move m53y(Square(6,3), Square(5,3), PPAWN, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m53u(Square(5,4), Square(5,3), PPAWN, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K3 K_PPAWN K_YORU), 
			 ki2::show(m53y, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K3 K_PPAWN K_UE), 
			 ki2::show(m53u, state));
    const Move m76h(Square(6,5), Square(7,6), SILVER, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m76u(Square(7,7), Square(7,6), SILVER, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R7 K_K6 K_SILVER K_HIKU), 
			 ki2::show(m76h, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R7 K_K6 K_SILVER K_UE), 
			 ki2::show(m76u, state));
    const Move m56h(Square(6,5), Square(5,6), SILVER, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m56u(Square(4,7), Square(5,6), SILVER, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K6 K_SILVER K_HIKU), 
			 ki2::show(m56h, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K6 K_SILVER K_UE), 
			 ki2::show(m56u, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  * -OU * \n"
			   "P2+KI * +KI *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  * +OU * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  * +GI * +GI *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +KI+KI *  *  * +GI+GI * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m81l(Square(9,2), Square(8,1), GOLD, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m81r(Square(7,2), Square(8,1), GOLD, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K1 K_GOLD K_HIDARI), 
			 ki2::show(m81l, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K1 K_GOLD K_MIGI), 
			 ki2::show(m81r, state));
    const Move m82l(Square(9,2), Square(8,2), GOLD, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m82r(Square(7,2), Square(8,2), GOLD, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K2 K_GOLD K_HIDARI), 
			 ki2::show(m82l, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K2 K_GOLD K_MIGI), 
			 ki2::show(m82r, state));
    const Move m56l(Square(6,5), Square(5,6), SILVER, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m56r(Square(4,5), Square(5,6), SILVER, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K6 K_SILVER K_HIDARI), 
			 ki2::show(m56l, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K6 K_SILVER K_MIGI), 
			 ki2::show(m56r, state));
    const Move m78l(Square(8,9), Square(7,8), GOLD, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m78s(Square(7,9), Square(7,8), GOLD, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R7 K_K8 K_GOLD K_HIDARI), 
			 ki2::show(m78l, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R7 K_K8 K_GOLD K_SUGU), 
			 ki2::show(m78s, state));
    const Move m38s(Square(3,9), Square(3,8), SILVER, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m38r(Square(2,9), Square(3,8), SILVER, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R3 K_K8 K_SILVER K_SUGU), 
			 ki2::show(m38s, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R3 K_K8 K_SILVER K_MIGI), 
			 ki2::show(m38r, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  * -OU * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  * +KI+KI+KI * +OU * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 * +TO *  *  *  * +GI * +GI\n"
			   "P8+TO *  *  *  *  *  *  *  * \n"
			   "P9+TO+TO+TO *  *  * +GI+GI * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m52l(Square(6,3), Square(5,2), GOLD, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m52s(Square(5,3), Square(5,2), GOLD, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m52r(Square(4,3), Square(5,2), GOLD, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K2 K_GOLD K_HIDARI), 
			 ki2::show(m52l, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K2 K_GOLD K_SUGU), 
			 ki2::show(m52s, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K2 K_GOLD K_MIGI), 
			 ki2::show(m52r, state));
    const Move m88r(Square(7,9), Square(8,8), PPAWN, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m88s(Square(8,9), Square(8,8), PPAWN, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m88y(Square(9,8), Square(8,8), PPAWN, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m88h(Square(8,7), Square(8,8), PPAWN, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m88lu(Square(9,9), Square(8,8), PPAWN, 
		     PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K8 K_PPAWN K_MIGI), 
			 ki2::show(m88r, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K8 K_PPAWN K_SUGU), 
			 ki2::show(m88s, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K8 K_PPAWN K_YORU), 
			 ki2::show(m88y, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K8 K_PPAWN K_HIKU), 
			 ki2::show(m88h, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K8 K_PPAWN K_HIDARI K_UE), 
			 ki2::show(m88lu, state));
    const Move m28s(Square(2,9), Square(2,8), SILVER, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m28r(Square(1,7), Square(2,8), SILVER, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m28lu(Square(3,9), Square(2,8), SILVER, 
		     PTYPE_EMPTY, false, BLACK);
    const Move m28lh(Square(3,7), Square(2,8), SILVER, 
		     PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R2 K_K8 K_SILVER K_SUGU), 
			 ki2::show(m28s, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R2 K_K8 K_SILVER K_MIGI), 
			 ki2::show(m28r, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R2 K_K8 K_SILVER K_HIDARI K_UE), 
			 ki2::show(m28lu, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R2 K_K8 K_SILVER K_HIDARI K_HIKU), 
			 ki2::show(m28lh, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1+RY *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  * -OU *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 * +RY *  *  *  * +OU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m82h(Square(9,1), Square(8,2), PROOK, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m82u(Square(8,4), Square(8,2), PROOK, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K2 K_PROOK K_HIKU), 
			 ki2::show(m82h, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K2 K_PROOK K_UE), 
			 ki2::show(m82u, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  * +RY *  *  *  * \n"
			   "P3 *  *  *  *  *  *  * +RY * \n"
			   "P4 *  *  *  *  *  * +OU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  * -OU *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m43y(Square(2,3), Square(4,3), PROOK, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m43h(Square(5,2), Square(4,3), PROOK, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R4 K_K3 K_PROOK K_YORU), 
			 ki2::show(m43y, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R4 K_K3 K_PROOK K_HIKU), 
			 ki2::show(m43h, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  * +RY *  * +RY * \n"
			   "P4 *  *  *  *  *  * +OU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  * -OU *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m43r(Square(2,3), Square(4,3), PROOK, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m43l(Square(5,3), Square(4,3), PROOK, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R4 K_K3 K_PROOK K_MIGI), 
			 ki2::show(m43r, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R4 K_K3 K_PROOK K_HIDARI), 
			 ki2::show(m43l, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  * +OU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+RY+RY *  *  *  * -OU *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m88l(Square(9,9), Square(8,8), PROOK, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m88r(Square(8,9), Square(8,8), PROOK, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K8 K_PROOK K_HIDARI), 
			 ki2::show(m88l, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K8 K_PROOK K_MIGI), 
			 ki2::show(m88r, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1+UM+UM *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  * +OU *  * \n"
			   "P5 *  *  *  * -OU *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  * +RY * \n"
			   "P9 *  *  *  *  *  *  *  * +RY\n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m17l(Square(2,8), Square(1,7), PROOK, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m17r(Square(1,9), Square(1,7), PROOK, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R1 K_K7 K_PROOK K_HIDARI), 
			 ki2::show(m17l, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R1 K_K7 K_PROOK K_MIGI), 
			 ki2::show(m17r, state));
    const Move m82l(Square(9,1), Square(8,2), PBISHOP, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m82r(Square(8,1), Square(8,2), PBISHOP, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K2 K_PBISHOP K_HIDARI), 
			 ki2::show(m82l, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K2 K_PBISHOP K_MIGI), 
			 ki2::show(m82r, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  * +UM *  *  *  *  * \n"
			   "P4 *  *  *  *  *  * +OU *  * \n"
			   "P5+UM *  *  * -OU *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m85y(Square(9,5), Square(8,5), PBISHOP, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m85h(Square(6,3), Square(8,5), PBISHOP, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K5 K_PBISHOP K_YORU), 
			 ki2::show(m85y, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R8 K_K5 K_PBISHOP K_HIKU), 
			 ki2::show(m85h, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  * +UM\n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  * +UM *  * \n"
			   "P5+OU *  *  * -OU *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m12h(Square(1,1), Square(1,2), PBISHOP, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m12u(Square(3,4), Square(1,2), PBISHOP, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R1 K_K2 K_PBISHOP K_HIKU), 
			 ki2::show(m12h, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R1 K_K2 K_PBISHOP K_UE), 
			 ki2::show(m12u, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5+OU *  *  * -OU *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+UM *  *  * +UM *  *  *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m77l(Square(9,9), Square(7,7), PBISHOP, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m77r(Square(5,9), Square(7,7), PBISHOP, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R7 K_K7 K_PBISHOP K_HIDARI), 
			 ki2::show(m77l, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R7 K_K7 K_PBISHOP K_MIGI), 
			 ki2::show(m77r, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5+OU *  *  * -OU *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  * +UM *  *  * \n"
			   "P8 *  *  *  *  *  *  *  * +UM\n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m29l(Square(4,7), Square(2,9), PBISHOP, 
		    PTYPE_EMPTY, false, BLACK);
    const Move m29r(Square(1,8), Square(2,9), PBISHOP, 
		    PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R2 K_K9 K_PBISHOP K_HIDARI), 
			 ki2::show(m29l, state));
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R2 K_K9 K_PBISHOP K_MIGI), 
			 ki2::show(m29r, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  * -OU *  *  *  * \n"
			   "P2 *  *  *  *  *  * +NK *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  * +OU *  *  *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    const Move m33ke(Square(3,3), KNIGHT, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R3 K_K3 K_KNIGHT), 
			 ki2::show(m33ke, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1+NK * +HI *  *  * -KI-OU-KY\n"
			   "P2 * +FU *  *  *  * -KI-GI * \n"
			   "P3-FU * -GI+UM-FU-FU *  * -FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  * -KE * -KE *  *  * \n"
			   "P6 *  *  *  * +FU *  *  *  * \n"
			   "P7+FU * +KI+FU * +FU+FU+FU+FU\n"
			   "P8 *  *  *  * +OU+KI * +GI * \n"
			   "P9+KY-HI+GI *  *  *  * +KE+KY\n"
			   "P+00KA00KY00FU00FU\n"
			   "P-00FU00FU00FU00FU\n"
			   "-\n").initialState());
    const Move m57kn(Square(4,5), Square(5,7), 
		     PKNIGHT, PTYPE_EMPTY, true, WHITE);
    BOOST_CHECK_EQUAL(std::string(K_WHITE_SIGN K_R5 K_K7 K_KNIGHT K_HIDARI K_NARU), 
			 ki2::show(m57kn, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  *  *  * -KY\n"
			   "P2 * -HI *  *  * -KI-OU *  * \n"
			   "P3-FU *  * +TO+FU-KI *  *  * \n"
			   "P4 *  * -FU * -FU-GI * -FU-FU\n"
			   "P5 * -FU *  * -GI-KE-FU *  * \n"
			   "P6 *  * +FU *  * -FU *  * +FU\n"
			   "P7+FU+FU+KE *  *  * +FU+FU * \n"
			   "P8 *  * +KI * +GI * +KI+OU * \n"
			   "P9+KY *  *  * +HI *  * +KE+KY\n"
			   "P+00KA00KA00FU\n"
			   "P-00GI00FU\n"
			   "+\n").initialState());
    const Move m52funari(Square(5,3), Square(5,2), 
		     PPAWN, PTYPE_EMPTY, true, BLACK);
    BOOST_CHECK_EQUAL(std::string(K_BLACK_SIGN K_R5 K_K2 K_PAWN K_NARU), 
			 ki2::show(m52funari, state));
  }
}

BOOST_AUTO_TEST_CASE(Ki2TestParseDate)
{
  using namespace boost::gregorian;
  osl::ki2::Ki2File ki2("record/ki2_test1.ki2", false);
  BOOST_CHECK(!ki2.load().start_date.is_special());
  BOOST_CHECK(date(2010, Nov, 13) == ki2.load().start_date);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
