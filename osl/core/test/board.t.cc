#include "osl/basic_type.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <iomanip>
#include <iostream>

using namespace osl;

static bool can_handle(Square pos)
{
  return (pos.index() > 0) && (pos.index() < Square::indexMax());
}

BOOST_AUTO_TEST_CASE(BoardTest24){
  BOOST_CHECK(can_handle(Square(1,1)));
  BOOST_CHECK(can_handle(Square(0,0)));
  // 24近傍が配列におさまると楽なこともあるけど，現状は違うようだ
  // BOOST_CHECK(! can_handle(Square(-1,-1)));
}

BOOST_AUTO_TEST_CASE(BoardTestConversion){
  for(int x=9;x>=1;x--)
    for(int y=1;y<=9;y++){
      Square pos(x,y);
      BOOST_CHECK(pos.x() == x);
      BOOST_CHECK(pos.y() == y);
    }
}
BOOST_AUTO_TEST_CASE(BoardTestOnBoard){
  BOOST_CHECK(Square::STAND().isOnBoard() ==false);
  for(int i=-(16*16);i<Square::SIZE+(16*16);i++)
    {
    Square pos=Square::nth((unsigned int)i);
    if(i>=0 && i<256 && 1<=pos.x() && pos.x()<=9 && 1<=pos.y() && pos.y()<=9){
      BOOST_CHECK(pos.isOnBoard() == true);
    }
    else{
      BOOST_CHECK(pos.isOnBoard() == false);
    }
  }
  BOOST_CHECK(Square(0, 4).isOnBoard() ==false);
  BOOST_CHECK(Square(10,5).isOnBoard() ==false);
  BOOST_CHECK(Square(4, 0).isOnBoard() ==false);
  BOOST_CHECK(Square(5,10).isOnBoard() ==false);
  for(int x=0;x<=10;x++)
    for(int y=0;y<=10;y++){
      Square pos(x,y);
      if(x==0 || x==10 || y==0 || y==10){
	if(!pos.isEdge()){
	  std::cerr << "position=" << pos << std::endl;
	}
	BOOST_CHECK(pos.isEdge());
      }
      else{
	if(pos.isEdge()){
	  std::cerr << "position=" << pos << std::endl;
	}
	BOOST_CHECK(!pos.isEdge());
      }
    }
  BOOST_CHECK(Square(0, 4).isEdge());
  BOOST_CHECK(Square(10,5).isEdge());
  BOOST_CHECK(Square(4, 0).isEdge());
  BOOST_CHECK(Square(5,10).isEdge());
}

BOOST_AUTO_TEST_CASE(BoardTestIsValid){
  BOOST_CHECK(Square::STAND().isValid() ==true);
  for(int x=9;x>0;x--)
    for(int y=1;y<=9;y++){
      Square pos(x,y);
      BOOST_CHECK(pos.isValid() == true);
    }
  BOOST_CHECK(Square(0, 4).isValid() ==false);
  BOOST_CHECK(Square(10,5).isValid() ==false);
  BOOST_CHECK(Square(4, 0).isValid() ==false);
  BOOST_CHECK(Square(5,10).isValid() ==false);
}

BOOST_AUTO_TEST_CASE(BoardTestOffsetShow){
  if (OslConfig::verbose())
  {
    std::cerr << Offset(-3,5) << std::endl;
    std::cerr << Offset(9,9) << std::endl;
  }
}
BOOST_AUTO_TEST_CASE(BoardTestOffsetSub){
  BOOST_CHECK(Square(9,9)-Square(7,6)==Offset(2,3));
  BOOST_CHECK(Square(6,7)-Square(9,9)==Offset(-3,-2));
}
BOOST_AUTO_TEST_CASE(BoardTestOffsetAdd){
  BOOST_CHECK(Square(9,9)-Offset(2,3)==Square(7,6));
  BOOST_CHECK(Square(6,7)+Offset(-3,-2)==Square(3,5));
  Square p=Square(6,7);
  p+=Offset(-2,2);
  BOOST_CHECK(p==Square(4,9));
  p-=Offset(-5,3);
  BOOST_CHECK(p==Square(9,6));
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
