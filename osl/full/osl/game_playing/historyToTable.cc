/* historyToTable.cc
 */
#include "osl/game_playing/historyToTable.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/pvHistory.h"
#include "osl/search/hashRejections.h"
#include "osl/hash/hashKeyStack.h"
#include "osl/repetitionCounter.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/container/moveStack.h"
#include "osl/csa.h"
#include <iostream>

const int osl::game_playing::HistoryToTable::LIMIT
= osl::search::SearchTable::HistorySpecialDepth;

void osl::game_playing::
HistoryToTable::adjustDominance(const HashKey& key, 
				search::SimpleHashTable& table,
				int black_win, int white_win,
				const Move& good_move)
{
  const PieceStand black_stand = key.pieceStand();
  const Player turn = key.turn();
  for (Ptype ptype: PieceStand::order)
  {
    if (black_stand.get(ptype))
    {
      // white win dominance
      PieceStand new_stand = black_stand;
      new_stand.sub(ptype);
      HashKey new_key = key;
      new_key.setPieceStand(new_stand);
      SimpleHashRecord *record = table.allocate(new_key, LIMIT);
      if (record)
      {
	const Move record_move = (turn == WHITE) ? good_move : Move::INVALID();
	record->setAbsoluteValue(record_move, white_win, LIMIT);
	record->qrecord.setHistoryValue(record_move, white_win);
      }
    }
    if (black_stand.canAdd(ptype))
    {
      // black win dominance
      // TODO:
      // - 加える持駒があるかどうかのチェックは本当は盤面を見る必要がある
      // - good_move がvalid であることの assert を入れる
      PieceStand new_stand = black_stand;
      new_stand.add(ptype);
      HashKey new_key = key;
      new_key.setPieceStand(new_stand);
      SimpleHashRecord *record = table.allocate(new_key, LIMIT);
      if (record)
      {
	const Move record_move = (turn == BLACK) ? good_move : Move::INVALID();
	record->setAbsoluteValue(record_move, black_win, LIMIT);
	record->qrecord.setHistoryValue(record_move, black_win);
      }
    }
  }
}

void osl::game_playing::
HistoryToTable::adjustTable(const GameState& state, SimpleHashTable& table,
			    int black_win, int draw, int white_win)
{
  const RepetitionCounter& counter = state.counter();
  // 優越関係より千日手が優先
  // 千日手は新しい局面優先
  HashKeyStack history = state.hashHistory(); // copy
  MoveStack move_history = state.moveHistory();
  move_history.push(Move::INVALID());
  assert(move_history.size() == history.size());

  HashKeyStack reverse_history;
  while (! history.empty())
  {
    const HashKey key = history.top();
    history.pop();
    assert(move_history.hasLastMove());
    const Move last_move = move_history.lastMove();
    move_history.pop();

    if (key != HashKey(state.state()))	// keep current state clean
      reverse_history.push(key);

    // set dominance
    adjustDominance(key, table, black_win, white_win, last_move);
  }

  while (! reverse_history.empty())
  {
    // set repetition
    const HashKey key = reverse_history.top();
    reverse_history.pop();

    SimpleHashRecord *record = table.allocate(key, LIMIT);
    const std::pair<Sennichite,int> result = counter.distanceToSennichite(key);
    if (result.first.isDraw())
    {
      record->setAbsoluteValue(Move::INVALID(), draw*result.second, LIMIT);
      record->qrecord.setHistoryValue(draw*result.second);
    }
    else
    {
      assert(result.first.hasWinner());
      const int value = (result.first.winner() == BLACK) ? black_win : white_win;
      record->setAbsoluteValue(Move::INVALID(), value, LIMIT);
      record->qrecord.setHistoryValue(value);
    }
  }
}

void osl::game_playing::
HistoryToTable::setPV(const PVHistory& pv_history, const GameState& gstate, search::SimpleHashTable& table)
{
  const Player Turn = gstate.state().turn();
  NumEffectState state(gstate.initialState());
  MoveStack history = gstate.moveHistory();
  HashKey key(state);
  for (int i=history.size(); i>0; --i) {
    const Move m = history.lastMove(i);
    if (! m.isNormal() || ! state.isValidMove(m)) {
      std::cerr << "setPV failed " << i << " " << m << "\n" << state;
#ifndef NDEBUG
      for (int j=history.size(); j>0; --j)
	std::cerr << history.lastMove(j) << " ";
      std::cerr << std::endl;
#endif
      return;
    }
    const MoveWithComment& pv = pv_history[(history.size()-i) % pv_history.size()];
    if (pv.root == key && state.turn() == Turn && !pv.moves.empty()) {
      if (table.isVerbose()) {
	std::cerr << "setPV " << csa::show(m) << " ";
	for (Move p: pv.moves)
	  std::cerr << csa::show(p);
	std::cerr << "\n";
      }
      if (! pv.move.isNormal() || ! state.isValidMove(pv.move)) 
      {
	std::cerr << "setPV failed (corrupt pv) " << pv.move << "\n";
      }
      else
      {
	NumEffectState state_copy = state;
	state_copy.makeMove(pv.move);
	HashKey cur = key.newHashWithMove(pv.move);
	for (Move move: pv.moves) {
	  SimpleHashRecord *record = table.allocate(cur, 1000);
	  if (record) {
	    if (move == Move::PASS(state_copy.turn()) // pass is allowed here
		|| state_copy.isValidMove(move)) {
	      record->setBestMove(move);
	    }
	    else {
	      std::cerr << "setPV failed (corrupt pv) " << i << " " << move << "\n";
	      break;
	    }
	  }
	  state_copy.makeMove(move);
	  cur = cur.newHashWithMove(move);
	}
      }
    }
    key = key.newHashWithMove(m);
    state.makeMove(m);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
