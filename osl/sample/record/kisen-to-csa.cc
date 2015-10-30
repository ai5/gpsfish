#include "osl/numEffectState.h"
#include "osl/record/kisen.h"
#include "osl/record/csaRecord.h"

#include <iostream>
#include <fstream>

void usage (const char *program_name)
{
  std::cerr << "Usage: " << program_name << " KISENFILE INDEX CSAFILE"
	    << std::endl;
  exit(1);
}

void convert(const char*kisen_filename, const char *csa_filename, size_t i)
{
  osl::record::KisenFile kisen(kisen_filename);
  std::ofstream ofs(csa_filename);

  if (i < kisen.size())
  {
    osl::NumEffectState state = kisen.initialState();
    ofs << state;
    const auto& moves = kisen.moves(i);
    for (size_t j = 0; j < moves.size(); ++j)
    {
      ofs << osl::csa::show(moves[j]) << std::endl;
    }
  }
  else
  {
    std::cerr << "Index out of bounds: " << i << std::endl;
  }
}

int main(int argc, char **argv)
{
  if (argc != 4)
    usage(argv[0]);

  convert(argv[1], argv[3], atoi(argv[2]));

  return 0;
}
