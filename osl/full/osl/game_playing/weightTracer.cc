/* weightTracer.cc
 */
#include "osl/game_playing/weightTracer.h"
#include "osl/game_playing/openingBookTracer.h"
#include "osl/book/openingBook.h"
#include "osl/csa.h"
#include "osl/random.h"
#include <boost/utility.hpp>
#include <iostream>
#include <ctime>
#ifdef _WIN32
#include <cstring>
#endif

osl::game_playing::
WeightTracer::WeightTracer(WeightedBook& b, bool v,
                           const int weight_coef_for_the_initial_move,
                           const int weight_coef)
  : book(b), state_index(b.startState()), start_index(b.startState()),
    turn(BLACK), 
    weight_coef_for_the_initial_move(weight_coef_for_the_initial_move),
    weight_coef(weight_coef)
{
  verbose = v;
}

osl::game_playing::
WeightTracer::WeightTracer(const WeightTracer& copy)
  : OpeningBookTracer(copy),
    book(copy.book), state_index(copy.state_index), 
    start_index(copy.start_index), turn(copy.turn), 
    state_stack(copy.state_stack),
    weight_coef_for_the_initial_move(copy.weight_coef_for_the_initial_move),
    weight_coef(copy.weight_coef)
{
}

osl::game_playing::OpeningBookTracer* osl::game_playing::
WeightTracer::clone() const
{
  return new WeightTracer(*this);
}

void osl::game_playing::
WeightTracer::update(Move move)
{
  if (verbose) {
    std::cerr << "WeightTracer "
	      << " Move: " << csa::show(move) << " ";
    const time_t now = time(0);
    char ctime_buf[64];
    std::cerr << ctime_r(&now, ctime_buf) 
	      << std::endl;
  }

  state_stack.push(state_index);
  assert(move.player() == turn);
  turn = alt(turn);

  if (! isOutOfBook())
  {
    const std::vector<book::WMove>& moves = book.moves(state_index);
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
WeightTracer::popMove()
{
  state_index = state_stack.top();
  state_stack.pop();
  turn = alt(turn);
  if (verbose)
    std::cerr << "WeightTracer " << this << " pop: " << turn << std::endl;
}

bool osl::game_playing::
WeightTracer::isOutOfBook() const
{
  return state_index < 0; 
}

const osl::Move osl::game_playing::
WeightTracer::selectMoveAtRandom(const std::vector<osl::book::WMove>& moves) const
{
  if (verbose)
    std::cerr << "book " << moves.size() << " candidates\n" << std::endl;

  int max_weight_index = 0;
  int max_weight = 0;
  int sum = 0;
  for (size_t i=0; i<moves.size(); ++i) {
    sum += moves[i].weight;
    if (max_weight < moves[i].weight) {
      max_weight = moves[i].weight;
      max_weight_index = i;
    }
  }

  const int random_value = time_seeded_random() % sum;
  if (verbose)
    std::cerr << "random number: " << random_value << std::endl;

  int maxIndex = -1;
  int weight = 0;
  for (size_t index = 0; index < moves.size(); index++)
  {
    weight += moves[index].weight;
    if (random_value <= weight)
    {
      maxIndex = index;
      break;
    }
  }

  if (maxIndex >= 0) {
    if (verbose) {
      const int weight = moves[maxIndex].weight;
      std::cerr << "book " 
		<< 100.0*weight/sum << '%';
      if (weight != max_weight)
	std::cerr << " (c.f. " << 100.0*max_weight/sum
		  << " " << csa::show(moves[max_weight_index].move)
		  << ")";
      std::cerr << std::endl;
    }
    return moves[maxIndex].move;
  }
  return Move::INVALID();
}

const osl::Move osl::game_playing::
WeightTracer::selectMove() const
{
  const std::vector<book::WMove> raw_moves = book.moves(state_index);
  std::vector<book::WMove> moves;

  int max_weight = 0;
  for (size_t i=0; i<raw_moves.size(); ++i) {
    if (max_weight < raw_moves[i].weight) {
      max_weight = raw_moves[i].weight;
    }
  }
  int sum = 0;
  const int coef = ((state_index == start_index) ? 
                    weight_coef_for_the_initial_move : weight_coef);
  for (size_t i=0; i<raw_moves.size(); ++i) {
    if (raw_moves[i].weight*coef < max_weight)
      continue;
    moves.push_back(raw_moves[i]);
    sum += raw_moves[i].weight;
  }

  if (sum == 0)
    return Move::INVALID();
  
  return selectMoveAtRandom(moves);
}


osl::game_playing::OpeningBookTracer* osl::game_playing::
DeterminateWeightTracer::clone() const
{
  return new DeterminateWeightTracer(*this);
}

const osl::Move osl::game_playing::
DeterminateWeightTracer::selectMove() const
{
  std::vector<book::WMove> moves = book.moves(state_index);
  const int original_size = moves.size();
  std::sort(moves.begin(), moves.end(), osl::book::WMoveSort());

  /*
   * Select top_n moves. 
   *   - WMoves with the same weight are included in the result. 
   *   The size of the result vector might be greater than topn.
   *   - Zero-weighed WMoves are exluded. The size of the result vector
   *   might be less than topn.
   */
  int top = topn;
  std::vector<book::WMove>::iterator it = moves.begin();
  for (/*none*/; it != moves.end() && top > 0; ++it) {
    if (it->weight == 0)
      break;

    if (it->weight != boost::next(it)->weight)
      top -= 1;
  }
  moves.erase(it, moves.end());

  if (verbose) {
    std::cerr << "book: there remain " << moves.size() << " candidates of " 
      << original_size << " moves\n";
  }

  if (moves.empty())
    return Move::INVALID();

  return selectMoveAtRandom(moves);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
