/* show_repetition.cc
 */
#include "osl/repetitionCounter.h"
#include "osl/record/csaRecord.h"
#include "osl/csa.h"
#include <iostream>
#include <cstdio>

void usage(const char *program_name)
{
}

using namespace osl;

void processRecord(std::vector<Move> const& moves)
{
  NumEffectState state;
  RepetitionCounter counter(state);
  for (size_t i=0; i<moves.size (); ++i)
  {
    std::cout << i+1 << " " << csa::show(moves[i]) << std::endl;
    std::cout << counter.isSennichite(state, moves[i]) << std::endl;
    std::cout << "(" 
	      << counter.isAlmostSennichite(HashKey(state).newHashWithMove(moves[i]))
	      << ")\n";
    state.makeMove(moves[i]);
    counter.push(state);
    const int times = counter.countRepetition(HashKey(state));
    if (times > 1)
    {
      std::cout << times
		<< "-times, first appeared at "
		<< counter.getFirstMove(HashKey(state))
		<< " check " << counter.checkCount(BLACK) 
		<< " " << counter.checkCount(WHITE)
		<< "\n";
    }
    std::cout << "\n";
  }
  std::cout << state << std::endl;
}

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
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
    usage(program_name);

  nice(20);
      
  //次に CSAファイルを処理
  for (int i=0; i<argc; ++i)
  {
    CsaFileMinimal file(argv [i]);
    const auto moves=file.load().moves;
    processRecord(moves);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
