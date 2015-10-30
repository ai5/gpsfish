#include "osl/basic_type.h"
#include "osl/bits/boardTable.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
using namespace osl;

BOOST_AUTO_TEST_CASE(DirectionTestInverse){
  BOOST_CHECK_EQUAL(DR,inverse(UL));
  BOOST_CHECK_EQUAL(D,inverse(U));
  BOOST_CHECK_EQUAL(DL,inverse(UR));
  BOOST_CHECK_EQUAL(R,inverse(L));
  BOOST_CHECK_EQUAL(L,inverse(R));
  BOOST_CHECK_EQUAL(UR,inverse(DL));
  BOOST_CHECK_EQUAL(U,inverse(D));
  BOOST_CHECK_EQUAL(UL,inverse(DR));
}
BOOST_AUTO_TEST_CASE(DirectionTestPrimDir){
  BOOST_CHECK_EQUAL(UL,primDir(UL));
  BOOST_CHECK_EQUAL(U,primDir(U));
  BOOST_CHECK_EQUAL(UR,primDir(UR));
  BOOST_CHECK_EQUAL(L,primDir(L));
  BOOST_CHECK_EQUAL(L,primDir(R));
  BOOST_CHECK_EQUAL(UR,primDir(DL));
  BOOST_CHECK_EQUAL(U,primDir(D));
  BOOST_CHECK_EQUAL(UL,primDir(DR));
}

BOOST_AUTO_TEST_CASE(DirectionTestTraits){
  BOOST_CHECK(DirectionTraitsGen<LONG_D>::blackDx==0);
  BOOST_CHECK(DirectionTraitsGen<LONG_D>::blackDy==1);
  BOOST_CHECK(DirectionTraits<LONG_D>::blackOffset()==(Offset(0,1)));
  BOOST_CHECK((DirectionPlayerTraits<LONG_D,BLACK>::offset())==(Offset(0,1)));
  BOOST_CHECK((DirectionPlayerTraits<LONG_D,BLACK>::offset())!=Offset::ZERO());
  BOOST_CHECK((DirectionPlayerTraits<UUR,WHITE>::offset())==(Offset(1,2)));
  BOOST_CHECK((DirectionPlayerTraits<UUR,WHITE>::offset())==Offset::makeDirect(18));
}

BOOST_AUTO_TEST_CASE(DirectionTestTable){
  BOOST_CHECK_EQUAL(Board_Table.getShortOffset(Offset32(6,0)),
		    Offset(1,0));
  BOOST_CHECK(DirectionTraitsGen<LONG_D>::blackDy==1);
  BOOST_CHECK(DirectionTraits<LONG_D>::blackOffset()==(Offset(0,1)));
  BOOST_CHECK((DirectionPlayerTraits<LONG_D,BLACK>::offset())==(Offset(0,1)));
  BOOST_CHECK((DirectionPlayerTraits<LONG_D,BLACK>::offset())!=Offset::ZERO());
  for(int x0=1;x0<=9;x0++)
    for(int y0=1;y0<=9;y0++){
      Square pos0(x0,y0);
      for(int x1=1;x1<=9;x1++)
	for(int y1=1;y1<=9;y1++){
	  Square pos1(x1,y1);
	  if(x0==x1 && y0==y1) continue;
	  if(x0==x1 || y0==y1 || abs(x0-x1)==abs(y0-y1)){
	    BOOST_CHECK_EQUAL(longToShort(Board_Table.getLongDirection<BLACK>(pos0,pos1)),
			      Board_Table.getShort8<BLACK>(pos0,pos1));
	  }
	}
    }
}

template<int i>
static void showDirect(){
  Direction d=static_cast<Direction>(i);
  std::ostringstream ss;
  ss << i << "," << d << "," << DirectionTraits<static_cast<Direction>(i)>::mask << std::endl;
}
BOOST_AUTO_TEST_CASE(DirectionTestShow){
  if (OslConfig::verbose()) {
    showDirect<0>();
    showDirect<1>();
    showDirect<2>();
    showDirect<3>();
    showDirect<4>();
    showDirect<5>();
    showDirect<6>();
    showDirect<7>();
    showDirect<8>();
    showDirect<9>();
    showDirect<10>();
    showDirect<11>();
    showDirect<12>();
    showDirect<13>();
    showDirect<14>();
    showDirect<15>();
    showDirect<16>();
    showDirect<17>();
    for(int dy=-8;dy<=8;dy++){
      for(int dx=-8;dx<=8;dx++){
	std::ostringstream ss;
	ss << "LongDirection<BLACK>(" << dx << "," << dy << ")=" << Board_Table.getLongDirection<BLACK>(Offset32(dx,dy)) << std::endl;
      }
    }
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
