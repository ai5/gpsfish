/* winCountTracer.h
 */
#ifndef GAME_PLAYING_WEIGHTTRACER_H
#define GAME_PLAYING_WEIGHTTRACER_H

#include "osl/game_playing/openingBookTracer.h"
#include "osl/book/openingBook.h"
#include <stack>
namespace osl
{
  namespace book
  {
    class WeightedBook;
  }
  namespace game_playing
  {
    /**
     * WeightedBookの追跡
     */
    class WeightTracer : public OpeningBookTracer
    {
    public:
      typedef book::WeightedBook WeightedBook;
    protected:
      WeightedBook& book;
      int state_index, start_index;
      Player turn;
      std::stack<int> state_stack;
      const osl::Move selectMoveAtRandom(const std::vector<osl::book::WMove>& moves) const;
      const int weight_coef_for_the_initial_move;
      const int weight_coef;
    public:
      explicit WeightTracer(WeightedBook&, bool verbose=false, 
                            const int weight_coef_for_the_initial_move = 16,
                            const int weight_coef = 10);
      WeightTracer(const WeightTracer&);
      OpeningBookTracer* clone() const;

      void update(Move);
      const Move selectMove() const;

      int stateIndex() const { return state_index; }
      bool isOutOfBook() const;
      void popMove();
    };

    class DeterminateWeightTracer : public WeightTracer
    {
      /**< select a move from topn moves */
      const int topn;
    public:
      explicit DeterminateWeightTracer(WeightedBook& book, bool verbose=false, const int topn=1,
                                       const int weight_coef_for_the_initial_move = 16,
                                       const int weight_coef = 10)
        : WeightTracer(book, verbose, weight_coef_for_the_initial_move, weight_coef), 
          topn(topn)
      {} 
      DeterminateWeightTracer(const DeterminateWeightTracer& copy)
        : WeightTracer(copy), topn(copy.getTopn()) {}
      OpeningBookTracer* clone() const;

      const Move selectMove() const;
      int getTopn() const {return topn;}
    };

  } // namespace game_playing
} // namespace osl

#endif // GAME_PLAYING_WEIGHTTRACER_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
