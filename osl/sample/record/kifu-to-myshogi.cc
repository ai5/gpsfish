/* kifu-to-myshogi.cc
 */
#include "osl/record/myshogi.h"
#include "osl/record/kakinoki.h"
#include <iostream>
using namespace std;
int main(int argc, char **argv) {
  if (! argv[1] || ! osl::KakinokiFile::isKakinokiFile(argv[1]))
    return 1;
  std::string filename = argv[1];
  const osl::KakinokiFile file(filename);
  const auto record = file.load();
  auto moves = record.moves();
  auto state = record.initialState();
  for (size_t i=0; i<moves.size(); ++i) {
    osl::NumEffectState next(state);
    next.makeMove(moves[i]);
    std::cout << osl::record::myshogi::show(next, moves[i], state, true) << "\n";
    state = next;
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
