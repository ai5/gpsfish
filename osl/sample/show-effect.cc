/* show-effect.cc
 */
#include "osl/effect_util/effectUtil.h"
#include "osl/record/csaRecord.h"
#include <iostream>
#include <cstdio>
using namespace osl;
Square target(5,8);

int main(int argc, char **argv)
{
  // const char *program_name = argv[0];
  bool error_flag = false;
  bool verbose = false;
  
  // extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "vh")) != EOF)
  {
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

  nice(20);
      
  //次に CSAファイルを処理
  for (int i=0; i<argc; ++i)
  {
    CsaFile file(argv [i]);
    NumEffectState state(file.initialState());
    PieceVector v;
    state.findEffect(BLACK, target, v);
    std::cout << v;
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
