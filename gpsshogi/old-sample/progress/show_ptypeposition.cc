/*
 * show progress::PtypeOSquare, PtypeRank
 */

#include "osl/progress/progressAnalyzer.h"
#include "osl/progress/ptypeoSquare.h"
#include "osl/progress/ptypeRank.h"
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
  cerr << "Usage: " << prog << " [-g] [-N atmost-N-games] [-w weights.txt] [-S skip] [-k kisenFileName] csa-filenames "
       << endl;
  // kisenファイル と csaファイル を再生
  exit(1);
}

bool verbose = false;

template <class ProgressType>
class StandardProgressAnazyzer : public ProgressAnazyzer
{
  ProgressType progress;
public:
  StandardProgressAnazyzer(bool raw_output, const char *filename)
    : ProgressAnazyzer(raw_output), progress(filename)
  {
    if (verbose)
      std::cerr << "initial " << progress.getVal() << "\n";
  }
  void processRecord(osl::vector<Move> const& moves)
  {
    progress.reset();
    NumEffectState state((SimpleState(HIRATE)));
    for (size_t i=0; i<moves.size(); ++i)
    {
      if (EffectUtil::isKingInCheck(alt(state.turn()), state))
      {
	// 自分の手番で相手の王が利きがある => 直前の手が非合法手
	std::cerr << "e"; // state;
	break;
      }
      progress.apply(moves[i]);
      state.doMove(moves[i]);
      const int cur = progress.getVal();
      if (verbose)
	std::cerr << cur << "\n";
      processMove(cur);
    }
    endRecond(moves.size());
  }
};

template <class ProgressType>
void analyze(bool raw_output, 
	     const char *weights_filename, const char *kisenFilename,
	     int argc, char **argv, size_t skip, size_t num_records)
{
  size_t record_processed = 0;
  StandardProgressAnazyzer<ProgressType> analyzer(raw_output, weights_filename);
      
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

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  const char *kisenFilename = 0;
  const char *weights_filename = "weights.txt";
  bool use_ptype_rank = false;
  bool raw_output = false;

  extern char *optarg;
  extern int optind;
  char c;
  size_t num_records = 1;
  size_t skip = 0;
  while ((c = getopt(argc, argv, "grN:S:k:w:vh")) != EOF)
  {
    switch(c)
    {
    case 'g': use_ptype_rank = true;
      break;
    case 'k': kisenFilename = optarg;
      break;
    case 'N': num_records = atoi(optarg);
      break;
    case 'r': raw_output = true;
      break;
    case 'S': skip = atoi(optarg);
      break;
    case 'w': weights_filename = optarg;
      break;
    case 'v': verbose = true;
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
      analyze<PtypeRank>(raw_output, 
			 weights_filename, kisenFilename, argc, argv,
			 skip, num_records);
    else
      analyze<PtypeOSquare>(raw_output,
			      weights_filename, kisenFilename, argc, argv,
			      skip, num_records);
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
