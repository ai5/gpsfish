/* gameManager.h
 */
#ifndef GAMEPLAYING_GAMEMANAGER_H
#define GAMEPLAYING_GAMEMANAGER_H
#include "osl/game_playing/timeKeeper.h" 
#include "osl/container.h"
namespace osl
{
  class Sennichite;
  namespace search
  {
    struct MoveWithComment;    
  }
  namespace game_playing
  {
    class GameState;
    class CsaLogger;
    class ComputerPlayer;
    
    class GameManager
    {
    protected:
      CArray<ComputerPlayer*,2> players;
      CArray<bool,2> computers;
      std::unique_ptr<GameState> state;
      std::unique_ptr<CsaLogger> logger;
      TimeKeeper time_keeper;
    private:
      int byoyomi;
      
      ComputerPlayer *player(Player turn) const
      {
	return players[turn];
      }
    public:
      struct EndGame {};
      /**
       * @param black, white 0 の場合，その手番をコンピュータにできない
       * @param logger 所有権移転．new したものを渡す
       */
      GameManager(ComputerPlayer *black, ComputerPlayer *white, CsaLogger *logger);
      virtual ~GameManager();
      void load(const char *csa_filename, bool verbose=false);
      void setTimeLeft(int black_time, int white_time);
      void setByoyomi(int seconds) { byoyomi = seconds; }

      void resetLogger(CsaLogger *l);
      
      void setComputerPlayer(Player turn, bool is_computer);
      bool isComputer(Player turn) const 
      {
	return computers[turn] && player(turn); 
      }

      /**
       * @param consumed 消費時間を返す
       */
      const search::MoveWithComment computeMove(int& consumed);
      int eval(Player turn, Move m);
    protected:
      const Sennichite pushMove(const search::MoveWithComment&, int seconds);
      void popMove();
    };

  } // namespace game_playing
} // namespace osl

#endif /* _GAMEMANAGER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
