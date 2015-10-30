/* safeFilter.h
 */
#ifndef _MOVE_ACTION_SAFE_FILTER_H
#define _MOVE_ACTION_SAFE_FILTER_H
#include "osl/state/numEffectState.h"
#include "osl/move_action/concept.h"
#include "osl/move_classifier/safeMove.h"
namespace osl
{
  namespace move_action
  {
    
    /**
     * 自玉に王手がかかっていない状況から
     * 着手して自殺手になる手を除く
     */
    template<Player P,class OrigAction>
    struct SafeFilter
    {
      BOOST_CLASS_REQUIRE(OrigAction,osl::move_action,Concept);
      const NumEffectState& state;
      OrigAction & action;
    public:
      SafeFilter(const NumEffectState& s, OrigAction & action) 
	: state(s), action(action) {
	assert(state.template kingSquare<P>().isPieceStand()
	       || !state.template 
	       hasEffectAt<PlayerTraits<P>::opponent>(state.template
						      kingSquare<P>())
	       || (state.dump(), 0));
      }
      bool isSafeMove(Ptype ptype,Square from,Square to)
      {
	return move_classifier::SafeMove<P>::isMember(state, ptype, from, to);
      }
      void simpleMove(Square from,Square to,Ptype ptype, bool isPromote,Player
#ifndef NDEBUG
		      p
#endif
	){
	assert(p == P);
	if(isSafeMove(ptype,from,to))
	  action.simpleMove(from,to,ptype,isPromote,P);
      
      }
      void unknownMove(Square from,Square to,Piece p1,Ptype ptype,bool isPromote,Player 
#ifndef NDEBUG
		       p
#endif
	){
	assert(p == P);
	if(isSafeMove(ptype,from,to))
	  action.unknownMove(from,to,p1,ptype,isPromote,P);
      }
      /**
       * dropMoveが自殺手になることはない
       */
      void dropMove(Square to,Ptype ptype,Player
#ifndef NDEBUG
		    p
#endif
	){
	assert(p == P);
	action.dropMove(to,ptype,P);
      }
    };
  } // namespace move_action
} // namespace osl

#endif /* _MOVE_ACTION_SAFE_FILTER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
