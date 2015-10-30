#ifndef OSL_GENERATE_OPEN_MOVES_H
#define OSL_GENERATE_OPEN_MOVES_H
#include "osl/move_generator/pieceOnBoard.h"
#include "osl/move_generator/move_action.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace move_generator
  {
    /**
     * 長い利きを止めている駒を動かして, 利きを伸ばす.
     * 
     * ただし，駒を動かした結果, Square toに利くようなものは取り除く
     */
    template<class Action>
    class Open
    {
    public:
      template<Player P>
      static void generate(const NumEffectState& state,Piece p,Action& action,Square to,Direction dir);
      
    };

    struct GenerateOpen
    {
      template<class Action>
      static void 
      generate(Player pl,const NumEffectState& state,Piece p,
	       Action& action,Square to,Direction dir)
      {
	if (pl == BLACK)
	  Open<Action>::template generate<BLACK>(state, p, action, to, dir);
	else
	  Open<Action>::template generate<WHITE>(state, p, action, to, dir);
      }
    };
    
  } // namespace move_generator
} // namespace osl
#endif /* OSL_GENERATE_OPEN_MOVES_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
