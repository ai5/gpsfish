/* quiescenceSearch2.tcc
 */
#ifndef OSL_QUIESCENCESEARCH2_TCC
#define OSL_QUIESCENCESEARCH2_TCC

#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceGenerator.h"
#include "osl/search/quiescenceLog.h"
#include "osl/search/searchTable.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/sortCaptureMoves.h"
#include "osl/search/moveStackRejections.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/hash/hashCollision.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/move_order/captureSort.h"
#include "osl/move_classifier/check_.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/pawnDropCheckmate.h"
#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/effect_util/neighboring8Direct.h"
#include "osl/eval/see.h"
#include "osl/stat/ratio.h"
#include "osl/hash/hashRandom.h"
#include "osl/oslConfig.h"
#include "osl/centering3x3.h"

#ifdef STAT_WIDTH_VS_LIMIT
#  include "osl/stat/average.h"
#  include <iostream>
#endif

#define quiecence_assert(x,m) assert((x) || state.abort(m))

#ifdef RICH_QSEARCH
// 増減1割
#  define QSEARCH_LAST_CHECK_PENALTY
// 5-20%
#  define QSEARCH_PESSIMISTIC_ESCAPE_THREAT
// 8%
#  define QSEARCH_THREATMATE
#endif

#ifdef EXTRA_RICH_QSEARCH
// 2倍
#  define QSEARCH_SET_MINIMUM_MOVES
#endif

// recordがないと意味があまりない
// #define MAKE_PV_IN_QUIESCE2

const int allocate_depth_in_threatmate = 400;

namespace osl
{
  namespace search
  {
    struct QSearch2PrivateTraits
    {
      enum { 
	KnightCaptureDepth = 1, 
	PawnCaptureDepth = 3, 
	FullPromoteDepth = 3,
	UtilizePromotedDepth = 5, 
	AttackPinnedDepth = 6,
      };
      enum { 
	EscapeDepthFromRoot = 1, 
	EscapeFromLastMoveDepthFromRoot = 3, 
	AttackKnightDepthFromRoot = 2,
	AttackGoldSilverDepthFromRoot = 2,
	DropDepthFromRoot = 2,
	AttackMajorPieceDepthFromRoot = 2,
	AdvanceBishopDepthFromRoot = 2,
	AttackKing8DepthFromRoot = 2,
      };
      enum {
	ThreatMateDepthFromRoot = 2,
      };
      enum { 
	/** pass をした時に消費する深さ */
	PassExtraDepth = 4,
      };
      enum {
	/**
	 * 指手が少なければ深さが先でも読む.
	 * "以下４手目以降では6手〜13手程度" (yss)
	 */
#ifdef QSEARCH_SET_MINIMUM_MOVES
	MinimumMoves = 6,
#else
	MinimumMoves = 0,
#endif
      };
    };

    struct QSearch2HelperBase
    {
      int& result;
      int alpha, beta;
      Move last_move;
      QSearch2HelperBase(int& r, int a, int b, Move l)
	: result(r), alpha(a), beta(b), last_move(l)
      {
      }
    };

    template <class QSearch2,Player P>
    struct QSearch2NextMove : public QSearch2HelperBase
    {
      typedef typename QSearch2::eval_t eval_t;
      QSearch2 *searcher;
      eval_t& ev;
      int additional_depth;
      QSearch2NextMove(int& r, QSearch2 *s, int a, int b, eval_t& e, Move l,
		       int additional)
	: QSearch2HelperBase(r, a, b, l), 
	  searcher(s), ev(e), additional_depth(additional)
      {
      }
      void operator()(Square)
      {
	result = (*searcher).template searchInternal<alt(P)>
	  (beta, alpha, ev, last_move, additional_depth, QSearch2::BeforeUpdate);
      }
    };
    template <class QSearch2,Player P>
    struct QSearch2NextTakeBack : QSearch2HelperBase
    {
      typedef typename QSearch2::eval_t eval_t;
      QSearch2 *searcher;
      eval_t& ev;
      QSearch2NextTakeBack(int& r, QSearch2 *s, int a, int b, eval_t& e, Move l)
	: QSearch2HelperBase(r, a, b, l), searcher(s), ev(e)
      {
      }
      void operator()(Square)
      {
	ev.update(searcher->currentState(), last_move);
	result = (*searcher).template takeBackValue<alt(P)>
	  (beta, alpha, ev, last_move);
      }
    };
    template <class QSearch2,Player P>
    struct QSearch2TakeBackOrChase : QSearch2HelperBase
    {
      typedef typename QSearch2::eval_t eval_t;
      QSearch2 *searcher;
      eval_t& ev;
      QSearch2TakeBackOrChase(int& r, QSearch2 *s, int a, int b, eval_t& e, Move l)
	: QSearch2HelperBase(r, a, b, l), searcher(s), ev(e)
      {
      }
      void operator()(Square)
      {
	ev.update(searcher->currentState(), last_move);
	result = (*searcher).template takeBackOrChase<alt(P)>
	  (beta, alpha, ev, last_move);
      }
    };
    template <class Eval, Player P>
    struct QSearch2SafeEscape
    {
      const NumEffectState *state;
      Eval& eval;
      Piece target;
      bool has_safe_escape;
      bool is_invalid;
      Move last_move;
      QSearch2SafeEscape(const NumEffectState *s, Piece t, Eval& e, Move l)
	: state(s), eval(e), target(t), has_safe_escape(false), is_invalid(false), last_move(l)
      {
      }
      void operator()(Square)
      {
	const Player Turn = alt(P);
	assert(state->turn() == Turn);
	if (state->inCheck(alt(Turn)))
	{
	  is_invalid = true;
	  return;
	}
	eval.update(*state, last_move);
	MoveVector dummy;
	has_safe_escape
	  = QuiescenceGenerator<Turn>::escapeByMoveOnly(*state, target, dummy);
      }
    };
    template <bool has_record>
    struct QSearch2Util
    {
      static bool moreMoves(const QuiescenceRecord *)
      {
	return false;
	// TODO: count number of moves outside of record
	// return (record->moves_size() < QSearch2PrivateTraits::MinimumMoves);
      }
    };
  } // namespace search
} // namespace osl


template <class EvalT>
template <osl::Player P> 
int osl::search::QuiescenceSearch2<EvalT>::
searchProbCut(int alpha, int beta, eval_t& ev, Move last_move)
{
  if (alpha != beta) {
    max_depth = QSearchTraits::MaxDepth/2;
    const int pawn_value2 = EvalT::captureValue(newPtypeO(alt(P),PAWN));
    const int margin = pawn_value2/2;
    const int alpha4 = EvalTraits<P>::max(alpha-margin, 
					  base_t::winThreshold(alt(P)));
    assert(EvalTraits<P>::notLessThan(alpha, alpha4));
    const int beta4  = EvalTraits<P>::min(beta +margin,
					  base_t::winThreshold(P));
    assert(EvalTraits<P>::notLessThan(beta4, beta));
    const int val4 = searchInternal<P>(alpha4, beta4, ev, last_move);
    if (EvalTraits<P>::betterThan(val4, beta4))
      return val4 - (beta4-beta);
    if (EvalTraits<P>::betterThan(alpha4, val4))
      return val4 - (alpha4-alpha);
  }
  max_depth = QSearchTraits::MaxDepth;
  return searchInternal<P>(alpha, beta, ev, last_move);
}

template <class EvalT>
template <osl::Player P, osl::Ptype PTYPE> 
inline
bool osl::search::QuiescenceSearch2<EvalT>::
generateAndExamineTakeBack2(MoveVector& moves, 
			    QuiescenceThreat& threat2, 
			    QuiescenceThreat& threat1,
			    int beta1, int beta2, eval_t const& ev)
{
  mask_t pieces = state.state().effectedMask(P).template selectBit<PTYPE>()
    & state.state().piecesOnBoard(alt(P)).getMask(PtypeFuns<PTYPE>::indexNum);
  if (PTYPE == PAWN || PTYPE == LANCE || PTYPE == KNIGHT)
    pieces &= ~state.state().effectedMask(alt(P)).getMask(PtypeFuns<PTYPE>::indexNum);
  while (pieces.any())
  {
    const Piece target = state.state().pieceOf(pieces.takeOneBit()+PtypeFuns<PTYPE>::indexNum*32);
    assert(target.isOnBoardByOwner<alt(P)>());
    assert(moves.empty());
    QuiescenceGenerator<P>::capture1(currentState(), target.square(), moves);
    const bool result
      = examineTakeBack2<P,false,true>(moves, threat2, threat1, 
				       beta1, beta2, ev);
    moves.clear();
    if (result)
      return result;
  }
  return false;
}


template <class EvalT>
template <osl::Player P, osl::Ptype PTYPE, bool has_record> 
inline
bool osl::search::QuiescenceSearch2<EvalT>::
examineCapture(QuiescenceRecord *record,
	       int& cur_val, MoveVector& moves, int& alpha, int beta, 
	       eval_t const& ev, Piece last_move_piece, int additional_depth)
{
  assert(alpha % 2);
  assert(beta % 2);

  {
    moves.clear();
    QuiescenceGenerator<P>::template capture<PTYPE,true>(state.state(), moves, last_move_piece);

    SortCaptureMoves::sortByTakeBack(state.state(), moves);

    return examineMoves<P,has_record,has_record,CAPTURE>
      (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev,
       additional_depth, last_move_piece.square());
  }
  return false;
}

#ifdef STAT_WIDTH_VS_LIMIT
namespace osl 
{
  namespace 
  {
    struct Reporter
    {
      stat::Average average;
      int count;
      Reporter() : count(0) {}
      ~Reporter()
      {
	std::cerr << "QuiescenceSearch2 " << average.getAverage() << std::endl;
      }
      void newRoot() { average.add(count); count=0; }
      void add() { ++count; }
    } _reporter;
  }
}
#endif

template <class EvalT>
template <osl::Player P, bool has_record, bool has_dont_capture, 
	  osl::search::QSearchTraits::MoveType move_type> 
bool osl::search::QuiescenceSearch2<EvalT>::
examineMoves(QuiescenceRecord *record, int& cur_val, 
	     const Move *first, const Move *last, 
	     int& alpha, int beta, eval_t const& ev,
	     int additional_depth, Square dont_capture)
{
  assert(alpha % 2);
  assert(beta % 2);

  assert(EvalTraits<P>::notLessThan(alpha, cur_val));
#if (! defined NDEBUG) && (! defined OSL_SMP)
  const bool in_pv = (alpha != beta);
#endif
  while (first != last)
  {
    const Move move = *first++;
    if (move_type == CHECK)
    {
      if (depth() <= 0
	  && ! move.isCapture() && ! isMajor(move.ptype()))
	continue;
    }

    if (has_dont_capture)
    {
      const Square to = move.to();
      if (to == dont_capture)
	continue;
    }
    assert((move_type == KING_ESCAPE) || (move_type == UNKNOWN)
	   || (! ShouldPromoteCut::canIgnoreAndNotDrop<P>(move)));

    if(MoveStackRejections::probe<P>(state.state(),state.history(),state.curDepth(),move,alpha,state.repetitionCounter().checkCount(alt(P)))){
	continue;
    }
#ifdef QSEARCH_DEBUG
    QuiescenceLog::pushMove(depth(), move, record);
#endif
    const HashKey new_hash = state.currentHash().newHashWithMove(move);
    int result;
    // 千日手確認
    const Sennichite next_sennichite
      = state.repetitionCounter().isAlmostSennichite(new_hash);
    if (next_sennichite.isDraw())
    {
      result = base_t::drawValue();
    }
    else if (next_sennichite.hasWinner())
    {
      result = base_t::winByFoul(next_sennichite.winner());
    }
    else
    {
      eval_t new_ev = ev;
#ifdef STAT_WIDTH_VS_LIMIT
      if (depthFromRoot() == 0)
	_reporter.add();
#endif
      assert(next_sennichite.isNormal());
      typedef QSearch2NextMove<QuiescenceSearch2,P> helper_t;
      if (has_record && alpha != beta
	  && record->bestMove().isNormal()) {
	// cut node
	helper_t helper(result, this, alpha, alpha, new_ev, move, 
			additional_depth);
	state.doUndoMoveOrPass<P,helper_t>(new_hash, move, helper);
	if (EvalTraits<P>::betterThan(result, alpha)) {
	  new_ev = ev;
	  helper_t helper(result, this, alpha, beta, new_ev, move, 
			  additional_depth);
	  state.doUndoMoveOrPass<P,helper_t>(new_hash, move, helper);
	}
      }
      else {
	// pv or all node
	helper_t helper(result, this, alpha, beta, new_ev, move, 
			additional_depth);
	state.doUndoMoveOrPass<P,helper_t>(new_hash, move, helper);
      }
      
      // 打歩詰を厳密に
      if (base_t::isWinValue(P, result) && (! move.isPass())
	  && move_classifier::MoveAdaptor<move_classifier::PawnDropCheckmate<P> >
	  ::isMember(state.state(), move))
      {
	result = base_t::winByFoul(alt(P));
      }
    }
    if (EvalTraits<P>::betterThan(result, cur_val))
    {
      cur_val = result;
      if (EvalTraits<P>::betterThan(result, alpha))
      {
	alpha = result + EvalTraits<P>::delta;
	if (has_record)
	{
	  if (base_t::isWinValue(P, cur_val))
	  {
	    Square king = state.state().kingSquare(alt(P));
	    if (Ptype_Table.hasUnblockableEffect(move.ptypeO(), move.to(), king)) {
	      record->setLowerBound(QSearchTraits::CheckmateSpecialDepth, cur_val, move);
	    }
	    else {
	      // 現在は取り合いの結果詰でも本当に詰とは限らない c.f. 中合い
	      record->setLowerBound(QSearchTraits::MaxDepth, cur_val, move);
	    }
	    assert(EvalTraits<P>::notLessThan(result, beta));
	    return true;
	  }
	  else
	  {
#ifndef OSL_SMP
	    assert((record->lowerDepth() < depth())
		   || EvalTraits<P>::notLessThan(cur_val, record->lowerBound())
		   || in_pv
		   || state.abort(move));
#endif
	    record->setLowerBound(depth(), cur_val, move);
	  }
	}
	if (EvalTraits<P>::notLessThan(result, beta))
	{
	  state.setKillerMove(move);
	  if (! move.isCapture()) 
	  {
	    const int d = depth();
	    state.historyTable().add(move, d*d);
	  }
	  return true;
	}
      }
    }
  }
  return false;
}

namespace osl
{
  namespace search
  {
    inline QuiescenceRecord *qallocate(SimpleHashTable& table,
				       const HashKey& key,
				       int minusDepthFromRoot,
				       SearchState2Core& state)
    {
      assert(minusDepthFromRoot <= 0 || minusDepthFromRoot == allocate_depth_in_threatmate);
      SimpleHashRecord *record 
	= table.allocate(key, minusDepthFromRoot);
      if (record) {
	state.setCurrentRecord(record);
	return &record->qrecord;
      }
      return 0;
    }
  }
}

template <class EvalT>
template <osl::Player P, bool has_record>
inline
int osl::search::QuiescenceSearch2<EvalT>::
staticValue(eval_t const& ev, int alpha, int beta, QuiescenceRecord *record)
{
  const bool in_pv = (alpha != beta);
#ifndef DONT_USE_CHECKMATE
  if (! in_pv) {
    bool in_threatmate = has_record && record->threatmate.maybeThreatmate(P);
    if (! in_threatmate
	&& (state.hasLastRecord(1) && state.lastRecord(1)))
      in_threatmate
	= (state.lastRecord(1)->threatmate().status(P).status() == ThreatmateState::CHECK_AFTER_THREATMATE);
    if (in_threatmate) {
      const int result = ev.value() + base_t::threatmatePenalty(P);
      return result;
    }
  }
  if (in_pv) {
    const Move last_move = state.lastMove();
    const Square king = state.state().kingSquare(P);
    const bool one_hop_prook
      = (last_move.isNormal() && last_move.ptype() == PROOK
	 && (last_move.capturePtype() == GOLD
	     || last_move.capturePtype() == SILVER)
	 && ((king.y() == last_move.to().y()
	      && abs(king.x() - last_move.to().x()) < 3)
	     || (king.x() == last_move.to().x()
		 && abs(king.y() - last_move.to().y()) < 3)));
    if (one_hop_prook && ! has_record) {
      record = qallocate(table, state.currentHash(), allocate_depth_in_threatmate, state);
    }
    if (has_record || record) {
      // try simulation if record exists
      Move checkmate_move=Move::INVALID();
      int threatmate_node = 0;
      if (record && record->threatmate.maybeThreatmate(P)) {
	threatmate_node += 50;
      } else if (one_hop_prook) {
	threatmate_node += 20;
      }
#ifdef QSEARCH_THREATMATE
      else if ((depthFromRoot() < QSearch2PrivateTraits::ThreatMateDepthFromRoot)
	       && state.tryThreatmate())
	threatmate_node += 20;
#endif
      bool lose = state.isThreatmateState<P>
	(record->threatmateNodesLeft(threatmate_node),
	 checkmate_move);
      if (! lose && record->threatmateNodesLeft(2))
	lose = state.isThreatmateStateShort<P>(2, checkmate_move);
      if (lose)
      {      
	const int result = ev.value() + base_t::threatmatePenalty(P);
	assert(checkmate_move.isValid());
	record->threatmate.setThreatmate(P, checkmate_move);
	record->setStaticValue(QuiescenceRecord::EXACT, result,
			       QSearchTraits::CheckmateSpecialDepth);
	assert(result % 2 == 0);
	return result;
      }
    }
  }
#endif
  if (! in_pv && has_record) {
    int static_value, static_value_depth;
    QuiescenceRecord::StaticValueType type;
    if (record->hasStaticValue(static_value, static_value_depth, type)) {
      // upper bound は depth に因らない
      if (EvalTraits<P>::betterThan(alpha, static_value)) {
	assert(static_value % 2 == 0);
	return static_value;
      }
      if (type == QuiescenceRecord::EXACT
	  && (static_value_depth >= depth())) {
	assert(static_value % 2 == 0);
	return static_value;
      }
    }
  }
  Move threatmate_move;
  if (ImmediateCheckmate::hasCheckmateMove<alt(P)>
      (state.state(), threatmate_move))
  {
    const int result = ev.value() + base_t::threatmatePenalty(P);
    if (has_record)
    {
      record->threatmate.setThreatmate(P, threatmate_move);
      record->setStaticValue(QuiescenceRecord::EXACT, result,
			     QSearchTraits::CheckmateSpecialDepth);
    }
    assert(result % 2 == 0);
    return result;
  }
  if (alpha == beta && EvalTraits<P>::betterThan(ev.value(), beta)) {
    // futility pruning of threat
    int expect = ev.value() + ev.captureValue(newPtypeO(P, GOLD));
    Piece threat = state.state().findThreatenedPiece(P);
    if (threat.isPiece())
      expect += ev.captureValue(threat.ptypeO());
    if (EvalTraits<P>::betterThan(expect, beta))
      return expect;
  }
  const int eval_alpha = alpha;
  QuiescenceThreat threat1, threat2;
  const int result = staticValueWithThreat<P>(ev, eval_alpha, threat1, threat2);
  if (has_record)
  {
    record->setStaticValue(EvalTraits<P>::betterThan(eval_alpha, result) 
			   ? QuiescenceRecord::UPPER_BOUND
			   : QuiescenceRecord::EXACT,
			   result, depth(),
			   threat1, threat2);
  }
  assert(result % 2 == 0);
  return result;
}

template <class EvalT>
template <osl::Player P>
int osl::search::QuiescenceSearch2<EvalT>::
searchInternal(int alpha, int beta, eval_t& ev, Move last_move,
	       int additional_depth, EvalUpdateState need_eval_update)
{
#ifdef STAT_WIDTH_VS_LIMIT
  if (depthFromRoot() == 0)
    _reporter.newRoot();
#endif
#ifdef QSEARCH_DEBUG
  if (depthFromRoot() == 0)
    QuiescenceLog::enter(state.state());
#endif
#ifdef MAKE_PV_IN_QUIESCE2
  state.initPV();
#endif
  ++node_count;
  assert(alpha % 2);
  assert(beta % 2);
  quiecence_assert(EvalTraits<P>::notLessThan(beta, alpha), last_move);
  assert(EvalTraits<P>::notLessThan(alpha, base_t::winThreshold(alt(P))));
  assert(EvalTraits<P>::notLessThan(base_t::winThreshold(P), beta));
  assert(last_move.player() == alt(P));
  
  // 自殺手を手生成でフィルタすると遅くなるので動かしてからチェック
  if (state.state().inCheck(alt(P)))
    return base_t::winByFoul(P);

  assert(state.hasLastRecord());
  QuiescenceRecord *record
    = qallocate(table, state.currentHash(), depth()-QSearchTraits::MaxDepth,
		state);
  const QuiescenceRecord *parent 
    = (state.hasLastRecord(1) && state.lastRecord(1)) 
    ? &(state.lastRecord(1)->qrecord) : 0;
  const bool near_checkmate = parent 
    && (parent->threatmate.maybeThreatmate(alt(P))
	|| parent->threatmate.mayHaveCheckmate(P)
	|| (parent->threatmate.status(P).status()
	    == ThreatmateState::CHECK_AFTER_THREATMATE));
  if (! record && near_checkmate)
  {
    const int depth = (alpha != beta) ? allocate_depth_in_threatmate : 0;
    record
      = qallocate(table, state.currentHash(), depth, state);
  }
  int result;
  if (! record) {
    result = searchMain<P,false>(0, alpha, beta, ev, last_move,
				 additional_depth, need_eval_update);
    if (near_checkmate) {
      if ((EvalTraits<P>::betterThan(alpha, result)
	   && parent->threatmate.maybeThreatmate(alt(P)))
	  || (EvalTraits<P>::betterThan(result, alpha)
	  && parent->threatmate.status(P).status()
	   == ThreatmateState::CHECK_AFTER_THREATMATE)) {
	record
	  = qallocate(table, state.currentHash(), allocate_depth_in_threatmate, state);
      }
    }
  }
  if (record)
  {
    const bool is_king_in_check = state.state().inCheck();
    record->updateThreatmate(P, (parent ? &(parent->threatmate) : 0), 
			     is_king_in_check);
    result = searchMain<P,true>(record, alpha, beta, ev, last_move,
				additional_depth, need_eval_update);
#ifdef MAKE_PV_IN_QUIESCE2
    if ((alpha != beta) && EvalTraits<P>::betterThan(result, alpha)) 
      state.makePV(record->bestMove());
#endif
  }
#ifdef QSEARCH_DEBUG
  QuiescenceLog::node(depth(), alpha, beta, result);
#endif
  assert(result % 2 == 0);
  return result;
}

template <class EvalT>
int osl::search::QuiescenceSearch2<EvalT>::
currentValueWithLastThreat(eval_t const& ev, Piece last_move_piece)
{
  int static_value = ev.value();
  if (! (depthFromRoot() < QSearch2PrivateTraits::EscapeFromLastMoveDepthFromRoot))
    return static_value;

  assert(last_move_piece.isPiece());
  const Player P = last_move_piece.owner();
  PieceVector targets;
  const Square from = last_move_piece.square();
  EffectUtil::findThreat<EvalT>(state.state(), from, last_move_piece.ptypeO(),
				targets);
  if (targets.empty())
    return static_value;
  if (targets[0].ptype() == KING)
  {
    if (targets.size() < 2)
      return static_value;
    // 王手の両取り
    int threat = eval_t::captureValue(targets[1].ptypeO());
    if (state.state().hasEffectAt(alt(P), targets[1].square()))
      threat += eval_t::captureValue(last_move_piece.ptypeO());
    assert(eval::betterThan(P, threat, 0));
    return static_value + threat;
  }
  int first_threat = eval_t::captureValue(targets[0].ptypeO());
  if (state.state().hasEffectAt(alt(P), targets[0].square()))
    first_threat += eval_t::captureValue(last_move_piece.ptypeO());
  assert(eval::betterThan(P, first_threat, 0));
  first_threat /= SecondThreat;
  if (targets.size() < 2)
    return static_value + (first_threat & (~0x1));

  int second_threat = eval_t::captureValue(targets[1].ptypeO());
  if (state.state().hasEffectAt(alt(P), targets[1].square()))
    second_threat += eval_t::captureValue(last_move_piece.ptypeO());
  assert(eval::betterThan(P, second_threat, 0));
  return static_value + ((first_threat + second_threat) & (~0x1));
}

template <class EvalT>
template <osl::Player P>
int osl::search::QuiescenceSearch2<EvalT>::
passValue(int alpha, int beta, eval_t const& ev)
{
  // TODO:
  // - pass pass を許すならloop確認
  static_assert(QSearch2PrivateTraits::EscapeDepthFromRoot <= 2, "");
  const Move pass = Move::PASS(P);
  int result;
  typedef QSearch2NextMove<QuiescenceSearch2,P> helper_t;
  helper_t helper(result, this, alpha, beta, ev, pass, 0);
  const HashKey new_hash = state.currentHash().newHashWithMove(pass);

  max_depth -= QSearch2PrivateTraits::PassExtraDepth;
  state.doUndoMoveOrPass<P,helper_t>(new_hash, pass, helper);
  max_depth += QSearch2PrivateTraits::PassExtraDepth;
  
  return result;
}

template <class EvalT>
template <osl::Player P, bool has_record>
int osl::search::QuiescenceSearch2<EvalT>::
searchMain(QuiescenceRecord *record, 
	   int alpha, int beta, eval_t& ev, Move last_move,
	   int additional_depth, EvalUpdateState& need_eval_update)
{
  const bool in_pv = (alpha != beta);
#if (! defined NDEBUG) && (! defined OSL_USE_RACE_DETECTOR)
  static stat::Ratio ratio("QSearch2: cut");
#endif
  assert((! has_record) || record);
  assert(alpha % 2);
  assert(beta % 2);
  assert((last_move == state.lastMove()) 
	 || ! last_move.isNormal() || ! state.lastMove().isNormal());
#if (!defined OSL_USE_RACE_DETECTOR) && (!defined MINIMAL)
  state.depth_node_count_quiesce[state.curDepth()]++;
#endif
#ifndef DONT_USE_CHECKMATE
  const int node_count_before = node_count;
#endif
  const Square last_to = last_move.to();
  int cur_val = base_t::winByCheckmate(alt(P));
  if (has_record)
  {
    if ((! in_pv && record->lowerDepth() >= depth())
	|| record->lowerDepth() >= QSearchTraits::HistorySpecialDepth)
    {
      if (EvalTraits<P>::notLessThan(record->lowerBound(), cur_val))
      {
	cur_val = record->lowerBound();
	if (EvalTraits<P>::betterThan(record->lowerBound(), alpha))
	{
	  alpha  = record->lowerBound() + EvalTraits<P>::delta;
	  if (EvalTraits<P>::betterThan(record->lowerBound(), beta))
	  {
#if (! defined NDEBUG) && (! defined OSL_USE_RACE_DETECTOR)
	    ratio.add(true);
#endif
	    return record->lowerBound();
	  }
	}
      }
    }
#ifndef DONT_USE_CHECKMATE
    assert(record);
    // try simulation if record exists
    if (in_pv) {
      Move checkmate_move=Move::INVALID();
      if (need_eval_update == BeforeUpdate) {
	ev.update(state.state(), last_move);
	need_eval_update = AfterUpdate;
      }
      bool win = state.isWinningState<P>
	(0, checkmate_move);
      if (! win && record->threatmate.mayHaveCheckmate(alt(P))) {
	win = state.isWinningStateShort<P>(2, checkmate_move);
      }
      if (win) {      
	const int result = base_t::winByCheckmate(P);
	assert(checkmate_move.isValid());
	assert(state.state().isValidMove(checkmate_move));
	record->setLowerBound(QSearchTraits::CheckmateSpecialDepth, 
			      result, checkmate_move);
	return result;
      }
    }
#endif
    if ((! in_pv && record->upperDepth() >= depth())
	|| record->upperDepth() >= QSearchTraits::HistorySpecialDepth)
    {
      if (EvalTraits<P>::betterThan(beta, record->upperBound()))
      {
	beta = record->upperBound() - EvalTraits<P>::delta;
	if (EvalTraits<P>::betterThan(alpha, record->upperBound()))
	{
#if (! defined NDEBUG) && (! defined OSL_USE_RACE_DETECTOR)
	  ratio.add(true);
#endif
	  return record->upperBound();
	}
      }
    }
#if (! defined NDEBUG) && (! defined OSL_USE_RACE_DETECTOR)
    ratio.add(false);
#endif
  }
  if (need_eval_update == BeforeUpdate) {
    ev.update(state.state(), last_move);
    need_eval_update = AfterUpdate;
  }
  const bool is_king_in_check = state.state().inCheck();
  MoveVector moves;
  if (is_king_in_check)
  {
    if (last_move.isNormal() && last_move.isCapture()
	&& unpromote(last_move.capturePtype()) == ROOK
	&& unpromote(last_move.ptype()) != ROOK)
      ++additional_depth;
    else     
      // 王手でのtakebackの応酬の延長
      if (state.lastMove(2).isNormal() // hasLastMove(3)を兼ねる
	  && state.lastMove(3).isNormal()
	  && state.lastMove(4).isNormal()
	  && state.lastMove(2).to() == last_move.to()
	  && state.lastMove(3).to() == last_move.to()
	  && state.lastMove(4).to() == last_move.to())
	++additional_depth;
    // 王手がかかっている時はきちんと逃げる
    {
      QuiescenceGenerator<P>::escapeKing(state.state(), moves);
      // GenerateEscape は玉で取る手が逃げる手より後回しになることがあるので
      move_order::CaptureSort::sort(moves.begin(), moves.end());
      examineMoves<P,has_record,false,KING_ESCAPE>
	(record, cur_val, &*moves.begin(), &*moves.end(),alpha, beta, ev,
	 additional_depth);
    }
    if (has_record)
    {
      if (EvalTraits<P>::betterThan(beta, cur_val))
	record->setUpperBound(depth(), cur_val);
    }
    return cur_val;
  }
  assert(! is_king_in_check);
  King8Info king_info(state.state().Iking8Info(alt(P)));
  PieceMask pins = state.state().pin(alt(P));
  Move checkmate_move=Move::INVALID();
  Square kingSquare=state.state().template kingSquare<alt(P)>();
  if ((depth() <= 4)
      ? ImmediateCheckmate::hasCheckmateMove<P>(state.state(), king_info,kingSquare,checkmate_move)
      : state.isWinningStateShort<P>(2, checkmate_move)) {
    const int result = base_t::winByCheckmate(P);
    assert(checkmate_move.isValid());
    if(has_record)
    {
      assert(state.state().isValidMove(checkmate_move));
      record->setLowerBound(QSearchTraits::CheckmateSpecialDepth, 
			    result, checkmate_move);
    }
    return result;
  }
  // assert(ev.value() == eval_t(state.state()).value()); // !!! SLOW !!!

  if (depth() <= 0 && has_record) {
    if (record->threatmate.maybeThreatmate(P))
      return ev.value() + base_t::threatmatePenalty(P);
    if (record->threatmate.mayHaveCheckmate(alt(P)))
      return ev.value() + base_t::threatmatePenalty(alt(P));
  }

  const Ptype last_capture = last_move.isNormal() 
    ? last_move.capturePtype() : PTYPE_EMPTY;
  const Ptype last_ptype = last_move.isNormal() 
    ? last_move.ptype() : PTYPE_EMPTY;
  const bool king_escape = (last_ptype == KING) && last_capture == PTYPE_EMPTY;

  if ((depth() + std::min(additional_depth,2) <= -2)
      || (depth() <= 0 && additional_depth == 0)
      || (! king_escape
	  && ((depth() + additional_depth <= 0)
	      || (depth() <= 0 && last_capture != PTYPE_EMPTY 
		  && last_ptype != KING))))
  {
    if (ev.progress16().value() == 15
	&& state.tryThreatmate()
	&& ImmediateCheckmate::hasCheckmateMove<alt(P)>(state.state()))
      return ev.value() + base_t::threatmatePenalty(P);
    const int base = takeBackValue<P>(alpha, beta, ev, last_move);
#ifdef QSEARCH_LAST_CHECK_PENALTY
    if ((last_move.ptype() == KING)
	&& (last_capture == PTYPE_EMPTY))
    {
      // 最後の手が王手回避 => ペナルティ
      // 銀くらい取れそう
      return base+eval_t::captureValue(newPtypeO(alt(P), KNIGHT));
    }
#endif
    return base;
  }
  // 王手でなければパスを認めるので最初に静的評価値を求める
  const int static_value
    = ((depth() <= 0)
       ? takeBackValue<P>(alpha, beta, ev, last_move)
#ifdef QSEARCH_REAL_PASS
       : ((depthFromRoot() < QSearch2PrivateTraits::EscapeDepthFromRoot)
	  && (! last_move.isPass()))
       ? passValue<P>(alpha, beta, ev)
#endif
       : staticValue<P,has_record>(ev, alpha, beta, record))
#ifndef MINIMAL
    + (OslConfig::evalRandom() ? HashRandom::value(state.currentHash()): 0)
#endif
    ;
  assert(static_value % 2 == 0);
  assert(! isWinValue(alt(P), static_value));
#ifdef QSEARCH_DEBUG
  QuiescenceLog::staticValue(depth(), static_value);
#endif
  if (EvalTraits<P>::notLessThan(static_value, cur_val))
  {
    cur_val = static_value;
    if (EvalTraits<P>::betterThan(cur_val, alpha))
    {
      alpha = cur_val + EvalTraits<P>::delta;
      if (has_record)
      {
#ifndef OSL_SMP
	assert((record->lowerDepth() < depth())
	       || EvalTraits<P>::notLessThan(cur_val, record->lowerBound())
	       || in_pv);
#endif
	record->setLowerBound(depth(), cur_val, record->bestMove());
      }
      if (EvalTraits<P>::betterThan(cur_val, beta))
	return cur_val;
    }
  }

  // 反映済のはず
  assert(alpha == EvalTraits<P>::max(alpha, cur_val + EvalTraits<P>::delta));
  assert(EvalTraits<P>::notLessThan(beta, alpha));

  Piece last_capture_piece = Piece::EMPTY();
  if (! has_record)
  {
    state.getBigramKillerMoves(moves);
    if (examineMoves<P,has_record,false,UNKNOWN>
	(record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev,
	 additional_depth))
      return cur_val;
    moves.clear();
  }
  else		
  {
    // has_record
    // best move
    const Move bestmove_in_record=record->bestMove();
    if (bestmove_in_record.isNormal())
    {
      assert(state.state().template isAlmostValidMove<true>(bestmove_in_record));
      assert(moves.empty());
      moves.push_back(bestmove_in_record);
      if (examineMoves<P,has_record,false,UNKNOWN>
	  (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev,
	   additional_depth))
	return cur_val;
      moves.clear();
      last_capture_piece = state.state().pieceOnBoard(bestmove_in_record.to());
    }
    // killer moves
    MoveVector killer_moves;
    state.getBigramKillerMoves(killer_moves);
    assert(moves.empty());
    MoveVector record_moves;
    record->loadMoves(record_moves);
    for (Move move: killer_moves)
    {
      if (std::find(record_moves.begin(), record_moves.end(), move)
	  == record_moves.end())
	moves.push_back(move);
    }
    record->addKillerMoves(moves);
    
    if (examineMoves<P,has_record,false,UNKNOWN>
	(record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev,
	 additional_depth))
      return cur_val;
    // already generated moves
    if (examineMoves<P,has_record,false,UNKNOWN>
	(record, cur_val, &*record_moves.begin(), &*record_moves.end(), alpha, beta, ev, additional_depth))
      return cur_val;
    moves.clear();
  }

  // TakeBack 優先
  assert(moves.empty());
  if (((! has_record) || record->movesEmpty())
      && (! last_to.isPieceStand()))
  {
    last_capture_piece = state.state().pieceOnBoard(last_to);
    QuiescenceGenerator<P>::capture(state.state(), 
					   last_capture_piece.square(), moves);
    if (examineMoves<P,has_record,false,CAPTURE>
	(record, cur_val, &*moves.begin(), &*moves.end(),alpha, beta, ev,
	 additional_depth))
    {
      if (has_record)
      {
	// 後で重複してしまうが記録してくれないと困るので
	record->addKillerMoves(moves);
      }
      return cur_val;
    }
  }

  // 詰めろ防止
  const bool has_threatmate
    = has_record 
    && record->threatmate.isThreatmate(P)
#ifdef OSL_SMP
    && record->threatmate.threatmateMove(P).isNormal()
#endif
    ;
  if (has_threatmate)
  {
    moves.clear();
    QuiescenceGenerator<P>::breakThreatmate
      (state.state(), record->threatmate.threatmateMove(P), pins, moves);
    if (examineMoves<P,has_record,false,KING_ESCAPE>
	(record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev,
	 additional_depth))
      return cur_val;
  }

  // 取る手
  if (examineCapture<P,ROOK,has_record>
      (record, cur_val, moves, alpha, beta, ev, last_capture_piece, additional_depth))
    return cur_val;
  if (examineCapture<P,BISHOP,has_record>
      (record, cur_val, moves, alpha, beta, ev, last_capture_piece, additional_depth))
    return cur_val;
  if (examineCapture<P,GOLD,has_record>
      (record, cur_val, moves, alpha, beta, ev, last_capture_piece, additional_depth))
    return cur_val;
  if (examineCapture<P,SILVER,has_record>
      (record, cur_val, moves, alpha, beta, ev, last_capture_piece, additional_depth))
    return cur_val;
  if ((depth() >= QSearch2PrivateTraits::KnightCaptureDepth)
      || max_depth <= 2
      || QSearch2Util<has_record>::moreMoves(record))
  {
    if (examineCapture<P,KNIGHT,has_record>
	(record, cur_val, moves, alpha, beta, ev, last_capture_piece, additional_depth))
      return cur_val;
    if (examineCapture<P,LANCE,has_record>
	(record, cur_val, moves, alpha, beta, ev, last_capture_piece, additional_depth))
      return cur_val;
  }
  // suggested move by evaluation function
  {
    const Move suggested = ev.suggestMove(state.state());
    if (suggested.isNormal()) {
      assert(state.state().isAlmostValidMove(suggested));
      moves.clear();
      moves.push_back(suggested);
      if (examineMoves<P,has_record,false,UNKNOWN>
	  (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev,
	   additional_depth)) {
	return cur_val;
      }
    }
  }
  // 王手
  const Move last2_move = state.lastMove(2);
  if ((depth() > 2
       || (depth() > 0 && !(has_record && record->threatmate.maybeThreatmate(P)))
       || (last2_move.isNormal() && last2_move.capturePtype() == ROOK))
      && ! (! in_pv && has_record && record->threatmate.maybeThreatmate(P)))
  {
    moves.clear();

    if (has_record)
      QuiescenceGenerator<P>::check(state.state(), pins,
				    (king_info.liberty() == 0),
				    record->sendOffSquare<P>(state.state()), 
				    moves);
    else
      QuiescenceGenerator<P>::check(state.state(), pins, moves,
				    (king_info.liberty() == 0));
    if (examineMoves<P,has_record,false,CHECK>
	(record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev,
	 additional_depth))
      return cur_val;
  }

  const Square my_king = state.state().template kingSquare<P>();

  if (depth() <= 0)
    goto finish;
  // futility pruning
  if (! in_pv
      && EvalTraits<P>::betterThan(alpha, ev.value() + (200+350+50*depth())*ev.captureValue(newPtypeO(alt(P),PAWN))/200))
    goto king_walk;
  if ((depth() >= QSearch2PrivateTraits::AttackPinnedDepth)
      || QSearch2Util<has_record>::moreMoves(record))
  {
    {
      moves.clear();
      QuiescenceGenerator<P>::attackToPinned(state.state(), pins, moves);
      if (examineMoves<P,has_record,false,ATTACK>
	  (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev,
	   additional_depth))
	return cur_val;
    }
  }

  if ((depthFromRoot() < QSearch2PrivateTraits::DropDepthFromRoot)
      || (isMajorBasic(last2_move.capturePtype())
	  && ((depthFromRoot() < QSearch2PrivateTraits::DropDepthFromRoot+2)
	      || (has_record && record->movesSizeLessThan(4))
	    ))
      || QSearch2Util<has_record>::moreMoves(record))
  {
    {
      moves.clear();
      QuiescenceGenerator<P>::dropMajorPiece3(state.state(), moves, state.historyTable());
      if (last_move.isNormal() && isPiece(last_move.capturePtype())
	  && unpromote(last_move.capturePtype())== BISHOP
	  && last2_move.isNormal() && last2_move.capturePtype() == BISHOP
	  && unpromote(last2_move.ptype()) == BISHOP) // 合わせた角を取った
      {
	const Square drop_again = last2_move.from();
	if (! state.state().template hasEffectAt<alt(P)>(drop_again)
	    // これ以降は多分常にtrue
	    && state.state().pieceOnBoard(drop_again) == Piece::EMPTY()
	    && state.state().template hasPieceOnStand<BISHOP>(P))
	  moves.push_back(Move(drop_again, BISHOP, P));
      }

      if (examineMoves<P,has_record,true,OTHER>
	  (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev,
	   additional_depth))
	return cur_val;
    }
  }
  if ((depth() >= QSearch2PrivateTraits::PawnCaptureDepth)
      || max_depth <= 2
      || QSearch2Util<has_record>::moreMoves(record))
  {
    if (examineCapture<P,PAWN,has_record>
	(record, cur_val, moves, alpha, beta, ev, last_capture_piece, additional_depth))
      return cur_val;
  }
  if ((depth() >= QSearch2PrivateTraits::FullPromoteDepth)
      || max_depth <= 2)
  {
    moves.clear();
    QuiescenceGenerator<P>::promote(state.state(), pins, moves);
    if (examineMoves<P,has_record,false,PROMOTE>
	(record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
      return cur_val;
  }
  else
  {
    moves.clear();
    QuiescenceGenerator<P>::template promoteN<ROOK,2>(state.state(), moves, state.historyTable());
    QuiescenceGenerator<P>::template promoteN<BISHOP,2>(state.state(), moves, state.historyTable());
    if (examineMoves<P,has_record,false,PROMOTE>
	(record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
      return cur_val;
    moves.clear();
    QuiescenceGenerator<P>::template promoteN<PAWN,2>(state.state(), moves, state.historyTable());
    QuiescenceGenerator<P>::template promoteN<LANCE,1>(state.state(), moves, state.historyTable());
    QuiescenceGenerator<P>::template promoteN<KNIGHT,1>(state.state(), moves, state.historyTable());
    if (examineMoves<P,has_record,false,PROMOTE>
	(record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
      return cur_val;
  }
  if (depthFromRoot() < QSearch2PrivateTraits::EscapeDepthFromRoot)
  {
    {
      moves.clear();
      QuiescenceGenerator<P>::escapeAll(state.state(), moves);
      if (examineMoves<P,has_record,false,ESCAPE>
	  (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
	return cur_val;
    }
  }
  else if ((depthFromRoot() < QSearch2PrivateTraits::EscapeFromLastMoveDepthFromRoot)
	   || (last_move.isDrop() && isMajorBasic(last_move.ptype()))
	   || (last_move.isNormal() && last2_move.isNormal() 
	       && isMajor(last2_move.ptype())
	       && state.state().hasEffectByPiece
	       (state.state().pieceOnBoard(last_to), last2_move.to()))
	   || QSearch2Util<has_record>::moreMoves(record))
  {
    {
      moves.clear();
      QuiescenceGenerator<P>::template escapeFromLastMove<EvalT>(state.state(), last_move, moves);
      if (examineMoves<P,has_record,false,ESCAPE>
	  (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
	return cur_val;
    }
  }
  if ((depthFromRoot() < QSearch2PrivateTraits::AttackMajorPieceDepthFromRoot)
      || (isMajor(last_move.ptype())
	  && last_move.isCapture()
	  && last_to.template canPromote<P>())
      || (state.state().hasEffectAt(P, last_to)
	  && (state.state().
	      template hasEffectByPtype<ROOK>(alt(P), last_to)))
      || ((depthFromRoot() < QSearch2PrivateTraits::AttackMajorPieceDepthFromRoot+2)
	  && ((last2_move.capturePtype()==KNIGHT)
	      || (last2_move.capturePtype()==LANCE)
	      || (last2_move.capturePtype()==BISHOP)))
      || QSearch2Util<has_record>::moreMoves(record))
  {
    {
      moves.clear();
      QuiescenceGenerator<P>::attackMajorPiece(state.state(), pins, moves);
      if (examineMoves<P,has_record,true,ATTACK>
	  (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
	return cur_val;
    }
  }
  {
    const QuiescenceRecord *parent 
      = (state.hasLastRecord(1) && state.lastRecord(1)) 
      ? &(state.lastRecord(1)->qrecord) : 0;
    if ((depthFromRoot() < QSearch2PrivateTraits::AttackKing8DepthFromRoot)
	|| (last_move.isNormal() && last_move.ptype() == KING
	    && last_move.isCapture())
	|| (((parent && parent->threatmate.isThreatmate(alt(P)))
	     || (king_info.liberty() == 0))
	    && depthFromRoot() < 2+QSearch2PrivateTraits::AttackKing8DepthFromRoot)
	|| QSearch2Util<has_record>::moreMoves(record))
    {
      {
	moves.clear();
	QuiescenceGenerator<P>::attackKing8(state.state(), pins, moves);
	if (examineMoves<P,has_record,false,ATTACK>
	    (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
	  return cur_val;
      }
    }
  }
  if ((depthFromRoot() < QSearch2PrivateTraits::AttackGoldSilverDepthFromRoot)
      || QSearch2Util<has_record>::moreMoves(record))
  {
    {
      moves.clear();
      QuiescenceGenerator<P>::attackGoldWithPawn(state.state(), pins, moves);
      QuiescenceGenerator<P>::attackSilverWithPawn(state.state(), pins, moves);
      if (examineMoves<P,has_record,false,ATTACK>
	  (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
	return cur_val;
    }
  }
  if ((depthFromRoot() < QSearch2PrivateTraits::AttackKnightDepthFromRoot)
      || QSearch2Util<has_record>::moreMoves(record))
  {
    {
      moves.clear();
      QuiescenceGenerator<P>::attackKnightWithPawn(state.state(), pins, moves);
      if (examineMoves<P,has_record,false,ATTACK>
	  (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
	return cur_val;
    }
  }
  if ((depth() >= QSearch2PrivateTraits::UtilizePromotedDepth)
      || QSearch2Util<has_record>::moreMoves(record))
  {
    if (last2_move.isNormal())
    {
      const Piece last_piece = state.state().pieceOnBoard(last2_move.to());
      assert(last_piece.isPiece());
      if (last_piece.owner() == P)
      {
	moves.clear();
	QuiescenceGenerator<P>::utilizePromoted(state.state(), last_piece, moves);
	if (examineMoves<P,has_record,true,OTHER>
	    (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
	  return cur_val;
      }
    }
  }
  
  if ((depthFromRoot() < QSearch2PrivateTraits::AdvanceBishopDepthFromRoot)
      || QSearch2Util<has_record>::moreMoves(record))
  {
    {
      moves.clear();
      QuiescenceGenerator<P>::advanceBishop(state.state(), moves);
      if (examineMoves<P,has_record,true,OTHER>
	  (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
	return cur_val;
    }
  }
king_walk:
  if (has_threatmate
      || (! my_king.template canPromote<alt(P)>() //自陣以外
	  && last2_move.isNormal() && last2_move.ptype() == KING))
  {				
    {
      moves.clear();
      QuiescenceGenerator<P>::kingWalk(state.state(), moves);
      if (examineMoves<P,has_record,true,OTHER>
	  (record, cur_val, &*moves.begin(), &*moves.end(), alpha, beta, ev, additional_depth))
	return cur_val;
    }
  }
finish:
  // cut しなかった
  assert(EvalTraits<P>::betterThan(beta, cur_val));
  assert(! isWinValue(alt(P), cur_val));
#ifndef DONT_USE_CHECKMATE
  const bool threatmate
    = EvalTraits<P>::betterThan(ev.captureValue(newPtypeO(P,KING)), cur_val);
  int check_after_threatmate = 0;
  if (in_pv 
      && (threatmate
	  || (check_after_threatmate = state.countCheckAfterThreatmate(alt(P),2))))
  {
    // test sudden checkmate
    int checkmate_nodes = (node_count - node_count_before)/2;
    if (check_after_threatmate)
    {
      if (depthFromRoot() == 1) 
      {
	const int sacrifice = state.countCheckAfterThreatmateSacrifice(alt(P),2);
	checkmate_nodes = std::max(checkmate_nodes, 
				   sacrifice*125+check_after_threatmate*50);
      }
      else 
      {
	checkmate_nodes = std::max(50, checkmate_nodes);
      }
    }
    if (threatmate)
    {
      if (! has_record)
	record = qallocate(table, state.currentHash(), allocate_depth_in_threatmate, state);
      checkmate_nodes = std::max(checkmate_nodes, 200);
    }
    Move check_move;
    const bool win = (record && checkmate_nodes >= 50)
      ? state.isWinningState<P>(record->checkmateNodesLeft(checkmate_nodes), 
				checkmate_move)
      : (((record && record->checkmateNodesLeft(2))
	  || (! has_record && threatmate))
	 ? state.isWinningStateShort<P>(2, checkmate_move)
	 : false);
    if (win)
    {
      const int result = base_t::winByCheckmate(P);
      assert(checkmate_move.isValid());
      if (! has_record && ! record)
	record = qallocate(table, state.currentHash(), allocate_depth_in_threatmate, state);
      if (has_record || record) {
	assert(state.state().isValidMove(checkmate_move));	
	record->setLowerBound(QSearchTraits::CheckmateSpecialDepth, 
			      result, checkmate_move);
      }
      return result;
    }
  }
#if 0
  // cache がないと重いのでとりあえずoff
  // しょうがないので詰将棋 (シミュレーションのみ) を呼ぶ
  if (! has_record)
  {
    assert(! record);
    Move checkmate_move=Move::INVALID();
    AttackOracleAges oracle_age_dummy;
    const bool win_found 	// TODO: last move のみとどちらが良い?
      = state.isWinningState<P>
      (0, checkmate_move, oracle_age_dummy);
    if (win_found)
    {
      const int result = base_t::winByCheckmate(P);
      assert(checkmate_move.isValid());
      record = qallocate(table, state.currentHash(), allocate_depth_in_threatmate, state);
      if (record)
      {
	record->setLowerBound(QSearchTraits::CheckmateSpecialDepth, 
			      result, checkmate_move);
      }
      return result;
    }
  }
#endif
#endif

  if (has_record)
  {
    if (EvalTraits<P>::betterThan(beta, cur_val))
      record->setUpperBound(depth(), cur_val);
  }
  return cur_val;
}

namespace osl
{  
  inline bool importantMove(const NumEffectState& state, Move move,
			    Square my_king, Square op_king)
  {
    if (move.ptype() == KING)
      return true;
    my_king = Centering3x3::adjustCenter(my_king);
    op_king = Centering3x3::adjustCenter(op_king);
    if (Neighboring8Direct::hasEffect(state, move.ptypeO(), move.to(), op_king)
	|| Neighboring8Direct::hasEffect(state, move.ptypeO(), move.to(), my_king))
      return true;
    return move.isCapture()
      && ((! move.isDrop() && state.longEffectAt(move.from()).any())
	  || Neighboring8Direct::hasEffect(state, move.capturePtypeO(), move.to(), my_king)
	  || Neighboring8Direct::hasEffect(state, move.capturePtypeO(), move.to(), op_king));
  }
}

template <class EvalT>
template <osl::Player P>
bool osl::search::QuiescenceSearch2<EvalT>::
examineTakeBack(const MoveVector& moves,
		int& cur_val, int& alpha, int beta, eval_t const& ev)
{
  assert(alpha % 2);
  assert(beta % 2);
  assert(EvalTraits<P>::betterThan(alpha, cur_val));
  assert(EvalTraits<P>::notLessThan(beta, alpha));

  const Square my_king = state.state().template kingSquare<P>();
  const Square op_king = state.state().template kingSquare<alt(P)>();
  
  for (Move move: moves)
  {
#ifdef QSEARCH_DEBUG
    QuiescenceLog::pushMove(depth(), move, 0);
#endif
    const int see = See::see(state.state(), move, state.state().pin(P), state.state().pin(alt(P)), &eval_t::Piece_Value);
    int result;
    if (see > 0 && importantMove(state.state(), move, my_king, op_king)) 
    {
      eval_t new_ev = ev;
      // TODO: futility pruning here
      typedef QSearch2NextTakeBack<QuiescenceSearch2,P> helper_t;
      helper_t helper(result, this, alpha, beta, new_ev, move);
      // 表を使わないのでcast
      state.doUndoMoveLight<P,helper_t>(move, helper);
    }
    else 
    {
      result = ev.value() + see*eval_t::seeScale()*EvalTraits<P>::delta;
    }
    // 安い順にsortしたので一直線に読む 王手回避はする
    if (! base_t::isWinValue(alt(P), result))
    {
      cur_val = EvalTraits<P>::max(cur_val, result);
      return EvalTraits<P>::notLessThan(result, beta);
    }
  }
  assert(EvalTraits<P>::betterThan(beta, cur_val));
  return false;
}

template <class EvalT>
template <osl::Player P, bool calm_move_only, bool first_normal_move_only>
bool osl::search::QuiescenceSearch2<EvalT>::
examineTakeBack2(const MoveVector& moves, 
		 QuiescenceThreat& threat2, QuiescenceThreat& threat1, 
		 int beta1, int beta2, eval_t const& ev)
{
  if (moves.empty())
    return false;
  // P は取る側
  using move_classifier::Check;
  using move_classifier::MoveAdaptor;
  assert(beta1 % 2);
  assert(beta2 % 2);
  assert(EvalTraits<P>::notLessThan(threat1.value, threat2.value));	// threat1 >= threat2
  assert(EvalTraits<P>::betterThan(beta1, threat1.value)); // beta1 > threat1
  assert(EvalTraits<P>::betterThan(beta2, threat2.value)); // beta2 > threat2
  assert(EvalTraits<P>::notLessThan(beta1, beta2)); // beta1 >= beta2
  assert(state.state().turn() == P);

  const Square my_king = state.state().template kingSquare<P>();
  const Square op_king = state.state().template kingSquare<alt(P)>();

  int best_value = threat2.value;
  for (Move move: moves)
  {
    const Square to = move.to();
    assert(! ShouldPromoteCut::canIgnoreAndNotDrop<P>(move));
    if (calm_move_only 
	&& (state.state().countEffect(alt(P),to) > state.state().countEffect(P,to)))
      continue;
#ifdef QSEARCH_DEBUG
    QuiescenceLog::pushMove(depth(), move, 0);
#endif
    int result;
    const int see = See::see(state.state(), move, state.state().pin(P), state.state().pin(alt(P)), &eval_t::Piece_Value);
    if (see > 0 && importantMove(state.state(), move, my_king, op_king)) 
    {
      eval_t new_ev = ev;
      // TODO: futility pruning here
      const int beta = EvalTraits<P>::notLessThan(threat1.value, beta2) ? beta2 : beta1;
      typedef QSearch2NextTakeBack<QuiescenceSearch2,P> helper_t;
      helper_t helper(result, this, 
		      threat2.value+EvalTraits<P>::delta, beta, new_ev, move);
      state.doUndoMoveLight<P,helper_t>(move, helper);
    }
    else
    {
      result = ev.value() + see*eval_t::seeScale()*EvalTraits<P>::delta;
    }
    // first_normal_move_only:安い順にsortしたので一直線に読む (王手回避はする)
    if (base_t::isWinValue(alt(P), result))
      continue;

    // 終了処理
    if (EvalTraits<P>::betterThan(result, best_value))
    {
      best_value = result;
      if (EvalTraits<P>::notLessThan(best_value, threat1.value))
      {
	threat2 = threat1;
	threat1 = QuiescenceThreat(best_value, move);
	if (EvalTraits<P>::betterThan(threat1.value, beta1)
	    || EvalTraits<P>::betterThan(threat2.value, beta2))
	  return true;
      } 
      else 
      {
	assert(EvalTraits<P>::notLessThan(best_value, threat2.value));
	threat2 = QuiescenceThreat(best_value, move);
	if (EvalTraits<P>::betterThan(threat2.value, beta2))
	  return true;
      }
    }
    if (first_normal_move_only)
      break;
  }
  return false;
  
  // 取り返す手でcutしなかった場合:
  // 逃げられない脅威の評価 (Opponent が逃げる)
  // 逃げられない場合はthreat1 だけでなくthreat2にもsetする
  assert(! moves.empty());
  if (! EvalTraits<P>::betterThan(best_value, threat2.value))
    return false;
  const Move threat_move = *moves.begin();
  if (! first_normal_move_only)
  {
    assert(state.lastMove().isPass());
    state.popPass();
    bool cut_by_threat2 = false;
    // 成る手が防げるか? TODO: 長い利きを近くで止める
    const Player Opponent = alt(P);
    MoveVector moves;
    move_generator::GenerateAddEffectWithEffect::generate<false>
      (Opponent, state.state(), threat_move.to(), moves);
    if (moves.empty())
    {
      threat2 = QuiescenceThreat(best_value, threat_move);
      if (EvalTraits<P>::betterThan(threat2.value, beta2))
	cut_by_threat2 = true;
    }
    state.pushPass();
    return cut_by_threat2;
  }
  else if ((depthFromRoot() < QSearch2PrivateTraits::EscapeFromLastMoveDepthFromRoot)
	   || (unpromote(moves[0].capturePtype()) == ROOK)
	   || (unpromote(moves[0].capturePtype()) == BISHOP))
  {
    assert(state.lastMove().isPass());
    state.popPass();
    bool cut_by_threat2 = false;
    const Square to = threat_move.to();
    const Piece target = state.state().pieceOnBoard(to);
    bool tried_escape
      = (depthFromRoot() < QSearch2PrivateTraits::EscapeDepthFromRoot);
#ifdef QSEARCH_PESSIMISTIC_ESCAPE_THREAT
    if (state.lastMove().isNormal())
    {
      // 直前の利きは逃げているはずなので，パスは pessimistic に
      // TODO: escapeFromOtherThanPawnに合わせて hasEffectIf に変更
      const Offset32 offset32(to, state.lastMove().to());
      const EffectContent effect
	= Ptype_Table.getEffect(state.lastMove().ptypeO(),offset32);
      tried_escape = effect.hasEffect();
    }
#endif
    if (! tried_escape)
    {
      const Player Opponent = alt(P);
      MoveVector escape;
      const bool safe_escape
	= QuiescenceGenerator<Opponent>::escapeByMoveOnly(state.state(), 
							  target, escape);
      if (safe_escape)
	goto finish;
      for (Move move: escape)
      {
	eval_t new_ev = ev;
	new_ev.update(state.state(), Move::PASS(P));
	int result;
	if (isMajor(move.ptype()))
	{
	  typedef QSearch2TakeBackOrChase<QuiescenceSearch2,Opponent> helper_t;
	  helper_t helper(result, this, best_value+EvalTraits<Opponent>::delta, 
			  threat2.value+EvalTraits<P>::delta, new_ev, move);
	  state.doUndoMoveLight<Opponent,helper_t>(move, helper);
	}
	else
	{
	  typedef QSearch2NextTakeBack<QuiescenceSearch2,Opponent> helper_t;
	  helper_t helper(result, this, best_value+EvalTraits<Opponent>::delta, 
			  threat2.value+EvalTraits<P>::delta, new_ev, move);
	  state.doUndoMoveLight<Opponent,helper_t>(move, helper);
	}
	if (EvalTraits<Opponent>::betterThan(result, best_value))
	{
	  best_value = result;
	  if (EvalTraits<Opponent>::notLessThan(result, threat2.value))
	    break;
	}
      }
    }
    if (EvalTraits<P>::betterThan(best_value, threat2.value))
    {
      threat2 = QuiescenceThreat(best_value, threat_move);
      if (EvalTraits<P>::betterThan(threat2.value, beta2))
      {
	cut_by_threat2 = true;
	goto finish;
      }
    }
  finish:
    state.pushPass();
    return cut_by_threat2;
  }
  return false;
}

template <class EvalT>
template <osl::Player P>
int osl::search::QuiescenceSearch2<EvalT>::
takeBackOrChase(int alpha, int beta, eval_t const& ev, Move last_move)
{
  assert(last_move.isNormal());
  int best_value = takeBackValue<P>(alpha, beta, ev, last_move);
  if (EvalTraits<P>::betterThan(best_value, beta))
    return best_value;

  MoveVector moves;
  QuiescenceGenerator<P>::capture1(state.state(), last_move.from(), moves);
  if (moves.empty())
    return best_value;
  for (Move move: moves)
  {
    eval_t new_ev = ev;

    typedef QSearch2SafeEscape<eval_t, P> helper_t;
    helper_t helper(&state.state(), 
		    state.state().pieceOnBoard(last_move.to()),
		    new_ev, move);
    state.doUndoMoveLight<P,helper_t>(move, helper);    
    if (helper.is_invalid)
      continue;

    int result = new_ev.value();
    if (! helper.has_safe_escape)
      result += new_ev.captureValue(last_move.ptypeO());
    if (state.state().template hasEffectByPtype<ROOK>(P, move.from()))
      result += (new_ev.captureValue(newPtypeO(alt(P),PROOK))
		 - new_ev.captureValue(newPtypeO(alt(P),ROOK)));
    best_value = EvalTraits<P>::max(result, best_value);
    break;			// 追撃は一手だけ試す
  }
  return best_value;
}

template <class EvalT>
template <osl::Player P>
int osl::search::QuiescenceSearch2<EvalT>::
takeBackValue(int alpha, int beta, eval_t const& ev, Move last_move)
{
  assert(alpha % 2);
  assert(beta % 2);
  
  ++node_count;
  assert(EvalTraits<P>::notLessThan(beta, alpha));
  if (state.state().inCheck(alt(P)))
    return base_t::winByFoul(P);
  if (last_move.isPass())
    return ev.value();
  
  const Square last_to = last_move.to();
  MoveVector moves;
  const Piece last_move_piece = state.state().pieceOnBoard(last_to);
  int cur_val;
  if (state.state().inCheck())
  {
    const bool check_by_lance = state.state().template hasEffectByPtypeStrict<LANCE>
      (alt(P), state.state().template kingSquare<P>());
    const bool has_safe_move
      = QuiescenceGenerator<P>::escapeKingInTakeBack(state.state(), moves, check_by_lance);
    cur_val = (has_safe_move 
	       ? currentValueWithLastThreat(ev, last_move_piece) 
	       : base_t::winByCheckmate(alt(P)));
    assert(cur_val % 2 == 0);
  }
  else
  {
    cur_val = currentValueWithLastThreat(ev, last_move_piece);
    assert(cur_val % 2 == 0);
    if (EvalTraits<P>::betterThan(cur_val, beta)) // generate の省略
      return cur_val;
    QuiescenceGenerator<P>::capture1(state.state(), 
				     last_move_piece.square(), moves);
  }
  if (EvalTraits<P>::betterThan(cur_val, alpha))
  {
    alpha = cur_val + EvalTraits<P>::delta;
    if (EvalTraits<P>::betterThan(cur_val, beta)) {
      return cur_val;
    }
  }

  assert(EvalTraits<P>::betterThan(alpha, cur_val));
  if (examineTakeBack<P>(moves, cur_val, alpha, beta, ev)) {
    assert(cur_val % 2 == 0);    
    return cur_val;
  }
  
  // cut しなかった
  assert(cur_val % 2 == 0);
  return cur_val;
}

template <class EvalT>
template <osl::Player P>
int osl::search::QuiescenceSearch2<EvalT>::
staticValueWithThreat(eval_t const& ev, int alpha, 
		      QuiescenceThreat& threat1, QuiescenceThreat& threat2)
{
  assert(alpha % 2);
  assert(! state.state().inCheck());
  const int static_value = ev.value();
  if (EvalTraits<P>::notLessThan(alpha, static_value))
    return static_value;
  const Player O = alt(P);
  const int FirstThreat = QSearchTraits::FirstThreat;
  const int SecondThreat
    = (depthFromRoot() < QSearch2PrivateTraits::EscapeDepthFromRoot)
    ? 1
    : QSearchTraits::SecondThreat;

  const int o_beta1
    = (EvalTraits<O>::min(base_t::winByCheckmate(O),
			  static_value - FirstThreat*(static_value - alpha))
       - ((FirstThreat % 2) ? 0 : EvalTraits<O>::delta));
  const int o_beta2
    = (EvalTraits<O>::min(base_t::winByCheckmate(O),
			  static_value - SecondThreat*(static_value - alpha))
       - ((SecondThreat % 2) ? 0 : EvalTraits<O>::delta));

  threat1.value = static_value;
  threat2.value = static_value;
  
  assert(state.state().turn() == P);
  state.pushPass();

  assert(! state.state().inCheck());

  assert(EvalTraits<O>::betterThan(o_beta1, threat1.value));
  assert(EvalTraits<O>::betterThan(o_beta2, threat1.value));
  assert(EvalTraits<O>::notLessThan(o_beta1, o_beta2));
  EvalT ev2(ev);
  ev2.update(state.state(), Move::PASS(P));

  MoveVector moves;
  if (generateAndExamineTakeBack2<O,ROOK>(moves, threat2, threat1, o_beta1, o_beta2, ev2))
    goto finish;
  if (generateAndExamineTakeBack2<O,BISHOP>(moves, threat2, threat1, o_beta1, o_beta2, ev2))
    goto finish;
  if (generateAndExamineTakeBack2<O,GOLD>(moves, threat2, threat1, o_beta1, o_beta2, ev2))
    goto finish;
  if (generateAndExamineTakeBack2<O,SILVER>(moves, threat2, threat1, o_beta1, o_beta2, ev2))
    goto finish;
  if (generateAndExamineTakeBack2<O,KNIGHT>(moves, threat2, threat1, o_beta1, o_beta2, ev2))
    goto finish;
  if (generateAndExamineTakeBack2<O,LANCE>(moves, threat2, threat1, o_beta1, o_beta2, ev2))
    goto finish;
  // 成る手は飛車と角と歩 (TODO: 玉の近傍)
  QuiescenceGenerator<O>::template promoteN<ROOK,1>(state.state(), moves, state.historyTable());
  if (examineTakeBack2<O,true,false>(moves, threat2, threat1, o_beta1, o_beta2, ev2))
    goto finish;
  moves.clear();
  QuiescenceGenerator<O>::template promoteN<BISHOP,1>(state.state(), moves, state.historyTable());
  if (examineTakeBack2<O,true,false>(moves, threat2, threat1, o_beta1, o_beta2, ev2))
    goto finish;
  moves.clear();
  QuiescenceGenerator<O>::template promoteN<PAWN,1>(state.state(), moves, state.historyTable());
  if (examineTakeBack2<O,true,false>(moves, threat2, threat1, o_beta1, o_beta2, ev2))
    goto finish;
  moves.clear();
  if (depth() >= QSearch2PrivateTraits::PawnCaptureDepth 
      || max_depth <= 2)
  {
    if (generateAndExamineTakeBack2<O,PAWN>(moves, threat2, threat1, o_beta1, o_beta2, ev2))
      goto finish;
  }
finish:
  state.popPass();
  // handle the first threat more seriously in some condition 
  if (threat1.move == threat2.move && threat1.move.isNormal()) {
    const Piece target = state.state().pieceOnBoard(threat1.move.to());
    if (isMajorBasic(target.ptype())
	&& target.square().template canPromote<O>()) {
      assert(alt(target.owner()) == O);
      assert(threat1.value % 2 == 0);
      return threat1.value;
    }
  }
  // usual KFEND-like threat handling
  const int result1 = (static_value - (static_value - threat1.value)/FirstThreat);
  const int result2 = (static_value - (static_value - threat2.value)/SecondThreat);

  const int result = EvalTraits<O>::max(result1, result2) & (~0x1);
  assert(result % 2 == 0);
  return result;
}

#endif /* OSL_QUIESCENCESEARCH2_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
