/* feature.cc
 */
#include "osl/rating/feature.h"
#include "osl/simpleState.tcc"
#include <sstream>

osl::rating::Feature::~Feature()
{
}

const osl::CArray<const char*,4> osl::rating::Check::check_property = {{ "Di", "DO", "OC", "Bo" }};
osl::rating::Check::Check(int p) : Feature(check_property[p]), property(p) 
{
}

bool osl::rating::Check::match(const NumEffectState& state, Move move, const RatingEnv&) const
{
  if (property == 0 || property == 1) {
    if (! (state.isDirectCheck(move) && ! state.isOpenCheck(move)))
      return false;
    return property == openLong(state, move);
  }
  if (property == 2)
    return ! state.isDirectCheck(move) && state.isOpenCheck(move);
  return state.isDirectCheck(move) && state.isOpenCheck(move);
}

const std::string osl::rating::
Block::name(int self, int opponent)
{
  std::ostringstream os;
  os << "B" << self << opponent;
  return os.str();  
}

const std::string osl::rating::Open::name(int property)
{
  std::ostringstream os;
  os << "Open" << property / 4 << property % 4;
  return os.str();
}

struct osl::rating::ImmediateAddSupport::Test
{
  bool *result;
  const NumEffectState& state;
  Move move;
  Test(bool *r, const NumEffectState& s, Move m)
    : result(r), state(s), move(m)
  {
  }
  template <Player P>
  void doAction(Piece /*piece*/, Square last_attack)
  {
    const Piece attacked = state.pieceAt(last_attack);
    if (attacked.isOnBoardByOwner(state.turn())
	&& state.hasEffectIf(move.ptypeO(), move.to(), last_attack))
      *result = true;
  }
};

osl::rating::
ImmediateAddSupport::ImmediateAddSupport(Ptype s, Ptype a)
  : Feature(std::string(Ptype_Table.getCsaName(s))+":"+Ptype_Table.getCsaName(a)),
    self(s), attack(a)
{
}

bool osl::rating::
ImmediateAddSupport::match(const NumEffectState& state, Move move, const RatingEnv& env) const
{
  if (move.ptype() != self)
    return false;
  const Move last_move=env.history.lastMove();
  if (! last_move.isNormal()) 
    return false;
  if (last_move.ptype() != attack)
    return false;
  const Square last_to = last_move.to();
  if (last_to==move.to())
    return false;		// TakeBack は除く
  bool result = false;
  Test action(&result, state, move);
  state.forEachEffectOfPiece(state.pieceOnBoard(last_to), action);
  return result;
}

int osl::rating::
ImmediateAddSupport::index(const NumEffectState& state, Move move, const RatingEnv& env)
{
  const Move last_move=env.history.lastMove();
  if (! last_move.isNormal()) 
    return -1;
  const Square last_to = last_move.to();
  if (last_to==move.to())
    return -1;		// TakeBack は除く
  bool result = false;
  const Piece last_piece = state.pieceOnBoard(last_to);
  // BoardMask state.changedEffects(alt(Turn))?
  if (! Ptype_Table.hasLongMove(last_piece.ptype()))
  {
    Test action(&result, state, move);
    state.forEachEffectOfPiece(last_piece, action);
  }
  else
  {
    const Player Turn = state.turn();
    PieceMask pieces = state.piecesOnBoard(Turn) & state.effectedMask(alt(Turn));
    mask_t m = pieces.getMask(0);
    while (m.any()) {
      const Piece p = state.pieceOf(m.takeOneBit());
      if (state.hasEffectByPiece(last_piece, p.square())
	  && state.hasEffectIf(move.ptypeO(), move.to(), p.square())) {
	result = true;
	break;
      }
    }
#if OSL_WORDSIZE == 32
    if (! result) {
      m = pieces.getMask(1);
      while (m.any()) {
	const Piece p = state.pieceOf(m.takeOneBit()+32);
	if (state.hasEffectByPiece(last_piece, p.square())
	    && state.hasEffectIf(move.ptypeO(), move.to(), p.square())) {
	  result = true;
	  break;
	}
      }
    }
#endif
  }
  if (! result)
    return -1;
  return (move.ptype() - PTYPE_PIECE_MIN) * (PTYPE_MAX+1 - PTYPE_PIECE_MIN)
    + last_move.ptype() - PTYPE_PIECE_MIN;
}

const std::string osl::rating::Chase::name(Ptype self, Ptype target, bool drop, OpponentType opponent_type)
{
  return std::string(Ptype_Table.getCsaName(self))
    +(drop ? "d" : "m")+">"+Ptype_Table.getCsaName(target)
    +(opponent_type == CAPTURE ? "c" : (opponent_type == DROP ? "d" : "e"));
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
