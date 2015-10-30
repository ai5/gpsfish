#include "osl/book/openingBook.h"
#include "osl/book/compactBoard.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

int osl::book::readInt(std::istream& is)
{
  int ret=0;
  CArray<char,4> cs;
  is.read(&cs[0],4);
  for (int i=0;i<4;i++) {
    ret = (ret<<8)|(cs[i]&255);
  }
  return ret;
}
  
void osl::book::writeInt(std::ostream& os, int n)
{
  CArray<char,4> buf;
  for (int i = 0; i < 4; i++)
  {
    buf[i] = (n >> (8 * (4 - i - 1))) & 255;
  }
  os.write(&buf[0], 4);
}

#ifndef MINIMAL
osl::book::
WinCountBook::WinCountBook(const char *filename)
  : ifs(filename, std::ios_base::binary)
{
  if (! ifs)
  {
    const char *message = "WinCountBook: open failed ";
    std::cerr << message << filename << std::endl;
    throw std::runtime_error(std::string(message) + filename);
  }
  nStates=readInt();
}

osl::book::
WinCountBook::~WinCountBook()
{
}

int osl::book::
WinCountBook::readInt()
{
  int ret=0;
  CArray<char,4> cs;
  ifs.read(&cs[0],4);
  for (int i=0;i<4;i++) {
    ret = (ret<<8)|(cs[i]&255);
  }
  return ret;
}

void osl::book::
WinCountBook::seek(int offset)
{
  ifs.seekg(offset,std::ios::beg);
}

std::vector<osl::book::OBMove> osl::book::
WinCountBook::moves(int stateIndex)
{
  assert(stateIndex >= 0);
  seek(4+16*stateIndex+8);
  int nMoves=readInt();
  int moveIndex=readInt();
  seek(4+16*nStates+8*moveIndex);
  std::vector<OBMove> moves;
  moves.reserve(nMoves);
  for(int i=0;i<nMoves;i++)
  {
    Move move=Move::makeDirect(readInt());
    int stateIndex=readInt();
    moves.push_back({move,stateIndex});
  }
  return moves;
}

int osl::book::
WinCountBook::winCount(int stateIndex)
{
  seek(4+16*stateIndex);
  return readInt();
}

int osl::book::
WinCountBook::loseCount(int stateIndex)
{
  seek(4+16*stateIndex+4);
  return readInt();
}

std::ostream& osl::book::operator<<(std::ostream& os, const WMove& w)
{
  writeInt(os, OMove(w.move));
  writeInt(os, w.stateIndex());
  writeInt(os, w.weight);
  return os;
}
#endif

std::istream& osl::book::operator>>(std::istream& is, WMove& w)
{
  w.move = OMove(readInt(is)).operator Move();
  w.state_index = readInt(is);
  w.weight = readInt(is);
  return is;
}

osl::book::
WeightedBook::WeightedBook(const char *filename)
  : ifs(filename, std::ios_base::binary)
{
  if (! ifs)
  {
    const char *message = "WeightedBook: open failed ";
    std::cerr << message << filename << std::endl;
    throw std::runtime_error(std::string(message) + filename);
  }
#ifndef NDEBUG
  int version = 
#endif
    readInt(ifs);
  assert(version == 1);
  n_states = readInt(ifs);
  n_moves = readInt(ifs);
  start_state = readInt(ifs);
}

osl::book::
WeightedBook::~WeightedBook()
{
}

void osl::book::
WeightedBook::seek(int offset)
{
  ifs.seekg(offset,std::ios::beg);
}

osl::book::WeightedBook::WMoveContainer osl::book::
WeightedBook::moves(int stateIndex, const bool visit_zero)
{
  assert(stateIndex >= 0);
  seek(HEADER_SIZE + STATE_SIZE * stateIndex);
  int moveIndex=readInt(ifs);
  int nWMoves=readInt(ifs);
  seek(HEADER_SIZE + STATE_SIZE * n_states + MOVE_SIZE * moveIndex);
  std::vector<WMove> moves;
  moves.reserve(nWMoves);
  for(int i=0;i<nWMoves;i++)
  {
    WMove wm;
    ifs >> wm;
    if (!visit_zero && wm.weight == 0) continue;
    moves.push_back(wm);
  }
  return moves;
}

osl::book::CompactBoard osl::book::
WeightedBook::compactBoard(int stateIndex)
{
  seek(HEADER_SIZE + STATE_SIZE * n_states + MOVE_SIZE * n_moves
       + BOARD_SIZE * stateIndex);
  CompactBoard board;
  ifs >> board;
  return board;
}

osl::SimpleState osl::book::
WeightedBook::board(int stateIndex)
{
  const CompactBoard board = compactBoard(stateIndex);
  return board.state();
}

int osl::book::
WeightedBook::whiteWinCount(int stateIndex)
{
  seek(HEADER_SIZE + STATE_SIZE * stateIndex);
  readInt(ifs);
  readInt(ifs);
  readInt(ifs);
  return readInt(ifs);
}

int osl::book::
WeightedBook::blackWinCount(int stateIndex)
{
  seek(HEADER_SIZE + STATE_SIZE * stateIndex);
  readInt(ifs);
  readInt(ifs);
  return readInt(ifs);
}

void osl::book::
WeightedBook::validate()
{
#ifndef NDEBUG
  {
    SimpleState state(HIRATE);
    SimpleState start = board(start_state);
    assert(state == start);
  }
#endif
  std::vector<char> visited(n_states);
  std::fill(visited.begin(), visited.end(), false);

  std::vector<int> stateToCheck;
  stateToCheck.push_back(start_state);
  visited[start_state] = true;

  while (!stateToCheck.empty())
  {
    const int index = stateToCheck.back();
    stateToCheck.pop_back();
    SimpleState state = board(index);
    for (WMove move: moves(index))
    {
      NumEffectState newState(state);
      newState.makeMove(move.move);
      const int nextIndex = move.stateIndex();

      SimpleState stateInFile = board(nextIndex);
      assert(newState == stateInFile);
      if (!visited[nextIndex])
      {
	stateToCheck.push_back(nextIndex);
	visited[nextIndex] = true;
      }
    }
  }
}

int osl::book::
WeightedBook::stateIndex(const SimpleState& state_to_look_for,
                            const bool visit_zero, 
                            const Player player)
{
  int ret = -1;
  const CompactBoard board_to_look_for(state_to_look_for);
  
  const CompactBoard start_state = compactBoard(startState());
  if (start_state == board_to_look_for)
  {
    ret = startState();
    return ret;
  }

  std::vector<char> states(totalState(), false); // mark states that have been visited.
  std::vector<int> stateToVisit;
  stateToVisit.push_back(startState());

  while (!stateToVisit.empty())
  {
    const int stateIndex = stateToVisit.back();
    stateToVisit.pop_back();
    states[stateIndex] = true;

    WMoveContainer v;
    if (visit_zero)
      v = moves(stateIndex);
    else
    {
      const CompactBoard stateIndexCB = compactBoard(stateIndex);
      const Player turn = stateIndexCB.turn();
      const bool zero_include = turn == player ? false : true;
      v = moves(stateIndex, zero_include);
    }
    for (WMove move: v)
    {
      const int nextIndex = move.stateIndex();
      if (! states[nextIndex])
      {
        const CompactBoard state = compactBoard(nextIndex);
        if (state == board_to_look_for)
        {
          ret = nextIndex;
          return ret;
        }

	stateToVisit.push_back(nextIndex);
      }
    } // each wmove
  } // while loop

  return ret;
}

int osl::book::
WeightedBook::stateIndex(const std::vector<osl::Move>& moves)
{
  int state_index = startState();
  for (Move move: moves)
  {
    const WMoveContainer wmoves = this->moves(state_index);
    WMoveContainer::const_iterator it = wmoves.begin();
    for (; it != wmoves.end(); ++it)
      if (it->move == move) break;
    if (it != wmoves.end())
    {
      state_index = it->stateIndex(); // next state to visit
      continue;
    }
    return -1; // not found
  }
  return state_index;
}


std::vector<int> osl::book::
WeightedBook::parents(const int target_state_index)
{
  std::vector<int> ret;

  if (startState() == target_state_index)
    return ret;
  
  std::vector<char> states(totalState(), false); // mark states that have been visited.
  std::vector<int> stateToVisit;
  stateToVisit.push_back(startState());

  while (!stateToVisit.empty())
  {
    const int stateIndex = stateToVisit.back();
    stateToVisit.pop_back();
    states[stateIndex] = true;

    const WMoveContainer moves = this->moves(stateIndex);
    for (WMove move: moves)
    {
      const int nextIndex = move.stateIndex();

      if (nextIndex == target_state_index)
        ret.push_back(stateIndex);

      if (! states[nextIndex])
	stateToVisit.push_back(nextIndex);
    } // each wmove
  } // while loop

  return ret;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
