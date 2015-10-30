#ifndef _GENERATE_OPEN_MOVES_TCC
#define _GENERATE_OPEN_MOVES_TCC

#include "osl/move_generator/open.h"
#include "osl/move_generator/pieceOnBoard.tcc"
#include "osl/move_generator/move_action.h"

template<class Action>
template<osl::Player P>
void osl::move_generator::Open<Action>::
generate(const NumEffectState& state,Piece p,Action& action,Square to,Direction dir)
{
  typedef move_action::NoAddEffectFilter<Action> action_t;
  action_t newAction(state,action,to);
  PieceOnBoard<action_t>::template generate<P,true>(state,p,newAction,1<<primDir(dir));
}

#endif /* _GENERATE_OPEN_MOVES_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
