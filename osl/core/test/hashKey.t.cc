#include "osl/hashKey.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <sstream>
#include <map>

using namespace osl;

BOOST_AUTO_TEST_CASE(HashKeyTestDump)
{
  SimpleState state(HIRATE);
  HashKey key(state);
  HashKey null_key;
  BOOST_CHECK(null_key != key);
  
  std::ostringstream ss;
  key.dumpContents(ss);
}

BOOST_AUTO_TEST_CASE(HashKeyTestFiles){
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  std::string filename;
  while ((ifs >> filename) && ++i<100) {
    if (filename == "") 
      break;

    RecordMinimal record=CsaFileMinimal(OslConfig::testCsaFile(filename)).load();
    NumEffectState state(record.initial_state);
    HashKey key(state);
    for (Move move: record.moves)
    {
      const HashKey next = key.newHashWithMove(move);
      {
	HashKey copy = key;
	copy = copy.newMakeMove(move);
	BOOST_CHECK_EQUAL(copy, next);
	BOOST_CHECK_EQUAL(copy.newUnmakeMove(move), key);
      }

      key = key.newHashWithMove(move);
      state.makeMove(move);
      
      const HashKey batch(state);
      BOOST_CHECK_EQUAL(batch, next);
      BOOST_CHECK_EQUAL(key, next);      
    }
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
