/* speculativeAllMoves.h
 */
#ifndef OSL_SPECULATIVEALLMOVES_H
#define OSL_SPECULATIVEALLMOVES_H

#include "osl/game_playing/computerPlayer.h"
#include "osl/game_playing/speculativeModel.h"
#include "osl/misc/lightMutex.h"
#include "osl/misc/milliSeconds.h"
#include <thread>
#include <condition_variable>

namespace osl
{
  namespace misc
  {
    class RealTime;
  }
  namespace search
  {
    struct TimeAssigned;
  }
  namespace game_playing
  {
    class SearchPlayer;
    /**
     * 1threadで全ての手を順番に投機的探索をする
     */
    class SpeculativeAllMoves : public SpeculativeModel
    {
    public:
      class SearchAllMoves;
      class ResultVector;
    private:
      std::shared_ptr<SearchAllMoves> searcher;
      std::unique_ptr<std::thread> thread;
      std::unique_ptr<ResultVector> results;
      std::mutex mutex;
      int last_search_seconds;
      bool has_byoyomi;
      bool allowed;
      HashKey search_state;
    public:
      SpeculativeAllMoves();
      ~SpeculativeAllMoves();

      void startSpeculative(const std::shared_ptr<GameState> state,
			    const SearchPlayer& main_player);
      void stopOtherThan(Move);
      void stopAll();

      void setMaxThreads(int new_max_threads)
      {
	std::lock_guard<std::mutex> lk(mutex);
	allowed = (new_max_threads > 0);
      }

      const MoveWithComment waitResult(Move last_move, search::TimeAssigned wait_for,
				       SearchPlayer& main_player, int byoyomi);

      void selectBestMoveCleanUp();
      void clearResource();
      const HashKey searchState() const { return search_state; }
    private:
      struct Runner;
    };

    class SpeculativeAllMoves::ResultVector
    {
      typedef FixedCapacityVector<std::pair<Move,MoveWithComment>,Move::MaxUniqMoves> vector_t;
      vector_t data;
      typedef LightMutex Mutex;
      mutable Mutex mutex;
    public:
      ResultVector();
      ~ResultVector();
      
      void add(Move prediction, const MoveWithComment& result);
      const MoveWithComment* find(Move prediction) const;
      void clear();
      void show(std::ostream&) const;
    };

    /**
     * 指手を生成し，結果をresultsにためる．
     * run を別threadで動かすことを想定しているが，逐次でもテスト可
     */
    class SpeculativeAllMoves::SearchAllMoves
    {
    public:
      enum Status { 
	INITIAL, RUNNING, PREDICTION1, PREDICTION2, SEARCH1, SEARCH2, FINISHED
      };
      struct Generator;
      friend struct Generator;
      friend class SpeculativeAllMoves;
    private:
      std::shared_ptr<GameState> state;
      std::shared_ptr<SearchPlayer> player;
      std::unique_ptr<Generator> generator;
      SpeculativeAllMoves::ResultVector& results;
      double next_iteration_coefficient;
      Move current_move;
      volatile Status status;
      int seconds;
      typedef std::mutex Mutex;
      mutable Mutex mutex;
      std::condition_variable condition;
      /** true なら次の予想探索にはいらない */
      volatile bool stop_flag;
    public:
      explicit SearchAllMoves(SpeculativeAllMoves::ResultVector&);
      ~SearchAllMoves();

      void setUp(const GameState&, const SearchPlayer&, int standard_seconds,
		 bool has_byoyomi);

      void run();
      
      void stopNow();
      void stopOtherThan(Move);
      void waitRunning();
      bool isFinished() const { return status == FINISHED; }

      void setTimeAssign(const search::TimeAssigned&);
      const time_point startTime();
      const Move currentMove() const;

      SearchPlayer* currentPlayer() { return player.get(); }
    private:
      const MoveWithComment testMove(Move);
      struct StatusLock;
    };
  } // game_playing
} // osl

#endif /* OSL_SPECULATIVEALLMOVES_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
