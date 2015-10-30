/* usiResponse.h
 */
#ifndef OSL_USIRESPONSE_H
#define OSL_USIRESPONSE_H
#include "osl/game_playing/usiState.h"
#include "osl/numEffectState.h"
#include "osl/container/moveLogProbVector.h"
#include <string>
namespace osl
{
  namespace game_playing
  {
    struct UsiState;
    class UsiResponse
    {
      const UsiState& usi_state;
      const bool new_move_probability, verbose;
    public:
      UsiResponse(const UsiState&, bool new_move_probability, bool verbose);
      ~UsiResponse();
      
      bool hasImmediateResponse(const std::string& command,
				std::string& out);
      void genmoveProbability(int limit, MoveLogProbVector& out);
    private:
      MoveVector generateGoodMoves();
      void genmoveProbability(int limit, std::string& out);
      void genmove(std::string& out);
      void csashow(const NumEffectState& state, std::string& out);
      void csamove(const NumEffectState& state, const std::string& str,
		   std::string& out);
      void ki2moves(const NumEffectState& current,
		    const std::string& moves_str, std::string& out);
      void ki2currentinfo(const NumEffectState& current, std::string& out);
      void isValidPosition(const std::string& line, std::string& out);
    };
  }
  using game_playing::UsiResponse;
}

#endif /* OSL_USIRESPONSE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
