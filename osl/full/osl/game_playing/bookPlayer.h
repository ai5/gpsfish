/* bookPlayer.h
 */
#ifndef GAME_PLAYING_BOOKPLAYER_H
#define GAME_PLAYING_BOOKPLAYER_H

#include "osl/game_playing/computerPlayer.h"

namespace osl
{
  namespace game_playing
  {
    class OpeningBookTracer;
    /**
     * 定跡がある間は定跡を使うComputerPlayer
     */
    class BookPlayer 
      : public ComputerPlayer,
	public ComputerPlayerSelectBestMoveInTime
    {
      std::unique_ptr<OpeningBookTracer> book;
      std::unique_ptr<ComputerPlayer> searcher;
      int book_limit;
      int current_moves;
      bool valid_initial_position;
    public:
      /** 所有権移転: new したものを渡すこと */
      BookPlayer(OpeningBookTracer*, ComputerPlayer*);
      ~BookPlayer();
      ComputerPlayer* clone() const;

      /** 何手まで定跡を使うかを設定. -1 なら無限大 */
      void setBookLimit(int new_limit);

      void setInitialState(const NumEffectState&);
      void pushMove(Move m);
      void popMove();
      const MoveWithComment selectBestMove(const GameState&, int seconds, int elapsed,
					int byoyomi);
      const MoveWithComment selectBestMoveInTime(const GameState&, const search::TimeAssigned&);

      bool bookAvailable() const;

      // delegations ...
      void allowSpeculativeSearch(bool value);
      virtual bool stopSearchNow();
      /** 注意: 定跡に関しては指定は無効 */
      void setRootIgnoreMoves(const MoveVector *rim, bool prediction);
    private:
      const Move moveByBook(const GameState& state);
    };

  } // namespace game_playing
} // namespace osl


#endif /* GAME_PLAYING_BOOKPLAYER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
