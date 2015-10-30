/* show-sibling.cc
 */
#include "osl/numEffectState.h"
#include "osl/record/csaRecord.h"
#include "osl/record/ki2.h"
#include "osl/record/kakinoki.h"
#include "osl/record/kisen.h"
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
using namespace osl;
using namespace std;
void run(const std::vector<Move>& moves) {
  NumEffectState state;
  for (size_t i=0; i<moves.size(); ++i) {
    const Move selected = moves[i];
    MoveVector all;
    state.generateLegal(all);

    cout << "selected " << csa::show(selected) << " others";
    for (size_t j=0; j<all.size(); ++j) {
      if (all[j] != selected)
	cout << " " << csa::show(all[j]);
    }
    cout << endl;
    
    state.makeMove(selected);
  }
}
int main(int argc, char **argv) {
  for (int i=1; i<argc; ++i) {
    const char *filename = argv[i];
    if (boost::algorithm::iends_with(filename, ".csa")) {
      const CsaFile csa(filename);
      run(csa.moves());
    }
    else if (boost::algorithm::iends_with(filename, ".ki2")) {
      const Ki2File ki2(filename);
      run(ki2.moves());
    }
    else if (boost::algorithm::iends_with(filename, ".kif")
	     && KakinokiFile::isKakinokiFile(filename)) {
      const KakinokiFile kif(filename);
      run(kif.moves());
    }
    else if (boost::algorithm::iends_with(filename, ".kif")) {
      KisenFile kisen(filename);
      for (size_t j=0; j<kisen.size(); ++j)
	run(kisen.moves(j));
    }
    else {
      cerr << "Unknown file type: " << filename << "\n";
      continue;
    }
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
