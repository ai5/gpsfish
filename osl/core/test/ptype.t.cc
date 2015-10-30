#include "osl/basic_type.h"
#include "osl/bits/ptypeTraits.h"
#include "osl/bits/ptypeTable.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <iomanip>

namespace osl
{
  inline void for_each_piece_ptype(void (*func)(Ptype))
  {
    for (int i=static_cast<int>(PTYPE_PIECE_MIN);i<=static_cast<int>(PTYPE_MAX);i++)
    {
      func(static_cast<Ptype>(i));
    }
  }
  
  inline void for_each_ptype(void (*func)(Ptype))
  {
    for (int i=static_cast<int>(PTYPE_MIN);i<=static_cast<int>(PTYPE_MAX);i++)
    {
      func(static_cast<Ptype>(i));
    }
  }
}

using namespace osl;

BOOST_AUTO_TEST_CASE(PtypeTestInput){
  std::istringstream is("GOLD ROOK PPAWN PAWN PBISHOP");
  Ptype g;
  is >> g;
  BOOST_CHECK_EQUAL( GOLD, g );
  is >> g;
  BOOST_CHECK_EQUAL( ROOK, g );
  is >> g;
  BOOST_CHECK_EQUAL( PPAWN, g );
  is >> g;
  BOOST_CHECK_EQUAL( PAWN, g );
  is >> g;
  BOOST_CHECK_EQUAL( PBISHOP, g );

}

BOOST_AUTO_TEST_CASE(PtypeTestCanDropTo){
  BOOST_CHECK_EQUAL( false, (PtypePlayerTraits<PAWN,BLACK>::canDropTo(Square(4,1))));
  BOOST_CHECK_EQUAL( false, Ptype_Table.canDropTo(BLACK,PAWN,Square(4,1)));
}
BOOST_AUTO_TEST_CASE(PtypeTestMayPromote){
  BOOST_CHECK((PtypePlayerTraits<PAWN,BLACK>::mayPromote(Square(9,2))));
  BOOST_CHECK((PtypePlayerTraits<PAWN,BLACK>::mayPromote(Square(8,4))));
  BOOST_CHECK((!PtypePlayerTraits<PAWN,BLACK>::mayPromote(Square(1,5))));

  BOOST_CHECK((PtypePlayerTraits<PAWN,WHITE>::mayPromote(Square(9,8))));
  BOOST_CHECK((PtypePlayerTraits<PAWN,WHITE>::mayPromote(Square(8,6))));
  BOOST_CHECK((!PtypePlayerTraits<PAWN,WHITE>::mayPromote(Square(1,5))));

  BOOST_CHECK((PtypePlayerTraits<SILVER,BLACK>::mayPromote(Square(9,2))));
  BOOST_CHECK((PtypePlayerTraits<SILVER,BLACK>::mayPromote(Square(8,4))));
  BOOST_CHECK((!PtypePlayerTraits<SILVER,BLACK>::mayPromote(Square(1,5))));

  BOOST_CHECK((PtypePlayerTraits<SILVER,WHITE>::mayPromote(Square(9,8))));
  BOOST_CHECK((PtypePlayerTraits<SILVER,WHITE>::mayPromote(Square(8,6))));
  BOOST_CHECK((!PtypePlayerTraits<SILVER,WHITE>::mayPromote(Square(1,5))));

  BOOST_CHECK((PtypePlayerTraits<KNIGHT,BLACK>::mayPromote(Square(9,3))));
  BOOST_CHECK((PtypePlayerTraits<KNIGHT,BLACK>::mayPromote(Square(8,5))));
  BOOST_CHECK((!PtypePlayerTraits<KNIGHT,BLACK>::mayPromote(Square(1,6))));

  BOOST_CHECK((PtypePlayerTraits<KNIGHT,WHITE>::mayPromote(Square(9,7))));
  BOOST_CHECK((PtypePlayerTraits<KNIGHT,WHITE>::mayPromote(Square(8,6))));
  BOOST_CHECK((!PtypePlayerTraits<KNIGHT,WHITE>::mayPromote(Square(1,2))));

  BOOST_CHECK((PtypePlayerTraits<LANCE,BLACK>::mayPromote(Square(1,9))));
  BOOST_CHECK((PtypePlayerTraits<LANCE,WHITE>::mayPromote(Square(8,2))));

  BOOST_CHECK((PtypePlayerTraits<ROOK,BLACK>::mayPromote(Square(1,9))));
  BOOST_CHECK((PtypePlayerTraits<ROOK,WHITE>::mayPromote(Square(8,2))));

  BOOST_CHECK((PtypePlayerTraits<BISHOP,BLACK>::mayPromote(Square(1,9))));
  BOOST_CHECK((PtypePlayerTraits<BISHOP,WHITE>::mayPromote(Square(8,2))));
  // 先手BISHOPが49,58,59,69などにいる場合はこれではチェックはできない．
  BOOST_CHECK((PtypePlayerTraits<BISHOP,BLACK>::mayPromote(Square(5,9))));

}
BOOST_AUTO_TEST_CASE(PtypeTestIsPromoted){
  BOOST_CHECK( isPromoted(PPAWN) == true);
  BOOST_CHECK( isPromoted(PROOK) == true);
  BOOST_CHECK( isPromoted(KING) == false);
  BOOST_CHECK( isPromoted(ROOK) == false);
}
static void checkCanPromote(Ptype ptype){
  if(isPromoted(ptype) || ptype==GOLD || ptype==KING){
    BOOST_CHECK( canPromote(ptype) == false);
  }
  else {
    BOOST_CHECK( canPromote(ptype) == true);
  }
}
BOOST_AUTO_TEST_CASE(PtypeTestCanPromote){
  for_each_piece_ptype(checkCanPromote);
}
BOOST_AUTO_TEST_CASE(PtypeTestPromote){
  BOOST_CHECK( promote(PAWN) == PPAWN);
  BOOST_CHECK( promote(ROOK) == PROOK);
}
BOOST_AUTO_TEST_CASE(PtypeTestUnpromote){
  BOOST_CHECK( unpromote(PPAWN) == PAWN);
  BOOST_CHECK( unpromote(PROOK) == ROOK);
  BOOST_CHECK( unpromote(KING) == KING);
  BOOST_CHECK( unpromote(ROOK) == ROOK);
}
BOOST_AUTO_TEST_CASE(PtypeTestAlt){
  for(int i=0;i<2;i++){
    Player pl=static_cast<Player>(-i);
    for(int k=PPAWN;k<=PTYPE_MAX;k++){
      Ptype ptype=static_cast<Ptype>(k);
      PtypeO ptypeO=newPtypeO(pl,ptype);
      PtypeO altPtypeO=newPtypeO(alt(pl),ptype);
      BOOST_CHECK_EQUAL(altPtypeO,alt(ptypeO));
    }
  }
}
BOOST_AUTO_TEST_CASE(PtypeTestAltIfPiece){
  for(int i=0;i<2;i++){
    Player pl=static_cast<Player>(-i);
    for(int k=PPAWN;k<=PTYPE_MAX;k++){
      Ptype ptype=static_cast<Ptype>(k);
      PtypeO ptypeO=newPtypeO(pl,ptype);
      PtypeO altPtypeO=newPtypeO(alt(pl),ptype);
      BOOST_CHECK_EQUAL(altPtypeO,altIfPiece(ptypeO));
    }
  }
  BOOST_CHECK_EQUAL(PTYPEO_EMPTY,altIfPiece(PTYPEO_EMPTY));
  BOOST_CHECK_EQUAL(PTYPEO_EDGE,altIfPiece(PTYPEO_EDGE));
}
// promote可能なら
static void checkPromoteUnpromote(Ptype ptype){
  if(canPromote(ptype)){
    BOOST_CHECK( unpromote(promote(ptype)) == ptype);
  }
}
BOOST_AUTO_TEST_CASE(PtypeTestPromoteUnpromote){
  for_each_piece_ptype(checkPromoteUnpromote);
}
static void checkTable(Ptype ptype){
  std::ostringstream ss;
  ss << ptype << ",Index=" << Ptype_Table.getIndex(ptype) 
     << ",maskLow=" << Ptype_Table.getMaskLow(ptype).value() 
     << ",moveMask=" << std::setbase(16) 
     << Ptype_Table.getMoveMask(ptype) 
     << std::setbase(10) << std::endl;
}

BOOST_AUTO_TEST_CASE(PtypeTestShow){
  if (OslConfig::verbose())
  {
    for_each_piece_ptype(checkTable);
    for(int i=0;i<2;i++){
      Player p=static_cast<Player>(-i);
      for(int j=0;j<SHORT_DIRECTION_SIZE;j++){
	Direction dir=static_cast<Direction>(j);
	for(int k=PPAWN;k<=PTYPE_MAX;k++){
	  Ptype ptype=static_cast<Ptype>(k);
	  std::cerr << p << "," << dir << "," << ptype << "," << std::setbase(16) 
		    << Ptype_Table.getShortMoveMask(p,static_cast<PtypeO>(k),dir)
		    << ","
		    << Ptype_Table.getShortMoveMask(p,static_cast<PtypeO>(k-16),dir)
		    << std::setbase(10) << std::endl;
	}
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(PtypeTestTable){
  BOOST_CHECK( Ptype_Table.hasLongMove(LANCE) );
  BOOST_CHECK( Ptype_Table.hasLongMove(ROOK) );

  BOOST_CHECK_EQUAL((unsigned long long)PtypeFuns<PAWN>::indexMask,0x3ffffuLL);
  BOOST_CHECK_EQUAL((unsigned long long)PtypeFuns<ROOK>::indexMask,0xc000000000uLL);
  BOOST_CHECK_EQUAL(Ptype_Table.getMaskLow(PAWN),mask_t::makeDirect(0x3ffffuLL));
  BOOST_CHECK_EQUAL(Ptype_Table.getMaskLow(ROOK),mask_t::makeDirect(0xc000000000uLL));

 {
   int dx= -1, dy=8;
   Ptype ptype=ROOK;
   const EffectContent effect=Ptype_Table.getEffect(newPtypeO(BLACK,ptype),
						    Offset32(dx,dy));
   BOOST_CHECK(! effect.hasBlockableEffect());
   BOOST_CHECK(! effect.hasUnblockableEffect());
 }
}

BOOST_AUTO_TEST_CASE(PtypeTestIsMajor)
{
  for (int i=PTYPE_MIN; i<PTYPE_MAX; ++i)
  {
    const Ptype ptype = static_cast<Ptype>(i);
    if (isPiece(ptype))
    {
      BOOST_CHECK_EQUAL((unpromote(ptype) == ROOK 
			    || unpromote(ptype) == BISHOP),
			   isMajor(ptype));
    }
    BOOST_CHECK_EQUAL((ptype == ROOK || ptype == BISHOP),
			 isMajorBasic(ptype));
  }  
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
