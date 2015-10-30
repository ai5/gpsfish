/* csaLogger.cc
 */
#include "osl/game_playing/csaLogger.h"
#include "osl/game_playing/timeKeeper.h"
#include "osl/search/moveWithComment.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/sennichite.h"
#include <iostream>
#ifdef _WIN32
#include <ctime>
#include <cstring>
#endif

osl::game_playing::
CsaLogger::CsaLogger(std::ostream& os)
    : output(os)
{
}
osl::game_playing::
CsaLogger::~CsaLogger() 
{
}

void osl::game_playing::
CsaLogger::init(const char *black, const char *white,
		const SimpleState& state)
{
  output << "N+" << black << std::endl
	 << "N-" << white << std::endl;
  output << state << std::flush;
  writeCurrentDate();
}

void osl::game_playing::
CsaLogger::pushMove(const Move& move, int seconds)
{
  output << csa::show(move)
	 << std::endl << "T" << seconds << std::endl << std::flush;
}

void osl::game_playing::
CsaLogger::pushMove(const MoveWithComment& move, int seconds)
{
  pushMove(move.move, seconds);
  if (! move.moves.empty()) 
  {
    output << "'** " << move.value;
    for (Move m: move.moves)
    {
      output << " " << csa::show(m);
    }
    output << std::endl << std::flush;
  }
}

void osl::game_playing::
CsaLogger::popMove()
{
  writeLine("%MATTA");	// csa の%MATTA は2手戻すので意味が違う?
}

void osl::game_playing::
CsaLogger::showTimeLeft(const TimeKeeper& keeper)
{
  output << "'time left " << keeper.timeLeft(BLACK) << " " << keeper.timeLeft(WHITE)
	 << std::endl << std::flush;
}

void osl::game_playing::
CsaLogger::writeLine(const char *line)
{
  output << line << std::endl << std::flush;
}

void osl::game_playing::
CsaLogger::writeComment(const char *comment)
{
  output << "'" << comment << std::endl << std::flush;
}

void osl::game_playing::
CsaLogger::writeCurrentDate()
{
  char ctime_buf[64];
  const time_t t = time(0);
  output << "'" << ctime_r(&t, ctime_buf);	// ctime returns string with "\n"
}

void osl::game_playing::
CsaLogger::resign(Player resigned)
{
  output << "%TORYO" << std::endl;
  writeWinner(alt(resigned));
  writeCurrentDate();
}

void osl::game_playing::
CsaLogger::inputError(const char *message)
{
  output << "'!!! input error: " << message << std::endl << std::flush;
}

void osl::game_playing::
CsaLogger::breakGame()
{
  output << "%CHUDAN" << std::endl << std::flush;
}

void osl::game_playing::
CsaLogger::endByRepetition(const Sennichite& result)
{
  output << "%SENNICHITE" << std::endl;
  output << "'" << result << std::endl << std::flush;
  assert(! result.isNormal());
  if (result.hasWinner())
    writeWinner(result.winner());
  else
    writeComment("draw");
  writeCurrentDate();
}

void osl::game_playing::
CsaLogger::endByDeclaration(Player declarer)
{
  output << "%KACHI" << std::endl;
  output << "'declared by " << declarer << std::endl << std::flush;
  writeCurrentDate();
}

void osl::game_playing::
CsaLogger::writeWinner(Player winner)
{
  output << "'" << winner << " win" << std::endl << std::flush;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
