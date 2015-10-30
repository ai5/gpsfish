/* usiState.h
 */
#ifndef OSL_USISTATE_H
#define OSL_USISTATE_H
#include "osl/numEffectState.h"
#include <vector>
namespace osl
{
  namespace game_playing
  {
    struct UsiState
    {
      NumEffectState initial_state;
      std::vector<Move> moves;
      volatile bool aborted;

      UsiState();
      ~UsiState();

      void reset(const NumEffectState&, const std::vector<Move>&);
      void parseUsi(const std::string&);
      void openFile(std::string);
      bool isSuccessorOf(const UsiState& parent);
      const NumEffectState currentState() const;

      const std::string usiString() const;
      const std::string usiBoard() const;
      void parseIgnoreMoves(const std::string& line,
			    MoveVector& ignore_moves) const;
    };
  }
  using game_playing::UsiState;
}


#endif /* OSL_USISTATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
