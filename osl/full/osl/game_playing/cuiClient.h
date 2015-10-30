/* cuiClient.h
 */
#ifndef GAME_PLAYING_CUICLIENT_H
#define GAME_PLAYING_CUICLIENT_H

#include "osl/game_playing/gameManager.h"
namespace osl
{
  namespace game_playing
  {
    class CuiClient : public GameManager
    {
    protected:
      std::istream& is;
      std::ostream& os;
    private:
      /** non-zero value forces resign */
      volatile int stop_by_outside;
    public:
      CuiClient(ComputerPlayer *black, ComputerPlayer *white,
		CsaLogger *l, std::istream&, std::ostream&);
      ~CuiClient();
      void run(const char *black, const char *white);
      void run();
      volatile int *stopFlag() { return &stop_by_outside; }
    protected:
      /** @return read next command immediately */
      virtual bool readAndProcessCommand()=0;
      virtual void processComputerMove(const search::MoveWithComment&, int seconds)=0;
    };
  } // namespace game_playing
} // namespace osl

#endif /* _CUICLIENT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
