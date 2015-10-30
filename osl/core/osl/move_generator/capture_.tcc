/* generateCaptureMoves.tcc
 */
#ifndef _GENERATECAPTUREMOVES_TCC
#define _GENERATECAPTUREMOVES_TCC

#include "osl/move_generator/capture_.h"
#include "osl/move_generator/pieceOnBoard.h"
#include "osl/move_generator/effect_action.h"
#include "osl/basic_type.h"

namespace osl
{
  namespace move_generator
  {
    using namespace effect_action;
    namespace capture
    {
      template<Player P,class Action>
      void generate(const NumEffectState& state,Square target,Action& action,PieceMask pieces)
      {
	Piece p1=state.pieceAt(target);
	while(pieces.any()){
	  int num=pieces.takeOneBit();
	  Piece p=state.pieceOf(num);
	  if(state.pinOrOpen(P).test(num) && !state.pinnedCanMoveTo<P>(p,target))
	    continue;
	  PieceOnBoard<Action>::template generatePiece<P>(state,p,target,p1,action);
	}
      }
    }

    template<class Action>
    template<Player P>
    void Capture<Action>::
    generate(const NumEffectState& state,Square target,Action& action)
    {
      assert(target.isOnBoard());
      PieceMask pieces=state.piecesOnBoard(P)&state.effectSetAt(target);
      capture::generate<P,Action>(state,target,action,pieces);
    }

    template<class Action>
    template<Player P>
    void Capture<Action>::
    escapeByCapture(const NumEffectState& state,Square target,Piece piece,Action& action)
    {
      PieceMask pieces=state.piecesOnBoard(P)&state.effectSetAt(target);
      pieces.reset(piece.number());
      capture::generate<P,Action>(state,target,action,pieces);
    }

  } // namespace move_generator
} // namespace osl

template<class Action>
template<osl::Player P>
void osl::move_generator::Capture<Action>::
generate1(const NumEffectState& state,Square target,
	  Action& action)
{
  Piece move = state.findCheapAttackNotBy(P, target, state.pinOrOpen(P));
  if (! move.isPiece())
    move = state.findCheapAttack(P, target);
  if (move.isPiece())
    PieceOnBoard<Action>::template generatePiece<P>
      (state,move,target,state.pieceAt(target),action);
}


#endif /* _GENERATECAPTUREMOVES_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
