#ifndef _EDITOR_H
#define _EDITOR_H
#include "osl/stl/vector.h"
#include "osl/record/opening/openingBook.h"
#include <relic.h>

class State
{
public:
  State(osl::SimpleState state,
	int whiteCount, int blackCount,
	osl::vector<osl::record::opening::WMove> moves)
    : whiteWinCount(whiteCount), blackWinCount(blackCount),
    sstate(state), next_moves(moves) {}
  State() {}
  friend std::ostream& operator<<(std::ostream& os, const State& s);
  friend std::istream& operator>>(std::istream& is, State& s);

  int getWhiteWinCount() const { return whiteWinCount; }
  int getBlackWinCount() const { return blackWinCount; }
  const osl::SimpleState getState() const { return sstate; }
  osl::vector<osl::record::opening::WMove> getMoves() const {
    return next_moves;
  }
private:
  int whiteWinCount;
  int blackWinCount;
  osl::SimpleState sstate;
  osl::vector<osl::record::opening::WMove> next_moves;
};

class Editor
{
public:
  explicit Editor(const char* filename);
  ~Editor();
  State getState(const osl::SimpleState& sstate);
  // returns true upon success
  bool addState(const State& state);
  bool setState(const State& state);
private:
  Editor(const Editor&);
  DBM *db;

  bool updateState(const State& state, bool update);
};

class KeyNotFoundException
{
public:
  KeyNotFoundException() {}
};

class KeyAlreadyExistsException
{
public:
  KeyAlreadyExistsException() {}
};

#endif // _EDITOR_H
