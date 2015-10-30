/*
 * show weights in progress::PtypeOSquare
 */
#include "osl/progress/ptypeoSquare.h"
#include "osl/progress/ptypeRank.h"

#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace osl;
using namespace osl::progress;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-g] [-w weights.txt] "
       << endl;
  // kisenファイル と csaファイル を再生
  exit(1);
}

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  const char *weights_filename = "weights.txt";
  bool use_ptype_rank = false;

  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "gw:vh")) != EOF)
  {
    switch(c)
    {
    case 'g': use_ptype_rank = true;
      break;
    case 'w': weights_filename = optarg;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag)
    usage(program_name);

  try
  {
    nice(20);
    if (use_ptype_rank)
    {
      PtypeRank progress(weights_filename);
      progress.show(std::cout);
    }
    else
    {
      PtypeOSquare progress(weights_filename);
      progress.show(std::cout);
    }
  }

  catch (std::exception& e)
  {
    std::cerr << e.what() << "\n";
    return 1;
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
