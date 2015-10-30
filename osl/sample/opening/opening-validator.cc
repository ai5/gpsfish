#include "osl/book/openingBook.h"
#include "osl/csa.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceSearch2.tcc"
#include "osl/eval/pieceEval.h"
#include <iostream>
#include <cstring>

using namespace osl::book;


static int qsearch(osl::SimpleState &s, osl::Move lastMove)
{
  typedef osl::search::QuiescenceSearch2<osl::eval::PieceEval> qsearch_t;

  osl::NumEffectState state(s);
  osl::search::SimpleHashTable table(100000, -16, false);
  osl::search::SearchState2Core::checkmate_t checkmate_searcher;
  osl::search::SearchState2Core core(state, checkmate_searcher);
  qsearch_t qs(core, table);
  osl::eval::PieceEval ev(state);
  return qs.search(state.turn(), ev, lastMove);
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " FILENAME" << std::endl;
    return 1;
  }

  WeightedBook book(argv[1]);
  bool states[book.totalState()];

  memset(states, 0, sizeof(bool) * book.totalState());
  std::vector<int> stateToVisit;

  stateToVisit.push_back(book.startState());
  const int threshold = osl::eval::Ptype_Eval_Table.value(osl::SILVER);

  while (!stateToVisit.empty())
  {
    const int stateIndex = stateToVisit.back();
    stateToVisit.pop_back();
    states[stateIndex] = true;

    osl::SimpleState state = book.board(stateIndex);
    int value = qsearch(state,
			osl::Move::PASS(alt(state.turn())));

    const auto moves = book.moves(stateIndex);
    for (size_t i = 0; i < moves.size(); i++)
    {
      const int nextIndex = moves[i].stateIndex();
      osl::SimpleState newState = book.board(nextIndex);
      osl::Move move = moves[i].move;
      int newValue = qsearch(newState, move);

      int diff = newValue - value;

      // the loss too big
      if ((state.turn() == osl::BLACK && diff < -threshold)
	  || (state.turn() == osl::WHITE && diff > threshold))
	std::cout << "----" << std::endl
		  << state << osl::csa::show(move) << std::endl;
      // the gain too big
      if ((state.turn() == osl::BLACK && diff > threshold)
	  || (state.turn() == osl::WHITE && diff < -threshold))
	std::cout << "++++" << std::endl
		  << state << osl::csa::show(move) << std::endl;

      if (! states[nextIndex])
      {
	stateToVisit.push_back(nextIndex);
      }
    }
  }
}
