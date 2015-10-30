#include "osl/game_playing/winCountTracer.h"
#include "osl/numEffectState.h"
#include "osl/book/openingBook.h"
#include "osl/usi.h"
#include "osl/oslConfig.h"

#include <iostream>

using namespace osl;
using namespace osl::game_playing;
using namespace osl::book;

void printStats(WinCountBook& book, int index)
{
  std::cout << "Win: " << book.winCount(index)
	    << "\t"
	    << "Lose: " << book.loseCount(index)
	    << std::endl;
}

void printNextMoves(WinCountBook& book, WinCountTracer& tracer,
		    NumEffectState* state)
{
  std::cout << "<moves>" << std::endl;
  auto moves = book.moves(tracer.stateIndex());
  if (moves.size() == 0)
    {
      std::cout << "No more moves in the book" << std::endl;
    }

  for (size_t i = 0; i < moves.size(); i++)
    {
      std::cout << "<move>" << std::endl;
      std::cout << psn::show(moves[i].move) << std::endl;
      printStats(book, moves[i].stateIndex());

      if (state != NULL)
	{
	  NumEffectState newState(*state);
	  newState.makeMove(moves[i].move);
	  std::cout << "<board>" << std::endl;
	  std::cout << newState << std::endl;
	  std::cout << "</board>" << std::endl;
	}
      std::cout << "</move>" << std::endl;
    }
  std::cout << "</moves>" << std::endl;
}

int main(int argc, char **argv)
{
  std::string bookFilename = OslConfig::openingBook();
  WinCountBook book(bookFilename.c_str());
  WinCountTracer tracer(book);
  NumEffectState state;

  char *programName = argv[0];
  bool showNextMoves = false;
  bool showBoards = false;
  bool trace = false;
  bool unknownOption = false;

  char c;
  while ((c = getopt(argc, argv, "nst")) != EOF)
    {
      switch(c)
	{
	case 'n':
	  showNextMoves = true;
	  break;
	case 's':
	  showBoards = true;
	  break;
	case 't':
	  trace = true;
	  break;
	default:
	  unknownOption = true;
	}
    }

  argc -= optind;
  argv += optind;

  if (unknownOption)
    {
      std::cerr << "Usage: " << programName << " [-n] [-s] [-t]" << std::endl
		<< "[-n show next moves] "
		<< "[-s show boards] "
		<< "[-t show next moves for every move]"
		<< std::endl;
      return 1;
    }

  std::string line;

  // When in trace mode, show the candidates for the first move, too.
  if (trace)
    {
      printNextMoves(book, tracer, showBoards ? &state : NULL);
    }

  while (!std::getline(std::cin, line).eof())
    {
      Move move = psn::strToMove(line, state);
      tracer.update(move);
      state.makeMove(move);

      if (trace)
	{
	  printNextMoves(book, tracer, showBoards ? &state : NULL);
	}

      if (tracer.isOutOfBook())
	{
	  std::cout << "Out of Book" << std::endl;
	  return 0;
	}
    }

  std::cout << "<total>" << std::endl;
  printStats(book, tracer.stateIndex());
  if (showBoards)
    {
      std::cout << "<board>" << std::endl;
      std::cout << state << std::endl;
      std::cout << "</board>" << std::endl;
    }
  std::cout << "</total>" << std::endl;

  if (showNextMoves && !trace)
    {
      printNextMoves(book, tracer, showBoards ? &state : NULL);
    }

  return 0;
}
