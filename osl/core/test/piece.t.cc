#include "osl/basic_type.h"
#include "osl/bits/pieceTable.h"
#include "osl/bits/pieceMask.h"
#include "osl/simpleState.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <iomanip>
#include <iostream>

using namespace osl;
using namespace osl::misc;

/* slowBsf.h
 */
namespace osl
{
  namespace misc
  {
  /**
   * non-0 の時に呼ぶべし.
   * やはりどう考えても遅い
   */
  inline int slowbsf(unsigned int mask){
    assert(mask);
    int mask0= (static_cast<int>(-(mask&0xffff)))>>31;
    mask&=(mask0^0xffff0000);
    int mask1= (static_cast<int>(-(mask&0xff00ff)))>>31;
    mask&=(mask1^0xff00ff00);
    int mask2= (static_cast<int>(-(mask&0xf0f0f0f)))>>31;
    mask&=(mask2^0xf0f0f0f0);
    int mask3= (static_cast<int>(-(mask&0x33333333)))>>31;
    mask&=(mask3^0xcccccccc);
    int mask4= (static_cast<int>(-(mask&0x55555555)))>>31;
    return 31+(mask0&0xfffffff0)+(mask1&0xfffffff8)+
      (mask2&0xfffffffc)+(mask3&0xfffffffe)+mask4;
  }
  }
}


BOOST_AUTO_TEST_CASE(PieceTest_testSize){
  BOOST_CHECK_EQUAL(sizeof(int), sizeof(Piece));
}

BOOST_AUTO_TEST_CASE(PieceTest_testShow){
  if (OslConfig::verbose())
  {
    std::ostringstream ss;
    for(int num=0;num<Piece::SIZE;num++)
      ss << num << ":" << Piece_Table.getPtypeOf(num) 
	 << std::setbase(10) << std::endl;
  }
}
BOOST_AUTO_TEST_CASE(PieceTest_testBsf){
  for(unsigned int i=1;i<i+11111;i+=11111){
    BOOST_CHECK(BitOp::bsf(i)==slowbsf(i));
  }
}
BOOST_AUTO_TEST_CASE(PieceTest_testAddSquare){
  Piece p1=Piece(BLACK,PPAWN,0,Square(7,5));
  BOOST_CHECK(p1.square()==Square(7,5));
  p1+=Offset(1,2);
  BOOST_CHECK(p1==Piece(BLACK,PPAWN,0,Square(8,7)));
  p1.setSquare(Square(6,4));
  BOOST_CHECK(p1==Piece(BLACK,PPAWN,0,Square(6,4)));
}
BOOST_AUTO_TEST_CASE(PieceTest_testIsEmpty){
  BOOST_CHECK(! Piece(BLACK,PPAWN,0,Square(8,7)).isEmpty());
  BOOST_CHECK(! Piece(WHITE,KING,30,Square(6,4)).isEmpty());
  BOOST_CHECK(Piece::EMPTY().isEmpty());
  BOOST_CHECK(! Piece::EDGE().isEmpty());
}
BOOST_AUTO_TEST_CASE(PieceTest_testIsEdge){
  BOOST_CHECK(! Piece(BLACK,PPAWN,0,Square(8,7)).isEdge());
  BOOST_CHECK(! Piece(WHITE,KING,30,Square(6,4)).isEdge());
  BOOST_CHECK(! Piece::EMPTY().isEdge());
  BOOST_CHECK(Piece::EDGE().isEdge());
}
BOOST_AUTO_TEST_CASE(PieceTest_testIsPiece)
{
  SimpleState state(HIRATE);
  for (int x=0; x<=10; ++x) {
    for (int y=0; y<=10; ++y) {
      const Piece p = state.pieceAt(Square(x,y));
      const bool is_piece = p.isPiece();
      const bool is_piece_naive = (! p.isEdge()) && (! p.isEmpty());
      BOOST_CHECK_EQUAL(is_piece_naive, is_piece);
    }
  }
}

BOOST_AUTO_TEST_CASE(PieceTest_testIsOnBoardBy){
  for(int pi=0;pi<2;pi++){
    for(int ptype=PTYPE_PIECE_MIN;ptype<=PTYPE_MAX;ptype++){
      Player pl=indexToPlayer(pi);
      for(int num=0;num<40;num++){
	{
	  Piece p(pl,(Ptype)ptype,num,Square::STAND());
	  BOOST_CHECK(!p.isOnBoardByOwner<BLACK>());
	  BOOST_CHECK(!p.isOnBoardByOwner<WHITE>());
	  BOOST_CHECK(!p.isOnBoardByOwner(BLACK));
	  BOOST_CHECK(!p.isOnBoardByOwner(WHITE));
	}
	for(int x=1;x<=9;x++)
	  for(int y=1;y<=9;y++){
	    Piece p(pl,(Ptype)ptype,num,Square(x,y));
	    if(pl==BLACK){
	      BOOST_CHECK(p.isOnBoardByOwner<BLACK>());
	      BOOST_CHECK(!p.isOnBoardByOwner<WHITE>());
	      BOOST_CHECK(p.isOnBoardByOwner(BLACK));
	      BOOST_CHECK(!p.isOnBoardByOwner(WHITE));
	    }
	    else{
	      BOOST_CHECK(!p.isOnBoardByOwner<BLACK>());
	      BOOST_CHECK(p.isOnBoardByOwner<WHITE>());
	      BOOST_CHECK(!p.isOnBoardByOwner(BLACK));
	      BOOST_CHECK(p.isOnBoardByOwner(WHITE));
	    }
	  }
      }
    }
  }
  {
    BOOST_CHECK(!Piece::EMPTY().isOnBoardByOwner<BLACK>());
    BOOST_CHECK(!Piece::EMPTY().isOnBoardByOwner<WHITE>());
    BOOST_CHECK(!Piece::EMPTY().isOnBoardByOwner(BLACK));
    BOOST_CHECK(!Piece::EMPTY().isOnBoardByOwner(WHITE));
  }
  {
    BOOST_CHECK(!Piece::EDGE().isOnBoardByOwner<BLACK>());
    BOOST_CHECK(!Piece::EDGE().isOnBoardByOwner<WHITE>());
    BOOST_CHECK(!Piece::EDGE().isOnBoardByOwner(BLACK));
    BOOST_CHECK(!Piece::EDGE().isOnBoardByOwner(WHITE));
  }
  Piece p1=Piece(BLACK,PPAWN,0,Square(8,7));
  Piece p2=Piece(WHITE,KING,30,Square(6,4));
  Piece p3=Piece(BLACK,PAWN,0,Square::STAND());
  Piece p4=Piece(WHITE,ROOK,39,Square::STAND());
  BOOST_CHECK(p1.isOnBoardByOwner<BLACK>());
  BOOST_CHECK(!p1.isOnBoardByOwner<WHITE>());
  BOOST_CHECK(!p2.isOnBoardByOwner<BLACK>());
  BOOST_CHECK(p2.isOnBoardByOwner<WHITE>());
  BOOST_CHECK(!p3.isOnBoardByOwner<BLACK>());
  BOOST_CHECK(!p3.isOnBoardByOwner<WHITE>());
  BOOST_CHECK(!p4.isOnBoardByOwner<BLACK>());
  BOOST_CHECK(!p4.isOnBoardByOwner<WHITE>());
  BOOST_CHECK(!Piece::EMPTY().isOnBoardByOwner<BLACK>());
  BOOST_CHECK(!Piece::EMPTY().isOnBoardByOwner<WHITE>());
  BOOST_CHECK(!Piece::EDGE().isOnBoardByOwner<BLACK>());
  BOOST_CHECK(!Piece::EDGE().isOnBoardByOwner<WHITE>());
  BOOST_CHECK(p1.isOnBoardByOwner(BLACK));
  BOOST_CHECK(!p1.isOnBoardByOwner(WHITE));
  BOOST_CHECK(!p2.isOnBoardByOwner(BLACK));
  BOOST_CHECK(p2.isOnBoardByOwner(WHITE));
  BOOST_CHECK(!p3.isOnBoardByOwner(BLACK));
  BOOST_CHECK(!p3.isOnBoardByOwner(WHITE));
  BOOST_CHECK(!p4.isOnBoardByOwner(BLACK));
  BOOST_CHECK(!p4.isOnBoardByOwner(WHITE));
  BOOST_CHECK(!Piece::EMPTY().isOnBoardByOwner(BLACK));
  BOOST_CHECK(!Piece::EMPTY().isOnBoardByOwner(WHITE));
  BOOST_CHECK(!Piece::EDGE().isOnBoardByOwner(BLACK));
  BOOST_CHECK(!Piece::EDGE().isOnBoardByOwner(WHITE));

  SimpleState state(HIRATE);
  for (int x=0; x<=10; ++x) {
    for (int y=0; y<=10; ++y) {
      const Piece p = state.pieceAt(Square(x,y));
      const bool is_onboard_by_owner[2] = {
	p.isPiece() && p.owner() == BLACK,
	p.isPiece() && p.owner() == WHITE,
      };

      BOOST_CHECK_EQUAL(is_onboard_by_owner[0],
			   p.isOnBoardByOwner(BLACK));
      if(is_onboard_by_owner[1] &&
	 !p.isOnBoardByOwner(WHITE)){
	std::ostringstream ss;
	ss << p << ",p.intValue()=" << p.intValue() << std::endl;
	Piece p1=Piece::EDGE();
	ss << p1 << ",p1.intValue()=" << p1.intValue() << std::endl;
      }
      BOOST_CHECK_EQUAL(is_onboard_by_owner[1],
			   p.isOnBoardByOwner(WHITE));
      BOOST_CHECK_EQUAL(is_onboard_by_owner[0],
			   p.isOnBoardByOwner<BLACK>());
      BOOST_CHECK_EQUAL(is_onboard_by_owner[1],
			   p.isOnBoardByOwner<WHITE>());
    }
  }
}
BOOST_AUTO_TEST_CASE(PieceTest_testPromote){
  Piece p1=Piece(BLACK,PAWN,0,Square(8,7));
  Piece p2=p1.promote();
  BOOST_CHECK(p2==Piece(BLACK,PPAWN,0,Square(8,7)));
  Piece p3=p2.unpromote();
  BOOST_CHECK(p1==p3);
  Piece p4=p1.checkPromote(0); 
  BOOST_CHECK(p1==p4);
  Piece p5=p1.checkPromote(1); 
  BOOST_CHECK(p2==p5);
}

BOOST_AUTO_TEST_CASE(PieceTest_testCanMoveOn){
  for(int pi=0;pi<2;pi++){
    for(int ptype=PTYPE_PIECE_MIN;ptype<=PTYPE_MAX;ptype++)
      for(int x=1;x<=9;x++)
	for(int y=1;y<=9;y++)
	  for(int num=0;num<40;num++){
	    Player pl=indexToPlayer(pi);
	    Piece p(pl,(Ptype)ptype,num,Square(x,y));
	    if(pl==BLACK){
	      BOOST_CHECK(!p.canMoveOn<BLACK>());
	      BOOST_CHECK(p.canMoveOn<WHITE>());
	    }
	    else{
	      BOOST_CHECK(!p.canMoveOn<WHITE>());
	      BOOST_CHECK(p.canMoveOn<BLACK>() ||
			     (std::cerr << "p=" << p << std::endl,0)
			     );
	    }
	  }
  }
  {
    BOOST_CHECK(Piece::EMPTY().canMoveOn<BLACK>());
    BOOST_CHECK(Piece::EMPTY().canMoveOn<WHITE>());
  }
  {
    BOOST_CHECK(!Piece::EDGE().canMoveOn<BLACK>());
    BOOST_CHECK(!Piece::EDGE().canMoveOn<WHITE>());
  }
}

BOOST_AUTO_TEST_CASE(PieceTest_testPieceMask){
  PieceMask pm;
  // std::cerr << pm << std::endl;
  for(int i=0;i<40;i++)
    pm.set(i);
  for(int i=5;i<30;i++)
    pm.reset(i);
#if OSL_WORDSIZE == 64
  BOOST_CHECK_EQUAL(mask_t::makeDirect(0xffc000001fuLL),pm.getMask(0));
#elif OSL_WORDSIZE == 32
  std::cerr << std::hex << mask_t::makeDirect(0xc000001f) << "\n";
  BOOST_CHECK_EQUAL(mask_t::makeDirect(0xc000001f),pm.getMask(0));
  BOOST_CHECK_EQUAL(mask_t::makeDirect(0xff),pm.getMask(1));
#endif
  for(int i=0;i<5;i++)
    BOOST_CHECK(pm.test(i));
  for(int i=5;i<30;i++)
    BOOST_CHECK(!pm.test(i));
  for(int i=30;i<40;i++)
    BOOST_CHECK(pm.test(i));
}
BOOST_AUTO_TEST_CASE(PieceTest_testIsPlayerPtype){
  for(int pi=0;pi<2;pi++){
    for(int ptypei=PTYPE_PIECE_MIN;ptypei<=PTYPE_MAX;ptypei++){
      Ptype ptype=(Ptype)ptypei;
      Player pl=indexToPlayer(pi);
      BOOST_CHECK(!Piece::EMPTY().isPlayerPtype(pl,ptype));
      BOOST_CHECK(!Piece::EDGE().isPlayerPtype(pl,ptype));
      if(isBasic(ptype)){
	BOOST_CHECK(!Piece::EMPTY().isPlayerBasicPtype(pl,ptype));
	BOOST_CHECK(!Piece::EDGE().isPlayerBasicPtype(pl,ptype));
      }
      for(int num=0;num<40;num++){
	{
	  Piece p(pl,(Ptype)ptype,num,Square::STAND());
	  for(int pi1=0;pi1<2;pi1++){
	    Player pl1=indexToPlayer(pi1);
	    for(int ptypei1=PTYPE_PIECE_MIN;ptypei1<=PTYPE_MAX;ptypei1++){
	      Ptype ptype1=(Ptype)ptypei1;
	      if(pl==pl1 && ptype==ptype1){
		BOOST_CHECK(p.isPlayerPtype(pl1,ptype1));
	      }
	      else{
		BOOST_CHECK(!p.isPlayerPtype(pl1,ptype1));
	      }
	      if(isBasic(ptype1)){
		if(pl==pl1 && unpromote(ptype)==ptype1){
		  BOOST_CHECK(p.isPlayerBasicPtype(pl1,ptype1));
		}
		else{
		  BOOST_CHECK(!p.isPlayerBasicPtype(pl1,ptype1));
		}
	      }
	    }
	  }
	  for(int x=1;x<=9;x++)
	    for(int y=1;y<=9;y++){
	      Piece p(pl,(Ptype)ptype,num,Square(x,y));
	      for(int pi1=0;pi1<2;pi1++){
		Player pl1=indexToPlayer(pi1);
		for(int ptypei1=PTYPE_PIECE_MIN;ptypei1<=PTYPE_MAX;ptypei1++){
		  Ptype ptype1=(Ptype)ptypei1;
		  if(pl==pl1 && ptype==ptype1){
		    BOOST_CHECK(p.isPlayerPtype(pl1,ptype1));
		  }
		  else{
		    BOOST_CHECK(!p.isPlayerPtype(pl1,ptype1));
		  }
		  if(isBasic(ptype1)){
		    if(pl==pl1 && unpromote(ptype)==ptype1){
		      BOOST_CHECK(p.isPlayerBasicPtype(pl1,ptype1));
		    }
		    else{
		      BOOST_CHECK(!p.isPlayerBasicPtype(pl1,ptype1));
		    }
		  }
		}
	      }
	    }
	}
      }
    }
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
