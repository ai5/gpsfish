/* moveGenerator.cc
 */
#include "osl/search/moveGenerator.h"
#include "osl/search/searchState2.h"
#include "osl/move_classifier/shouldPromoteCut.h"
#include "osl/search/sortCaptureMoves.h"
#include "osl/search/breakThreatmate.h"
#include "osl/search/analyzer/categoryMoveVector.h"
#include "osl/move_generator/capture_.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_generator/promote_.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/attackToPinned.h"
#include "osl/move_classifier/check_.h"
#include "osl/move_classifier/safeMove.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/rating/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "osl/eval/pieceEval.h"
#include "osl/eval/progressEval.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/stat/average.h"
#include <iostream>
#include <iomanip>

// #define STAT_WIDTH_VS_LIMIT

#ifndef NDEBUG
#  define SAFE_MOVE_ONLY
#endif
const int max_see = 20000;

static const osl::rating::FeatureSet *static_feature_set;
static const osl::rating::FeatureSet& feature_set()
{
  return *static_feature_set;
}

void osl::search::MoveGenerator::initOnce()
{
  if (static_feature_set == 0)
    static_feature_set = &rating::StandardFeatureSet::instance();
}

namespace osl
{
  namespace search
  {
    const CArray2d<MoveGenerator::generator_t, 2, MoveGenerator::FINISH> MoveGenerator::Generators = {{
	0,
	&MoveGenerator::generateKingEscape<BLACK>,
	&MoveGenerator::generateTakeBack<BLACK>,
	&MoveGenerator::generateBreakThreatmate<BLACK>,
	&MoveGenerator::generateCapture<BLACK>,
	0,
	&MoveGenerator::generateTesuji<BLACK>,
	&MoveGenerator::generateAll<BLACK>,

	0,
	&MoveGenerator::generateKingEscape<WHITE>,
	&MoveGenerator::generateTakeBack<WHITE>,
	&MoveGenerator::generateBreakThreatmate<WHITE>,
	&MoveGenerator::generateCapture<WHITE>,
	0,
	&MoveGenerator::generateTesuji<WHITE>,
	&MoveGenerator::generateAll<WHITE>,
    }};
    const CArray<const char *, MoveGenerator::FINISH> MoveGenerator::GeneratorNames = {{
      "INITIAL", "KING_ESCAPE", "TAKEBACK", "BREAK_THREATMATE", "TACTICAL", "SENTINEL", "TESUJI", "ALL", 
    }};
#ifdef STAT_WIDTH_VS_LIMIT
    struct WidthVSLimit 
    {
      CArray<stat::Average,10> averages;

      ~WidthVSLimit()
      {
	report();
      }
      stat::Average& average(int limit) 
      {
	limit /= 100 - 3;
	return averages[std::min(std::max(limit,0),(int)averages.size()-1)];
      }
      void report()
      {
	std::cerr << "WidthVSLimit@MoveGenerator\n";
	for (int limit=300; limit<300+(int)averages.size()*100; limit+=100) {
	  std::cerr << std::setw(5) << limit << " " << average(limit).getAverage() << std::endl;
	}
      }
    } Width_VS_Limit;
#endif
    template
    void MoveGenerator::init<osl::eval::ProgressEval>(
      int limit, const SimpleHashRecord *record, const osl::eval::ProgressEval&,
      const NumEffectState&, bool in_pv, Move hash_move, bool quiesce);
    template
    void MoveGenerator::init<osl::eval::ml::OpenMidEndingEval>(
      int limit, const SimpleHashRecord *record,
      const osl::eval::ml::OpenMidEndingEval&,
      const NumEffectState&, bool in_pv, Move hash_move, bool quiesce);
  }
}

/* ------------------------------------------------------------------------- */

osl::search::
MoveMarker::MoveMarker() : cur(1) 
{
  marker.fill(0);
}

void osl::search::
MoveMarker::clear() 
{
  ++cur;
  if (cur == 0) {
    marker.fill(0);
    cur = 1;
  }
}

bool osl::search::
MoveMarker::registerIfNew(const NumEffectState& state, Move m)
{
  value_t& val = marker[toIndex(m)][pieceIndex(state, m)];
  if (val == cur)
    return false;
  val = cur;
  return true;
}

bool osl::search::
MoveMarker::registered(const NumEffectState& state, Move m) const
{
  return marker[toIndex(m)][pieceIndex(state, m)] == cur;
}

/* ------------------------------------------------------------------------- */

osl::search::
MoveGenerator::MoveGenerator() : record(0), progress(16)
{
}

int osl::search::
MoveGenerator::captureValue(Ptype ptype)
{
  if (! isPiece(ptype))
    return 0;
  int result 
    = eval::Ptype_Eval_Table.captureValue(newPtypeO(WHITE, ptype));
  assert(result >= 0);
  return result;
}

template <class EvalT>
void osl::search::
MoveGenerator::init(int l, const SimpleHashRecord *r,
		    const EvalT& eval,
		    const NumEffectState& state, bool in_pv, Move hash_move,
		    bool quiesce) 
{
  assert(r);
  assert(l > 0);
  limit = l;
  record = r;
  cur_state = INITIAL;
  moves.clear();
  cur_index = tried = 0;
  progress = eval.progress32();
  eval_suggestion = eval.suggestMove(state);

  marker.clear();
  env.make(state, state.pin(state.turn()), state.pin(alt(state.turn())),
	   eval.progress16());
  if (hash_move.isNormal())
    marker.registerMove(state, hash_move);
#ifndef MINIMAL
  in_quiesce = quiesce;
#endif
  this->in_pv = in_pv;
}

void osl::search::
MoveGenerator::dump() const
{
  std::cerr << "generator " << cur_state << " index " << cur_index 
	    << " limit " << limit << " tried " << tried << "\n";
  std::cerr << moves.size() << "\n"
#ifndef MINIMAL
   << moves
#endif
    ;
}

template <osl::Player P>
const osl::MoveLogProb osl::search::
MoveGenerator::nextTacticalMoveWithGeneration(const SearchState2& state) 
{	
  assert(record);
  while (true) {
    assert(cur_index >= moves.size());
    if (cur_state == KING_ESCAPE && record->inCheck()) {
      cur_state = FINISH;
      break;
    }
    if (++cur_state >= TACTICAL_FINISH)
      break;
    // generate
    assert(Generators[playerToIndex(P)][cur_state]);
    (this->*Generators[playerToIndex(P)][cur_state])(state);
    if (cur_index < moves.size()) {
      ++tried;
      return moves[cur_index++];
    } 
  }
  return MoveLogProb();
}

template <osl::Player P>
const osl::MoveLogProb osl::search::
MoveGenerator::nextMoveWithGeneration(const SearchState2& state) 
{	
  assert(record);
  while (true) {
    assert(cur_index >= moves.size());
    if (++cur_state >= FINISH)
      break;
    // generate
    assert(Generators[playerToIndex(P)][cur_state]);
    (this->*Generators[P][cur_state])(state);
    if (cur_index < moves.size()) {
      ++tried;
      return moves[cur_index++];
    } 
  }
  return MoveLogProb();
}

template <osl::Player P>
void osl::search::
MoveGenerator::generateKingEscape(const SearchState2& sstate)
{
  env.history = sstate.history();
  if (! record->inCheck()) 
    return;

  const NumEffectState& state = sstate.state();
  const Piece king = state.kingPiece<P>();
  assert(state.hasEffectAt(alt(P), king.square()));

  MoveVector src;
  move_generator::GenerateEscape<P>::generate(state,king,src);
  size_t last = src.size();
  for (size_t i=0; i<last; ++i)
    if (src[i].hasIgnoredUnpromote<P>())
      src.push_back(src[i].unpromote());

  if (src.size() == 1) {
    moves.push_back(MoveLogProb(src[0], 20));
    return;
  }
  for (Move move: src) {
    const int prob = std::min(limit, feature_set().logProbKingEscape(state, env, move));
    assert(prob > 0);
    moves.push_back(MoveLogProb(move, prob));
  }
  moves.sortByProbability();
}



template <osl::Player P>
void osl::search::
MoveGenerator::generateBreakThreatmate(const SearchState2& sstate)
{
  const NumEffectState& state = sstate.state();
  const Move threatmate_move = record->threatmate().threatmateMove(state.turn());
  if (! threatmate_move.isNormal())
    return;
  BreakThreatmate::generate(limit, state, threatmate_move, moves);
  for (const MoveLogProb& move: moves)
    marker.registerMove(state, move.move());
}

template <osl::Player P>
void osl::search::
MoveGenerator::generateTakeBack(const SearchState2& sstate)
{
  using namespace move_action;
  const Move last_move = sstate.lastMove();
  if (! last_move.isNormal())
    return;
  const Square last_to = last_move.to();
  
  const NumEffectState& state = sstate.state();
#ifndef MINIMAL
  if (in_quiesce)
    return quiesceCapture<P>(state, last_to);
#endif
  MoveVector src;
  move_generator::GenerateCapture::generate(state, last_to, src);

  assert(moves.empty());
  for (Move move: src) {
    assert(! ShouldPromoteCut::canIgnoreMove<P>(move));
    const int prob = feature_set().logProbTakeBack(state, env, move);
#ifdef OSL_SMP
    if (! move.isDrop() && move.ptype() != KING
	&& env.my_pin.test(state.pieceOnBoard(move.from()).number())) {
      if (move_classifier::KingOpenMove<P>::isMember(state, move.ptype(), move.from(), move.to()))
	continue;
    }
#endif
    if (prob <= std::min(200, limit) && marker.registerIfNew(state, move))
      moves.push_back(MoveLogProb(move, prob));
  }
  moves.sortByProbability();
}

namespace osl
{
  template <Player P, Ptype PTYPE>
  static void makeCapture(const NumEffectState& state, 
				       MoveVector& out)
  {
    move_action::Store store(out);
    mask_t pieces = state.piecesOnBoard(alt(P)).template selectBit<PTYPE>() 
      & state.effectedMask(P).getMask(PtypeFuns<PTYPE>::indexNum);
    while (pieces.any())
    {
      const Piece p = state.pieceOf(pieces.takeOneBit()+PtypeFuns<PTYPE>::indexNum*32);
      assert(p.isOnBoardByOwner<alt(P)>());
      move_generator::GenerateCapture::generate(P,state, p.square(), store);
    }
  }
}

template <osl::Player P>
void osl::search::
MoveGenerator::addCapture(const NumEffectState& state, const RatingEnv& env, const MoveVector& src)
{
#ifndef MINIMAL
  if (in_quiesce) {
    for (Move move: src) {
      assert(!ShouldPromoteCut::canIgnoreMove<P>(move));
      const int see = PieceEval::computeDiffAfterMoveForRP(state, move);
      if (see < 0)
	continue;
      moves.push_back(MoveLogProb(move, max_see - see));
    }
    return;
  }
#endif
  for (Move move: src) {
    assert(! ShouldPromoteCut::canIgnoreMove<P>(move));
#ifdef SAFE_MOVE_ONLY
    if (! move.isDrop() && move.ptype() != KING
	&& env.my_pin.test(state.pieceOnBoard(move.from()).number())) {
      if (move_classifier::KingOpenMove<P>::isMember(state, move.ptype(), move.from(), move.to()))
	continue;
    }
#endif
    const int prob = feature_set().logProbSeePlus(state, env, move);
    // const int prob = feature_set().logProbTakeBack(state, env, move);
    if (prob <= 200 && marker.registerIfNew(state, move)) {
      moves.push_back(MoveLogProb(move, prob));
    }
  }
  return;
}

template <osl::Player P>
void osl::search::
MoveGenerator::generateCapture(const SearchState2& sstate)
{
  using namespace move_action;

  const NumEffectState& state = sstate.state();
  MoveVector src;
#if 1
  // lance, bishop, rook
  makeCapture<P,LANCE>(state, src);
  makeCapture<P,BISHOP>(state, src);
  makeCapture<P,ROOK>(state, src);
  // knight, silver, gold
  makeCapture<P,KNIGHT>(state, src);
  makeCapture<P,SILVER>(state, src);
  makeCapture<P,GOLD>(state, src);
#else
  makeCaptureOtherThanPawn<P>(state, src);
#endif
  addCapture<P>(state, env, src);
}

template <osl::Player P>
void osl::search::
MoveGenerator::generateTesuji(const SearchState2& sstate)
{
  const NumEffectState& state = sstate.state();
  if (! state.inCheck() && eval_suggestion.isNormal()
      && marker.registerIfNew(state, eval_suggestion)) {
    assert(sstate.state().isValidMove(eval_suggestion));
    moves.push_back(MoveLogProb(eval_suggestion, 250));
  }
#ifndef MINIMAL
  if (in_quiesce) {
    MoveVector src;
    move_generator::Promote<P>::generate(state, src);
    makeCapture<P,PAWN>(state, src);
    addCapture<P>(state, env, src);
  }
#endif
}

#ifndef MINIMAL
template <osl::Player P>
void osl::search::
MoveGenerator::quiesceCapture(const NumEffectState& state, Square to)
{
  MoveVector moves;
  move_generator::GenerateCapture::generate(state, to, moves);

  for (Move move: moves) {
    assert(!ShouldPromoteCut::canIgnoreAndNotDrop<P>(move));
    int see = PieceEval::computeDiffAfterMoveForRP(state, move);
    if (see < 0)
      continue;
    this->moves.push_back(MoveLogProb(move, max_see - see));
  }
  this->moves.sortByProbabilityReverse();
}
#endif

template <osl::Player P>
void osl::search::
MoveGenerator::generateAll(const SearchState2& sstate)
{
#ifndef MINIMAL
  if (in_quiesce) 
    return;
#endif
  const NumEffectState& state = sstate.state();
  MoveLogProbVector all;
  feature_set().generateLogProb(state, env, limit, all, in_pv);
#ifdef STAT_WIDTH_VS_LIMIT
  const size_t moves_size_before = moves.size();
#endif
  for (const MoveLogProb& move: all) {
    assert(!ShouldPromoteCut::canIgnoreAndNotDrop<P>(move.move()));
    const Move m = move.move();
    int limit = move.logProb();
    if (this->limit >= 400) {
      using namespace move_classifier;
      if (m.isCaptureOrPromotion()
	  || (in_pv && MoveAdaptor<Check<P> >::isMember(state, move.move())))
	limit = std::min(limit, 400);
    }
    if (limit <= this->limit
	&& marker.registerIfNew(state, move.move())) {
#ifndef NDEBUG
      if (! m.isDrop()) {
	assert(! (env.my_pin.test(state.pieceOnBoard(m.from()).number())
		  && move_classifier::KingOpenMove<P>::isMember(state, m.ptype(), m.from(), m.to())));
	assert(! (m.ptype() == KING && state.hasEffectAt(alt(P), m.to())));
      }
#endif
      moves.push_back(MoveLogProb(move.move(), limit));
    }
  }
#ifdef STAT_WIDTH_VS_LIMIT
  Width_VS_Limit.average(limit).add(moves.size() - moves_size_before);
#endif
}

#ifndef MINIMAL
void osl::search::
MoveGenerator::generateAll(Player P, const SearchState2& state, 
			   analyzer::CategoryMoveVector& out)
{
  assert(moves.size() == 0);

  for (int i=0; i<FINISH; ++i) {
    if (! Generators[playerToIndex(P)][i])
      continue;
    (this->*Generators[playerToIndex(P)][i])(state);
    out.push_front(analyzer::CategoryMoves(moves, GeneratorNames[i]));
    bool generated = moves.size();
    moves.clear();    
    if (i == KING_ESCAPE && generated)
      break;
  }
  out.reverse();
}
#endif

template <osl::Player P>
void
#if (defined __GNUC__) && (! defined GPSONE) && (! defined GPSUSIONE)
__attribute__ ((used))
#endif
  osl::search::
MoveGenerator::generateAll(const SearchState2& state, MoveLogProbVector& out) 
{
  using namespace move_classifier;
  for (MoveLogProb m = nextTacticalMove<P>(state);
       m.validMove(); m = nextTacticalMove<P>(state)) {
    assert(state.state().isValidMove(m.move()));
    if (ConditionAdaptor<SafeMove>::isMember(state.state(), m.move()))
      out.push_back(m);
  }
  for (MoveLogProb m = nextMove<P>(state); m.validMove(); 
       m = nextMove<P>(state)) {
    assert(state.state().isValidMove(m.move()));
    if (ConditionAdaptor<SafeMove>::isMember(state.state(), m.move()))
      out.push_back(m);
  }
}

void osl::search::
MoveGenerator::generateAll(Player P, const SearchState2& state, MoveLogProbVector& out) 
{
  if (P==BLACK)
    generateAll<BLACK>(state, out);
  else
    generateAll<WHITE>(state, out);
}

namespace osl
{
  namespace search
  {
    template const MoveLogProb MoveGenerator::nextMoveWithGeneration<BLACK>(const SearchState2&);
    template const MoveLogProb MoveGenerator::nextMoveWithGeneration<WHITE>(const SearchState2&);

    template const MoveLogProb MoveGenerator::nextTacticalMoveWithGeneration<BLACK>(const SearchState2&);
    template const MoveLogProb MoveGenerator::nextTacticalMoveWithGeneration<WHITE>(const SearchState2&);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
