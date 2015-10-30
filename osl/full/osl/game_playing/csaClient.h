/* csaClient.h
 */
#ifndef GAME_PLAYING_CSACLIENT_H
#define GAME_PLAYING_CSACLIENT_H
#include "osl/game_playing/cuiClient.h"
#include <string>
namespace osl
{
  namespace game_playing
  {
    class CsaClient : public CuiClient
    {
      bool show_move_with_comment;
      bool silent;
      std::string line;
    public:
      CsaClient(ComputerPlayer *black, ComputerPlayer *white,
		CsaLogger *l,
		std::istream&, std::ostream&);
      ~CsaClient();
      void setShowMoveWithComment(bool value=true);
      void setSilent(bool new_value=true) {
	silent = new_value;
      }
    private:
      bool readAndProcessCommand();
      void processComputerMove(const search::MoveWithComment&, int seconds);
    };
  } // namespace game_playing
} // namespace osl


#endif /* _CSACLIENT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
