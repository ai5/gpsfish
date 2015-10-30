#include "osl/numEffectState.h"
#include "osl/bits/numSimpleEffect.tcc"
#include "osl/csa.h"
#include "osl/move_generator/effect_action.h"
#include "osl/oslConfig.h"

#include <boost/progress.hpp>
#include <boost/test/unit_test.hpp>
#include <iomanip>
#include <fstream>
#include <vector>

using namespace osl;
using namespace osl::effect_action;

struct StoreSquareVector{
  std::vector<Square> store;
  template<Player P,Ptype Type>
  void doActionPtype(Piece p,Square pos){
    store.push_back(pos);
  }
  template<Player P>
  void doAction(Piece /*p*/,Square pos){
    store.push_back(pos);
  }
  bool isMember(Square pos) const{
    return std::find(store.begin(),store.end(),pos)!=store.end();
  }
};

BOOST_AUTO_TEST_CASE(StateTestHasEffectFromTo)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY * +UM *  *  *  * -KE-KY\n"
			   "P2 *  *  *  *  *  *  * -OU * \n"
			   "P3 *  *  *  *  * -HI * -FU-FU\n"
			   "P4-FU * -FU * -FU-KI-FU-GI * \n"
			   "P5 *  *  *  *  *  *  *  * +FU\n"
			   "P6+FU+FU+FU+KI+FU * +FU *  * \n"
			   "P7 * +KI * +FU *  * -UM *  * \n"
			   "P8 * +OU * +GI *  * -NG *  * \n"
			   "P9+KY+KE *  *  *  *  *  * -RY\n"
			   "P+00KI00GI00KY00FU\n"
			   "P-00KE00KE00FU00FU00FU00FU\n"
			   "+\n").initialState());
    BOOST_CHECK(state.hasEffectIf(newPtypeO(BLACK,PBISHOP),
					 Square(8,2),
					 Square(3,7)));
  }
  {
    NumEffectState state(CsaString(
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
			   "+\n").initialState());
    // (8,3)の竜
    BOOST_CHECK(state.hasEffectIf(newPtypeO(BLACK,PROOK),Square(8,3),Square(8,9)));
    BOOST_CHECK(!state.hasEffectIf(newPtypeO(BLACK,PROOK),Square(8,3),Square(3,3)));
    // (4,2)の銀
    BOOST_CHECK(state.hasEffectIf(newPtypeO(WHITE,SILVER),Square(4,2),Square(5,3)));
    BOOST_CHECK(!state.hasEffectIf(newPtypeO(WHITE,SILVER),Square(4,2),Square(3,2)));
    // (8,2)の竜, 空白でも可能
    BOOST_CHECK(state.hasEffectIf(newPtypeO(BLACK,PROOK),Square(8,2),Square(7,1)));
  }
}

BOOST_AUTO_TEST_CASE(StateTestForEachEffectOfPiece)
{
  NumEffectState state(CsaString(
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
			 "+\n").initialState());
  {
    // 53の馬
    StoreSquareVector store;
    state.forEachEffectOfPiece<BLACK,PBISHOP,StoreSquareVector>(Square(5,3),store);
    BOOST_CHECK(store.isMember(Square(4,2)));
    BOOST_CHECK(store.isMember(Square(2,6)));
    // block
    BOOST_CHECK(!store.isMember(Square(3,1)));
    BOOST_CHECK(!store.isMember(Square(5,1)));
  }
  {
    // 53の馬
    StoreSquareVector store;
    Piece p=state.pieceAt(Square(5,3));
    state.forEachEffectOfPiece<StoreSquareVector>(p,store);
    BOOST_CHECK(store.isMember(Square(4,2)));
    BOOST_CHECK(store.isMember(Square(2,6)));
    // block
    BOOST_CHECK(!store.isMember(Square(3,1)));
    BOOST_CHECK(!store.isMember(Square(5,1)));
  }
}

NumEffectState testPosition() 
{
  static NumEffectState state(CsaString(
				"P1-KY-KE *  *  *  *  *  *  * \n"
				"P2 *  *  *  *  *  * -KI-OU * \n"
				"P3 *  * -FU *  * -KI *  *  * \n"
				"P4-FU-HI *  *  *  *  * +GI-KY\n"
				"P5 * -FU+FU * -FU-FU-FU-FU+FU\n"
				"P6+FU *  *  *  *  * +KE *  * \n"
				"P7 * +FU * +FU * +FU+FU+FU-KY\n"
				"P8+KY *  *  *  * +KI+KI *  * \n"
				"P9 * +KE-UM * +KA *  * +KE+OU\n"
				"P-00FU\n"
				"P+00FU\n"
				"P-00FU\n"
				"P-00GI\n"
				"P-00GI\n"
				"P-00GI\n"
				"P-00HI\n"
				"+\n"
				).initialState());
  return state;
}


BOOST_AUTO_TEST_CASE(StateTestPieceHasEffectTo)
{
  NumEffectState state0 = testPosition();
  {
    // (7,9)の馬
    Piece p=state0.pieceAt(Square(7,9));
    BOOST_CHECK(state0.hasEffectByPiece(p,Square(3,5)));
    BOOST_CHECK(state0.hasEffectByPiece(p,Square(8,9)));
    BOOST_CHECK(state0.hasEffectByPiece(p,Square(9,7)));
    BOOST_CHECK(!state0.hasEffectByPiece(p,Square(7,9)));
    BOOST_CHECK(!state0.hasEffectByPiece(p,Square(2,4)));
    BOOST_CHECK(!state0.hasEffectByPiece(p,Square(9,9)));
  }
  {
    Piece p=state0.pieceAt(Square(2,2));
    BOOST_CHECK(state0.hasEffectByPiece(p,Square(1,3)));
    BOOST_CHECK(state0.hasEffectByPiece(p,Square(1,1)));
    BOOST_CHECK(!state0.hasEffectByPiece(p,Square(2,2)));
    BOOST_CHECK(!state0.hasEffectByPiece(p,Square(4,4)));
  }

  {
    NumEffectState state(CsaString(
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
			   "+\n").initialState());
    
    {
      // (7,7)の馬
      Piece p=state.pieceAt(Square(7,7));
      BOOST_CHECK(state.hasEffectByPiece(p,Square(9,5)));
      BOOST_CHECK(state.hasEffectByPiece(p,Square(7,6)));
      BOOST_CHECK(state.hasEffectByPiece(p,Square(6,6)));
      BOOST_CHECK(!state.hasEffectByPiece(p,Square(5,5)));
      BOOST_CHECK(!state.hasEffectByPiece(p,Square(7,5)));
      BOOST_CHECK(!state.hasEffectByPiece(p,Square(7,7)));
    }
    {
      Piece p=state.pieceAt(Square(1,1));
      BOOST_CHECK(state.hasEffectByPiece(p,Square(1,3)));
      BOOST_CHECK(!state.hasEffectByPiece(p,Square(1,1)));
      BOOST_CHECK(!state.hasEffectByPiece(p,Square(2,3)));
      BOOST_CHECK(!state.hasEffectByPiece(p,Square(1,4)));
    }
  }
}

BOOST_AUTO_TEST_CASE(StateTestForeachEffect)
{
  PieceVector myPieces;
  StorePiece myPiecesStore(&myPieces);
  {
    NumEffectState state0 = testPosition();
    Square to=Square(2,8);
    state0.forEachEffect<BLACK,StorePiece>
      (to, myPiecesStore);
    BOOST_CHECK(myPieces.isMember(state0.pieceAt(Square(1,9))));
    BOOST_CHECK(myPieces.isMember(state0.pieceAt(Square(3,8))));
    BOOST_CHECK(!myPieces.isMember(state0.pieceAt(Square(2,8))));
  }
}

BOOST_AUTO_TEST_CASE(StateTestForeachEffectNotBy)
{
  PieceVector myPieces;
  StorePiece myPiecesStore(&myPieces);
  {
    NumEffectState state0 = testPosition();
    Square to=Square(2,8);
    Piece p=state0.pieceAt(Square(1,9));
    state0.forEachEffectNotBy<BLACK,StorePiece>
      (to, p, myPiecesStore);
    BOOST_CHECK(!myPieces.isMember(state0.pieceAt(Square(1,9))));
    BOOST_CHECK(myPieces.isMember(state0.pieceAt(Square(3,8))));
    BOOST_CHECK(!myPieces.isMember(state0.pieceAt(Square(2,8))));
  }
}

BOOST_AUTO_TEST_CASE(StateTestForeachEffectOfPiece)
{
  NumEffectState state1(CsaString(
		       "P1+NY * +TO *  *  * -OU-KE-KY\n"
		       "P2 *  *  *  *  * -GI-KI *  * \n"
		       "P3 * +RY *  * +UM * -KI-FU-FU\n"
		       "P4 *  * +FU-FU *  * -FU *  * \n"
		       "P5 *  * -KE * +FU * +KY+FU * \n"
		       "P6-KE *  * +FU+GI-FU *  * +FU\n"
		       "P7 *  * -UM *  *  *  *  *  * \n"
		       "P8 *  *  *  *  *  *  *  *  * \n"
		       "P9 * +OU * -GI *  *  *  * -NG\n"
		       "P+00FU\n"
		       "P+00FU\n"
		       "P+00FU\n"
		       "P+00FU\n"
		       "P+00FU\n"
		       "P+00FU\n"
		       "P+00FU\n"
		       "P+00KE\n"
		       "P+00KI\n"
		       "P-00KI\n"
		       "P-00KY\n"
		       "P+00HI\n"
		       "+\n"
		       ).initialState());
  PieceVector myPieces;
  StorePiece myPiecesStore(&myPieces);
  state1.forEachEffectOfPiece<StorePiece>(state1.pieceAt(Square(3,5)),myPiecesStore);
  NumEffectState state0 = testPosition();
  BOOST_CHECK(!myPieces.isMember(state0.pieceAt(Square(1,3))));
}


BOOST_AUTO_TEST_CASE(StateTestHasEffectBy)
{
  NumEffectState state0 = testPosition();
  // 7-3 には後手の利きがある．
  BOOST_CHECK((state0.hasEffectAt(WHITE,Square(7,3))));
  // 7-1 には後手の利きがない
  BOOST_CHECK(!(state0.hasEffectAt(WHITE,Square(7,1))));
  // 9-5 には先手の利きがある．
  BOOST_CHECK((state0.hasEffectAt(BLACK,Square(9,5))));
  // 9-8 には後手の利きがない
  BOOST_CHECK(!(state0.hasEffectAt(BLACK,Square(9,8))));
}

BOOST_AUTO_TEST_CASE(StateTestHasEffectByWithRemove)
{
  {
    NumEffectState state1(CsaString(
			 "P1-KY+RY *  *  *  *  *  * -KY\n"
			 "P2 *  *  *  * +UM * +NK *  * \n"
			 "P3-FU-OU-GI-FU-FU-FU *  * -FU\n"
			 "P4 *  * -FU *  *  *  *  *  * \n"
			 "P5 *  *  *  * +KA *  *  *  * \n"
			 "P6 *  *  *  *  *  *  *  *  * \n"
			 "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
			 "P8 *  * -NK * +OU *  *  *  * \n"
			 "P9+KY+KE * -HI * +KI+GI * +KY\n"
			 "P-00FU\n"
			 "P+00FU\n"
			 "P+00FU\n"
			 "P-00FU\n"
			 "P-00FU\n"
			 "P+00KE\n"
			 "P-00GI\n"
			 "P-00GI\n"
			 "P+00KI\n"
			 "P-00KI\n"
			 "P-00KI\n"
			 "-\n"
			 ).initialState());
    NumEffectState sState(state1);
    // 8-4 には8-3を取り除くと先手の利きがある．
    BOOST_CHECK((sState.
		    hasEffectByWithRemove<BLACK>(Square(8,4),Square(8,3))));
    // 8-9 には8-3を取り除くと先手の利きがある．
    BOOST_CHECK((sState.
		    hasEffectByWithRemove<BLACK>(Square(8,9),Square(8,3))));
  }
  {
    NumEffectState state1(CsaString(
			 "P1-KY-KE+GI+KA *  * +RY * -KY\n"
			 "P2 *  * -OU * -KI * +NK *  * \n"
			 "P3-FU * -GI-FU-FU-FU *  * -FU\n"
			 "P4 *  * -FU *  *  *  *  *  * \n"
			 "P5 *  *  *  * +KA *  *  *  * \n"
			 "P6 *  *  *  *  *  *  *  *  * \n"
			 "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
			 "P8 *  * -NK * +OU *  *  *  * \n"
			 "P9+KY+KE * -HI * +KI+GI * +KY\n"
			 "P+00FU\n"
			 "P+00FU\n"
			 "P+00FU\n"
			 "P-00FU\n"
			 "P-00FU\n"
			 "P-00GI\n"
			 "P-00KI\n"
			 "P-00KI\n"
			 "-\n"
			 ).initialState());
    NumEffectState sState(state1);
    // 8-3 には7-2を取り除くと先手の利きがある．
    BOOST_CHECK((sState.
		    hasEffectByWithRemove<BLACK>(Square(8,3),Square(7,2))));
  }
}

BOOST_AUTO_TEST_CASE(StateTestHasEffectNotBy)
{
  NumEffectState state0 = testPosition();
  // 9-3 は後手の香車(9-1),桂馬(8-1)の利きがある．
  BOOST_CHECK((state0.hasEffectNotBy(WHITE,state0.pieceAt(Square(9,1)),Square(9,3))));
  // 9-2 は後手の香車(9-1)利きのみ
  BOOST_CHECK(!(state0.hasEffectNotBy(WHITE,state0.pieceAt(Square(9,1)),Square(9,2))));
}

BOOST_AUTO_TEST_CASE(StateTestHasMultipleEffectBy)
{
  NumEffectState state0 = testPosition();
  // 9-3 には後手の利きが複数ある．
  BOOST_CHECK((state0.hasMultipleEffectAt(WHITE,Square(9,3))));
  // 5-7 には後手の利きが複数ない
  BOOST_CHECK(!(state0.hasMultipleEffectAt(WHITE,Square(5,7))));
  // 2-8 には先手の利きが複数ある．
  BOOST_CHECK((state0.hasMultipleEffectAt(BLACK,Square(2,8))));
  // 2-3 には先手の利きが複数ない
  BOOST_CHECK(!(state0.hasMultipleEffectAt(BLACK,Square(2,3))));
}

BOOST_AUTO_TEST_CASE(StateTestHasEffectPiece)
{
  NumEffectState state0 = testPosition();
  {
    Piece p=state0.findCheapAttack(WHITE,Square(5,7));
    BOOST_CHECK(p==state0.pieceAt(Square(7,9)));
  }
  {
    // 2,3への利きはどちらでも良し
    Piece p=state0.findCheapAttack(WHITE,Square(2,3));
    BOOST_CHECK(p==state0.pieceAt(Square(2,2)) ||
		   p==state0.pieceAt(Square(3,2)));
  }
  {
    //利きがない場合は呼んで良い
    Piece p=state0.findCheapAttack(WHITE,Square(5,9));
    BOOST_CHECK(p==Piece::EMPTY());
  }
}

BOOST_AUTO_TEST_CASE(StateTestHasEffectDir)
{
  NumEffectState state0 = testPosition();
  BOOST_CHECK((state0.hasEffectInDirection<LONG_DR,WHITE>(Square(9,7))));
  BOOST_CHECK((state0.hasEffectInDirection<LONG_UL,BLACK>(Square(7,7))));
  BOOST_CHECK((! state0.hasEffectInDirection<LONG_UL,BLACK>(Square(4,8))));
}

BOOST_AUTO_TEST_CASE(StateTestHasEffectByPtype)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE * -KI * -OU * -KE-KY\n"
			   "P2 * -HI *  *  *  * -KI *  * \n"
			   "P3-FU *  * -FU-FU-FU * -FU-FU\n"
			   "P4 *  * -FU * -GI * -FU-GI * \n"
			   "P5 * -FU *  *  *  *  *  *  * \n"
			   "P6 *  * +FU+FU *  *  *  *  * \n"
			   "P7+FU+FU+KE *  * +FU+FU * +FU\n"
			   "P8 *  * +KI+GI * +GI * +HI+KA\n"
			   "P9+KY *  * +OU * +KI * +KE+KY\n"
			   "P+00FU\n"
			   "P-00KA00FU\n"
			   "+\n").initialState());
    BOOST_CHECK(state.hasEffectByPtype<BISHOP>(BLACK, Square(5,4)));
    BOOST_CHECK(! state.hasEffectByPtype<BISHOP>(WHITE, Square(5,4)));
    BOOST_CHECK(! state.hasEffectByPtype<BISHOP>(BLACK, Square(5,5)));
    BOOST_CHECK(! state.hasEffectByPtype<BISHOP>(WHITE, Square(5,5)));
  }
  {
    const NumEffectState state(CsaString(
			   "P1-OU *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  * +GI * \n"
			   "P3 *  *  *  *  * +NK * +NG * \n"
			   "P4 *  *  * +KE *  *  *  *  * \n"
			   "P5 *  *  *  *  * +TO *  *  * \n"
			   "P6 *  *  * +FU+FU *  *  *  * \n"
			   "P7 * +NY *  *  * +UM *  *  * \n"
			   "P8 *  *  *  *  * +KA+RY *  * \n"
			   "P9 *  * +KY *  * +HI *  * +OU\n"
			   "P-00KI00KI00KI00KI00GI00GI00KE00KE00KY00KY00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    BOOST_CHECK(state.isConsistent(true));

    BOOST_CHECK(state.hasEffectByPtypeStrict<PAWN>(BLACK, Square(6,5)));
    BOOST_CHECK(!state.hasEffectByPtypeStrict<PPAWN>(BLACK, Square(6,5)));
    BOOST_CHECK(state.hasEffectByPtypeStrict<PAWN>(BLACK, Square(5,5)));
    BOOST_CHECK(state.hasEffectByPtypeStrict<PPAWN>(BLACK, Square(5,5)));
    BOOST_CHECK(state.hasEffectByPtypeStrict<PPAWN>(BLACK, Square(4,6)));
    BOOST_CHECK(!state.hasEffectByPtypeStrict<PAWN>(BLACK, Square(4,6)));

    BOOST_CHECK(state.hasEffectByPtypeStrict<KNIGHT>(BLACK, Square(5,2)));
    BOOST_CHECK(state.hasEffectByPtypeStrict<PKNIGHT>(BLACK, Square(5,2)));
    BOOST_CHECK(state.hasEffectByPtypeStrict<KNIGHT>(BLACK, Square(7,2)));
    BOOST_CHECK(!state.hasEffectByPtypeStrict<PKNIGHT>(BLACK, Square(7,2)));
    BOOST_CHECK(!state.hasEffectByPtypeStrict<KNIGHT>(BLACK, Square(4,2)));
    BOOST_CHECK(state.hasEffectByPtypeStrict<PKNIGHT>(BLACK, Square(4,2)));
  }
}


BOOST_AUTO_TEST_CASE(StateTestHasEffectByNotPinned)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE * -KI * -OU * -KE-KY\n"
			   "P2 *  *  *  *  *  * -KI *  * \n"
			   "P3-FU *  * -FU-FU-FU * -FU-FU\n"
			   "P4 *  * -FU * -GI * -FU-GI * \n"
			   "P5 * -FU * +FU *  *  *  *  * \n"
			   "P6-KA+FU+FU-HI *  *  *  *  * \n"
			   "P7+FU+KI+KE *  * +FU+FU * +FU\n"
			   "P8 *  *  * +GI *  * +GI+HI+KA\n"
			   "P9+KY *  * +OU * +KI * +KE+KY\n"
			   "P+00FU\n"
			   "P-00FU\n"
			   "+\n").initialState());
    // 68銀はpinned
    BOOST_CHECK(!state.hasEffectByNotPinned(BLACK,Square(5,7)));
    // 67は銀で取れるが，pinnedな銀なのでfalse
    BOOST_CHECK(!state.hasEffectByNotPinned(BLACK,Square(6,7)));
    // 98は関係ない香車による利きあり
    BOOST_CHECK(state.hasEffectByNotPinned(BLACK,Square(9,8)));
    // 88はpinnedの金による利きのみ
    BOOST_CHECK(!state.hasEffectByNotPinned(BLACK,Square(8,8)));
    
  }
}

BOOST_AUTO_TEST_CASE(StateTestHasEffectByNotPinnedAndKing)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE * -KI * -OU * -KE-KY\n"
			   "P2 *  *  *  *  *  * -KI *  * \n"
			   "P3-FU *  * -FU-FU-FU * -FU-FU\n"
			   "P4 *  * -FU * -GI * -FU-GI * \n"
			   "P5 * -FU * +FU *  *  *  *  * \n"
			   "P6-KA+FU+FU-HI *  *  *  *  * \n"
			   "P7+FU+KI+KE *  * +FU+FU * +FU\n"
			   "P8 *  *  * +GI *  * +GI+HI+KA\n"
			   "P9+KY *  * +OU * +KI * +KE+KY\n"
			   "P+00FU\n"
			   "P-00FU\n"
			   "+\n").initialState());
    // 68銀はpinned
    BOOST_CHECK(!state.hasEffectByNotPinnedAndKing(BLACK,Square(5,7)));
    // 67は銀で取れるが，pinnedな銀なのでfalse
    BOOST_CHECK(!state.hasEffectByNotPinnedAndKing(BLACK,Square(6,7)));
    // 98は関係ない香車による利きあり
    BOOST_CHECK(state.hasEffectByNotPinnedAndKing(BLACK,Square(9,8)));
    // 88はpinnedの金による利きのみ
    BOOST_CHECK(!state.hasEffectByNotPinnedAndKing(BLACK,Square(8,8)));
    // 78は玉の利きのみ
    BOOST_CHECK(!state.hasEffectByNotPinnedAndKing(BLACK,Square(7,8)));    
  }
}

static void testFile(const std::string& filename)
{
  RecordMinimal record=CsaFileMinimal(filename).load();
  NumEffectState state=record.initial_state;
  for (auto move:record.moves) {
    BOOST_CHECK(state.isValidMove(move));
    state.makeMove(move);
    BOOST_CHECK(state.isConsistent());
  }
}

BOOST_AUTO_TEST_CASE(StateTestCsaFileMinimals)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=100;
  if (OslConfig::inUnitTestShort())
    count=10;
  std::string filename;
  while ((ifs >> filename) && (++i<count)) {
    if (filename == "") 
      break;
    testFile(OslConfig::testCsaFile(filename));
  }
}

BOOST_AUTO_TEST_CASE(StateTestCountEffect2)
{
  NumEffectState state=CsaString(
    "P1-KY-KE * -KI-OU-KI-GI-KE-KY\n"
    "P2 * -HI *  *  *  *  * -KA * \n"
    "P3-FU-FU * -FU-FU-FU-FU-FU-FU\n"
    "P4 *  * -FU-GI *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  * +FU *  *  *  *  *  * \n"
    "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
    "P8 * +KA+KI *  *  *  * +HI * \n"
    "P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
    "+\n").initialState();
  BOOST_CHECK_EQUAL(2, std::min(2, state.countEffect(BLACK, Square(7,7))));
  BOOST_CHECK_EQUAL(2, std::min(2, state.countEffect(BLACK, Square(6,6))));
  BOOST_CHECK_EQUAL(1, std::min(2, state.countEffect(BLACK, Square(5,6))));
  BOOST_CHECK_EQUAL(0, std::min(2, state.countEffect(BLACK, Square(6,5))));
  BOOST_CHECK_EQUAL(2, std::min(2, state.countEffect(WHITE, Square(7,3))));
  BOOST_CHECK_EQUAL(2, std::min(2, state.countEffect(WHITE, Square(7,5))));
  BOOST_CHECK_EQUAL(1, std::min(2, state.countEffect(WHITE, Square(6,5))));
  BOOST_CHECK_EQUAL(1, std::min(2, state.countEffect(WHITE, Square(5,5))));
  BOOST_CHECK_EQUAL(0, std::min(2, state.countEffect(WHITE, Square(4,5))));
}

BOOST_AUTO_TEST_CASE(StateTestCountEffect)
{
  NumEffectState state=CsaString(
    "P1-KY-KE * -KI-OU-KI-GI-KE-KY\n"
    "P2 * -HI *  *  *  *  * -KA * \n"
    "P3-FU-FU * -FU-FU-FU-FU-FU-FU\n"
    "P4 *  * -FU-GI *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  * +FU *  *  *  *  *  * \n"
    "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
    "P8 * +KA+KI *  *  *  * +HI * \n"
    "P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
    "+\n").initialState();
  BOOST_CHECK_EQUAL(3, state.countEffect(BLACK, Square(7,7)));
  BOOST_CHECK_EQUAL(2, state.countEffect(BLACK, Square(6,6)));
  BOOST_CHECK_EQUAL(1, state.countEffect(BLACK, Square(5,6)));
  BOOST_CHECK_EQUAL(0, state.countEffect(BLACK, Square(6,5)));
  
  BOOST_CHECK_EQUAL(2, state.countEffect(WHITE, Square(7,3)));
  BOOST_CHECK_EQUAL(2, state.countEffect(WHITE, Square(7,5)));
  BOOST_CHECK_EQUAL(1, state.countEffect(WHITE, Square(6,5)));
  BOOST_CHECK_EQUAL(1, state.countEffect(WHITE, Square(5,5)));
  BOOST_CHECK_EQUAL(0, state.countEffect(WHITE, Square(4,5)));
  BOOST_CHECK_EQUAL(4, state.countEffect(WHITE, Square(5,2)));
}

BOOST_AUTO_TEST_CASE(StateTestCountEffectPin)
{
  NumEffectState state=CsaString(
    "P1-KY * -KI * -KE *  *  * +RY\n"
    "P2 * -OU * -GI+GI *  *  *  * \n"
    "P3 * -GI * -KI *  *  *  *  * \n"
    "P4-FU+KE-FU-FU-FU * +KA * -FU\n"
    "P5 * -FU *  *  * +FU *  *  * \n"
    "P6+FU * +FU+FU+FU *  *  * +FU\n"
    "P7 * +FU *  *  * +GI *  *  * \n"
    "P8 * +OU+KI-RY *  * +FU *  * \n"
    "P9+KY *  *  *  *  *  *  * +KY\n"
    "P+00KA00KE00KY00FU00FU00FU\n"
    "P-00KI00KE00FU\n"
    "-\n").initialState();
  BOOST_CHECK_EQUAL(2, state.countEffect(BLACK, Square(7,7)));
  PieceMask pin;
  pin.set(state.pieceOnBoard(Square(7,8)).number());
  BOOST_CHECK_EQUAL(1, state.countEffect(BLACK, Square(7,7), pin));
}

BOOST_AUTO_TEST_CASE(StateTestPinAttacker)
{
  NumEffectState state=CsaString(
    "P1-KY * -KI * -KE *  *  * +RY\n"
    "P2 * -OU * -GI+GI *  *  *  * \n"
    "P3 * -GI * -KI *  *  *  *  * \n"
    "P4-FU+KE-FU-FU-FU *  *  * -FU\n"
    "P5 * -FU *  *  * +FU *  *  * \n"
    "P6+FU * +FU+FU+FU+KA *  * +FU\n"
    "P7 * +FU *  *  * +GI *  *  * \n"
    "P8 * +OU+KI-RY *  * +FU *  * \n"
    "P9+KY *  *  *  *  *  *  * +KY\n"
    "P+00KA00KE00KY00FU00FU00FU\n"
    "P-00KI00KE00FU\n"
    "-\n").initialState();
  {
    Piece p=state.pieceAt(Square(6,4));
    Piece attacker=state.pinAttacker(p);
    BOOST_CHECK_EQUAL(Square(4,6),attacker.square());
  }
  {
    Piece p=state.pieceAt(Square(7,8));
    Piece attacker=state.pinAttacker(p);
    BOOST_CHECK_EQUAL(Square(6,8),attacker.square());
  }
}

BOOST_AUTO_TEST_CASE(StateTestKingCanBeCaptured)
{
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
  BOOST_CHECK(! state.inCheck(BLACK));
  BOOST_CHECK(! state.inCheck(WHITE));
  state.makeMove(Move(Square(8,1),Square(7,1),PPAWN,PTYPE_EMPTY,false,BLACK));
  BOOST_CHECK(! state.inCheck(BLACK));
  BOOST_CHECK(! state.inCheck(WHITE));
  state.makeMove(Move(Square::STAND(),Square(8,8),PAWN,PTYPE_EMPTY,false,WHITE));
  BOOST_CHECK(state.inCheck(BLACK));
  BOOST_CHECK(! state.inCheck(WHITE));
  state.makeMove(Move(Square(7,1),Square(8,1),PPAWN,PTYPE_EMPTY,false,BLACK));
  BOOST_CHECK(state.inCheck(BLACK));
  BOOST_CHECK(! state.inCheck(WHITE));
}

BOOST_AUTO_TEST_CASE(StateTestEffectPtype)
{
  NumEffectState state(CsaString(
			 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			 "P2 * -HI *  *  *  *  * -KA * \n"
			 "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
			 "P4 *  *  *  *  *  * -FU *  * \n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  * +FU *  *  *  *  *  * \n"
			 "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
			 "P8 * +KA *  *  *  *  * +HI * \n"
			 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			 "+\n").initialState());
  BOOST_CHECK_EQUAL(Square(2,2), state.findAttackAt<BISHOP>(WHITE, Square(6,6)).square());
}
BOOST_AUTO_TEST_CASE(StateTestChangedEffect)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=3000;
  if (OslConfig::inUnitTestShort())
    count=10;
  std::string filename;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTest() >= 2)
    progress.reset(new boost::progress_display(count, std::cerr));
  while ((ifs >> filename) && (++i<count)) {
    if (progress)
      ++(*progress);
    if (filename == "") 
      break;
    RecordMinimal record=CsaFileMinimal(OslConfig::testCsaFile(filename)).load();
    NumEffectState state(record.initial_state);
    for(auto move:record.moves){
      PieceMask before[9][9][2];
      {
	for(int x=1;x<=9;x++)
	  for(int y=1;y<=9;y++){
	    const Square pos(x,y);
	    BOOST_CHECK_EQUAL(state.hasLongEffectAt<LANCE>(BLACK, pos),
				 state.longEffectAt<LANCE>(pos, BLACK).any());
	    BOOST_CHECK_EQUAL(state.hasLongEffectAt<LANCE>(WHITE, pos),
				 state.longEffectAt<LANCE>(pos, WHITE).any());
	    BOOST_CHECK_EQUAL(state.hasLongEffectAt<BISHOP>(BLACK, pos),
				 state.longEffectAt<BISHOP>(pos, BLACK).any());
	    BOOST_CHECK_EQUAL(state.hasLongEffectAt<BISHOP>(WHITE, pos),
				 state.longEffectAt<BISHOP>(pos, WHITE).any());
	    BOOST_CHECK_EQUAL(state.hasLongEffectAt<ROOK>(BLACK, pos),
				 state.longEffectAt<ROOK>(pos, BLACK).any());
	    BOOST_CHECK_EQUAL(state.hasLongEffectAt<ROOK>(WHITE, pos),
				 state.longEffectAt<ROOK>(pos, WHITE).any());
	    for (int z=0; z<2; ++z) {
	      const Player pl = indexToPlayer(z);
	      before[x-1][y-1][z]=
		state.effectSetAt(pos)&state.piecesOnBoard(pl);
	    }
	  }
      }
      state.makeMove(move);
      PieceMask after[9][9][2];
      {
	for(int x=1;x<=9;x++)
	  for(int y=1;y<=9;y++){
	    const Square pos(x,y);
	    for (int z=0; z<2; ++z) {
	      const Player pl = indexToPlayer(z);
	      after[x-1][y-1][z]=
		state.effectSetAt(pos)&state.piecesOnBoard(pl);
	    }
	  }
	for(int x=1;x<=9;x++)
	  for(int y=1;y<=9;y++){
	    const Square pos(x,y);
	    for (int z=0; z<2; ++z) {
	      const Player pl = indexToPlayer(z);
	      PieceMask b=before[x-1][y-1][z];
	      PieceMask a=after[x-1][y-1][z];
	      if(a!=b || (pos==move.to() && move.capturePtype()!=PTYPE_EMPTY)){
		for(int num=0;num<48;num++){
		  if(b.test(num)!=a.test(num) || 
		     (pos==move.to() && move.capturePtype()!=PTYPE_EMPTY && (b.test(num) || a.test(num)))){
		    BOOST_CHECK(state.changedPieces().test(num));
		  }
		}
	      }
	      if(state.changedEffects(pl).test(pos)){
		Piece p=state.pieceAt(move.to());
		if(b==a){
		  if(!a.test(p.number())){
		    std::cerr << std::endl << state << ",move=" << move << ",x=" << x << ",y=" << y << std::endl;
		    std::cerr << "changed[" << pl << "]=" << std::endl << state.changedEffects(pl);
		  }
		}
	      }
	      else{
		if(b!=a){
		  std::cerr << state << ",move=" << move << ",x=" << x << ",y=" << y << std::endl;
		  std::cerr << "before " << b << "\nafter " << a << "\n";
		  std::cerr << "changed[ " << pl << "]=" << std::endl << state.changedEffects(pl);
		}
		BOOST_CHECK_EQUAL(b, a);
	      }
	    }
	  }
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(StateTestSelectLong)
{
  NumEffectState state(CsaString(
			 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			 "P2 * -HI *  *  *  *  * -KA * \n"
			 "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
			 "P4 *  *  *  *  *  * -FU *  * \n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  * +FU *  *  *  *  *  * \n"
			 "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
			 "P8 * +KA *  *  *  *  * +HI * \n"
			 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			 "+\n").initialState());
  BOOST_CHECK(state.longEffectAt<LANCE>(Square(9,2)).any());
  BOOST_CHECK(state.longEffectAt<ROOK>(Square(9,2)).any());
  BOOST_CHECK(state.longEffectAt(Square(9,2), WHITE).countBit()==2);
}

static void testEffectedState(NumEffectState const& state,Move move)
{
  PieceMask b_mask=state.effectedMask(BLACK);
  PieceMask w_mask=state.effectedMask(WHITE);
  for(int num=0;num<40;num++){
    Piece p=state.pieceOf(num);
    if(p.isOnBoard()){
      Square pos=p.square();
      if(b_mask.test(num)){
	if(!state.hasEffectAt(BLACK,pos)){
	  std::cerr << std::endl << state << std::endl;
	  std::cerr << "b_mask=" << b_mask << ",num=" << num << ",pos=" << pos << ",move=" << move << std::endl;
	}
	BOOST_CHECK(state.hasEffectAt(BLACK,pos));
      }
      else{
	if(state.hasEffectAt(BLACK,pos)){
	  std::cerr << std::endl << state << std::endl;
	  std::cerr << "b_mask=" << b_mask << ",num=" << num << ",pos=" << pos << ",move=" << move << std::endl;
	}
	BOOST_CHECK(!state.hasEffectAt(BLACK,pos));
      }
      if(w_mask.test(num)){
	if(!state.hasEffectAt(WHITE,pos)){
	  std::cerr << std::endl << state << std::endl;
	  std::cerr << "w_mask=" << w_mask << ",num=" << num << ",pos=" << pos << ",move=" << move << std::endl;
	}
	BOOST_CHECK(state.hasEffectAt(WHITE,pos));
      }
      else{
	if(state.hasEffectAt(WHITE,pos)){
	  std::cerr << std::endl << state << std::endl;
	  std::cerr << "w_mask=" << w_mask << ",num=" << num << ",pos=" << pos << ",move=" << move << std::endl;
	}
	BOOST_CHECK(!state.hasEffectAt(WHITE,pos));
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(StateTestEffected)
{
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
  testEffectedState(state,Move::INVALID());
  { // simple move
    NumEffectState state1=state;
    Move move(Square(8,3),Square(8,2),PROOK,PTYPE_EMPTY,false,BLACK);
    state1.makeMove(move);
    testEffectedState(state1,move);
  }
  { // drop move
    NumEffectState state1=state;
    Move move(Square(8,2),ROOK,BLACK);
    state1.makeMove(move);
    testEffectedState(state1,move);
  }
  { // capture move
    NumEffectState state1=state;
    Move move(Square(5,3),Square(4,2),PBISHOP,SILVER,false,BLACK);
    state1.makeMove(move);
    testEffectedState(state1,move);
  }
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=100;
  if (OslConfig::inUnitTestShort())
    count=10;
  std::string filename;
  while ((ifs >> filename) && (++i<count)) {
    if (filename == "") 
      break;
    RecordMinimal record=CsaFileMinimal(OslConfig::testCsaFile(filename)).load();
    NumEffectState state(record.initial_state);
    for(auto move:record.moves){
      state.makeMove(move);
      testEffectedState(state,move);
    }
  }
}

BOOST_AUTO_TEST_CASE(StateTestKingMobility)
{
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
  BOOST_CHECK_EQUAL(Square(8, 10), state.kingMobilityAbs(BLACK, U));
  BOOST_CHECK_EQUAL(Square(8, 3), state.kingMobilityAbs(BLACK, D));
  BOOST_CHECK_EQUAL(Square(6, 9), state.kingMobilityAbs(BLACK, L));
  BOOST_CHECK_EQUAL(Square(10, 9), state.kingMobilityAbs(BLACK, R));
  BOOST_CHECK_EQUAL(Square(5, 6), state.kingMobilityAbs(BLACK, DL));
  BOOST_CHECK_EQUAL(Square(7, 10), state.kingMobilityAbs(BLACK, UL));
  BOOST_CHECK_EQUAL(Square(10, 7), state.kingMobilityAbs(BLACK, DR));
  BOOST_CHECK_EQUAL(Square(9, 10), state.kingMobilityAbs(BLACK, UR));

  BOOST_CHECK_EQUAL(Square(3, 2), state.kingMobilityAbs(WHITE, U));
  BOOST_CHECK_EQUAL(Square(3, 0), state.kingMobilityAbs(WHITE, D));
  BOOST_CHECK_EQUAL(Square(2, 1), state.kingMobilityAbs(WHITE, L));
  BOOST_CHECK_EQUAL(Square(8, 1), state.kingMobilityAbs(WHITE, R));
  BOOST_CHECK_EQUAL(Square(4, 2), state.kingMobilityAbs(WHITE, UR));
  BOOST_CHECK_EQUAL(Square(4, 0), state.kingMobilityAbs(WHITE, DR));
  BOOST_CHECK_EQUAL(Square(1, 3), state.kingMobilityAbs(WHITE, UL));
  BOOST_CHECK_EQUAL(Square(2, 0), state.kingMobilityAbs(WHITE, DL));

  BOOST_CHECK_EQUAL(Square(8, 3), state.kingMobilityOfPlayer(BLACK, U));
  BOOST_CHECK_EQUAL(Square(8, 10), state.kingMobilityOfPlayer(BLACK, D));
  BOOST_CHECK_EQUAL(Square(10, 9), state.kingMobilityOfPlayer(BLACK, L));
  BOOST_CHECK_EQUAL(Square(6, 9), state.kingMobilityOfPlayer(BLACK, R));
  BOOST_CHECK_EQUAL(Square(5, 6), state.kingMobilityOfPlayer(BLACK, UR));
  BOOST_CHECK_EQUAL(Square(9, 10), state.kingMobilityOfPlayer(BLACK, DL));
  BOOST_CHECK_EQUAL(Square(10, 7), state.kingMobilityOfPlayer(BLACK, UL));
  BOOST_CHECK_EQUAL(Square(7, 10), state.kingMobilityOfPlayer(BLACK, DR));

  BOOST_CHECK_EQUAL(Square(3, 2), state.kingMobilityOfPlayer(WHITE, U));
  BOOST_CHECK_EQUAL(Square(3, 0), state.kingMobilityOfPlayer(WHITE, D));
  BOOST_CHECK_EQUAL(Square(2, 1), state.kingMobilityOfPlayer(WHITE, L));
  BOOST_CHECK_EQUAL(Square(8, 1), state.kingMobilityOfPlayer(WHITE, R));
  BOOST_CHECK_EQUAL(Square(4, 2), state.kingMobilityOfPlayer(WHITE, UR));
  BOOST_CHECK_EQUAL(Square(4, 0), state.kingMobilityOfPlayer(WHITE, DR));
  BOOST_CHECK_EQUAL(Square(1, 3), state.kingMobilityOfPlayer(WHITE, UL));
  BOOST_CHECK_EQUAL(Square(2, 0), state.kingMobilityOfPlayer(WHITE, DL));
}

BOOST_AUTO_TEST_CASE(StateTestLongEffectOfDirection)
{
  {
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
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(5,3), UL));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(5,3), U));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(5,3), UR));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(8,3)), state.findLongAttackAt(Square(5,3), L));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(5,3), R));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(5,3), DL));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(5,3), D));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(5,3), R));

    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(4,2), UL));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(4,2), U));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(5,3)), state.findLongAttackAt(Square(4,2), UR));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(4,2), L));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(4,2), R));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(4,2), DL));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(4,2), D));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(4,2), R));

    // king?
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(8,3), D));
  }
  {
    NumEffectState state=CsaString(
      "P1-KY-KE+GI+KA *  * +RY * -KY\n"
      "P2 *  * -OU * -KI * +NK *  * \n"
      "P3-FU * -GI-FU-FU-FU *  * -FU\n"
      "P4 *  * -FU *  *  *  *  *  * \n"
      "P5 *  *  *  * +KA *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
      "P8 *  * -NK * +OU *  *  *  * \n"
      "P9+KY+KE * -HI * +KI+GI * +KY\n"
      "P+00FU00FU00FU\n"
      "P-00KI00KI00GI00FU00FU\n"
      "-\n").initialState();
    BOOST_CHECK(state.isConsistent());
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(7,2), UR));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(6,1)), state.findLongAttackAt(Square(7,2), DL));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findLongAttackAt(Square(3,2), D));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(3,1)), state.findLongAttackAt(Square(3,2), U));
  }
}

BOOST_AUTO_TEST_CASE(StateTestFindCheapThreat)
{
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
    BOOST_CHECK_EQUAL(state.pieceAt(Square(5,3)), 
			 state.findCheapAttack(BLACK, Square(5,2)));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(6,3)), 
			 state.findCheapAttack(BLACK, Square(6,2)));
    BOOST_CHECK_EQUAL(Piece::EMPTY(),
			 state.findCheapAttack(BLACK, Square(5,1)));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(2,9)), 
			 state.findCheapAttack(BLACK, Square(3,7)));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(5,8)), 
			 state.findCheapAttack(BLACK, Square(4,9)));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(5,8)), 
			 state.findCheapAttack(BLACK, Square(4,7)));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(1,9)), 
			 state.findCheapAttack(BLACK, Square(1,7)));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(5,9)), 
			 state.findCheapAttack(BLACK, Square(9,9)));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(5,9)), 
			 state.findCheapAttack(BLACK, Square(2,9)));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(2,8)), 
			 state.findCheapAttack(BLACK, Square(1,9)));

    BOOST_CHECK_EQUAL(state.pieceAt(Square(5,4)), 
			 state.findCheapAttack(WHITE, Square(5,5)));
  }
}

BOOST_AUTO_TEST_CASE(StateTestFindThreatenedPiece)
{
  {
    NumEffectState state;
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findThreatenedPiece(BLACK));
    BOOST_CHECK_EQUAL(Piece::EMPTY(), state.findThreatenedPiece(WHITE));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 * -HI *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU-FU-FU * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 * +FU+FU+FU+FU+FU+FU+FU+FU\n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "P+00FU00FU\n"
			   "+\n").initialState()); 
    BOOST_CHECK_EQUAL(state.pieceAt(Square(1,7)), state.findThreatenedPiece(BLACK));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(9,3)), state.findThreatenedPiece(WHITE));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI-GI * -KY\n"
			   "P2 * -HI *  *  *  *  * -KA * \n"
			   "P3 * -FU-FU-FU-FU-FU-FU-FU * \n"
			   "P4 *  *  *  * +KE *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  * -KE *  *  *  * \n"
			   "P7 * +FU+FU+FU+FU+FU+FU+FU * \n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY * +GI+KI+OU+KI+GI+KE+KY\n"
			   "P+00FU00FU00FU00FU\n"
			   "+\n").initialState()); 
    BOOST_CHECK_EQUAL(state.pieceAt(Square(5,4)), state.findThreatenedPiece(BLACK));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(5,6)), state.findThreatenedPiece(WHITE));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI *  * -KY\n"
			   "P2 * -HI *  *  *  *  * -KA * \n"
			   "P3 * -FU-FU-FU-FU-FU-FU-FU * \n"
			   "P4 *  *  * +GI+KE *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  * -KE-GI *  *  * \n"
			   "P7 * +FU+FU+FU+FU+FU+FU+FU * \n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY *  * +KI+OU+KI+GI+KE+KY\n"
			   "P+00FU00FU00FU00FU\n"
			   "+\n").initialState()); 
    BOOST_CHECK_EQUAL(state.pieceAt(Square(6,4)), state.findThreatenedPiece(BLACK));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(4,6)), state.findThreatenedPiece(WHITE));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI * -OU-KI *  * -KY\n"
			   "P2 * -HI *  *  *  *  * -KA * \n"
			   "P3 * -FU-FU-FU-FU-FU-FU-FU * \n"
			   "P4 *  * +KI+GI+KE *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  * -KE-GI-KI *  * \n"
			   "P7 * +FU+FU+FU+FU+FU+FU+FU * \n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY *  * +KI+OU * +GI+KE+KY\n"
			   "P+00FU00FU00FU00FU\n"
			   "+\n").initialState()); 
    BOOST_CHECK_EQUAL(state.pieceAt(Square(7,4)), state.findThreatenedPiece(BLACK));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(3,6)), state.findThreatenedPiece(WHITE));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI * -OU-KI *  * -KY\n"
			   "P2 * -HI *  *  *  *  *  *  * \n"
			   "P3 * -FU-FU-FU-FU-FU-FU-FU * \n"
			   "P4 * +KA+KI+GI+KE *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  * -KE-GI-KI-KA * \n"
			   "P7 * +FU+FU+FU+FU+FU+FU+FU * \n"
			   "P8 *  *  *  *  *  *  * +HI * \n"
			   "P9+KY *  * +KI+OU * +GI+KE+KY\n"
			   "P+00FU00FU00FU00FU\n"
			   "+\n").initialState()); 
    BOOST_CHECK_EQUAL(state.pieceAt(Square(8,4)), state.findThreatenedPiece(BLACK));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(2,6)), state.findThreatenedPiece(WHITE));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI * -OU-KI *  * -KY\n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 * -FU-FU-FU-FU-FU-FU-FU * \n"
			   "P4+HI+KA+KI+GI+KE *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  * -KE-GI-KI-KA-HI\n"
			   "P7 * +FU+FU+FU+FU+FU+FU+FU * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+KY *  * +KI+OU * +GI+KE+KY\n"
			   "P+00FU00FU00FU00FU\n"
			   "+\n").initialState()); 
    BOOST_CHECK_EQUAL(state.pieceAt(Square(9,4)), state.findThreatenedPiece(BLACK));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(1,6)), state.findThreatenedPiece(WHITE));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI * -OU-KI *  * -KY\n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 * -FU-FU-FU-FU-FU-FU-FU * \n"
			   "P4+UM+KA+KI+GI+KE *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  * -KE-GI-KI-HI-RY\n"
			   "P7 * +FU+FU+FU+FU+FU+FU+FU * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+KY *  * +KI+OU * +GI+KE+KY\n"
			   "P+00FU00FU00FU00FU\n"
			   "+\n").initialState()); 
    BOOST_CHECK_EQUAL(state.pieceAt(Square(9,4)), state.findThreatenedPiece(BLACK));
    BOOST_CHECK_EQUAL(state.pieceAt(Square(1,6)), state.findThreatenedPiece(WHITE));
  }
}

static void testFileEvasion(const std::string& filename)
{
  RecordMinimal record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initial_state);
  bool in_check = false;
  for (Move move: record.moves) {    
    state.makeMove(move);
    if (in_check != state.wasCheckEvasion(move))
      std::cerr << state << move << ' ' << in_check << "\n";
    BOOST_CHECK_EQUAL(in_check, state.wasCheckEvasion(move));
    in_check = state.inCheck();
  }
}

BOOST_AUTO_TEST_CASE(StateTestWasCheckEvasion)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=10000;
  if (OslConfig::inUnitTestShort())
    count=10;
  std::string filename;
  while ((ifs >> filename) && (++i<count)) {
    if (filename == "") 
      break;
    testFileEvasion(OslConfig::testCsaFile(filename));
  }
}
BOOST_AUTO_TEST_CASE(StateTestCheckShadow)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 * -HI *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "+\n").initialState()); 
    BOOST_CHECK(state.checkShadow(BLACK).none());
    BOOST_CHECK(state.checkShadow(WHITE).none());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 * -HI *  *  *  *  *  *  * \n"
			   "P3-FU-FU-FU-FU+TO-FU-FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  * -KA\n"
			   "P6 *  *  *  * +KY *  *  *  * \n"
			   "P7+FU+FU+FU+FU+FU+FU-TO+FU+FU\n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE * \n"
			   "+\n").initialState()); 
    PieceMask bs=state.checkShadow(BLACK), ws=state.checkShadow(WHITE);
    BOOST_CHECK_EQUAL(1, bs.countBit());
    BOOST_CHECK_EQUAL(1, ws.countBit());

    BOOST_CHECK_EQUAL(state[Square(5,3)], state.pieceOf(bs.takeOneBit()));
    BOOST_CHECK_EQUAL(state[Square(3,7)], state.pieceOf(ws.takeOneBit()));
  }
}

BOOST_AUTO_TEST_CASE(NumEffectStateTestEmulateCapture)
{
  NumEffectState state(CsaString(
			      "P1+NY * +TO *  *  * -OU-KE-KY\n"
			      "P2 *  *  *  *  * -GI-KI *  * \n"
			      "P3 * +RY *  * +UM * -KI-FU-FU\n"
			      "P4 *  * +FU-FU *  * -FU *  * \n"
			      "P5 *  * -KE * +FU * +KY+FU * \n"
			      "P6-KE *  * +FU+GI-FU *  * +FU\n"
			      "P7 *  * -UM *  *  *  *  *  * \n"
			      "P8 *  *  *  *  *  *  *  *  * \n"
			      "P9 * +OU * -GI *  *  *  * -NG\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00KE\n"
			      "P+00KI\n"
			      "P-00KI\n"
			      "P-00KY\n"
			      "P+00HI\n"
			      "+\n"
			      ).initialState());
  {
    SimpleState state1=state.emulateCapture(state.pieceAt(Square(5,3)),WHITE);
    BOOST_CHECK(state1.isConsistent());
    BOOST_CHECK_EQUAL(state.turn(), state1.turn());
    NumEffectState state2(CsaString(
				 "P1+NY * +TO *  *  * -OU-KE-KY\n"
				 "P2 *  *  *  *  * -GI-KI *  * \n"
				 "P3 * +RY *  *  *  * -KI-FU-FU\n"
				 "P4 *  * +FU-FU *  * -FU *  * \n"
				 "P5 *  * -KE * +FU * +KY+FU * \n"
				 "P6-KE *  * +FU+GI-FU *  * +FU\n"
				 "P7 *  * -UM *  *  *  *  *  * \n"
				 "P8 *  *  *  *  *  *  *  *  * \n"
				 "P9 * +OU * -GI *  *  *  * -NG\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00KE\n"
				 "P+00KI\n"
				 "P-00KI\n"
				 "P-00KY\n"
				 "P-00KA\n"
				 "P+00HI\n"
				 "+\n"
				 ).initialState());

    BOOST_CHECK_EQUAL(state1,state2);
  }
  state.changeTurn();
  {
    SimpleState state1=state.emulateCapture(state.pieceAt(Square(5,3)),WHITE);
    BOOST_CHECK(state1.isConsistent());
    BOOST_CHECK_EQUAL(state.turn(), state1.turn());
  }
}

BOOST_AUTO_TEST_CASE(NumEffectStateTestEmulateHandPiece)
{
  NumEffectState state(CsaString(
			      "P1+NY * +TO *  *  * -OU-KE-KY\n"
			      "P2 *  *  *  *  * -GI-KI *  * \n"
			      "P3 * +RY *  * +UM * -KI-FU-FU\n"
			      "P4 *  * +FU-FU *  * -FU *  * \n"
			      "P5 *  * -KE * +FU * +KY+FU * \n"
			      "P6-KE *  * +FU+GI-FU *  * +FU\n"
			      "P7 *  * -UM *  *  *  *  *  * \n"
			      "P8 *  *  *  *  *  *  *  *  * \n"
			      "P9 * +OU * -GI *  *  *  * -NG\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00FU\n"
			      "P+00KE\n"
			      "P+00KI\n"
			      "P-00KI\n"
			      "P-00KY\n"
			      "P+00HI\n"
			      "+\n"
			      ).initialState());
  {
    SimpleState state1=state.emulateHandPiece(BLACK,WHITE,ROOK);
    BOOST_CHECK(state1.isConsistent());
    BOOST_CHECK_EQUAL(state.turn(), state1.turn());
    NumEffectState state2(CsaString(
				 "P1+NY * +TO *  *  * -OU-KE-KY\n"
				 "P2 *  *  *  *  * -GI-KI *  * \n"
				 "P3 * +RY *  * +UM * -KI-FU-FU\n"
				 "P4 *  * +FU-FU *  * -FU *  * \n"
				 "P5 *  * -KE * +FU * +KY+FU * \n"
				 "P6-KE *  * +FU+GI-FU *  * +FU\n"
				 "P7 *  * -UM *  *  *  *  *  * \n"
				 "P8 *  *  *  *  *  *  *  *  * \n"
				 "P9 * +OU * -GI *  *  *  * -NG\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00FU\n"
				 "P+00KE\n"
				 "P+00KI\n"
				 "P-00KI\n"
				 "P-00KY\n"
				 "P-00HI\n"
				 "+\n"
				 ).initialState());

    BOOST_CHECK_EQUAL(state1,state2);
  }
  state.changeTurn();
  {
    SimpleState state1=state.emulateHandPiece(BLACK,WHITE,ROOK);
    BOOST_CHECK(state1.isConsistent());
    BOOST_CHECK_EQUAL(state.turn(), state1.turn());
  }
}

BOOST_AUTO_TEST_CASE(NumEffectStateTestRotate180)
{
  {
    NumEffectState state(CsaString(
			"P1-KY * +UM *  *  *  * -KE-KY\n"
			"P2 *  *  *  *  *  *  * -OU * \n"
			"P3 *  *  *  *  * -HI * -FU-FU\n"
			"P4-FU * -FU * -FU-KI-FU-GI * \n"
			"P5 *  *  *  *  *  *  *  * +FU\n"
			"P6+FU+FU+FU+KI+FU * +FU *  * \n"
			"P7 * +KI * +FU *  * -UM *  * \n"
			"P8 * +OU * +GI *  * -NG *  * \n"
			"P9+KY+KE *  *  *  *  *  * -RY\n"
			"P+00KI00GI00KY00FU\n"
			"P-00KE00KE00FU00FU00FU00FU\n"
			"+\n").initialState());
    NumEffectState state2(CsaString(
			 "P1+RY *  *  *  *  *  * -KE-KY\n"
			 "P2 *  * +NG *  * -GI * -OU * \n"
			 "P3 *  * +UM *  * -FU * -KI * \n"
			 "P4 *  * -FU * -FU-KI-FU-FU-FU\n"
			 "P5-FU *  *  *  *  *  *  *  * \n"
			 "P6 * +GI+FU+KI+FU * +FU * +FU\n"
			 "P7+FU+FU * +HI *  *  *  *  * \n"
			 "P8 * +OU *  *  *  *  *  *  * \n"
			 "P9+KY+KE *  *  *  * -UM * +KY\n"
			 "P+00KE00KE00FU00FU00FU00FU\n"
			 "P-00KI00GI00KY00FU\n"
			 "-\n").initialState());
    BOOST_CHECK_EQUAL(state2, state.rotate180());
  }
}

BOOST_AUTO_TEST_CASE(NumEffectStateTestBoard0)
{
  NumEffectState state;
  BOOST_CHECK(state.pieceAt(Square::STAND()).isEdge());
  BOOST_CHECK(state.pieceAt(Square::makeDirect(0)).isEdge());
}

#if 0
void pawnMaskStateTest::setUp(){
#endif

BOOST_AUTO_TEST_CASE(NumEffectStateTestShow){
  NumEffectState state(CsaString(
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
"+\n").initialState());

  BOOST_CHECK(state.isConsistent() || state.isConsistent(true) );
  std::ostringstream ss;
  {
    ss << state;
    NumEffectState state1;
    ss << state1;
  }
}

BOOST_AUTO_TEST_CASE(NumEffectStateTestIsValid){
  NumEffectState state(CsaString(
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
"+\n").initialState());

  // 後手の手
  BOOST_CHECK(state.isValidMove(Move(Square(6,9),Square(7,8),SILVER,PTYPE_EMPTY,false,WHITE),false)==false);
  // 空白以外へのput
  BOOST_CHECK(state.isValidMove(Move(Square(2,1),PAWN,BLACK),false)==false);
  // 持っていない駒
  BOOST_CHECK(state.isValidMove(Move(Square(9,9),SILVER,BLACK),false)==false);
  // 二歩
  BOOST_CHECK(state.isValidMove(Move(Square(7,1),PAWN,BLACK),false)==false);
  // 二歩ではない
  BOOST_CHECK(state.isValidMove(Move(Square(8,2),PAWN,BLACK),false)==true);
  // 動けない場所
  BOOST_CHECK(state.isValidMove(Move(Square(4,1),PAWN,BLACK),false)==false);
  // fromにあるのがその駒か
  BOOST_CHECK(state.isValidMove(Move(Square(8,2),Square(7,3),PROOK,PTYPE_EMPTY,false,BLACK),false)==false);
  // toにあるのが，相手の駒か空白か?
  BOOST_CHECK(state.isValidMove(Move(Square(8,1),Square(9,1),PPAWN,PLANCE,false,BLACK),false)==false);
  BOOST_CHECK(state.isValidMove(Move(Square(8,1),Square(7,1),PPAWN,PTYPE_EMPTY,false,BLACK),false)==true);
  BOOST_CHECK(state.isValidMove(Move(Square(5,3),Square(4,2),PBISHOP,SILVER,false,BLACK),false)==true);

      // その offsetの動きがptypeに関してvalidか?
  BOOST_CHECK(state.isValidMove(Move(Square(8,1),Square(9,2),PPAWN,PTYPE_EMPTY,false,BLACK),false)==false);
      // 離れた動きの時に間が全部空いているか?
  BOOST_CHECK(state.isValidMove(Move(Square(8,3),Square(6,3),PROOK,PTYPE_EMPTY,false,BLACK),false)==true);
  BOOST_CHECK(state.isValidMove(Move(Square(8,3),Square(4,3),PROOK,PTYPE_EMPTY,false,BLACK),false)==false);
      // capturePtypeが一致しているか?
  BOOST_CHECK(state.isValidMove(Move(Square(5,3),Square(4,2),PBISHOP,PTYPE_EMPTY,false,BLACK),false)==false);
      // promoteしている時にpromote可能か
  BOOST_CHECK(state.isValidMove(Move(Square(8,1),Square(7,1),PPAWN,PTYPE_EMPTY,true,BLACK),false)==false);
      // promoteしていない時に強制promoteでないか?
#if 0
  // 王手をかけられる
  // 現在のpawnMaskStateは判断できない
  BOOST_CHECK(state.isValidMove(Move(Square(8,9),Square(8,8),KING,PTYPE_EMPTY,false,BLACK),false)==false);
#endif

  const NumEffectState s2(CsaString(
"P1-KY *  *  *  *  *  * -KE-KY\n"
"P2 *  *  *  *  *  * -KI * -OU\n"
"P3-FU *  *  *  *  * -GI-GI * \n"
"P4 *  * -FU * -FU * -FU-FU-FU\n"
"P5 * -FU *  *  * +FU * +FU * \n"
"P6 *  * +FU * +FU+KA+FU * +FU\n"
"P7+FU * -TO *  * +KI+KE *  * \n"
"P8+HI *  *  *  *  * +GI+OU * \n"
"P9+KY *  * -RY * +KI *  * +KY\n"
"P+00FU00FU00FU00KE00KI\n"
"P-00KE00GI00KA\n"
"+\n").initialState());
  const Move illegal = Move(Square(2,5),Square(2,4),PPAWN,PAWN,true,BLACK);
  BOOST_CHECK(! s2.isValidMove(illegal,false));
}

template<typename State>
struct CheckOnBoard{
  State& state;
  int ret;
  CheckOnBoard(State& s) : state(s)
  {
  }
  void operator()(Square /*to*/){
    BOOST_CHECK(state.isConsistent());
    int count=0;
    for(int num=0;num<40;num++)
      if(state.isOnBoard(num)) count++;
    ret=count;
  }
};

const Move b5364uma = Move(Square(5,3),Square(6,4),PBISHOP,PAWN,false,BLACK);

static void pieceNumConsistentBeforeCapture(NumEffectState& state)
{
  BOOST_CHECK(state.isConsistent());
  for (int i=0; i<Piece::SIZE; ++i)
  {
    BOOST_CHECK(state.isOnBoard(i));
    BOOST_CHECK(! state.isOffBoard(i));

    const Piece target = state.pieceOf(i);
    const Square pos = target.square();
    const Piece pieceOfSameLocation = state.pieceAt(pos);
    BOOST_CHECK_EQUAL(target, pieceOfSameLocation);
  }
}

BOOST_AUTO_TEST_CASE(NumEffectStateTestPieceConsistent)
{
  NumEffectState s(CsaString(
"P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
"P2 * -HI *  *  *  *  * -KA * \n"
"P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
"P8 * +KA *  *  *  *  * +HI * \n"
"P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
"+\n").initialState());
  pieceNumConsistentBeforeCapture(s);
  s.makeMove(Move(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK));
  pieceNumConsistentBeforeCapture(s);
  s.makeMove(Move(Square(3,3),Square(3,4),PAWN,PTYPE_EMPTY,false,WHITE));
  pieceNumConsistentBeforeCapture(s);
  const Move move = Move(Square(7,6),Square(7,5),PAWN,PTYPE_EMPTY,false,BLACK);
  s.makeMove(move);
  const Piece expected = s.pieceAt(move.to());
  pieceNumConsistentBeforeCapture(s);
  for (int i=0; i<Piece::SIZE; ++i)
  {
    BOOST_CHECK(s.isOnBoard(i));
    BOOST_CHECK(! s.isOffBoard(i));
    const Piece target = s.pieceOf(i);
    BOOST_CHECK((target.square() != move.to())
		   || (expected == target));
    // 動いた駒自身でなければ，駒が重なることはない
  }
}

BOOST_AUTO_TEST_CASE(NumEffectStateTestCaptureBug)
{
  NumEffectState s(CsaString(
    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
    "P2 * -HI *  *  *  *  * -KA * \n"
    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
    "P8 * +KA *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "+\n").initialState());
  s.makeMove(Move(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK));
  s.makeMove(Move(Square(3,3),Square(3,4),PAWN,PTYPE_EMPTY,false,WHITE));
  BOOST_CHECK(s.isConsistent());

  const Square from = Square(8,8);
  const Square to = Square(2,2);
  const Move move = Move(from,to,PBISHOP,BISHOP,true,BLACK);
  BOOST_CHECK(s.isValidMove(move,true));
  s.makeMove(move);
  BOOST_CHECK(s.isConsistent());
  const Piece expected = s.pieceAt(move.to());
  for (int i=0; i<Piece::SIZE; ++i)
  {
    if(s.isOnBoard(i)){
      const Piece target = s.pieceOf(i);
      BOOST_CHECK((target.square() != move.to())
		     || (expected == target)); // 動いた駒自身でなければ，駒が重なることはない
    }
  }
}

BOOST_AUTO_TEST_CASE(UnblockableCheckTestSamples)
{
  {
    const NumEffectState state(CsaString(
				 "P1 *  *  *  *  *  *  * -OU * \n"
				 "P2 *  *  *  *  *  *  * +KI * \n"
				 "P3 *  *  *  *  *  *  * +FU * \n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7 *  *  *  *  *  *  *  *  * \n"
				 "P8 *  *  *  *  *  *  *  *  * \n"
				 "P9 *  * +OU *  *  *  *  *  * \n"
				 "P+00KI\n"
				 "P-00AL\n"
				 "-\n").initialState());
    BOOST_CHECK(state.inUnblockableCheck(WHITE));
    BOOST_CHECK(! state.inUnblockableCheck(BLACK));
  }
  {
    const NumEffectState state(CsaString(
				 "P1 *  *  *  *  * +HI * -OU * \n"
				 "P2 *  *  *  *  *  *  * +GI * \n"
				 "P3 *  *  *  *  *  *  * +FU * \n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  * -KY *  *  *  *  *  * \n"
				 "P7 *  *  *  *  *  *  *  *  * \n"
				 "P8 *  *  *  *  *  *  *  *  * \n"
				 "P9 *  * +OU *  *  *  *  *  * \n"
				 "P+00KI\n"
				 "P-00AL\n"
				 "-\n").initialState());
    BOOST_CHECK(state.inUnblockableCheck(WHITE));
    BOOST_CHECK(! state.inUnblockableCheck(BLACK));
  }
  {
    const NumEffectState state(CsaString(
				 "P1 *  *  *  *  * +HI * -OU * \n"
				 "P2 *  *  *  *  *  *  * +KA * \n"
				 "P3 *  *  *  *  *  *  * +FU * \n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  * -KY *  *  *  *  *  * \n"
				 "P7 *  *  *  *  *  *  *  *  * \n"
				 "P8 *  *  * -NG *  *  *  *  * \n"
				 "P9 *  * +OU *  *  *  *  *  * \n"
				 "P+00KI\n"
				 "P-00AL\n"
				 "-\n").initialState());
    BOOST_CHECK(! state.inUnblockableCheck(WHITE));
    BOOST_CHECK(state.inUnblockableCheck(BLACK));
  }
}

BOOST_AUTO_TEST_CASE(GenerateLegalMovesTestGenerate)
{
  const NumEffectState state(CsaString(
			       "P1-KY *  * -KI *  *  * -KE-KY\n"
			       "P2 * -OU-GI *  * -HI * -KA * \n"
			       "P3 *  * -KE-KI-FU *  * -FU * \n"
			       "P4 * -FU-FU-FU-GI * -FU * -FU\n"
			       "P5-FU *  *  *  * -FU * +KE * \n"
			       "P6 *  * +FU * +GI * +FU+FU+FU\n"
			       "P7+FU+FU+KA+FU+FU *  *  *  * \n"
			       "P8+KY * +KI+KI *  *  *  *  * \n"
			       "P9+OU+KE+GI *  * +HI *  * +KY\n"
			       "P-00FU\n"
			       "+\n").initialState());
  {
    MoveVector moves;
    state.generateLegal(moves);
    Move m22UM = Move(Square(7,7), Square(2,2), PBISHOP, BISHOP, true,  BLACK);
    Move m22KA = Move(Square(7,7), Square(2,2), BISHOP,  BISHOP, false, BLACK);
    BOOST_CHECK(moves.isMember(m22UM));
    BOOST_CHECK(!moves.isMember(m22KA));
  }
  {
    MoveVector moves;
    state.generateWithFullUnpromotions(moves);
    Move m22UM = Move(Square(7,7), Square(2,2), PBISHOP, BISHOP, true,  BLACK);
    Move m22KA = Move(Square(7,7), Square(2,2), BISHOP,  BISHOP, false, BLACK);
    BOOST_CHECK(moves.isMember(m22UM));
    BOOST_CHECK(moves.isMember(m22KA));
  }
}

BOOST_AUTO_TEST_CASE(GenerateLegalMovesTestUnique)
{
  NumEffectState state(CsaString(
			 "P1-KY-KE *  *  *  *  *  * -KY\n"
			 "P2 * +HI-GI *  *  * -KI-GI * \n"
			 "P3-FU+TO-OU+FU-FU-FU-KE * -FU\n"
			 "P4 *  *  * -FU *  *  * -FU * \n"
			 "P5 *  *  *  *  *  * -FU *  * \n"
			 "P6 *  *  *  *  * +FU *  * +FU\n"
			 "P7+FU * -TO * +FU * +FU+FU * \n"
			 "P8 * -HI *  *  *  * +GI+OU * \n"
			 "P9+KY+KE * +KI * +KI * +KE+KY\n"
			 "P+00KA\n"
			 "P-00KA00KI00GI00FU00FU\n"
			 "-\n").initialState());
  MoveVector moves;
  state.generateWithFullUnpromotions(moves);
  for (Move move: moves) {
    BOOST_CHECK_EQUAL(1, std::count(moves.begin(), moves.end(), move));
  }
}

BOOST_AUTO_TEST_CASE(FastCopierTestCopy)
{
  NumEffectState state, state2;
  BOOST_CHECK(state.isConsistent());
  BOOST_CHECK(state2.isConsistent());

  state2.copyFrom(state);

  BOOST_CHECK(state.isConsistent());
  BOOST_CHECK(state2.isConsistent());

  state.makeMove(Move(Square(7,7), Square(7,6), PAWN, PTYPE_EMPTY, false, BLACK));

  BOOST_CHECK(state.isConsistent());
  BOOST_CHECK(state2.isConsistent());
  
  state2.copyFrom(state);

  BOOST_CHECK(state.isConsistent());
  BOOST_CHECK(state2.isConsistent());

}

BOOST_AUTO_TEST_CASE(FastCopierTestFile)
{
  NumEffectState state, state2;
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string filename;
  size_t count = 0, limit = OslConfig::inUnitTestShort() ? 512 : 65536;
  while ((ifs >> filename) && count++ < limit) {
    if (filename == "") 
      break;
    if (count % 128 == 0)
	std::cerr << '.';
    auto record=CsaFileMinimal(OslConfig::testCsaFile(filename)).load();
    state = NumEffectState(record.initialState());
    auto& moves=record.moves;

    for (size_t i=0; i<moves.size(); ++i) {
      BOOST_CHECK(state.isConsistent());
      BOOST_CHECK(state2.isConsistent());

      state.makeMove(moves[i]);
      state2.copyFrom(state);

      BOOST_CHECK(state.isConsistent());
      BOOST_CHECK(state2.isConsistent());
      BOOST_CHECK_EQUAL(state, state2);
      BOOST_CHECK_EQUAL(state.changedEffects(BLACK), state2.changedEffects(BLACK));
      BOOST_CHECK_EQUAL(state.changedEffects(WHITE), state2.changedEffects(WHITE));
      BOOST_CHECK_EQUAL(state.changedPieces(), state2.changedPieces());
      BOOST_CHECK_EQUAL(state.effectedChanged(BLACK), state2.effectedChanged(BLACK));
      BOOST_CHECK_EQUAL(state.effectedChanged(WHITE), state2.effectedChanged(WHITE));
    }
  }
}

BOOST_AUTO_TEST_CASE(EffectUtilTestSafeCaptureNotByKing)
{
  const char *state_false_string =
    "P1-OU-KI * +HI *  *  *  *  * \n"
    "P2+FU *  *  *  *  *  *  *  * \n"
    "P3 * +TO *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  * +OU\n"
    "P+00FU\n"
    "P-00AL\n"
    "-\n";
  const NumEffectState state_false(CsaString(state_false_string).initialState());
  const Square target = Square(9,2);
  BOOST_CHECK_EQUAL(Piece::EMPTY(), state_false.safeCaptureNotByKing
		       (WHITE, target));
  
  const char *state_true_string =
    "P1-OU-KI * +KA *  *  *  *  * \n"
    "P2+FU *  *  *  *  *  *  *  * \n"
    "P3 * +TO *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  * +OU\n"
    "P+00FU\n"
    "P-00AL\n"
    "-\n";
  const NumEffectState state_true(CsaString(state_true_string).initialState());
  BOOST_CHECK_EQUAL(state_true.pieceOnBoard(Square(8,1)), 
		    state_true.safeCaptureNotByKing
		       (WHITE, target));
}

BOOST_AUTO_TEST_CASE(StateTestFindCheapThreatPromoted)
{  
  const NumEffectState state(CsaString(
				     "P1-KY *  *  * -OU * -FU *  * \n"
				     "P2 *  * -KI * -KI-GI-KI *  * \n"
				     "P3-FU-FU * -FU-FU-FU *  *  * \n"
				     "P4 *  * +KE *  *  *  *  * -FU\n"
				     "P5 * -KE *  *  *  *  *  *  * \n"
				     "P6 * +FU *  *  *  *  *  *  * \n"
				     "P7+FU * +FU+FU *  * +RY-NG+FU\n"
				     "P8 * +GI+KI *  *  *  * +HI * \n"
				     "P9+KY+KE *  *  * +KY * +OU+KY\n"
				     "P+00KA00KA00GI00FU00FU\n"
				     "P-00KE00FU00FU00FU00FU\n"
				     "+\n").initialState());
  BOOST_CHECK_EQUAL(state[Square(2,8)], state.findCheapAttack(BLACK, Square(2,7)));
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
