/* moveScore.h
 */
#ifndef OSL_SEARCH_MOVESCORE_H
#define OSL_SEARCH_MOVESCORE_H
#include "osl/numEffectState.h"
namespace osl
{
  namespace search
  {
    struct MoveScore {
      Move move;
      int score;
      static MoveScore* sortPositive(MoveScore *f, MoveScore *l);

      static MoveScore* generateCapture
      (const NumEffectState& state, MoveScore *out);
      template <Player P>
      static MoveScore* generateCapture
      (const NumEffectState& state, MoveScore *out);
      static MoveScore* generateNoCapture
      (const NumEffectState& state, MoveScore *out);
      static MoveScore* generateCheckNoCapture
      (const NumEffectState& state, MoveScore *out);
      static MoveScore* generateAll
      (const NumEffectState& state, MoveScore *out);
      static MoveScore* generateKingEscape
      (const NumEffectState& state, MoveScore *out);
    };

    inline bool operator<(const MoveScore& f, const MoveScore& s) {
      return f.score < s.score; 
    }
    inline bool operator>(const MoveScore& f, const MoveScore& s) {
      return f.score > s.score; 
    }
  }
}

#endif /* OSL_SEARCH_MOVESCORE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
