/* escape.h
 */
#ifndef GROUP_ESCAPE_H
#define GROUP_ESCAPE_H

#include "osl/rating/group.h"
#include "osl/rating/feature/escape.h"

namespace osl
{
  namespace rating
  {
    struct FromEffectGroup : public Group
    {
      FromEffectGroup();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (move.isDrop())
	  return -1;
	const int index = CountEffect2::index(state, move.from(), env);
	const int progress8 = env.progress.value()/2;
	return index*8 + progress8;
      }
      bool effectiveInCheck() const { return true; }
    };

    struct PtypeAttackedGroup : public Group
    {
      PtypeAttackedGroup();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (move.isDrop())
	  return -1;
	const int progress8 = env.progress.value()/2;
	const Ptype self = move.oldPtype();
	const Ptype attack = state.findCheapAttack(alt(move.player()), move.from()).ptype();
	const int index = (self-PTYPE_PIECE_MIN)*(PTYPE_MAX+1-PTYPE_MIN)+attack;
	return index*8 + progress8;
      }
      bool effectiveInCheck() const { return true; }
    };

    struct ToSupportedGroup : public Group
    {
      ToSupportedGroup() : Group("ToSupported")
      {
	for (int p=0; p<8; ++p)	// progress8
	  push_back(new ToSupported());
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (! (*this)[0].match(state, move, env))
	  return -1;
	const int progress8 = env.progress.value()/2;
	return progress8;
      }
      bool effectiveInCheck() const { return true; }
    };

    struct ImmediateEscapeGroup : public Group
    {
      ImmediateEscapeGroup();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatchWithoutProgress(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (move.isDrop())
	  return -1;
	const Move last_move = env.history.lastMove();
	if (! last_move.isNormal()
	    || ! state.hasEffectIf(last_move.ptypeO(), last_move.to(), move.from()))
	  return -1;
	return (move.ptype() - PTYPE_PIECE_MIN) * (PTYPE_MAX+1 - PTYPE_PIECE_MIN)
	  + last_move.ptype() - PTYPE_PIECE_MIN;
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	const int index = findMatchWithoutProgress(state, move, env);
	if (index < 0)
	  return index;
	const int progress8 = env.progress.value()/2;
	return index*8 + progress8;
      }
    };

    class KingEscapeGroup : public Group
    {
    public:
      KingEscapeGroup();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (! state.inCheck())
	  return -1;
	const Ptype self = move.ptype();
	const int index = self-PTYPE_PIECE_MIN;
	assert((*this)[index].match(state, move, env));
	return index;    
      }
      bool effectiveInCheck() const { return true; }
    };
  }
}

#endif /* GROUP_ESCAPE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
