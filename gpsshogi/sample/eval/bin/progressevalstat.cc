/* progressevalstat.cc
 */
#include "moveData.h"
#include "osl/eval/progressEval.h"
#include "eval/progressEval.h"
#include "osl/record/kisen.h"
#include "osl/oslConfig.h"
#include <boost/foreach.hpp>
#include <algorithm>
#include <iostream>
using namespace std;
int main(int argc, char **argv) {
  if (argc <= 1)
    return 1;
  osl::OslConfig::setUp();
  std::string filename = osl::OslConfig::home()+"/data/progresseval.txt";
  osl::eval::ProgressEval::setUp(filename.c_str());
  gpsshogi::StableProgressEval eval;
  if (! eval.load(filename.c_str())) {
    std::cerr << "load failed\n";
    return 1;
  }
  osl::KisenFile kisen_file(argv[1]);
  for (size_t i=0; i<kisen_file.size(); ++i) {
    osl::NumEffectState state(kisen_file.initialState());
    std::vector<osl::Move> moves = kisen_file.moves(i);

    gpsshogi::MoveData data;
    for (osl::Move move: moves) {
      data.clear();
      eval.features(state, data, 0);      
      std::cerr << data.value << ' ' << data.value-data.diffs.back().second
		<< ' ' << abs(data.diffs.back().second) << "\n";
      state.makeMove(move);
    }
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
