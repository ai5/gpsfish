#include "osl/move_generator/promote_.h"
#include "osl/move_generator/promote_.tcc"
#include "osl/move_generator/move_action.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <fstream>
#include <iostream>

typedef osl::NumEffectState test_state_t;
using namespace osl;

using namespace osl;
using namespace osl::move_generator;
using namespace osl::move_action;

BOOST_AUTO_TEST_CASE(GeneratePromoteTestMove)
{
  {
    NumEffectState state(CsaString(
			   "P1+NY+TO * +HI * -KI-OU-KE-KY\n"
			   "P2 *  *  *  *  * -GI *  *  *\n"
			   "P3 *  * +GI * +UM * -KI-FU-FU\n"
			   "P4 *  * +FU-FU * +FU *  *  *\n"
			   "P5 *  * -KE+FU+FU *  * +FU *\n"
			   "P6+KE *  *  *  * -FU *  * +FU\n"
			   "P7 *  * -UM *  *  *  *  *  *\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU * -GI *  *  *  * -NG\n"
			   "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
			   "P-00KI00KY00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());

    MoveVector moves;
    Promote<BLACK,true>::generate(state,moves);

    // 銀
    BOOST_CHECK(moves.isMember(Move(Square(7,3),Square(8,4),PSILVER,PTYPE_EMPTY,true,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(7,3),Square(6,2),PSILVER,PTYPE_EMPTY,true,BLACK)));
    // captureする手は生成しない
    BOOST_CHECK(!moves.isMember(Move(Square(7,3),Square(6,4),PSILVER,PAWN,true,BLACK)));
    // 歩
    BOOST_CHECK(moves.isMember(Move(Square(4,4),Square(4,3),PPAWN,PTYPE_EMPTY,true,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(2,5),Square(2,4),PTYPE_EMPTY,PAWN,false,BLACK)));
    // 飛車
    BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(5,1),PROOK,PTYPE_EMPTY,true,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(7,1),PROOK,PTYPE_EMPTY,true,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(6,2),PROOK,PTYPE_EMPTY,true,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(6,3),PROOK,PTYPE_EMPTY,true,BLACK)));
    // capture moveは生成しない．
    BOOST_CHECK(!moves.isMember(Move(Square(6,1),Square(6,4),PROOK,PAWN,true,BLACK)));
    BOOST_CHECK((size_t)9==moves.size());
  }
  {
    NumEffectState state(CsaString(
			   "P1+NY+TO * +HI * -KI-OU-KE-KY\n"
			   "P2 *  *  *  *  * -GI *  *  *\n"
			   "P3 *  * +GI * +UM * -KI-FU-FU\n"
			   "P4 *  * +FU-FU * +FU *  *  *\n"
			   "P5 *  * -KE+FU+FU *  * +FU *\n"
			   "P6+KE *  *  *  * -FU *  * +FU\n"
			   "P7 *  * -UM *  *  *  *  *  *\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU * -GI *  *  *  * -NG\n"
			   "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
			   "P-00KI00KY00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    Promote<BLACK,false>::generate(state,moves);

    // 銀
    BOOST_CHECK(moves.isMember(Move(Square(7,3),Square(8,4),PSILVER,PTYPE_EMPTY,true,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(7,3),Square(6,2),PSILVER,PTYPE_EMPTY,true,BLACK)));
    // captureする手も生成する
    BOOST_CHECK(moves.isMember(Move(Square(7,3),Square(6,4),PSILVER,PAWN,true,BLACK)));
    // 歩
    BOOST_CHECK(moves.isMember(Move(Square(4,4),Square(4,3),PPAWN,PTYPE_EMPTY,true,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(2,5),Square(2,4),PTYPE_EMPTY,PAWN,false,BLACK)));
    // 飛車
    BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(5,1),PROOK,PTYPE_EMPTY,true,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(7,1),PROOK,PTYPE_EMPTY,true,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(6,2),PROOK,PTYPE_EMPTY,true,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(6,3),PROOK,PTYPE_EMPTY,true,BLACK)));
    // capture moveも生成する．
    BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(6,4),PROOK,PAWN,true,BLACK)));
    BOOST_CHECK_EQUAL((size_t)12,moves.size());
  }
}


static void testFile(const std::string& filename)
{
  RecordMinimal record=CsaFileMinimal(filename).load();
  NumEffectState state=record.initial_state;
  for (auto record_move:record.moves)
  {
    MoveVector all;
    state.generateAllUnsafe(all);
    { // check nocapture
      MoveVector promote;
      if(state.turn()==BLACK)
	Promote<BLACK,true>::generate(state,promote);
      else
	Promote<WHITE,true>::generate(state,promote);
      {
	// uniqueなmove
	size_t orig_size=promote.size();
	promote.unique();
	BOOST_CHECK(promote.size()==orig_size);
      }
      for (auto move:all) {
	if(move.isPromotion() && move.capturePtype()==PTYPE_EMPTY)
	  BOOST_CHECK(promote.isMember(move));
      }
      for (auto move:promote) {
	BOOST_CHECK(state.isValidMove(move,true)  
		       && move.isPromotion() 
		       && move.capturePtype()==PTYPE_EMPTY);
      }
    }
    { // check capture
      MoveVector promote;
      if(state.turn()==BLACK)
	Promote<BLACK,false>::generate(state,promote);
      else
	Promote<WHITE,false>::generate(state,promote);
      {
	// uniqueなmove
	size_t orig_size=promote.size();
	promote.unique();
	BOOST_CHECK(promote.size()==orig_size);
      }
      for (auto move:all) {
	if (move.isPromotion()){
	  BOOST_CHECK(promote.isMember(move));
	}
      }
      for (auto move:promote) {
	BOOST_CHECK(state.isValidMove(move,true)  && move.isPromotion());
      }
    }
    state.makeMove(record_move);
  }
}

BOOST_AUTO_TEST_CASE(GeneratePromoteTestFile)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=1000;
  if (OslConfig::inUnitTestShort()) 
    count=10;
  std::string filename;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong())
    progress.reset(new boost::progress_display(count, std::cerr));
  while((ifs >> filename) && filename != "" && ++i<count) {
    if (progress)
      ++(*progress);
    testFile(OslConfig::testCsaFile(filename));
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
