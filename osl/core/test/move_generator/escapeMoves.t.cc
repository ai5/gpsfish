#include "osl/move_generator/escape_.h"
#include "osl/move_generator/move_action.h"
#include "osl/csa.h"
#include "osl/numEffectState.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::move_action;
using namespace osl::move_generator;

BOOST_AUTO_TEST_CASE(EscapeMovesTestBug)
{
  NumEffectState state(CsaString(
			 "P1-KY *  *  *  *  *  *  * +UM\n"
			 "P2 * +KI *  * +HI *  *  * -KY\n"
			 "P3 *  * -KE *  *  *  *  *  * \n"
			 "P4-FU-OU *  * -KI *  * -FU-FU\n"
			 "P5 * +KE-GI-FU *  *  * +KE * \n"
			 "P6+FU+OU *  * +FU+FU *  * +FU\n"
			 "P7 *  *  *  * +GI *  *  *  * \n"
			 "P8 *  *  *  *  *  *  * +HI * \n"
			 "P9+KY *  * -UM *  *  *  * +KY\n"
			 "P+00FU00FU00FU00FU00FU00FU00FU00FU00KI\n"
			 "P-00FU00FU00KE00GI00GI00KI\n"
			 "+\n").initialState());
  const Square king_square(8,6);
#if 0
  {
    Liberty8<BLACK> liberty(state, king_square);
    BOOST_CHECK_EQUAL(2, liberty.count());
  }
  {
    effect::Liberty8<BLACK> liberty(sate, king_square);
    BOOST_CHECK_EQUAL(2, liberty.count());
  }
#endif
  MoveVector moves;
  {
    Store store(moves);
    Escape<Store>::generateCaptureKing<BLACK>(state,state.pieceAt(king_square),
			  Square(7,5), store);
  }
  for (auto move:moves) {
    BOOST_CHECK(state.isSafeMove(move));
  }
  moves.clear();
  {
    Store store(moves);
    Escape<Store>::
      generateEscape<BLACK,KING>(state,state.pieceAt(king_square),store);
  }
  for (auto move:moves) {
    BOOST_CHECK(state.isSafeMove(move));
  }
  BOOST_CHECK_EQUAL((size_t)2, moves.size());

  moves.clear();
  {
    Store store(moves);
    Escape<Store>::
      generateKingEscape<BLACK,false>(state, store);
  }
  for (auto move:moves) {
    BOOST_CHECK(state.isSafeMove(move));
  }
  BOOST_CHECK_EQUAL((size_t)2, moves.size());
}

BOOST_AUTO_TEST_CASE(EscapeMovesTestValid){
  {
    NumEffectState state1=CsaString(
      "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
      "P2 *  *  *  *  *  * -KI *  * \n"
      "P3-FU * -FU-FU-FU-FU *  * -FU\n"
      "P4 *  * +HI *  *  *  *  *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
      "P8 * +GI+KI *  *  *  *  *  * \n"
      "P9+KY+KE *  * +OU+KI+GI+KE+KY\n"
      "P+00FU\n"
      "P+00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P+00KA\n"
      "P-00KA\n"
      "P+00HI\n"
      "-\n"
      ).initialState();
    MoveVector moves;
    {
      Store store(moves);
      Escape<Store>::
	generateMoves<WHITE,false>(state1,state1.pieceAt(Square(7,3)),state1.pieceAt(Square(7,4)),store);
    }
    {
      size_t orig_size=moves.size();
      moves.unique();
      BOOST_CHECK_EQUAL(orig_size,moves.size());
    }    
    BOOST_CHECK(moves.isMember(Move(Square(7,3),Square(7,4),PAWN,ROOK,false,WHITE)));
  }
  {
    NumEffectState state1=CsaString(
      "P1-KY+RY *  * -FU-OU * -KE-KY\n"
      "P2 *  *  *  * +GI-GI-KI *  * \n"
      "P3-FU * -FU-FU *  *  *  *  * \n"
      "P4 * -FU *  *  * -FU+FU * -FU\n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  * +FU+FU *  *  *  *  * \n"
      "P7+FU+FU+GI *  * +FU+GI * +FU\n"
      "P8 *  * +KI * +KI *  *  *  * \n"
      "P9+KY+KE * +OU+KE * -HI+KE+KY\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00KI\n"
      "P-00KA\n"
      "P-00KA\n"
      "+\n"
      ).initialState();
    MoveVector moves;
    {
      Store store(moves);
      Escape<Store>::
	generateMoves<WHITE,false>(state1,state1.pieceAt(Square(4,1)),state1.pieceAt(Square(5,2)),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(4,1),Square(5,2),KING,SILVER,false,WHITE)));
    BOOST_CHECK(moves.isMember(Move(Square(4,1),Square(3,1),KING,PTYPE_EMPTY,false,WHITE)));
    // 開き王手になっている
    BOOST_CHECK(!moves.isMember(Move(Square(5,1),Square(5,2),PAWN,SILVER,false,WHITE)));
  }

  NumEffectState state(CsaString(
			 "P1+NY *  *  *  *  * -OU * -KY\n"
			 "P2 * +TO *  *  * -GI-KI *  *\n"
			 "P3 * -RY *  * +UM * -KI-FU-FU\n"
			 "P4 *  * +FU-FU *  *  *  *  *\n"
			 "P5+KE * -KE * +FU *  * +FU *\n"
			 "P6 *  *  * +FU+GI-FU *  * +FU\n"
			 "P7+KE * -UM *  *  *  *  *  *  *\n"
			 "P8 *  *  *  *  *  *  *  *  * \n"
			 "P9 * +OU * -GI *  *  *  * -NG\n"
			 "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
			 "P-00KI00KY00FU00FU\n"
			 "P-00AL\n"
			 "+\n"
			 ).initialState());
  MoveVector moves;
  {
    Store store(moves);
    Escape<Store >::
      generateMoves<BLACK,false>(state,state.pieceAt(Square(8,9)),state.pieceAt(Square(8,3)),store);
  }
  {
    size_t orig_size=moves.size();
    moves.unique();
    BOOST_CHECK_EQUAL(orig_size,moves.size());
  }
  for (size_t i=0;i<moves.size();i++)
    BOOST_CHECK(state.isValidMove(moves[i]));
  // 本当に escape になっているかはチェックしない
}


BOOST_AUTO_TEST_CASE (EscapeMovesTestCapture) {
  NumEffectState state(CsaString(
			 "P1+NY *  *  *  *  * -OU * -KY\n"
			 "P2 * +TO *  *  * -GI-KI *  *\n"
			 "P3 * -RY *  * +UM * -KI-FU-FU\n"
			 "P4 *  * +FU-FU *  *  *  *  *\n"
			 "P5+KE * -KE * +FU *  * +FU *\n"
			 "P6 *  *  * +FU+GI-FU *  * +FU\n"
			 "P7+KE * -UM *  *  *  *  *  *  *\n"
			 "P8 *  *  *  *  *  *  *  *  * \n"
			 "P9 * +OU * -GI *  *  *  * -NG\n"
			 "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
			 "P-00KI00KY00FU00FU\n"
			 "P-00AL\n"
			 "+\n"
			 ).initialState());
  MoveVector moves;
  {
    Store store(moves);
    Escape<Store>::
      generateMoves<BLACK,false>(state,state.pieceAt(Square(8,9)),
				 state.pieceAt(Square(8,3)),store);
  }
  {
    size_t orig_size=moves.size();
    moves.unique();
    BOOST_CHECK_EQUAL(orig_size,moves.size());
  }
  BOOST_CHECK(moves.isMember(Move(Square(9,5),Square(8,3),PKNIGHT,PROOK,true,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(9,5),Square(8,3),KNIGHT,PROOK,false,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,2),Square(8,3),PPAWN,PROOK,false,BLACK)));
}

/**
 * 当たりをつけられている駒を逃げる手を生成する
 */
BOOST_AUTO_TEST_CASE(EscapeMovesTestEscape) {
  NumEffectState state(CsaString(
			 "P1+NY *  *  *  *  * -OU * -KY\n"
			 "P2 * +TO *  *  * -GI-KI *  *\n"
			 "P3 * -RY *  * +UM * -KI-FU-FU\n"
			 "P4 *  * +FU-FU *  *  *  *  *\n"
			 "P5+KE * -KE * +FU *  * +FU *\n"
			 "P6 *  *  * +FU+GI-FU *  * +FU\n"
			 "P7+KE * -UM *  *  *  *  *  *  *\n"
			 "P8 *  *  *  *  *  *  *  *  * \n"
			 "P9 * +OU * -GI *  *  *  * -NG\n"
			 "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
			 "P-00KI00KY00FU00FU\n"
			 "P-00AL\n"
			 "+\n"
			 ).initialState());
  MoveVector moves;
  {
    Store store(moves);
    Escape<Store>::
      generateMoves<BLACK,false>(state,state.pieceAt(Square(8,9)),
				 state.pieceAt(Square(8,3)),store);
  }
  {
    size_t orig_size=moves.size();
    moves.unique();
    BOOST_CHECK_EQUAL(orig_size,moves.size());
  }
  // valid escape
  BOOST_CHECK(moves.isMember(Move(Square(8,9),Square(9,8),KING,PTYPE_EMPTY,false,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,9),Square(7,9),KING,PTYPE_EMPTY,false,BLACK)));
  // 
  BOOST_CHECK(!moves.isMember(Move(Square(8,9),Square(9,9),KING,PTYPE_EMPTY,false,BLACK)));
  //
  BOOST_CHECK(state.hasEffectAt(WHITE,Square(8,8)));
  BOOST_CHECK(!moves.isMember(Move(Square(8,9),Square(8,8),KING,PTYPE_EMPTY,false,BLACK)));
  BOOST_CHECK(!moves.isMember(Move(Square(8,9),Square(7,8),KING,PTYPE_EMPTY,false,BLACK)));
}

BOOST_AUTO_TEST_CASE(EscapeMovesTestBlocking){
  NumEffectState state(CsaString(
			 "P1+NY *  *  *  *  * -OU * -KY\n"
			 "P2 * +TO *  *  * -GI-KI *  *\n"
			 "P3 * -RY *  * +UM * -KI-FU-FU\n"
			 "P4 *  * +FU-FU *  *  *  *  *\n"
			 "P5+KE * -KE * +FU *  * +FU *\n"
			 "P6 *  *  * +FU+GI-FU *  * +FU\n"
			 "P7+KE * -UM *  *  *  *  *  *  *\n"
			 "P8 *  *  *  *  *  *  *  *  * \n"
			 "P9 * +OU * -GI *  *  *  * -NG\n"
			 "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
			 "P-00KI00KY00FU00FU\n"
			 "P-00AL\n"
			 "+\n"
			 ).initialState());
  MoveVector moves;
  {
    Store store(moves);
    Escape<Store >::
      generateMoves<BLACK,false>(state,state.pieceAt(Square(8,9)),
				 state.pieceAt(Square(8,3)),store);
  }
  {
    size_t orig_size=moves.size();
    moves.unique();
    BOOST_CHECK_EQUAL(orig_size,moves.size());
  }
  
  BOOST_CHECK(moves.isMember(Move(Square(9,7),Square(8,5),KNIGHT,PTYPE_EMPTY,false,BLACK)));
  //
  BOOST_CHECK(!moves.isMember(Move(Square(9,8),Square(8,8),KING,PTYPE_EMPTY,false,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,4),PAWN,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,5),PAWN,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,6),PAWN,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,7),PAWN,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,8),PAWN,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,4),LANCE,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,5),LANCE,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,6),LANCE,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,7),LANCE,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,8),LANCE,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,4),KNIGHT,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,5),KNIGHT,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,6),KNIGHT,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,7),KNIGHT,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,8),KNIGHT,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,4),GOLD,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,5),GOLD,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,6),GOLD,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,7),GOLD,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(8,8),GOLD,BLACK)));
  {
    NumEffectState state1=CsaString(
      "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
      "P2 *  *  *  *  *  * -KI-KA * \n"
      "P3-FU * -FU-FU-FU-FU *  * -FU\n"
      "P4 *  *  *  *  *  * +HI *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 * -HI+FU *  *  *  *  *  * \n"
      "P7+FU *  * +FU+FU+FU+FU * +FU\n"
      "P8 * +KA+KI *  *  *  *  *  * \n"
      "P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
      "P+00FU\n"
      "P+00FU\n"
      "P+00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "-\n"
      ).initialState();
    moves.clear();
    {
      Store store(moves);
      Escape<Store>::
	generateMoves<WHITE,false>(state1,state1.pieceAt(Square(3,2)),
				   state1.pieceAt(Square(3,4)),store);
    }
    {
      size_t orig_size=moves.size();
      moves.unique();
      BOOST_CHECK_EQUAL(orig_size,moves.size());
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,3),PAWN,WHITE)));
  }
}

static void testMoveFile(const std::string& filename){
  RecordMinimal record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initial_state);
  for (auto record_move:record.moves){
    MoveVector all;
    state.generateAllUnsafe(all);
    {
      size_t orig_size=all.size();
      all.unique();
      BOOST_CHECK_EQUAL(orig_size,all.size());
    }
    for (int num=0;num<40;num++){
      Piece p=state.pieceOf(num);
      const Player pl=state.turn();
      if (p.isOnBoard() && p.owner()==pl){
	Square pos=p.square();
	if (state.hasEffectAt(alt(pl),pos)){
	  if(p.ptype()!=KING &&
	     state.hasEffectAt(alt(pl),state.kingSquare(pl)))
	    continue;
	  //
	  Piece attacker=state.findCheapAttack(alt(pl),pos);
	  if (p.ptype() == KING)
	    pl == BLACK 
	      ? state.findCheckPiece<BLACK>(attacker)
	      : state.findCheckPiece<WHITE>(attacker);
	  //
	  MoveVector escape, escape_cheap;
	  {
	    Store store(escape), store_cheap(escape_cheap);
	    if(pl==BLACK) {
	      Escape<Store>::
		generateMoves<BLACK,false>(state,p,attacker,store);
	      Escape<Store>::
		generateMoves<BLACK,true>(state,p,attacker,store_cheap);
	    }
	    else
	    {
	      Escape<Store>::
		generateMoves<WHITE,false>(state,p,attacker,store);
	      Escape<Store>::
		generateMoves<WHITE,true>(state,p,attacker,store_cheap);
	    }
	  }
	  // 逃げる手は全部生成する
	  for (Move move: all) {
	    if(!state.isSafeMove(move)) continue;
	    assert(state.isConsistent(true));
	    NumEffectState next_state = state;
	    next_state.makeMove(move);
	    assert(next_state.isConsistent(true));
	    if (!next_state.hasEffectAt(next_state.turn(),
					next_state.pieceOf(num).square()))
	    {
	      BOOST_CHECK(escape.isMember(move));
	    }
	  }
	  // 王手の場合は生成した手は全部逃げる手
	  if(p.ptype()==KING){
	    for (Move move: escape) { 
	      assert(state.isConsistent(true));
	      BOOST_CHECK(! move.ignoreUnpromote());
	      
	      NumEffectState next_state = state;
	      next_state.makeMove(move);
	      BOOST_CHECK(!next_state.inCheck(pl));
	    }
	    for (Move move: escape_cheap) { 
	      BOOST_CHECK(! move.ignoreUnpromote());	      
	      NumEffectState next_state = state;
	      next_state.makeMove(move);
	      BOOST_CHECK(! next_state.inCheck(pl));
	    }
	  }
	}
      }
    }
    assert(state.isConsistent(true));
    state.makeMove(record_move);
    assert(state.isConsistent(true));
  }
}

BOOST_AUTO_TEST_CASE(EscapeMovesTestMoveMember) {
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=500;
  if (OslConfig::inUnitTestShort()) 
      count=10;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong())
    progress.reset(new boost::progress_display(count, std::cerr));
  std::string filename;
  while((ifs >> filename) && filename != "" && ++i<count){
    if (progress)
      ++(*progress);
    testMoveFile(OslConfig::testCsaFile(filename));
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
