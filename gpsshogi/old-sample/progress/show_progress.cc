/*
 * show_progress
 */

#include "osl/progress/progressAnalyzer.h"
#include "osl/record/record.h"
#include "osl/record/csa.h"
#include "osl/record/kisen.h"
#include "osl/effectUtil.h"
#include "osl/numEffectState.h"
#include "osl/progress.h"
#include "osl/applyMove.h"

#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace osl;
using namespace osl::progress;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-r] [-N atmost-N-games] [-S skip] [-k kisenFileName] csa-filenames "
       << "-r raw output\n"
       << endl;
  exit(1);
}

class StandardProgressAnazyzer : public ProgressAnazyzer
{
public:
  explicit StandardProgressAnazyzer(bool raw_output) 
    : ProgressAnazyzer(raw_output)
  {
  }
  void processRecord(osl::vector<Move> const& moves)
  {
    NumEffectState state((SimpleState(HIRATE)));
    Progress progress(state);
    for (size_t i=0; i<moves.size(); ++i)
    {
      if (EffectUtil::isKingInCheck(alt(state.turn()), state))
      {
	// 自分の手番で相手の王が利きがある => 直前の手が非合法手
	std::cerr << "e"; // state;
	break;
      }
      ApplyMoveOfTurn::doMove(progress, state, moves[i]);
      state.doMove(moves[i]);
      const int cur = progress.getVal();
      processMove(cur);
    }
    endRecond(moves.size());
  }
};

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  const char *kisenFilename = 0;
  bool error_flag = false;
  bool raw_output=false;
  size_t num_records = 1;
  size_t skip = 0;

  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "N:S:rk:vh")) != EOF)
  {
    switch(c)
    {
    case 'k': kisenFilename = optarg;
      break;
    case 'N': num_records = atoi(optarg);
      break;
    case 'S': skip = atoi(optarg);
      break;
    case 'r': raw_output = true;
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
    size_t record_processed = 0;

    StandardProgressAnazyzer analyzer(raw_output);
      
    //最初は Kisenファイルを処理
    if (kisenFilename)
    {
      KisenFile kisenFile(kisenFilename);
	  
      for (size_t i=skip; i<kisenFile.size(); i++)
      {
	if (++record_processed > num_records)
	  break;
	const vector<Move> moves=kisenFile.getMoves(i);
	analyzer.processRecord(moves);
      }
    }

    //次に CSAファイルを処理
    for (int i=0; i<argc; ++i)
    {
      if (++record_processed > num_records)
	break;
      CsaFile file(argv [i]);
      const vector<Move> moves=file.getRecord().getMoves();

      analyzer.processRecord(moves);
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
