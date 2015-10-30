/* squareGroup.h
 */
#ifndef _POSITIONGROUP_H
#define _POSITIONGROUP_H

#include "osl/rating/group.h"
#include "osl/rating/feature/square.h"

namespace osl
{
  namespace rating
  {
    struct RelativeKingXGroup : public Group
    {
      bool attack;
      RelativeKingXGroup(bool a);
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	const int progress8 = env.progress.value()/2;
	const int index = RelativeKingX::index(attack, state, move);
	return index*8 + progress8;
      }  
      bool effectiveInCheck() const { return true; }
    };

    struct RelativeKingYGroup : public Group
    {
      bool attack;
      RelativeKingYGroup(bool a);
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	const int progress8 = env.progress.value()/2;
	const int index = RelativeKingY::index(attack, state, move);
	return index*8+progress8;
      }  
      bool effectiveInCheck() const { return true; }
    };

    struct SquareXGroup : public Group
    {
      SquareXGroup();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& , Move move, const RatingEnv& env) const
      {
	const int progress8 = env.progress.value()/2;
	int index = DropPtype::UNIT*(SquareX::makeX(move)-1);
	index += DropPtype::index(move);
	return index*8+progress8;
      }
      bool effectiveInCheck() const { return true; }
    };

    struct SquareYGroup : public Group
    {
      SquareYGroup();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	const int progress8 = env.progress.value()/2;
	int index = DropPtype::UNIT*(SquareY::makeY(move)-1);
	index += DropPtype::index(move);
	return index*8+progress8;
      }
      bool effectiveInCheck() const { return true; }
    };
  }
}


#endif /* _POSITIONGROUP_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
