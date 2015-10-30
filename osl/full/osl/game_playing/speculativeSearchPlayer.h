/* speculativeSearchPlayer.h
 */
#ifndef OSL_SPECULATIVESEARCHPLAYER_H
#define OSL_SPECULATIVESEARCHPLAYER_H

#include "osl/game_playing/computerPlayer.h"
#include "osl/game_playing/speculativeModel.h"

namespace osl
{
  namespace game_playing
  {
    class SearchPlayer;
    class SpeculativeModel;
    /**
     * 相手時間中に探索
     */
    class SpeculativeSearchPlayer : public ComputerPlayer
    {
      std::unique_ptr<SearchPlayer> main_player;
      std::shared_ptr<GameState> previous_state;
      std::unique_ptr<SpeculativeModel> speculative;
      Player my_turn;
    public:
      /** 所有権移転 */
      SpeculativeSearchPlayer(Player my_turn, SearchPlayer *);
      ~SpeculativeSearchPlayer();
      ComputerPlayer* clone() const;

      void pushMove(Move m);
      void popMove();
      bool stopSearchNow();

      /** ThreadForEachMove のみに有効 */
      void setMaxThreads(int new_max_threads);

      const MoveWithComment selectBestMove(const GameState&, int limit, int elapsed, int byoyomi);
      search::TimeAssigned standardSearchSeconds(const GameState&, int limit, int elapsed, int byoyomi) const;
    private:
      void selectBestMoveCleanUp(const GameState& state);
    };
  } // game_playing
} // osl

#endif /* OSL_SPECULATIVESEARCHPLAYER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
