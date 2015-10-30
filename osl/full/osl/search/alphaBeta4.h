/* alphaBeta4.h
 */
#ifndef OSL_SEARCH_ALPHABETA4_H
#define OSL_SEARCH_ALPHABETA4_H

#include "osl/numEffectState.h"
#include "osl/search/searchTimer.h"
#include "osl/search/fixedEval.h"
// temporal
#include "osl/search/searchState2.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/eval/progressEval.h"

namespace osl
{
  namespace search
  {
    class CountRecorder;
    class SimpleHashTable;
    struct MoveWithComment;
  }
  namespace search4
  {
    using search::CountRecorder;
    using search::SimpleHashTable;
    using search::MoveWithComment;
    using search::SearchState2;
    using search::SearchTimer;
    using search::FixedEval;
    using search::TimeAssigned;
    class AlphaBeta4 : public SearchTimer, FixedEval
    {
    public:
      // interface required for game_playing::SearchPlayer
      typedef SearchState2::checkmate_t checkmate_t;
      typedef eval::ml::OpenMidEndingEval eval_t;
      // typedef eval::PieceEval eval_t;
      // typedef eval::ProgressEval eval_t;
      AlphaBeta4(const NumEffectState& s, checkmate_t& checker,
		 SimpleHashTable *t, CountRecorder&);
      ~AlphaBeta4();
      Move computeBestMoveIteratively
      (int limit, int step, 
       int initial_limit=600,
       size_t node_limit=1600000,
       const TimeAssigned& assign=TimeAssigned(milliseconds(60*1000)),
       MoveWithComment *additional_info=0);
      bool isReasonableMove(Move move, int pawn_sacrifice=1);
      void setRootIgnoreMoves(const MoveVector *rim, bool);
      void setHistory(const MoveStack& h);
      void enableMultiPV(unsigned int) {}
    };
    
  }
  using search4::AlphaBeta4;
}

#endif /* OSL_SEARCH_ALPHABETA4_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
