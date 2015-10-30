/* winCountTracer.h
 */
#ifndef GAME_PLAYING_WINCOUNTTRACER_H
#define GAME_PLAYING_WINCOUNTTRACER_H

#include "osl/game_playing/openingBookTracer.h"
#include <stack>
namespace osl
{
  namespace book
  {
    class WinCountBook;
  }
  namespace game_playing
  {
    /**
     * WinCountBookの追跡
     */
    class WinCountTracer : public OpeningBookTracer
    {
    public:
      typedef book::WinCountBook WinCountBook;
    private:
      WinCountBook& book;
      int state_index;
      Player turn;
      int randomness;
      bool verbose;
      std::stack<int> state_stack;
    public:
      /* @param randomness ゼロ以外の場合，最良でない手も確率的に選択 */
      explicit WinCountTracer(WinCountBook&, 
			      int randomness=0, bool verbose=false);
      WinCountTracer(const WinCountTracer&);
      OpeningBookTracer* clone() const;

      void update(Move);
      const Move selectMove() const;

      int stateIndex() const { return state_index; }
      bool isOutOfBook() const;
      void popMove();
    };
  } // namespace game_playing
} // namespace osl

#endif /* _WINCOUNTTRACER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
