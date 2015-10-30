#include "osl/move_generator/capture_.h"
#include "osl/move_generator/move_action.h"
#include "osl/csa.h"
#include "osl/numEffectState.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <iostream>
#include <fstream>

typedef osl::NumEffectState test_state_t;

using namespace osl;
using namespace osl::move_generator;
using namespace osl::move_action;

BOOST_AUTO_TEST_CASE(GenerateCaptureMovesTestMove)
{
  NumEffectState state(CsaString(
			 "P1+NY+TO * +HI *  * -OU-KE-KY\n"
			 "P2 *  *  *  *  * -GI-KI *  *\n"
			 "P3 *  * +GI * +UM * -KI-FU-FU\n"
			 "P4 *  * +FU-FU *  *  *  *  *\n"
			 "P5 *  * -KE+FU+FU *  * +FU *\n"
			 "P6+KE *  *  *  * -FU *  * +FU\n"
			 "P7 *  * -UM *  *  *  *  *  *\n"
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
    GenerateCapture::generate(BLACK,state,Square(6,4),store);
  }
  moves.unique();
  BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(6,4),PROOK,PAWN,true,BLACK)) ||
		 (std::cerr << state << moves << std::endl,0));
  // the next move is not generated bacause the rook should promote
  BOOST_CHECK(moves.isMember(Move(Square(7,3),Square(6,4),PSILVER,PAWN,true,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(7,3),Square(6,4),SILVER,PAWN,false,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(6,5),Square(6,4),PAWN,PAWN,false,BLACK)));
  BOOST_CHECK(moves.isMember(Move(Square(5,3),Square(6,4),PBISHOP,PAWN,false,BLACK)));
  BOOST_CHECK(moves.size()==5);

  for(auto move:moves)
    BOOST_CHECK(state.isValidMove(move));
}

static void testFile(const std::string& filename)
{
  RecordMinimal record=CsaFileMinimal(filename).load();
  NumEffectState state=record.initial_state;
  for(Move move:record.moves)
  {
    MoveVector all;
    state.generateAllUnsafe(all);
    // 王手がかかっているときはcaptureを呼ばない
    if(state.inCheck())
      continue;
    for(int y=1;y<=9;y++)
      for(int x=9;x>0;x--)
      {
	Square pos(x,y);
	Piece p=state.pieceAt(pos);
	if(! p.isEmpty() && p.owner()==alt(state.turn()))
	{
	  MoveVector capture;
	  {
	    Store store(capture);
	    GenerateCapture::generate(state.turn(),state,pos,store);
	  }
	  capture.unique();
	  for (Move m:capture) {
	    BOOST_CHECK(state.isValidMove(m,true) && m.to()==pos);
	    BOOST_CHECK(!m.ignoreUnpromote());
	    BOOST_CHECK(state.isSafeMove(m));
	  }
	  for(Move m:all)
	  {
	    if(m.to()==pos)
	    {
	      if(!state.isSafeMove(m)) continue;
	      BOOST_CHECK(capture.isMember(m));
	    }
	  }
	}
      }
    state.makeMove(move);
  }
}

BOOST_AUTO_TEST_CASE(GenerateCaptureMovesTestFile)
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
