#include "openingBookConverter.h"

#include "osl/book/compactBoard.h"
#include "osl/record/record.h"
#include <iostream>

OpeningBookConverter::OpeningBookConverter(const char *filename)
{
  std::ifstream ifs(filename);
  int nStates = osl::book::readInt(ifs);
  states.reserve(nStates);
  for (int i = 0 ; i < nStates; i++)
  {
    int blackWin = osl::book::readInt(ifs);
    int whiteWin = osl::book::readInt(ifs);
    int nMove = osl::book::readInt(ifs);
    int index = osl::book::readInt(ifs);
    states.push_back(OBState(index, nMove, blackWin, whiteWin));
  }
  while (true)
  {
    osl::Move move=osl::Move::makeDirect(osl::book::readInt(ifs));
    int stateIndex=osl::book::readInt(ifs);
    if (!ifs)
      break;
    moves.push_back({move,stateIndex});
  }
}

void
OpeningBookConverter::write(const char* filename)
{
  std::ofstream ofs(filename);
  osl::book::writeInt(ofs, states.size());
  for (auto it = states.begin(); it != states.end(); ++it)
  {
    osl::book::writeInt(ofs, it->getOBMoveIndex());
    osl::book::writeInt(ofs, it->getNOBMove());
    osl::book::writeInt(ofs, it->getBlackWinCount());
    osl::book::writeInt(ofs, it->getWhiteWinCount());
  }
  for (auto it = moves.begin(); it != moves.end(); ++it)
  {
    osl::book::writeInt(ofs, it->move.intValue());
    osl::book::writeInt(ofs, it->stateIndex());
  }
}

void
OpeningBookConverter::writeInNewFormat(std::ofstream& ofs)
{
  std::vector<int> weights(moves.size());
  osl::book::writeInt(ofs, 1);		// version number
  osl::book::writeInt(ofs, states.size());
  osl::book::writeInt(ofs, moves.size());
  osl::book::writeInt(ofs, 0);		// Start state index
  for (auto it = states.begin(); it != states.end(); ++it)
  {
    osl::book::writeInt(ofs, it->getOBMoveIndex());
    osl::book::writeInt(ofs, it->getNOBMove());
    int total_wins = 0;
    std::vector<int> wins;
    wins.reserve(it->getNOBMove());
    for (int i = 0; i < it->getNOBMove(); i++)
    {
      const osl::book::OBMove& move
	= moves.at(i + it->getOBMoveIndex());

      const OBState& state = states.at(move.stateIndex());
      if (move.move.player() == osl::BLACK)
	wins.push_back(state.getBlackWinCount());
      else
	wins.push_back(state.getWhiteWinCount());

      total_wins += wins.at(i);
    }
    for (int i = 0; i < it->getNOBMove(); i++)
    {
      if (total_wins != 0)
	weights.at(i + it->getOBMoveIndex()) = (wins.at(i) * 10000 / total_wins);
      else
	weights.at(i + it->getOBMoveIndex()) = 0;
    }
    osl::book::writeInt(ofs, it->getBlackWinCount());
    osl::book::writeInt(ofs, it->getWhiteWinCount());
  }
  int i = 0;
  for (std::vector<osl::book::OBMove>::const_iterator it = moves.begin();
       it != moves.end(); ++it, ++i)
  {
    osl::book::WMove wmove = {it->move, it->stateIndex(), weights.at(i)};
    ofs << wmove;
  }
}

void
OpeningBookConverter::writeInNewFormat(const char* filename)
{
  std::ofstream ofs(filename);
  writeInNewFormat(ofs);
}

void
OpeningBookConverter::writeInNewEditFormat(const char* filename)
{
  std::ofstream ofs(filename);
  writeInNewFormat(ofs);
  std::vector<osl::SimpleState> simpleStates(states.size());
  std::vector<bool> visited(states.size());
  std::vector<int> toTraceIndex;

  for (unsigned int i = 0; i < states.size(); i++)
    visited[i] = false;

  assert(states.size() >= 1);

  // First entry is assmed to be the start state.
  toTraceIndex.push_back(0);
  simpleStates[0] = osl::SimpleState(osl::HIRATE);
  visited[0] = true;

  while (!toTraceIndex.empty())
  {
    const int index = toTraceIndex.back();
    toTraceIndex.pop_back();
    const OBState& s = states.at(index);
    const int moveIndex = s.getOBMoveIndex();
    for (int i = 0; i < s.getNOBMove(); i++)
    {
      const osl::book::OBMove& m = moves.at(moveIndex + i);
      const int nextState = m.stateIndex();
      if (!visited[nextState])
      {
	toTraceIndex.push_back(nextState);
	osl::NumEffectState newState(simpleStates[index]);
	newState.makeMove(m.move);
	simpleStates[nextState] = newState;
	visited[nextState] = true;
      }
    }
  }
  for (unsigned int i = 0; i < states.size(); i++)
  {
    osl::book::CompactBoard board(simpleStates[i]);
    ofs << board;
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
