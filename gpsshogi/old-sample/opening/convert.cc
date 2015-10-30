#include "editor.h"
#include "converter.h"
#include "osl/record/opening/openingBook.h"
#include "osl/record/compactBoard.h"

#include <iostream>
#include <cstring>

void usage(char *program_name)
{
  std::cerr << "Usage: " << program_name << " -edit PLAIN-FILE DATABASE-FILE\n"
	    << "       " << program_name << " -opening DATABASE-FILE PLAIN-FILE"
	    << std::endl;
}

int main(int argc, char **argv)
{
  if (argc != 4)
  {
    usage(argv[0]);
    return 1;
  }

  bool edit = true;
  if (strcmp(argv[1], "-edit") == 0)
    edit = true;
  else if (strcmp(argv[1], "-opening") == 0)
    edit = false;
  else
  {
    usage(argv[0]);
    return 1;
  }

  if (edit)
  {
    osl::record::opening::WeightedBook book(argv[2]);
    Editor editor(argv[3]);

    int nStates = book.getTotalState();
    for (int i = 0; i < nStates; i++)
    {
      State state(book.getBoard(i), book.getWhiteWinCount(i),
		  book.getBlackWinCount(i), book.getMoves(i));
      editor.addState(state);
    }
  }
  else
  {
    convertToOpening(argv[2], argv[3]);
  }
  return 0;
}
