#include "osl/numEffectState.h"
#include "osl/record/kisen.h"

#include <iostream>

void usage (const char *program_name)
{
  std::cerr << "Usage: " << program_name << " KISEN-FILE [out]"
	    << std::endl;
  exit(1);
}

void check_all(const char*filename, const char *output)
{
  osl::record::KisenFile kisen(filename);
  std::unique_ptr<std::ofstream> os;
  if (output) {
    os.reset(new std::ofstream(output));
  }

  for (size_t i = 0; i < kisen.size(); i++)
  {
    std::cout << i;
    if ((i % 16) == 15 || i + 1 == kisen.size())
      std::cout << std::endl;
    else
      std::cout << ' ';
    osl::NumEffectState state(kisen.initialState());
    std::vector<osl::Move> moves;
    size_t j = 0;
    try {
      moves = kisen.moves(i);
      for (; j < moves.size(); j++)
      {
	const osl::Square opKingSquare 
	  = state.kingSquare(alt(state.turn()));
	if (state.hasEffectAt(state.turn(), opKingSquare))
	{
	  if (j)
	    --j;
	  break;
	}
	state.makeMove(moves[j]);
      }
      moves.resize(j);
    }
    catch (osl::csa::CsaIOError& e) {
      std::cerr << e.what();
    }
    
    if (os) {
      osl::record::KisenWriter out(*os);
      out.save({kisen.initialState(), moves});
    }
  }
}

int main(int argc, char **argv)
{
  if (! (argc == 2 || argc == 3))
    usage(argv[0]);

  check_all(argv[1], (argc == 3) ? argv[2] : "");

  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
