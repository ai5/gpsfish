/* analyzer.cc
 */
#include "osl/annotate/analyzer.h"
#include "osl/checkmate/dualDfpn.h"
#include "osl/checkmate/dfpn.h"
#ifdef OSL_DFPN_SMP
#  include "osl/checkmate/dfpnParallel.h"
#endif
#include "osl/threatmate/mlPredictor.h"
#include "osl/effect_util/neighboring8Direct.h"
#include "osl/eval/see.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/gameState.h"

const int checkmate_limit = 1000000/2;

osl::annotate::
Analyzer::~Analyzer()
{
}

osl::annotate::Trivalent osl::annotate:: 
Analyzer::isCheckmate(NumEffectState& state, Move& best_move, bool attack, size_t *node_count)
{
#ifdef OSL_DFPN_SMP
  checkmate::DfpnParallel dfpn;
#else
  checkmate::Dfpn dfpn;
#endif
  checkmate::DfpnTable table(attack ? state.turn() : alt(state.turn()));
  dfpn.setTable(&table);
  const PathEncoding path(state.turn());
  Move test;
  const ProofDisproof pdp
    = attack
    ? dfpn.hasCheckmateMove(state, HashKey(state), path, checkmate_limit, test)
    : dfpn.hasEscapeMove(state, HashKey(state), path, checkmate_limit*2, Move::PASS(alt(state.turn())));
  if (node_count)
    *node_count = dfpn.nodeCount();
  if (pdp.isCheckmateSuccess())
  {
    best_move = test;
    return True;
  }
  return pdp.isFinal() ? False : Unknown;
}



void osl::annotate::
RepetitionAnalyzer::match(AnalysesResult& shared,
			 const NumEffectState& src, const std::vector<Move>& moves,
			 int last_move)
{
  shared.repetition.clear();
  NumEffectState state; // assume initial position
  HashKey key(state);
  std::vector<HashKey> keys;
  for (int i=0; i<=last_move; ++i) {
    Move move = moves[i];
    keys.push_back(key);
    if (! state.isValidMove(move))
      return;
    key = key.newMakeMove(move);
    state.makeMove(move);
  }
  if (HashKey(src) != key)
    return;
  for (size_t i=0; i<keys.size(); ++i) 
    if (keys[i] == key)
      shared.repetition.push_back(i);
}

void osl::annotate::
CheckmateAnalyzer::match(AnalysesResult& shared,
			 const NumEffectState& src, const std::vector<Move>& /*moves*/,
			 int /*last_move*/)
{
  if (! src.inCheck()) 
  {
    shared.checkmate = False;
    return;
  }
  NumEffectState s(src);
  Move dummy;
  shared.checkmate = isCheckmate(s, dummy, false);
}

void osl::annotate::
CheckmateWin::match(AnalysesResult& shared,
		    const NumEffectState& src, const std::vector<Move>& /*moves*/,
		    int /*last_move*/)
{
  if (src.inCheck()) 
  {
    shared.checkmate_win = False;
    return;
  }
  NumEffectState s(src);
  shared.checkmate_win = isCheckmate(s, shared.checkmate_move, true);
}

void osl::annotate:: 
EscapeFromCheck::match(AnalysesResult& shared,
		       const NumEffectState& src, const std::vector<Move>& moves,
		       int last_move)
{
  shared.escape_from_check = matchMain(src, moves, last_move) ? True : False;
}

bool osl::annotate:: 
EscapeFromCheck::matchMain(const NumEffectState& src, const std::vector<Move>& moves,
			   int last_move)
{
  if (last_move < 0) 
    return false;
  if (moves[last_move].ptype() == KING) 
  {
    if (src.hasEffectAt(src.turn(), moves[last_move].from()))
      return true;
    if (moves[last_move].capturePtype() != PTYPE_EMPTY)
    {
      const PtypeO captured = moves[last_move].capturePtypeO();
      if (src.hasEffectIf(captured, moves[last_move].to(),
			      moves[last_move].from()))
	return true;
    }
    return false;
  }
  const PieceMask pin = src.pin(alt(src.turn()));
  if (pin.test(src.pieceAt(moves[last_move].to()).number()))
    return true;
  if (moves[last_move].capturePtype() != PTYPE_EMPTY)
  {
    const PtypeO captured = moves[last_move].capturePtypeO();
    if (src.hasEffectIf(captured, moves[last_move].to(),
			    src.kingSquare(alt(src.turn()))))
      return true;
  }
  return false;
}

void osl::annotate:: 
ThreatmateAnalyzer::match(AnalysesResult& shared,
			  const NumEffectState& src, const std::vector<Move>& moves,
			  int last_move)
{
  if (src.inCheck())
  {
    shared.threatmate = False;
    return;
  }
  NumEffectState s(src);
  s.changeTurn();
  shared.threatmate = isCheckmate(s, shared.threatmate_move, true, &shared.threatmate_node_count);
  threatmate::MlPredictor predictor;
  shared.threatmate_probability = (last_move >= 0) ? predictor.probability(src, moves[last_move]) : 0.0;
}


void osl::annotate::
CheckmateForCapture::match(AnalysesResult& shared,
			   const NumEffectState& src, const std::vector<Move>& history,
			   int last_move)
{
  if (last_move < 0 || shared.escape_from_check == True)
    return;
  const Square last_to = history[last_move].to();
  if (! src.hasEffectAt(src.turn(), last_to))
    return;
  MoveVector all, moves;
  src.generateLegal(all);
  for (Move m: all)
    if (m.to() == last_to)
      moves.push_back(m);
  if (moves.empty())
    return;
  for (Move move: moves)
  {
    DualDfpn dfpn;
    NumEffectState s(src);
    s.makeMove(move);
    Move checkmate_move;
    const bool checkmate
#ifdef OSL_DFPN_SMP
      = dfpn.isWinningStateParallel(checkmate_limit, s, HashKey(s), PathEncoding(s.turn()),
				    checkmate_move, move);
#else
      = dfpn.isWinningState(checkmate_limit, s, HashKey(s), PathEncoding(s.turn()),
			    checkmate_move, move);
#endif
    if (checkmate) 
    { 
      ++shared.checkmate_for_capture.checkmate_count;
      if (See::see(src, move) > 0)
	++shared.checkmate_for_capture.see_plus_checkmate_count;
    }
    else
      ++shared.checkmate_for_capture.safe_count;
  }
}

void osl::annotate::
CheckmateForEscape::match(AnalysesResult& shared,
			  const NumEffectState& src, const std::vector<Move>& history,
			  int last_move)
{
  if (last_move < 0)
    return;
  const Square last_to = history[last_move].to();
  if (! src.inCheck() || src.hasEffectAt(src.turn(), last_to))
    return;
  // 取れない王手
  MoveVector moves;
  src.generateLegal(moves);
  if (moves.empty())
    return;
  for (Move move: moves) 
  {
    // treat chuai as safe
    const Square to = move.to();
    if (src.hasEffectAt(alt(src.turn()), to)
	&& (src.countEffect(src.turn(), to)
	    - (move.isDrop() ? 0 : 1) == 0))
    {
      ++shared.checkmate_for_escape.safe_count;
      continue;
    }

    DualDfpn dfpn;
    NumEffectState s(src);
    s.makeMove(move);
    Move checkmate_move;
    const bool checkmate
#ifdef OSL_DFPN_SMP
      = dfpn.isWinningStateParallel(checkmate_limit, s, HashKey(s), PathEncoding(s.turn()),
				    checkmate_move, move);
#else
      = dfpn.isWinningState(checkmate_limit, s, HashKey(s), PathEncoding(s.turn()),
			    checkmate_move, move);
#endif
    if (checkmate)
      ++shared.checkmate_for_escape.checkmate_count;
    else
      ++shared.checkmate_for_escape.safe_count;
  }
}

bool osl::annotate::
ThreatmateIfMorePieces::suitable(const NumEffectState& state, Piece p)
{
  // 外す対象
  // - 取る側が長い利きで、守備側から利きあり ... 利きが外れると色々おこりやすい
  // - 取られる駒が玉の周囲に利いていて、守備側から利きあり .. 取り返しの影響が大きい
  if (state.hasEffectAt(p.owner(), p.square()))
  {
    if (state.longEffectAt(p.square(), alt(p.owner())).any())
      return false;
    if (Neighboring8Direct::hasEffect
	(state, p.ptypeO(), p.square(), state.kingSquare(p.owner())))
      return false;
  }
  return true;
}

void osl::annotate::
ThreatmateIfMorePieces::match(AnalysesResult& shared,
			      const NumEffectState& src, const std::vector<Move>& /*history*/,
			      int last_move)
{
  if (last_move < 0)
    return;
  if (src.inCheck() || shared.threatmate == True)
    return;

  const PieceMask effected_pieces = src.effectedMask(alt(src.turn())) & src.piecesOnBoard(src.turn());
  for (Ptype ptype: PieceStand::order)
  {
    DualDfpn dfpn;
    if (src.hasPieceOnStand(src.turn(), ptype))
    {
      NumEffectState s(src.emulateHandPiece(src.turn(), alt(src.turn()), ptype));
      s.setTurn(alt(src.turn()));
      
      Move hand_move;
      const bool threatmate
#ifdef OSL_DFPN_SMP
	= dfpn.isWinningStateParallel(checkmate_limit, s, HashKey(s), PathEncoding(s.turn()),
				      hand_move, Move::PASS(alt(s.turn())));
#else
	= dfpn.isWinningState(checkmate_limit, s, HashKey(s), PathEncoding(s.turn()),
			      hand_move, Move::PASS(alt(s.turn())));
#endif
      if (threatmate)
	shared.threatmate_if_more_pieces.hand_ptype.push_back(ptype);
      continue;
    }
    mask_t m = effected_pieces.getMask(Ptype_Table.getIndex(ptype))
      & Ptype_Table.getMaskLow(ptype);
    if (! m.any()) 
      continue;

    Piece p = src.pieceOf(m.takeOneBit());
    while (m.any() && !suitable(src, p))
      p = src.pieceOf(m.takeOneBit());
    if (! suitable(src, p))
      continue;
    assert(p.isOnBoard());
    assert(unpromote(p.ptype()) == ptype);
    NumEffectState s(src.emulateCapture(p, alt(src.turn())));
    s.setTurn(alt(src.turn()));
    if (s.inCheck() || s.inCheck(alt(s.turn())))
      continue;
	    
    Move board_move;
    const bool threatmate
#ifdef OSL_DFPN_SMP
      = dfpn.isWinningStateParallel(checkmate_limit, s, HashKey(s), PathEncoding(s.turn()),
				    board_move, Move::PASS(alt(s.turn())));
#else
      = dfpn.isWinningState(checkmate_limit, s, HashKey(s), PathEncoding(s.turn()),
			    board_move, Move::PASS(alt(s.turn())));
#endif
    if (threatmate)
      shared.threatmate_if_more_pieces.board_ptype.push_back(p);
  }
}

namespace osl
{
  namespace
  {
    MoveWithComment do_search(const NumEffectState& src, int seconds)
    {
      game_playing::AlphaBeta2OpenMidEndingEvalPlayer player;
      player.setNextIterationCoefficient(1.0);
      player.setVerbose(0);
      player.setTableLimit(100000, 200);
  
      game_playing::GameState state(src);
      search::TimeAssigned time(milliseconds(seconds*1000));
      return player.searchWithSecondsForThisMove(state, time);
    }
  }
}


void osl::annotate::
Vision3::match(AnalysesResult& shared,
	       const NumEffectState& src, const std::vector<Move>& history,
	       int last_move)
{
  if (last_move < 0)
    return;
  if (src.inCheck() || shared.threatmate == True
      || shared.checkmate == True || shared.checkmate_win == True
      || shared.escape_from_check == True)
    return;

  // better to takeback?
  search::MoveWithComment response = do_search(src, 1);
  if (! response.move.isNormal()
      || response.move.to() == history[last_move].to())
    return;

  NumEffectState s = src;
  s.changeTurn();

  // get pv after pass
  MoveWithComment pv = do_search(s, 2);
  if (! pv.move.isNormal())
    return;
  if (See::see(s, pv.move) > 0) {
    if (pv.move.from() == history[last_move].to())
      return;
    const Piece p = s.pieceAt(pv.move.to());
    if (p.isPiece()
	&& ! s.hasEffectAt(alt(s.turn()), pv.move.to())
	&& src.effectedChanged(alt(s.turn())).test(p.number()))
      return;
  }
  typedef eval::ml::OpenMidEndingEval eval_t; 
  shared.vision.cur_eval
    = eval_t(s).value() * 200.0/eval_t::captureValue(newPtypeO(WHITE,PAWN));
  if (pv.value*eval::delta(s.turn()) 
      < shared.vision.cur_eval*eval::delta(s.turn())+200
      || abs(shared.vision.cur_eval) >= 1000)
    return;
  shared.vision.eval = pv.value;
  shared.vision.pv.push_back(pv.move);
  for (Move m: pv.moves)
    shared.vision.pv.push_back(m);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
