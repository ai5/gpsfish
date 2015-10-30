/* consistencyTest.h
 */
#ifndef TEST_PROGRESS_CONSISTENCYTEST_H
#define TEST_PROGRESS_CONSISTENCYTEST_H

#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <string>
#include <iostream>

template <class Progress>
void consistencyTest()
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
    Progress progress(state);
    
    for (auto m:record.moves)
    {
      state.makeMove(m);
      progress.update(state, m);
      const Progress new_progress(state);
      BOOST_CHECK_EQUAL(new_progress.progress(BLACK), 
			   progress.progress(BLACK));
      BOOST_CHECK_EQUAL(new_progress.progress(WHITE), 
			   progress.progress(WHITE));
      BOOST_CHECK(new_progress.progress16().isValid());
      BOOST_CHECK(progress.progress16().isValid());
    }
  }
}

template <class Progress>
void consistencyTestUpdate()
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
    Progress progress(state);
    
    for (auto m:record.moves)
    {
      state.makeMove(m);
      progress.update(state, m);
      const Progress new_progress(state);
      if (new_progress.progress(BLACK) != progress.progress(BLACK)
	  || new_progress.progress(WHITE) != progress.progress(WHITE)) {
	std::cerr << state << "moves " << i << " " << m << "\n";
	std::cerr << (new_progress.progress(BLACK) != progress.progress(BLACK)
		      ? "BLACK" : "WHITE") << "\n";
      }
      BOOST_CHECK_EQUAL(new_progress.progress(BLACK), 
			   progress.progress(BLACK));
      BOOST_CHECK_EQUAL(new_progress.progress(WHITE), 
			   progress.progress(WHITE));
      BOOST_CHECK(new_progress.progress16().isValid());
      BOOST_CHECK(progress.progress16().isValid());
    }
  }
}


#endif /* TEST_PROGRESS_CONSISTENCYTEST_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
