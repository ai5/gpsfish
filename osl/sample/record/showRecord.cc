/*
 * 
 */

#include "osl/numEffectState.h"
#include "osl/record/csaRecord.h"
#include "osl/record/kisen.h"
#include "osl/effect_util/effectUtil.h"

#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace osl;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-N atmost-N-games] [-k kisen_fileName] csa-filenames "
       << endl;
  // kisenファイル と csaファイル を再生
  exit(1);
}

void processKifu (std::vector<Move> const& moves)
{
  NumEffectState state;
  std::cout << state << std::endl;
  for (size_t i=0; i<moves.size (); ++i)
    {
      if (state.inCheck(alt(state.turn())))
      {
	// 自分の手番で相手の王が利きがある => 直前の手が非合法手
	std::cerr << "e"; // state;
	break;
      }
      state.makeMove(moves[i]);
      std::cout << state << std::endl;
    }
  std::cout << state << std::endl;
}

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  bool verbose = false;
  const char *kisen_filename = 0;
  
  extern char *optarg;
  extern int optind;
  char c;
  size_t num_records = 1;
  while ((c = getopt(argc, argv, "N:k:vh")) != EOF)
    {
      switch(c)
	{
	case 'k': kisen_filename = optarg;
	  break;
	case 'N': num_records = atoi(optarg);
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
      size_t record_processed = 0;
      
      //最初は Kisenファイルを処理
      if (kisen_filename)
	{
	  KisenFile kisen_file(kisen_filename);
	  
	  for (size_t i=0; i<kisen_file.size(); i++)
	    {
	      if (++record_processed > num_records)
		break;
	      const auto moves=kisen_file.moves(i);
	      processKifu (moves);
	    }
	}

      //次に CSAファイルを処理
      for (int i=0; i<argc; ++i)
	{
	  if (++record_processed > num_records)
	    break;
	  CsaFile file(argv [i]);
	  const auto moves=file.load().moves();
	  processKifu (moves);
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
