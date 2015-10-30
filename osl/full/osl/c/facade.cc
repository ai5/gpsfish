/* facade.c
 */
#include "osl/c/facade.h"
#include "osl/checkmate/dualDfpn.h"
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/usiState.h"
#include "osl/game_playing/usiResponse.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/csa.h"
#include "osl/usi.h"
#include "osl/record/kanjiPrint.h"
#include <string>
#include <sstream>
#include <cstdio>
#include <cstring>

extern "C" 
void osl_init()
{
  osl::OslConfig::setUp();
}

extern "C" 
int checkmate_attack(const char *state_str, int& limit, char *move)
{
  osl::DualDfpn checkmate;
  osl::Move checkmate_move;
  osl::NumEffectState state(osl::CsaString(state_str).initialState());
  osl::HashKey key(state);
  osl::PathEncoding pe(state.turn());
  const bool win = checkmate.isWinningState(limit, state, key, pe,
					    checkmate_move);
  limit = checkmate.totalNodeCount();
  if (win) {
    const std::string checkmate_move_str = 
      osl::csa::show(checkmate_move);
    sprintf(move, "%s", checkmate_move_str.c_str());
  }
  return win;
}

extern "C" 
int checkmate_escape(const char *state_str, int limit)
{
  osl::DualDfpn checkmate;
  osl::Move checkmate_move;
  osl::NumEffectState state(osl::CsaString(state_str).initialState());
  osl::HashKey key(state);
  osl::PathEncoding pe(state.turn());
  const bool escape = checkmate.isLosingState(limit, state, key, pe);
  return escape;
}
  
extern "C" 
int search(const char *state_str, int seconds, int verbose, char *move)
{
  osl::game_playing::AlphaBeta2OpenMidEndingEvalPlayer player;
  player.setNextIterationCoefficient(1.7);
  player.setVerbose(verbose);
  if (osl::OslConfig::isMemoryLimitEffective()) 
  {
    player.setTableLimit(std::numeric_limits<size_t>::max(), 200);
    player.setNodeLimit(std::numeric_limits<size_t>::max());
  }
  else 
  {
    player.setTableLimit(3000000, 200);
  }
  player.setDepthLimit(2000, 400, 200);

  osl::game_playing::GameState state(osl::CsaString(state_str).initialState());
  osl::Move best_move = player.searchWithSecondsForThisMove(state, osl::search::TimeAssigned(osl::milliseconds(seconds*1000))).move;

  const std::string best_move_str = osl::csa::show(best_move);
  sprintf(move, "%s", best_move_str.c_str());

  const osl::SimpleHashTable *table = player.table();
  const osl::HashKey key(state.state());

  const osl::SimpleHashRecord *record = table->find(key);
  int value = record ? record->lowerBound() : 0;
  return value;
}

extern "C"
int usiMovesToKanji(const char *command, char *out, int out_size)
{
  assert(out_size>0);
  osl::game_playing::UsiState usi_state;
  osl::game_playing::UsiResponse res(usi_state, true, false);
  std::string ret;

  res.hasImmediateResponse(std::string(command), ret);

  const int size = std::min(out_size, static_cast<int>(ret.size()));
  memcpy(out, ret.c_str(), size);
  return size;
}

extern "C"
int usiMovesToPositionString(const char *moves_str, char *out, int out_size)
{
  assert(out_size>0);

  osl::NumEffectState state;
  std::vector<osl::Move> moves;
  std::istringstream is(moves_str);
  std::string s;
  while (is >> s) {
    const osl::Move move = osl::usi::strToMove(s, state);
    moves.push_back(move);
    state.makeMove(move);
  }
  assert(!moves.empty());

  osl::Move last_move;
  if (! moves.empty()) {
    last_move = moves.back();
  }

  std::ostringstream os;
  osl::record::KanjiPrint printer(os, std::make_shared<osl::record::KIFCharacters>());
  printer.print(state, &last_move);

  const std::string ret = os.str();
  const int size = std::min(out_size, static_cast<int>(ret.size()));
  memcpy(out, ret.c_str(), size);
  return size;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
