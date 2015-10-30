#include "editor.h"
#include "osl/record/compactBoard.h"
#include "osl/record/record.h"
#include <stdexcept>
#include <iostream>
#include <sstream>

std::ostream& operator<<(std::ostream& os, const State& s)
{
  os << osl::record::CompactBoard(s.sstate);
  osl::record::writeInt(os, s.getWhiteWinCount());
  osl::record::writeInt(os, s.getBlackWinCount());
  osl::record::writeInt(os, s.next_moves.size());
  for (size_t i = 0; i < s.next_moves.size(); i++)
  {
    os << s.next_moves[i];
  }
  return os;
}

std::istream& operator>>(std::istream& is, State& s)
{
  osl::record::CompactBoard board;
  is >> board;
  s.sstate = board.getState();
  s.whiteWinCount = osl::record::readInt(is);
  s.blackWinCount = osl::record::readInt(is);
  int size = osl::record::readInt(is);
  s.next_moves.reserve(size);
  
  for (int i = 0; i < size; i++)
  {
    osl::record::opening::WMove move;
    is >> move;
    s.next_moves.push_back(move);
  }
  return is;
}

Editor::Editor(const char* filename)
{
  db = dbm_open(const_cast<char *>(filename), O_RDWR | O_CREAT, 00644);
  if (!db)
    throw std::runtime_error("Failed to open DBM");
}

Editor::~Editor()
{
  dbm_close(db);
}

bool Editor::updateState(const State& state, bool update)
{
  std::ostringstream oss(std::ostringstream::out);
  oss << state;
  const std::string& state_string = oss.str();
  osl::record::CompactBoard board(state.getState());
  std::ostringstream oss_key(std::ostringstream::out);
  oss_key << board;
  const std::string& board_string = oss_key.str();

  datum key, val, fetch_val;

  key.dptr = (void *)(board_string.data());
  key.dsize = board_string.length();

  val.dptr = (void *)state_string.data();
  val.dsize = state_string.length();

  fetch_val = dbm_fetch(db, key);
  if (!update && fetch_val.dptr)
    throw KeyAlreadyExistsException();

  return dbm_store(db, key, val, DBM_REPLACE) == 0;
}

bool Editor::addState(const State& state)
{
  return updateState(state, false);
}

bool Editor::setState(const State& state)
{
  return updateState(state, true);
}

State Editor::getState(const osl::SimpleState& sstate)
{
  datum key, val;

  osl::record::CompactBoard board(sstate);
  std::ostringstream oss_key(std::ostringstream::out);
  oss_key << board;

  const std::string& key_string = oss_key.str();
  key.dptr = (void *)key_string.data();
  key.dsize = key_string.length();
  val = dbm_fetch(db, key);

  if (! val.dptr)
    throw KeyNotFoundException();

  std::string str((char *)val.dptr, val.dsize);
  std::istringstream iss(str);
  State state;
  iss >> state;
  return state;
}
