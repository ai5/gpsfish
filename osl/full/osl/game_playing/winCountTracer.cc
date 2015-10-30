/* winCountTracer.cc
 */
#include "osl/game_playing/winCountTracer.h"
#include "osl/game_playing/openingBookTracer.h"
#include "osl/book/openingBook.h"
#include "osl/random.h"
#include <iostream>

osl::game_playing::
WinCountTracer::WinCountTracer(WinCountBook& b, int r, bool v)
  : book(b), state_index(0), turn(BLACK), randomness(r), verbose(v)
{
  if (randomness < 0)
    randomness = 0;
}

osl::game_playing::
WinCountTracer::WinCountTracer(const WinCountTracer& copy)
  : OpeningBookTracer(copy),
    book(copy.book), state_index(copy.state_index), turn(copy.turn),
    randomness(copy.randomness), verbose(copy.verbose), 
    state_stack(copy.state_stack)
{
}

osl::game_playing::OpeningBookTracer* osl::game_playing::
WinCountTracer::clone() const
{
  return new WinCountTracer(*this);
}

void osl::game_playing::
WinCountTracer::update(Move move)
{
  state_stack.push(state_index);
  assert(move.player() == turn);
  turn = alt(turn);
  if (! isOutOfBook())
  {
    const std::vector<book::OBMove>& moves = book.moves(state_index);
    for (size_t i=0; i<moves.size(); i++)
    {
      if(moves[i].move == move)
      {
	state_index = moves[i].stateIndex();
	if (verbose)
	  std::cerr << "book: " 
		    << state_stack.top() << "->" << state_index << "\n";
	return;
      }
    }
    if (verbose)
      std::cerr << "book: end" << "\n";
  }
  state_index = -1;
}

void osl::game_playing::
WinCountTracer::popMove()
{
  state_index = state_stack.top();
  state_stack.pop();
  turn = alt(turn);
}

bool osl::game_playing::
WinCountTracer::isOutOfBook() const
{
  return state_index < 0; 
}

const osl::Move osl::game_playing::
WinCountTracer::selectMove() const
{
  assert(randomness >= 0);

  int maxWin = -1, maxIndex = -1;
  std::vector<book::OBMove> moves = book.moves(state_index);
  
  for (size_t index=0; index<moves.size(); ++index)
  {
    Move move=moves[index].move;
    const int curIndex = moves[index].stateIndex();
    const int winNum = (book.winCount(curIndex)
			+ (randomness ? (time_seeded_random() % randomness) : 0));
    const int loseNum = (book.loseCount(curIndex)
			+ (randomness ? (time_seeded_random() % randomness) : 0));
    if (verbose)
      std::cerr << move << "," << winNum << "/" << loseNum << std::endl;
    if (turn == BLACK) 
    {
      if (winNum >= maxWin + randomness)
      {
	maxWin=winNum;
	maxIndex=index;
      }
    }
    else 
    {
      if (loseNum >= maxWin + randomness)
      {
	maxWin=winNum;
	maxIndex=index; 
      }
    }
  }
  if (verbose)
    std::cerr << std::endl;

  if (maxIndex >= 0)
    return moves[maxIndex].move;
  return Move::INVALID();
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
