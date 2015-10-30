/* ptypeProgress.t.cc
 */

#include "osl/progress/ptypeProgress.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::progress;

BOOST_AUTO_TEST_CASE(PtypeProgressTestConsistent)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int max=100;
  if (OslConfig::inUnitTestShort())
      max=10;
  std::string filename;
  int i=0;
  while((ifs >> filename) && (++i<max)) 
  {
    std::string full_filename = OslConfig::testCsaFile(filename);

    auto record=CsaFileMinimal(full_filename).load();
    NumEffectState state(record.initialState());
    const auto& moves=record.moves;
    PtypeProgress incremental(state);
    for (size_t i=0; i<moves.size(); ++i)
    {
      state.makeMove(moves[i]);
      incremental.update(state, moves[i]);
      const PtypeProgress batch(state);
      BOOST_CHECK_EQUAL(incremental, batch);
    }
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
