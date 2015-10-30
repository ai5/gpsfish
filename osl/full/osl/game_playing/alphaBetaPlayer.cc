/* alphaBetaPlayer.cc
 */
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/searchPlayer.tcc"
#include "osl/search/alphaBeta2.h"
#include "osl/search/alphaBeta3.h"
#include "osl/search/alphaBeta4.h"
#include "osl/search/usiProxy.h"
#include "osl/search/simpleHashTable.h"
#include "osl/eval/progressEval.h"
#include "osl/eval/pieceEval.h"
#include <iostream>

#ifndef MINIMAL
osl::game_playing::
AlphaBeta2ProgressEvalPlayer::AlphaBeta2ProgressEvalPlayer()
{
}

osl::game_playing::
AlphaBeta2ProgressEvalPlayer::~AlphaBeta2ProgressEvalPlayer()
{
}

osl::game_playing::ComputerPlayer* osl::game_playing::
AlphaBeta2ProgressEvalPlayer::clone() const 
{
  return cloneIt(*this);
}

const osl::search::MoveWithComment osl::game_playing::
AlphaBeta2ProgressEvalPlayer::searchWithSecondsForThisMove(const GameState& gs, const search::TimeAssigned& org)
{
  const milliseconds consumed
    = setUpTable(gs, pawnValueOfTurn<AlphaBeta2ProgressEval>(gs.state().turn()));
  const search::TimeAssigned msec(adjust(org, consumed));
  searcher.reset();
  try 
  {
    searcher.reset(new AlphaBeta2ProgressEval(gs.state(), *checkmate_ptr, table_ptr.get(),
					      *recorder_ptr));
  }
  catch (std::bad_alloc&)
  {
    std::cerr << "panic. allocation of AlphaBeta2 failed\n";
  }
  return SearchPlayer::search<AlphaBeta2ProgressEval>(gs, msec);
}

bool osl::game_playing::
AlphaBeta2ProgressEvalPlayer::isReasonableMove(const GameState& gs,
				   Move move, int pawn_sacrifice)
{
  setUpTable(gs, pawnValueOfTurn<AlphaBeta2ProgressEval>(gs.state().turn()));
  AlphaBeta2ProgressEval searcher(gs.state(), *checkmate_ptr, table_ptr.get(),
				  *recorder_ptr);  
  return SearchPlayer::isReasonableMoveBySearch(searcher, move, pawn_sacrifice);
}
#endif

/* ------------------------------------------------------------------------- */

osl::game_playing::
AlphaBeta2OpenMidEndingEvalPlayer::AlphaBeta2OpenMidEndingEvalPlayer()
{
}

osl::game_playing::
AlphaBeta2OpenMidEndingEvalPlayer::~AlphaBeta2OpenMidEndingEvalPlayer()
{
}

osl::game_playing::ComputerPlayer* osl::game_playing::
AlphaBeta2OpenMidEndingEvalPlayer::clone() const 
{
  return cloneIt(*this);
}

const osl::search::MoveWithComment osl::game_playing::
AlphaBeta2OpenMidEndingEvalPlayer::searchWithSecondsForThisMove(const GameState& gs, const search::TimeAssigned& org)
{
  const milliseconds consumed = setUpTable(gs, pawnValueOfTurn<AlphaBeta2OpenMidEndingEval>(gs.state().turn()));
  const search::TimeAssigned msec(adjust(org, consumed));
  searcher.reset();
  try 
  {
    searcher.reset(new AlphaBeta2OpenMidEndingEval(gs.state(), *checkmate_ptr, table_ptr.get(),
					  *recorder_ptr));
  }
  catch (std::bad_alloc&)
  {
    std::cerr << "panic. allocation of AlphaBeta2 failed\n";
  }
  return SearchPlayer::search<AlphaBeta2OpenMidEndingEval>(gs, msec);
}

const osl::search::MoveWithComment osl::game_playing::
AlphaBeta2OpenMidEndingEvalPlayer::analyzeWithSeconds(const GameState& gs, const search::TimeAssigned& org,
						      search::AlphaBeta2SharedRoot& out)
{
  const search::MoveWithComment
    result = searchWithSecondsForThisMove(gs, org);
  out = dynamic_cast<AlphaBeta2OpenMidEndingEval&>(*searcher).sharedRootInfo();
  return result;
}

bool osl::game_playing::
AlphaBeta2OpenMidEndingEvalPlayer::isReasonableMove(const GameState& gs,
				   Move move, int pawn_sacrifice)
{
  setUpTable(gs, pawnValueOfTurn<AlphaBeta2OpenMidEndingEval>(gs.state().turn()));
  AlphaBeta2OpenMidEndingEval searcher(gs.state(), *checkmate_ptr, table_ptr.get(),
				  *recorder_ptr);  
  return SearchPlayer::isReasonableMoveBySearch(searcher, move, pawn_sacrifice);
}

/* ------------------------------------------------------------------------- */

#ifndef MINIMAL
osl::game_playing::
AlphaBeta3OpenMidEndingEvalPlayer::AlphaBeta3OpenMidEndingEvalPlayer()
{
}

osl::game_playing::
AlphaBeta3OpenMidEndingEvalPlayer::~AlphaBeta3OpenMidEndingEvalPlayer()
{
}

osl::game_playing::ComputerPlayer* osl::game_playing::
AlphaBeta3OpenMidEndingEvalPlayer::clone() const 
{
  return cloneIt(*this);
}

const osl::search::MoveWithComment osl::game_playing::
AlphaBeta3OpenMidEndingEvalPlayer::searchWithSecondsForThisMove(const GameState& gs, const search::TimeAssigned& org)
{
  const milliseconds consumed = setUpTable(gs, pawnValueOfTurn<AlphaBeta3>(gs.state().turn()));
  const search::TimeAssigned msec(adjust(org, consumed));
  searcher.reset();
  try 
  {
    searcher.reset(new AlphaBeta3(gs.state(), *checkmate_ptr, table_ptr.get(),
				  *recorder_ptr));
  }
  catch (std::bad_alloc&)
  {
    std::cerr << "panic. allocation of AlphaBeta3 failed\n";
  }
  return SearchPlayer::search<AlphaBeta3>(gs, msec);
}

bool osl::game_playing::
AlphaBeta3OpenMidEndingEvalPlayer::isReasonableMove(const GameState& gs,
				   Move move, int pawn_sacrifice)
{
  setUpTable(gs, pawnValueOfTurn<AlphaBeta3>(gs.state().turn()));
  AlphaBeta3 searcher(gs.state(), *checkmate_ptr, table_ptr.get(),
				  *recorder_ptr);  
  return SearchPlayer::isReasonableMoveBySearch(searcher, move, pawn_sacrifice);
}
#endif
/* ------------------------------------------------------------------------- */

osl::game_playing::
AlphaBeta4Player::AlphaBeta4Player()
{
}

osl::game_playing::
AlphaBeta4Player::~AlphaBeta4Player()
{
}

osl::game_playing::ComputerPlayer* osl::game_playing::
AlphaBeta4Player::clone() const 
{
  return cloneIt(*this);
}

const osl::search::MoveWithComment osl::game_playing::
AlphaBeta4Player::searchWithSecondsForThisMove(const GameState& gs, const search::TimeAssigned& org)
{
  const milliseconds consumed = setUpTable(gs, pawnValueOfTurn<AlphaBeta4>(gs.state().turn()));
  const search::TimeAssigned msec(adjust(org, consumed));
  searcher.reset();
  try 
  {
    searcher.reset(new AlphaBeta4(gs.state(), *checkmate_ptr, table_ptr.get(),
				  *recorder_ptr));
  }
  catch (std::bad_alloc&)
  {
    std::cerr << "panic. allocation of AlphaBeta4 failed\n";
  }
  return SearchPlayer::search<AlphaBeta4>(gs, msec);
}

bool osl::game_playing::
AlphaBeta4Player::isReasonableMove(const GameState& gs,
				   Move move, int pawn_sacrifice)
{
  setUpTable(gs, pawnValueOfTurn<AlphaBeta4>(gs.state().turn()));
  AlphaBeta4 searcher(gs.state(), *checkmate_ptr, table_ptr.get(),
				  *recorder_ptr);  
  return SearchPlayer::isReasonableMoveBySearch(searcher, move, pawn_sacrifice);
}

/* ------------------------------------------------------------------------- */

osl::game_playing::
UsiProxyPlayer::UsiProxyPlayer()
{
}

osl::game_playing::
UsiProxyPlayer::~UsiProxyPlayer()
{
}

osl::game_playing::ComputerPlayer* osl::game_playing::
UsiProxyPlayer::clone() const 
{
  return cloneIt(*this);
}

const osl::search::MoveWithComment osl::game_playing::
UsiProxyPlayer::searchWithSecondsForThisMove(const GameState& gs, const search::TimeAssigned& org)
{
  using search::UsiProxy;
  const milliseconds consumed = setUpTable(gs, pawnValueOfTurn<UsiProxy>(gs.state().turn()));
  const search::TimeAssigned msec(adjust(org, consumed));
  searcher.reset();
  try 
  {
    searcher.reset(new UsiProxy(gs.state(), *checkmate_ptr, table_ptr.get(),
				  *recorder_ptr));
  }
  catch (std::bad_alloc&)
  {
    std::cerr << "panic. allocation of UsiProxy failed\n";
  }
  return SearchPlayer::search<UsiProxy>(gs, msec);
}

bool osl::game_playing::
UsiProxyPlayer::isReasonableMove(const GameState& gs,
				   Move move, int pawn_sacrifice)
{
  setUpTable(gs, pawnValueOfTurn<search::UsiProxy>(gs.state().turn()));
  search::UsiProxy searcher(gs.state(), *checkmate_ptr, table_ptr.get(),
			    *recorder_ptr);  
  return SearchPlayer::isReasonableMoveBySearch(searcher, move, pawn_sacrifice);
}

/* ------------------------------------------------------------------------- */

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
