/*
 * 
 */
#include "osl/record/csaRecord.h"
#include "osl/numEffectState.h"
#include "osl/effect_util/effectUtil.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <unistd.h>

using namespace osl;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " csa-filenames "
       << endl;
  // kisenファイル と csaファイル を再生
  exit(1);
}

int main(int argc, char **argv)
{
  try {
    nice(20);
      
    //次に CSAファイルを処理
    for (int i=1; i<argc; ++i) {
      CsaFile file(argv [i]);
      const auto moves=file.load().moves();
      NumEffectState state;
      for(Move m: moves)
	state.makeMove(m);
      std::string new_name = std::string("tmp/")+argv[i];
      std::ofstream os(new_name);
      os << state;
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
