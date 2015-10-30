/* alphaBeta3.h
 */
#ifndef OSL_ALPHABETA3_H
#define OSL_ALPHABETA3_H
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
    class AlphaBeta3 : public SearchTimer, FixedEval
    {
    public:
      // interface required for game_playing::SearchPlayer
      typedef SearchState2::checkmate_t checkmate_t;
      typedef eval::ml::OpenMidEndingEval eval_t;
      // typedef eval::PieceEval eval_t;
      // typedef eval::ProgressEval eval_t;
      AlphaBeta3(const NumEffectState& s, checkmate_t& checker,
		 SimpleHashTable *t, CountRecorder&);
      ~AlphaBeta3();
      Move computeBestMoveIteratively(int limit, int step, 
				      int initial_limit=600,
				      size_t node_limit=1600000,
				      const TimeAssigned& assign=TimeAssigned(milliseconds(60*1000)),
				      MoveWithComment *additional_info=0);
      bool isReasonableMove(Move move, int pawn_sacrifice=1);
      void setRootIgnoreMoves(const MoveVector *rim, bool);
      void setHistory(const MoveStack& h);
      void enableMultiPV(unsigned int) {}

      static void showNodeDepth(std::ostream&);
      static void clearNodeDepth();

      // original staff
      enum MoveCategory { Initial, KingEscape, Pass, TakeBack, Capture, Killer, CaptureAll, All };
      enum { MaxDepth = 64 };
      enum NodeType { PvNode = 0, CutNode = 1, AllNode = -1 };
      struct SearchInfo;
      struct PVInfo
      {
	Move move;
	int height;
	bool in_check;
      };
      struct PVVector : public FixedCapacityVector<PVInfo,MaxDepth> 
      {
	void setPV(Move m, const SearchInfo&, const PVVector&);
      };
      struct SearchInfo
      {
	SearchInfo();
	// input
	// (modified: alpha, node_type, eval)
	Move moved;
	HashKey hash_key;
	PathEncoding path;
	int height, extended;
	int alpha, beta;
	NodeType node_type;
	eval_t eval;		// before moved
	// output
	int search_value;
	int moves_tried;
	bool in_check;
	PVVector pv;
	// work area
	MoveVector moves;
	MoveCategory move_type;
	unsigned int move_index;
      };
    private:
      template <Player P> struct CallSearch;
      template <Player P> struct CallQuiesce;
      friend struct CallSearch<BLACK>;
      friend struct CallSearch<WHITE>;
      friend struct CallQuiesce<BLACK>;
      friend struct CallQuiesce<WHITE>;
      Move searchRoot(int limit);
      template <Player P> int makeMoveAndSearch(Move, int consume);
      template <Player P> void presearch();
      template <Player P> void search();
      template <Player P> Move nextMove();
      template <Player P> void quiesceRoot();
      template <Player P> int makeMoveAndQuiesce(Move);
      template <Player P> void quiesce();
    private:
      volatile int stop_by_alarm;
      NumEffectState state;
      int depth;
      CountRecorder& recorder;
      SimpleHashTable *table_common;
    public: // public for test
      template <Player P>
      static void generateAllMoves(const NumEffectState& state, const SearchInfo&, SearchInfo&);
      template <Player P>
      static void generateCapture(const NumEffectState& state, SearchInfo&);
      template <Player P>
      static void generateCaptureAll(const NumEffectState& state, SearchInfo&);
      template <Player P>      
      static bool seePlusLight(const NumEffectState& state, Move m);
    private:
      bool reductionOk() const;
      int evalValue() const;
    };
    
  }
  using search::AlphaBeta3;
}

#endif /* OSL_ALPHABETA3_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
