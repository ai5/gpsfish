/* usiResponse.cc
 */
#include "osl/game_playing/usiResponse.h"
#include "osl/game_playing/gameState.h"
#include "osl/move_probability/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "osl/rating/featureSet.h"
#include "osl/progress.h"
#include "osl/sennichite.h"
#include "osl/csa.h"
#include "osl/usi.h"
#ifndef MINIMAL
#  include "osl/record/ki2.h"
#endif
#include <sstream>
#include <iostream>

osl::game_playing::
UsiResponse::UsiResponse(const UsiState& u, bool n, bool v)
  : usi_state(u), new_move_probability(n), verbose(v)
{
}
osl::game_playing::
UsiResponse::~UsiResponse()
{
}

osl::MoveVector osl::game_playing::
UsiResponse::generateGoodMoves()
{
  GameState state(usi_state.initial_state);
  for (Move m: usi_state.moves) 
    state.pushMove(m);

  MoveVector normal_or_win_or_draw, loss;
  state.generateNotLosingMoves(normal_or_win_or_draw, loss);
  if (verbose && ! loss.empty()) {
    std::cerr << "  removed losing move ";
    for (Move m: loss)
      std::cerr << csa::show(m);
    std::cerr << "\n";
  }
  return normal_or_win_or_draw;
}

void osl::game_playing::
UsiResponse::genmoveProbability(int limit, MoveLogProbVector& out)
{
  GameState gstate(usi_state.initial_state);
  for (Move m: usi_state.moves) 
    gstate.pushMove(m);
  const NumEffectState& state= gstate.state();
  const MoveStack& history = gstate.moveHistory();
  progress::ml::NewProgress progress(state);
  MoveLogProbVector moves;
  if (new_move_probability) {
    const move_probability::StandardFeatureSet& feature_set
      = move_probability::StandardFeatureSet::instance();
    Move threatmate = move_probability::StateInfo::findShortThreatmate
      (state, history.lastMove());
    move_probability::StateInfo info(state, progress.progress16(),
				     history, threatmate);
    feature_set.generateLogProb(info, moves);
  } else {
    const rating::StandardFeatureSet& feature_set
      = rating::StandardFeatureSet::instance();
    rating::RatingEnv env;
    env.history = history;
    env.make(state, state.pin(state.turn()), state.pin(alt(state.turn())),
	     progress.progress16());
    feature_set.generateLogProb(state, env, limit, moves);
  }
  for (size_t i=1; i<moves.size(); ++i)
    if (moves[i].logProb() <= moves[i-1].logProb() && moves[i-1].logProb()+1<=limit)
      moves[i].setLogProb(moves[i-1].logProb()+1);

  MoveVector good_moves = generateGoodMoves();
  for (MoveLogProb move: moves) 
    if (good_moves.isMember(move.move())) 
      out.push_back(move);
}

void osl::game_playing::
UsiResponse::genmoveProbability(int limit, std::string& out)
{
  MoveLogProbVector moves;
  genmoveProbability(limit, moves);
  out = "genmove_probability";
  for (MoveLogProb move: moves) {
    out += " ";
    out += usi::show(move.move());
    out += " ";
    out += std::to_string(move.logProb());
  }
}

void osl::game_playing::
UsiResponse::genmove(std::string& out)
{
  MoveVector moves = generateGoodMoves();
  out = "genmove";
  for (Move move: moves) {
    out += " ";
    out += usi::show(move);
  }
}

void osl::game_playing::
UsiResponse::csashow(const NumEffectState& state, std::string& out)
{
  std::ostringstream os;
  os << state;
  os << "csashowok";
  out = os.str();
}

void osl::game_playing::
UsiResponse::csamove(const NumEffectState& state, const std::string& str,
		     std::string& out)
{
  const Move move = usi::strToMove(str, state);
  out = "csamove ";
  out += csa::show(move);
}

#ifndef MINIMAL
/**
 * Convert a usi moves string to a ki2 (Kanji) moves string. 
 */
void osl::game_playing::
UsiResponse::ki2moves(const NumEffectState& current,
		      const std::string& moves_str, std::string& out)
{
  NumEffectState state(current);
  std::vector<Move> moves;
  std::istringstream is(moves_str);
  std::string s;
  while (is >> s) {
    const Move move = usi::strToMove(s, state);
    moves.push_back(move);
    state.makeMove(move);
  }
  assert(!moves.empty());
  
  Move last_move;
  if (! usi_state.moves.empty()) {
    last_move = usi_state.moves.back();
  }
  out = "ki2moves ";
  out += ki2::show(&*moves.begin(), &*moves.end(),
			   current, last_move);
}

/**
 * Outputs the number of moves and the last move in the ki2 format.
 */
void osl::game_playing::
UsiResponse::ki2currentinfo(const NumEffectState& current, std::string& out)
{
  Move last_last_move, last_move;
  if (1 <= usi_state.moves.size()) {
    last_move = usi_state.moves.back();
  }
  if (2 <= usi_state.moves.size()) {
    last_last_move = usi_state.moves[usi_state.moves.size()-2];
  }
 
  if (last_move.isValid()) {
    out = "ki2currentinfo ";
    out += std::to_string(usi_state.moves.size());
    out += " ";
    out += ki2::show(last_move, current, last_last_move);
  } else {
    out = "ki2currentinfo";
  }
}
#endif

void osl::game_playing::
UsiResponse::isValidPosition(const std::string& line, std::string& out)
{
  out = "invalid";
  if (line.find("position") == 0) {
    try {
      NumEffectState initial_state;
      std::vector<Move> moves;
      usi::parse(line.substr(8), initial_state, moves);
      out = "valid";
    }
    catch (std::exception& e) {
      // fall through
    }
  }
}

bool osl::game_playing::
UsiResponse::hasImmediateResponse(const std::string& command,
				  std::string& out)
{
  if (command.find("genmove_probability") == 0) {
    int limit = 2000, value;
    std::istringstream is(command.substr(strlen("genmove_probability")));
    if (is >> value)
      limit = value;
    genmoveProbability(limit, out);
    return true;
  }
  if (command.find("genmove") == 0) {
    genmove(out);
    return true;
  }
  const NumEffectState state = usi_state.currentState();
  if (command.find("csashow") == 0) {
    csashow(state, out);
    return true;
  }
  if (command.find("csamove ") == 0) {
    csamove(state, command.substr(8), out);
    return true;
  }
#ifndef MINIMAL
  if (command.find("ki2moves ") == 0) {
    ki2moves(state, command.substr(9), out);
    return true;
  }
  if (command.find("ki2currentinfo") == 0) {
    ki2currentinfo(state, out);
    return true;
  }
#endif
  if (command.find("isvalidposition ") == 0) {
    isValidPosition(command.substr(16), out);
    return true;
  }
  if (command.find("echo ") == 0) {
    out = command.substr(5);
    return true;
  }
  return false;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
