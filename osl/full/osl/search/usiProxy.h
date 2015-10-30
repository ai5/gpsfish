/* alphaBeta4.h
 */
#ifndef OSL_SEARCH_USIPROXY_H
#define OSL_SEARCH_USIPROXY_H

#include "osl/numEffectState.h"
#include "osl/search/searchTimer.h"
#include "osl/search/fixedEval.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/container/moveStack.h"
#include "osl/checkmate/dualDfpn.h"
#include <queue>
#include <mutex>
namespace osl
{
  namespace search
  {
    struct MoveWithComment;
    struct SimpleHashTable;
    struct CountRecorder;
    class UsiProxy : public SearchTimer, FixedEval
    {
      NumEffectState state;
      MoveStack history;
      const MoveVector *root_ignore_moves; // acquaintance
      bool prediction_for_speculative_search;
      struct Proxy;
      std::shared_ptr<Proxy> proxy;
      static std::mutex mutex;
      static std::queue<std::shared_ptr<Proxy>> proxy_pool;
      static std::string default_program;
      static std::vector<std::string> initial_commands;
    public:
      /**
       * Regits a USI program.
       *
       * @param program path to a usi program.
       *                (default $OSL_HOME/../gpsfish_dev/src/gpsfish)
       * @param initial_commands commands to send at start up
       * @return true if an appropriate program exists; otherwise, false
       */
      static bool setUp(const std::string& program="",
			const std::vector<std::string>& initial_commands={});
      static std::string getProgram() { return default_program; }

      // interface required for game_playing::SearchPlayer
      typedef DualDfpn checkmate_t;
      typedef eval::ml::OpenMidEndingEval eval_t;
      UsiProxy(const NumEffectState& s, checkmate_t& checker,
	       SimpleHashTable *t, CountRecorder&);
      UsiProxy(const UsiProxy&);
      UsiProxy& operator=(const UsiProxy&) = delete;      
      ~UsiProxy();

      Move computeBestMoveIteratively
      (int limit, int step, int initial_limit=600,
       size_t node_limit=1600000,
       const TimeAssigned& assign=TimeAssigned(milliseconds(60*1000)),
       MoveWithComment *additional_info=0);

      bool isReasonableMove(Move move, int pawn_sacrifice=1);
      void setRootIgnoreMoves(const MoveVector *rim, bool);
      void setHistory(const MoveStack& h);
      void enableMultiPV(unsigned int) {}
      std::vector<Move> toCSA(const std::vector<std::string>&) const;
    };    
  }
}

#endif /* OSL_SEARCH_ALPHABETA4_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
