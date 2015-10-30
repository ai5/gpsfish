/* csaLogger.h
 */
#ifndef GAME_PLAYING_CSALOGGER_H
#define GAME_PLAYING_CSALOGGER_H

#include "osl/basic_type.h"
#include <iosfwd>
namespace osl
{
  class Move;
  class Sennichite;
  namespace state
  {
    class SimpleState;
  }
  namespace search
  {
    struct MoveWithComment;
  }
  namespace game_playing
  {
    class TimeKeeper;
    /**
     * 棋譜の記録
     */
    class CsaLogger
    {
      std::ostream& output;
    public:
      explicit CsaLogger(std::ostream& os);
      ~CsaLogger();

      void init(const char *black, const char *white,
		const SimpleState& state);
      
      void pushMove(const Move& move, int seconds);
      void pushMove(const search::MoveWithComment& move, int seconds);
      void popMove();
      void showTimeLeft(const TimeKeeper&);
      void writeComment(const char *comment);
      void resign(Player resigned);
      void inputError(const char *);
      void breakGame();
      void endByRepetition(const Sennichite&);
      void endByDeclaration(Player declarer);
    private:
      void writeLine(const char *line);
      void writeWinner(Player winner);
      void writeCurrentDate();
    };

  } // namespace game_playing
} // namespace osl

#endif /* GAME_PLAYING_CSALOGGER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
