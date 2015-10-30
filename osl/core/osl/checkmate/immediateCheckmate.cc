/* immediateCheckmate.cc
 */

#include "osl/checkmate/immediateCheckmate.tcc"

namespace osl
{
  namespace checkmate
  {
    template 
    bool ImmediateCheckmate::
    hasCheckmateMove<BLACK>(NumEffectState const&, King8Info, Square, Move&);
    template 
    bool osl::checkmate::ImmediateCheckmate::
    hasCheckmateMove<WHITE>(NumEffectState const&, King8Info, Square, Move&);

    template 
    bool ImmediateCheckmate::
    hasCheckmateMove<BLACK>(NumEffectState const&, Move&);
    template 
    bool osl::checkmate::ImmediateCheckmate::
    hasCheckmateMove<WHITE>(NumEffectState const&, Move&);

    template 
    bool ImmediateCheckmate::
    hasCheckmateMove<BLACK>(NumEffectState const&);
    template 
    bool osl::checkmate::ImmediateCheckmate::
    hasCheckmateMove<WHITE>(NumEffectState const&);
  }
}

bool osl::checkmate::ImmediateCheckmate::
hasCheckmateMove(Player pl,NumEffectState const& state)
{
  if(pl==BLACK)
    return hasCheckmateMove<BLACK>(state);
  else
    return hasCheckmateMove<WHITE>(state);

}
bool osl::checkmate::ImmediateCheckmate::
hasCheckmateMove(Player pl,NumEffectState const& state,Move& bestMove)
{
  if(pl==BLACK)
    return hasCheckmateMove<BLACK>(state,bestMove);
  else
    return hasCheckmateMove<WHITE>(state,bestMove);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

