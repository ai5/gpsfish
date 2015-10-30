/* speculativeModel.h
 */
#ifndef OSL_SPECULATIVEMODEL_H
#define OSL_SPECULATIVEMODEL_H

#include "osl/game_playing/computerPlayer.h"
#include "osl/search/searchTimer.h"

namespace osl
{
  namespace game_playing
  {
    class SearchPlayer;
    /**
     * 相手の手の予測1つにつき1thread
     */
    class SpeculativeModel
    {
    public:
      virtual ~SpeculativeModel();

      virtual void setMaxThreads(int);

      virtual void startSpeculative(const std::shared_ptr<GameState> state,
				    const SearchPlayer& main_player)=0;
      virtual void stopOtherThan(Move)=0;
      virtual void stopAll()=0;
      virtual const HashKey searchState() const=0;

      /**
       * @param byoyomi 対局条件を伝えるために利用
       */
      virtual const MoveWithComment waitResult(Move last_move, search::TimeAssigned,
					       SearchPlayer& main_player,
					       int byoyomi)=0;

      virtual void selectBestMoveCleanUp()=0;
      void clearResource();
    };
  } // game_playing
} // osl

#endif /* OSL_SPECULATIVEMODEL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
