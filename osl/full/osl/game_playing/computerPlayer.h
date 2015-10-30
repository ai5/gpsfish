/* computerPlayer.h
 */
#ifndef GAME_PLAYING_COMPUTERPLAYER_H
#define GAME_PLAYING_COMPUTERPLAYER_H

#include "osl/search/moveWithComment.h"
#include "osl/numEffectState.h"
namespace osl
{
  namespace container 
  {
    class MoveVector;
  }
  namespace search
  {
    struct TimeAssigned;
  }
  namespace game_playing
  {
    class GameState;
    class ComputerPlayer
    {
    protected:
      bool speculative_search_allowed;
    public:
      ComputerPlayer() : speculative_search_allowed(false)
      {
      }
      virtual ~ComputerPlayer();
      /** new したものを返す */
      virtual ComputerPlayer* clone() const = 0;

      virtual void pushMove(Move m)=0;
      virtual void popMove()=0;
      /** @return success to stop */
      virtual bool isReasonableMove(const GameState&,
				    Move move, int pawn_sacrifice);
      /**
       * @param seconds 残り持ち時間
       */
      virtual const MoveWithComment selectBestMove(const GameState&, int seconds, int elapsed,
						int byoyomi)=0;

      virtual void setInitialState(const NumEffectState&);
      /**
       * 相手時間の探索を許可する (GameManager が操作)
       */
      virtual void allowSpeculativeSearch(bool value);
      /** 探索をとめる */
      virtual bool stopSearchNow();

      virtual void setRootIgnoreMoves(const MoveVector *rim, bool prediction);
    };

    class ComputerPlayerSelectBestMoveInTime
    {
    public:
      virtual ~ComputerPlayerSelectBestMoveInTime();
      virtual const MoveWithComment selectBestMoveInTime(const GameState&, const search::TimeAssigned&)=0;      
    };
    /**
     * 常に投了する
     */
    class ResignPlayer : public ComputerPlayer
    {
    public:
      ~ResignPlayer();
      ComputerPlayer* clone() const 
      {
	return new ResignPlayer();
      }
      void pushMove(Move m);
      void popMove();
      const MoveWithComment selectBestMove(const GameState&, int, int, int);
    };

    /**
     * 合法手をランダムに指す
     */
    class RandomPlayer : public ComputerPlayer
    {
    public:
      ComputerPlayer* clone() const 
      {
	return new RandomPlayer();
      }
      ~RandomPlayer();
      void pushMove(Move m);
      void popMove();
      const MoveWithComment selectBestMove(const GameState&, int, int, int);
    };

  } // namespace game_playing
} // namespace osl

#endif /* GAME_PLAYING_COMPUTERPLAYER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
