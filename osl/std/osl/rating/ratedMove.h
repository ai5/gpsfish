/* ratedMove.h
 */
#ifndef OSL_RATEDMOVE_H
#define OSL_RATEDMOVE_H

#include "osl/basic_type.h"
#include <iosfwd>

namespace osl
{
  namespace rating
  {
    class RatedMove
    {
      Move my_move;
      signed short all_rating, optimistic_rating;
    public:
      RatedMove(Move move, int rating, int optimistic) : my_move(move), all_rating(rating), optimistic_rating(optimistic)
      {
      }
      RatedMove(Move move, int rating) : my_move(move), all_rating(rating), optimistic_rating(rating)
      {
      }
      RatedMove() : all_rating(0), optimistic_rating(0) {}
      void setRating(int rating)  { all_rating = rating; }
      void setOptimisticRating(int rating)  { optimistic_rating = rating; }

      const Move move() const { return my_move; }
      int rating() const { return all_rating; }
      int optimisticRating() const { return optimistic_rating; }
    };

    std::ostream& operator<<(std::ostream& os, RatedMove const& moveLogProb);

    inline bool operator==(RatedMove const& lhs, RatedMove const& rhs)
    {
      return lhs.move()==rhs.move() && lhs.rating()==rhs.rating();
    }
    inline bool operator<(RatedMove const& lhs, RatedMove const& rhs)
    {
      if (lhs.rating() != rhs.rating())
	return lhs.rating() < rhs.rating();
      if (lhs.optimisticRating() != rhs.optimisticRating())
	return lhs.optimisticRating() < rhs.optimisticRating();
      return lhs.move() < rhs.move();
    }
    inline bool operator>(RatedMove const& lhs, RatedMove const& rhs)
    {
      if (lhs.rating() != rhs.rating())
	return lhs.rating() > rhs.rating();
      if (lhs.optimisticRating() != rhs.optimisticRating())
	return lhs.optimisticRating() > rhs.optimisticRating();
      return lhs.move() < rhs.move();
    }
  }
  using rating::RatedMove;
} // namespace osl


#endif /* OSl_RATEDMOVE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
