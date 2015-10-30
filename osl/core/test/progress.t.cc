#include "osl/progress.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
BOOST_AUTO_TEST_CASE(NewProgressTestInit)
{
  BOOST_CHECK(NewProgress::setUp());
}

BOOST_AUTO_TEST_CASE(NewProgressTestConsistentUpdate)
{
  using namespace osl;

  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 10 : 200) && (ifs >> file_name) ; i++)
  {
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const RecordMinimal record=CsaFileMinimal(file_name).load();

    NumEffectState state(record.initial_state);
    NewProgress progress(state);
    
    for (auto m:record.moves)
    {
      state.makeMove(m);
      progress.update(state, m);
      const NewProgress new_progress(state);
      BOOST_CHECK_EQUAL(new_progress.progress16(BLACK).value(),
			   progress.progress16(BLACK).value());
      BOOST_CHECK_EQUAL(new_progress.progress16(WHITE).value(),
			   progress.progress16(WHITE).value());
      BOOST_CHECK_EQUAL(new_progress.progress(), 
			   progress.progress());
      BOOST_CHECK(new_progress == progress);
      BOOST_CHECK(new_progress.progress() <= progress.maxProgress());
      BOOST_CHECK(new_progress.progress16().isValid());
      BOOST_CHECK(progress.progress16().isValid());
    }
  }
}

BOOST_AUTO_TEST_CASE(NewProgressTestSymmetry)
{
  using namespace osl;

  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 10 : 200) && (ifs >> file_name) ; i++)
  {
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const auto record=CsaFileMinimal(file_name).load();

    NumEffectState state(record.initialState());
    NewProgress progress(state);
    for (auto m:record.moves) {
      state.makeMove(m);
      progress.update(state, m);

      NumEffectState state_r(state.rotate180());
      NewProgress progress_r(state_r);
      BOOST_CHECK_EQUAL(progress.progress16(BLACK).value(),
			   progress_r.progress16(WHITE).value());
      BOOST_CHECK_EQUAL(progress.progress16(WHITE).value(),
			   progress_r.progress16(BLACK).value());
      BOOST_CHECK_EQUAL(progress.progressAttack(BLACK).value(),
			   progress_r.progressAttack(WHITE).value());
      BOOST_CHECK_EQUAL(progress.progressDefense(WHITE).value(),
			   progress_r.progressDefense(BLACK).value());
    }
  }
}
