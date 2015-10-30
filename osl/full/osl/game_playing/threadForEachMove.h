/* threadForEachMove.h
 */
#ifndef OSL_THREADFOREACHMOVE_H
#define OSL_THREADFOREACHMOVE_H

#include "osl/game_playing/computerPlayer.h"
#include "osl/game_playing/speculativeModel.h"

namespace osl
{
  namespace game_playing
  {
    class SearchPlayer;
    /**
     * 相手の手の予測1つにつき1thread
     */
    class ThreadForEachMove : public SpeculativeModel
    {
      struct SpeculativeThread;
      std::unique_ptr<SpeculativeThread> speculative_thread0;
      std::unique_ptr<SpeculativeThread> speculative_thread1;
      int max_threads;
    public:
      explicit ThreadForEachMove(int max_threads=1);
      ~ThreadForEachMove();

      void setMaxThreads(int new_max_threads)
      {
	max_threads = new_max_threads;
      }
      void startSpeculative(const std::shared_ptr<GameState> state,
			    const SearchPlayer& main_player);
      void stopOtherThan(Move);
      void stopAll();

      const MoveWithComment waitResult(Move last_move, int wait_for,
				       SearchPlayer& main_player, int);

      void selectBestMoveCleanUp();
    };
  } // game_playing
} // osl

#endif /* OSL_THREADFOREACHMOVE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
