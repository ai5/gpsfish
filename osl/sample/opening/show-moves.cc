#include "osl/book/openingBook.h"
#include "osl/record/csaRecord.h"
#include "osl/hashKey.h"
#include "osl/oslConfig.h"
#include <unordered_map>
#include <iostream>

using namespace osl;
using namespace osl::record;
using namespace osl::book;

typedef std::unordered_map<HashKey, WeightedBook::WMoveContainer, std::hash<HashKey>> state_map;
void show(const std::string& filename,
	  const state_map& states, const SimpleState& state)
{
  state_map::const_iterator it = states.find(HashKey(state));
  if (it == states.end())
  {
    std::cout << filename << "\t" << "Not found" << std::endl;
  }
  else
  {
    std::cout << filename;
    const WeightedBook::WMoveContainer &moves = it->second;
    for (size_t j = 0; j < moves.size(); ++j)
    {
      std::cout << "\t" << osl::csa::show(moves[j].move)
		<< "\t" << moves[j].weight;
    }
    std::cout << std::endl;
  }
}
int main(int argc, char **argv)
{
  std::string book_filename = OslConfig::openingBook();
  WeightedBook book(book_filename.c_str());

  state_map states;
  {
    std::vector<int> state_stack;
    state_stack.push_back(book.startState());

    while (!state_stack.empty())
    {
      const int index = state_stack.back();
      state_stack.pop_back();

      const SimpleState state = book.board(index);
      const HashKey key = HashKey(state);
      if (states.find(key) == states.end())
      {
	WeightedBook::WMoveContainer moves = book.moves(index);
	for (size_t i = 0; i < moves.size(); ++i)
	{
	  state_stack.push_back(moves[i].stateIndex());
	}
	states[key] = moves;
      }
    }
  }

  for (int i = 1; i < argc; ++i)
  {
    const std::string filename(argv[i]);
    osl::CsaFileMinimal csa(filename);

    NumEffectState state = csa.initialState();
    auto record_moves = csa.moves();
    if (record_moves.empty() || !(state == SimpleState(HIRATE)))
      show(filename, states, state);
    for (Move move: record_moves) {
      state.makeMove(move);
      show(filename, states, state);
    }      
  }

  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
