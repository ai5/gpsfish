#include "osl/record/kisen.h"
#include "osl/record/ki2.h"
#include "osl/oslConfig.h"
#include <boost/filesystem.hpp>

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <iterator>

using namespace osl;

static const char *filename = OslConfig::testPublicFile("floodgate2010/gps_l-vs-Bonanza.kif");
static const char *ipxFilename = OslConfig::testPublicFile("floodgate2010/gps_l-vs-Bonanza.ipx");

BOOST_AUTO_TEST_CASE(KisenTestShow){
  KisenFile kisenFile(filename);
  auto moves = kisenFile.moves(0);  
  BOOST_CHECK( !moves.empty() );
  if (OslConfig::verbose())
      copy(moves.begin(), moves.end(), 
	   std::ostream_iterator<Move>( std::cout , "\n" ));
}

BOOST_AUTO_TEST_CASE(KisenTestConsistent){
  KisenFile kisenFile(filename);
  const size_t count=OslConfig::inUnitTestShort()
    ? (size_t)100u : std::min((size_t)10000u, kisenFile.size());
  for (size_t i=0;i<count;i++) {
    if (i==172518) 
      continue;
    NumEffectState state=kisenFile.initialState();
    auto moves=kisenFile.moves(i);
    for (size_t j=0;j<moves.size();j++) {
      state.makeMove(moves[j]);
      BOOST_CHECK(state.isConsistent(true));
    }
  }
}

BOOST_AUTO_TEST_CASE(KisenTestGetPlayer){
  KisenIpxFile kisenIpxFile(ipxFilename);
  BOOST_CHECK("Bonanza"==kisenIpxFile.player(2,BLACK) ||
		 (std::cerr <<kisenIpxFile.player(2,BLACK) << std::endl,0)
		 );
  BOOST_CHECK("gps_l"==kisenIpxFile.player(2,WHITE) ||
		 (std::cerr <<kisenIpxFile.player(2,WHITE) << std::endl,0)
		 );
}

BOOST_AUTO_TEST_CASE(KisenTestGetRating){
  KisenIpxFile kisenIpxFile(ipxFilename);
  BOOST_CHECK_EQUAL(777u,kisenIpxFile.rating(6,BLACK));
  BOOST_CHECK_EQUAL(777u,kisenIpxFile.rating(6,WHITE));
}

BOOST_AUTO_TEST_CASE(KisenTestGetStartDate){
  using namespace boost::gregorian;
  KisenIpxFile kisenIpxFile(ipxFilename);
  const date answer(2010, Jan, 03);
  BOOST_CHECK_EQUAL(answer.year(),  kisenIpxFile.startDate(6).year());
  BOOST_CHECK_EQUAL(answer.month(), kisenIpxFile.startDate(6).month());
  BOOST_CHECK_EQUAL(answer.day(),   kisenIpxFile.startDate(6).day());
}

BOOST_AUTO_TEST_CASE(KisenTestResult){
  KisenIpxFile kisenIpxFile(ipxFilename);
  KisenFile kisenFile(filename);
  const size_t count=OslConfig::inUnitTestShort()
    ? (size_t)100u : std::min((size_t)5000u, kisenFile.size());
  for(size_t i=0;i<count;i++){
    auto moves = kisenFile.moves(i);  
    if (moves.size() >= 256) continue;
    unsigned int result=kisenIpxFile.result(i);
    if(result==KisenIpxFile::BLACK_WIN || result==KisenIpxFile::BLACK_WIN_256){
      BOOST_CHECK((moves.size()%2)==1);
    }
    else if(result==KisenIpxFile::WHITE_WIN || result==KisenIpxFile::WHITE_WIN_256){
      BOOST_CHECK((moves.size()%2)==0);
    }
  }
}

BOOST_AUTO_TEST_CASE(KisenTestGetIpxFileName) {
  KisenFile kisenFile(filename);
  BOOST_CHECK_EQUAL(std::string(ipxFilename), kisenFile.ipxFileName());
}

BOOST_AUTO_TEST_CASE(KisenTestSave) {
  using namespace boost::gregorian;
  Ki2File ki2("record/ki2_test1.ki2", false);
  {
    std::ofstream ofs("record/test_kisen_t_temp.ipx");
    record::KisenIpxWriter writer(ofs);
    writer.save(ki2.load(), 100, 1100, "black", "white");
  }
  record::KisenIpxFile ipx("record/test_kisen_t_temp.ipx");
  BOOST_CHECK(date(2010, Nov, 13) == ipx.startDate(0));
  BOOST_CHECK_EQUAL(std::string("black"), ipx.title(0, BLACK));
  BOOST_CHECK_EQUAL(std::string("white"), ipx.title(0, WHITE));
  BOOST_CHECK_EQUAL(100u, ipx.rating(0, BLACK));
  BOOST_CHECK_EQUAL(1100u, ipx.rating(0, WHITE));
  {
    using namespace boost::filesystem;
    path ipx("record/test_kisen_t_temp.ipx");
    if (exists(ipx))
      remove(ipx);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
