/* show-state.cc
 */

#include "osl/progress/effect5x3.h"
#include "osl/progress.h"
#include "osl/record/csaRecord.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace osl;
using namespace osl::progress;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-a] csa-filename"
       << endl;
  exit(1);
}

void show(const char *filename);

bool show_all_states = false;
int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;

  // extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "at:f:vh")) != EOF)
  {
    switch(c)
    {
    case 'a':	show_all_states = true;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag)
    usage(program_name);

  progress::ml::NewProgress::setUp();
  
  for (int i=0; i<argc; ++i)
  {
    show(argv[i]);
  }
}

void show(const NumEffectState& state)
{
  std::cout << state;
  const int progress_black = Effect5x3::makeProgress(BLACK,state);
  const int progress_white = Effect5x3::makeProgress(WHITE,state);
  std::cout << "black " << progress_black << "\n";
  std::cout << "white " << progress_white << "\n";
  std::cout << "total " << progress_black + progress_white << "\n";
  std::cout << "test " << progress::ml::NewProgress(state).progress16().value() << "\n";
}

void show(const char *filename)
{
  std::cout << filename << "\n";
  CsaFileMinimal file(filename);
  const auto moves = file.load().moves;
  NumEffectState state(file.load().initial_state);
  for (unsigned int i=0; i<moves.size(); i++)
  {
    if (show_all_states)
      show(state);
    const Move m = moves[i];
    state.makeMove(m);
  }
  show(state);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
