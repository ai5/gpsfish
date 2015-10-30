/* patternGroup.h
 */
#ifndef _PATTERNGROUP_H
#define _PATTERNGROUP_H

#include "osl/rating/group.h"
#include "osl/rating/feature/pattern.h"

namespace osl
{
  namespace rating
  {
    struct PatternGroup : public Group
    {
      static std::string name(Direction direction, Direction direction2);
      Direction direction, direction2;
      CArray2d<unsigned char, 2, Square::SIZE> target_table;
      explicit PatternGroup(Direction d, Direction d2 = Pattern::INVALID);
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move m, const RatingEnv&) const;
      bool effectiveInCheck() const { return true; }
    };


    class PatternLongGroup : public Group
    {
    public:
      static const CArray<Direction,4> rook_direction4;
      static const CArray<Direction,4> bishop_direction4;
    private:
      static std::string name(int direction_id);
      int direction_id;
    public:
      explicit PatternLongGroup(int d);

      Direction makeDirection(Ptype ptype) const 
      {
	return (unpromote(ptype) == BISHOP) ? bishop_direction4[direction_id] : rook_direction4[direction_id];
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move m, const RatingEnv& env) const;
      bool effectiveInCheck() const { return true; }
    };


    class PatternLongGroup2 : public Group
    {
      static std::string name(int direction_id);
      int direction_id;
    public:
      PatternLongGroup2(int d);

      Direction makeDirection(Ptype ptype) const 
      {
	return (unpromote(ptype) == BISHOP) 
	  ? PatternLongGroup::bishop_direction4[direction_id] 
	  : PatternLongGroup::rook_direction4[direction_id];
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move m, const RatingEnv& env) const;
      bool effectiveInCheck() const { return true; }
    };

    class PatternBlockGroup : public Group
    {
      Ptype attacker;
    public:
      explicit PatternBlockGroup(Ptype attacker);
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move m, const RatingEnv& env) const;
    };
  }
}

#endif /* _PATTERNGROUP_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
