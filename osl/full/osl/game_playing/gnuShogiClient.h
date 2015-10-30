/* gnuShogiClient.h
 */
#ifndef GAMEPLAYING_GNUSHOGICLIENT_H
#define GAMEPLAYING_GNUSHOGICLIENT_H

#include "osl/game_playing/cuiClient.h"
namespace osl
{
  namespace game_playing
  {
    struct GnuShogiQuit {};
    class GnuShogiClient : public CuiClient
    {
    public:
      GnuShogiClient(ComputerPlayer *black, ComputerPlayer *white,
		     CsaLogger *l,
		     std::istream&, std::ostream&);
      ~GnuShogiClient();
    private:
      bool readAndProcessCommand();
      void processComputerMove(const search::MoveWithComment&, int seconds);
      void preComputeNextMove();
    };

  } // namespace game_playing
} // namespace osl


#endif /* _GNUSHOGICLIENT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
