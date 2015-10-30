/*
 * 
 */

#include "osl/search/sortCaptureMoves.h"
#include "osl/numEffectState.h"
#include "osl/move_generator/allMoves.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/record/csaRecord.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace osl;

int main(int argc, char **argv)
{
  // const char *program_name = argv[0];
  bool error_flag = false;
  bool verbose = false;
  // const char *kisenFilename = 0;
  
  // extern char *optarg;
  extern int optind;
  char c;
  // size_t num_records = 1;
  while ((c = getopt(argc, argv, "vh")) != EOF) {
    switch(c)
    {
    case 'v': verbose = true;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag)
    return 1;

  try {
    nice(20);
    // size_t record_processed = 0;
      
    for (int i=0; i<argc; ++i) {
      CsaFile file(argv [i]);
      const Record record = file.load();
      NumEffectState state(record.initialState());

      MoveVector moves;
      state.generateAllUnsafe(moves);
      search::SortCaptureMoves::sortByTakeBack(state, moves);
      std::cout << state << moves << "\n";
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
