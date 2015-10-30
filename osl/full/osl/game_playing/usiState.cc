/* usiState.cc
 */
#include "osl/game_playing/usiState.h"
#include "osl/record/ki2.h"
#include "osl/record/kakinoki.h"
#include "osl/record/csaRecord.h"
#include "osl/usi.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
osl::game_playing::
UsiState::UsiState() : aborted(false) 
{
}

osl::game_playing::
UsiState::~UsiState()
{
}

void osl::game_playing::
UsiState::reset(const NumEffectState& i, const std::vector<Move>& m)
{
  initial_state = i;
  moves = m;
  aborted = false;
}

bool osl::game_playing::
UsiState::isSuccessorOf(const UsiState& parent)
{
  return ! aborted && ! parent.aborted
    && initial_state == parent.initial_state
    && moves.size() == parent.moves.size()+1
    && std::equal(parent.moves.begin(), parent.moves.end(), moves.begin());
}

const osl::NumEffectState osl::game_playing::
UsiState::currentState() const
{
  NumEffectState state(initial_state);
  for (Move m: moves)
    state.makeMove(m);
  return state;
}

void osl::game_playing::
UsiState::parseUsi(const std::string& line)
{
  assert(line.find("position") == 0);
  usi::parse(line.substr(8), initial_state, moves);
}

void osl::game_playing::
UsiState::openFile(std::string filename)
{
  boost::algorithm::trim(filename);
  boost::algorithm::trim_left(filename);
  Record record;
#ifndef MINIMAL
  if (boost::algorithm::iends_with(filename, ".ki2"))
  {
    const Ki2File ki2(filename);
    record = ki2.load();
  }
  else if (boost::algorithm::iends_with(filename, ".kif"))
  {
    const KakinokiFile kif(filename);
    record = kif.load();
  }
  else
#endif
  {
    const CsaFile csa(filename.c_str());
    record = csa.load();
  }
  initial_state = record.initialState();
  moves = record.moves();
}

const std::string osl::game_playing::
UsiState::usiString() const
{
  std::string ret;
  ret.reserve(16+90+10+5*moves.size());
  ret = "position ";
  ret += usi::show(initial_state);
  ret += " moves";
  for (Move move: moves) {
    ret += " ";
    ret += usi::show(move);
  }
  return ret;
}

const std::string osl::game_playing::
UsiState::usiBoard() const
{
  std::string ret = "position ";
  ret += usi::show(currentState());
  return ret;
}

void osl::game_playing::
UsiState::parseIgnoreMoves(const std::string& line,
			   MoveVector& ignore_moves) const
{
  assert(line.find("ignore_moves") == 0);
  std::istringstream is(line);
  std::string word;
  is >> word;
  NumEffectState state(currentState());
  ignore_moves.clear();
  while (is >> word) {
    ignore_moves.push_back(usi::strToMove(word, state));
  }
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
