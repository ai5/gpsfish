/* checkFilter.h
 */
#ifndef _MOVE_ACTION_CHECK_FILTER_H
#define _MOVE_ACTION_CHECK_FILTER_H
#include "osl/position.h"
#include "osl/player.h"
#include "osl/ptype.h"
#include "osl/move_action/concept.h"
#include "osl/move_classifier/check_.h"

namespace osl
{
  namespace move_action
  {
    
    /**
     * 自玉に王手がかかっていない状況から
     * 着手して自殺手になる手を除く
     */
    template<Player P,class OrigAction>
    struct CheckFilter
    {
      BOOST_CLASS_REQUIRE(OrigAction,osl::move_action,Concept);
      const NumEffectState& state;
      OrigAction & action;
    public:
      CheckFilter(const NumEffectState& s, OrigAction & action) 
	: state(s), action(action) {
      }
      void simpleMove(Square from,Square to,Ptype ptype, bool isPromote,Player p){
	assert(p == P);
	typedef move_classifier::Check<P> check_t;
	if(check_t::isDirectCheck(state,ptype,to) ||
	   move_classifier::OpenCheck<P>::isMember(state,ptype,from,to))
	  action.simpleMove(from,to,ptype,isPromote,P);
      }
      void unknownMove(Square from,Square to,Piece p1,Ptype ptype,bool isPromote,Player p){
	if(move_classifier::DirectCheck<P>::isMember(state,ptype,to) ||
	   move_classifier::OpenCheck<P>::isMember(state,ptype,from,to))
	  action.unknownMove(from,to,p1,ptype,isPromote,P);
      }
      void dropMove(Square to,Ptype ptype,Player p){
	assert(p == P);
	if(move_classifier::DirectCheck<P>::isMember(state,ptype,to))
	  action.dropMove(to,ptype,P);
      }
    };
  } // namespace move_action
} // namespace osl

#endif /* _MOVE_ACTION_CHECK_FILTER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
