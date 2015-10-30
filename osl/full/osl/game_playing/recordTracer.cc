/* recordTracer.cc
 */
#include "osl/game_playing/recordTracer.h"
#include "osl/record/kisen.h"
#include "osl/csa.h"
#include <iostream>

osl::game_playing::
RecordTracer::RecordTracer(const std::vector<Move>& m, bool v)
  : moves(m), verbose(v)
{
  state_index.push(moves.empty() ? -1 : 0);
  if (verbose && (! moves.empty()))
    std::cerr << "book: expect " << csa::show(moves[0])
	      << "\n";
}

osl::game_playing::
RecordTracer::RecordTracer(const RecordTracer& copy)
  : OpeningBookTracer(copy),
    moves(copy.moves), state_index(copy.state_index), verbose(copy.verbose)
{
}

osl::game_playing::
RecordTracer::~RecordTracer()
{
}

osl::game_playing::OpeningBookTracer* osl::game_playing::
RecordTracer::clone() const
{
  return new RecordTracer(*this);
}

void osl::game_playing::
RecordTracer::update(Move move)
{
  if ((! isOutOfBook())
      && (move == moves.at(stateIndex())))
  {
    const size_t next_index = stateIndex()+1;
    if (next_index < moves.size())
    {
      state_index.push(next_index);
      if (verbose)
	std::cerr << "book: expect " << csa::show(moves[next_index])
		  << "\n";
      return;
    }
  }
  state_index.push(-1);
}

const osl::Move osl::game_playing::
RecordTracer::selectMove() const
{
  if (isOutOfBook())
    return Move::INVALID();
  return moves.at(stateIndex());
}

bool osl::game_playing::
RecordTracer::isOutOfBook() const 
{
  return stateIndex() < 0; 
}
void osl::game_playing::
RecordTracer::popMove()
{
  state_index.pop();
}

const osl::game_playing::RecordTracer osl::game_playing::
RecordTracer::kisenRecord(const char *filename, int id,
			  unsigned int num_moves, bool verbose)
{
  KisenFile kisen(filename);
  std::vector<Move> moves = kisen.moves(id);
  if (moves.size() > num_moves)
    moves.resize(num_moves);
  return RecordTracer(moves, verbose);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
