/* alphaBeta2.h
 */
#ifndef OSL_ALPHA_BETA2_H
#define OSL_ALPHA_BETA2_H

#include "osl/search/realizationProbability.h"
#include "osl/search/searchBase.h"
#include "osl/search/searchState2.h"
#include "osl/search/searchRecorder.h"
#include "osl/search/passCounter.h"
#include "osl/search/killerMoveTable.h"
#include "osl/search/searchTimer.h"
#include "osl/eval/evalTraits.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/eval/progressEval.h"
#include "osl/container/moveStack.h"
#include "osl/container/moveLogProbVector.h"
#include "osl/stat/average.h"
#include "osl/oslConfig.h"
#include <boost/scoped_array.hpp>
#include <boost/noncopyable.hpp>
#include <iosfwd>

namespace osl
{
  namespace search
  {
    class SimpleHashRecord;
    class SimpleHashTable;
    class MoveGenerator;
    struct MoveWithComment;

    class AlphaBeta2Window
    {
      CArray<int,2> values;
    public:
      explicit AlphaBeta2Window(int a=0) { values.fill(a); }
      AlphaBeta2Window(int a, int b) 
      {
	values[0] = a;
	values[1] = b;
      }
      AlphaBeta2Window(Player P, int a=0, int b=0) 
      {
	alpha(P) = a;
	beta(P) = b;
      }
      int& alpha(Player P) { return values[P]; }
      int& beta(Player P) { return values[alt(P)]; }

      int alpha(Player P) const { return values[P]; }
      int beta(Player P) const { return values[alt(P)]; }
      bool isConsistent() const { 
	return eval::notLessThan(BLACK, beta(BLACK), alpha(BLACK));
      }
      bool null() const { return values[0] == values[1]; }
      bool operator==(const AlphaBeta2Window& r) const
      {
	return values == r.values;
      }
    };

    /**
     * AlphaBeta2Tree のデータメンバーでdefault copy constructor でcopy可能なもの。
     */
    template <class EvalT>
    struct AlphaBeta2Common
#if OSL_WORDSIZE == 32
      : public misc::Align16New
#endif
    {
      static int rootLimitBias() 
      {
	return 0;
      }
      static int leafLimit() 
      {
	static int value = 300 + rootLimitBias();
	return value;
      }

      enum { MaxDepth = SearchState2Core::MaxDepth };
      EvalT eval;
      PassCounter pass_count;
      enum MoveType { INITIAL, HASH=INITIAL, TACTICAL, KILLER, PASS, ALL, FINISH };
      /** 現在の深さでの作成状態, nextMove() で利用 */
      CArray<MoveType, MaxDepth> move_type;
      CArray<bool, MaxDepth> in_pv;
      typedef FixedCapacityVector<Move,4> killer_t;
      CArray<killer_t, MaxDepth> killers;
      const MoveVector *root_ignore_moves; // acquaintance
      bool prediction_for_speculative_search;
      /** experimental */
      int multi_pv;
      
      explicit AlphaBeta2Common(const NumEffectState& s) 
	: eval(s), root_ignore_moves(0), prediction_for_speculative_search(false),
	  multi_pv(0)
      {
	in_pv[0] = true;
      }
    };
    struct RootPV
    {
      SearchState2::PVVector pv;
      int depth, eval;
      RootPV(int root_limit, const SearchState2::PVVector &p, int v)
	: pv(p), depth(root_limit), eval(v)
      {
      }
    };
    struct AlphaBeta2SharedRoot
    {
      /** value for each pv-update, for each iteration */
      std::vector<int> root_values, root_values_for_iteration;
      std::vector<Move> best_move_for_iteration;
      /** history of pv */
      std::vector<RootPV> last_pv;
      /** best move of the previous completed iteration */
      Move last_root_move;
      /** interim value for the current iteration */
      int last_root_value_update;
      AlphaBeta2SharedRoot() : last_root_value_update(0)
      {
      }
      void showLastPv(int limit) const;
      int sameBestMoves() const 
      {
	int ret = 0;
	for (int i=best_move_for_iteration.size()-2; i>=0; --i, ++ret)
	  if (best_move_for_iteration[i] != best_move_for_iteration[i+1])
	    break;
	return ret;
      }
    };

    template <class EvalT> struct AlphaBeta2Parallel;
    /**
     * "tree" of AlphaBeta2, copied by split
     */
    template <class EvalT>
    class AlphaBeta2Tree
      : public SearchBase<EvalT,SimpleHashTable,CountRecorder,RealizationProbability>,
	public SearchState2, public SearchTimer, protected AlphaBeta2Common<EvalT>, boost::noncopyable
    {
    public:
      typedef EvalT eval_t;
      typedef AlphaBeta2Common<EvalT> common_t;
      enum { MaxDepth = SearchState2Core::MaxDepth };
    protected:
      /** 静止探索も含めたノード数 */
      size_t node_count;
      FixedCapacityVector<MoveGenerator*, MaxDepth> generators;
      stat::Average mpn, mpn_cut, alpha_update, last_alpha_update;
      stat::Average ext, ext_limit;
      std::shared_ptr<AlphaBeta2Parallel<EvalT> > shared;
      std::shared_ptr<AlphaBeta2SharedRoot> shared_root;
    protected:
      static CArray<int, SearchState2Core::MaxDepth> depth_node_count;
      AlphaBeta2Tree(const NumEffectState& s, checkmate_t& checker,
		     SimpleHashTable *t, CountRecorder&);
      // share parallel data for split
      AlphaBeta2Tree(const AlphaBeta2Tree& src, AlphaBeta2Parallel<EvalT> *);
      ~AlphaBeta2Tree();
    private:
      void throwStop();
    public:
      struct BetaCut {};
      bool stopping() const
      {	
	return stop_tree || SearchTimer::stopping();
      }
      void testStop() { 
	throwIfNoMoreTime(this->recorder.allNodeCount());
	if (stop_tree)
	  throw BetaCut();
      }
    public:
      typedef AlphaBeta2Window Window;
      size_t nodeCount() const { return node_count; }
      static int rootAlpha(Player P, int last_value, Progress16 progress);
      static int stableThreshold(Player P, int last_value);

      template <Player P>
      const MoveLogProb nextMove();
    protected:
      void updateRootPV(Player P, std::ostream&, int, Move);
      void addMultiPV(Player P, int, Move);
      bool isStable(Player P, int new_value) const;
      void showFailLow(int result, Move m) const;
    private:
      void showPV(std::ostream&, int, Move, char stable) const;
    public:
      template <Player P> struct NextMove;
      friend struct NextMove<BLACK>;
      friend struct NextMove<WHITE>;
      template <Player P> struct NextQMove;
      friend struct NextQMove<BLACK>;
      friend struct NextQMove<WHITE>;
    protected:
      /**
       * alphaBetaSearch (move)
       * - makeMove(move)
       * - => alphaBetaSearchAfterMove
       * -- search extension etc.
       * -- => searchAllMoves => alphaBetaSearch (child move)
       * - unmakeMove(move)
       */
      template <Player P>
      int alphaBetaSearch(const MoveLogProb& move, Window window,
			  bool in_pv);
      template <Player P>
      int alphaBetaSearchAfterMove(const MoveLogProb& move, 
				   Window window, bool in_pv);
      template <Player P> int quiesce(Window);
      template <Player P> int quiesceStable(Window);
      template <Player P> int quiesceExp(Window);

      template <Player P>
      int searchAllMoves(SimpleHashRecord*, Window w);
      template <Player P>
      int searchAllMoves(Move m, int limit_consumption, 
			 SimpleHashRecord*, Window w);

      /** 初めの方で詰みを読む */
      template <Player P>
      bool tryCheckmate(SimpleHashRecord *record, bool in_pv, Move& checkmate_move);
      /** 負けそうな時にさらに詰みを読む */
      template <Player P>
      bool tryCheckmateAgain(SimpleHashRecord *record, Move& checkmate_move,
			     int node_count,
			     int best_value);

      /** 詰めろの有無を確認 */
      template <Player P>
      void testThreatmate(SimpleHashRecord *record, bool in_pv);

      /** alpha値が求まった後で他の手を調べる */
      template <Player P>
      void examineMovesRoot(const MoveLogProbVector&, size_t, Window,
			    MoveLogProb&, int&);

      template <Player P> int quiesceRoot(Window, int depth_left, Move& best_move, DualThreatmateState);
      template <Player P> int quiesce(Window, int depth_left, DualThreatmateState);
      template <Player P>
      bool quiesceWithMove(Move, Window&, int, Move&, int&, const DualThreatmateState&);

      void updateCheckmateCount();
      bool tryPass(SimpleHashRecord *record, Player P) const;
      MoveGenerator& makeGenerator();
      static MoveGenerator *alloc();
      static void dealloc(MoveGenerator *);
#ifdef OSL_SMP
    public:
      friend struct AlphaBeta2Parallel<EvalT>;
      struct NodeProperty;
      template <Player P> struct SearchJob;
      struct SearchJobData;
      struct Shared;
      friend struct Shared;
      friend struct SearchJob<BLACK>;
      friend struct SearchJob<WHITE>;
    protected:
      template <Player P>
      void examineMovesRootPar(const MoveLogProbVector&, size_t, Window,
			       MoveLogProb&, int&);
      void examineMovesRootPar(int tree_id);
      template <Player P>
      void testMoveRoot(int tree_id, const MoveLogProb&);

      template <Player P>
      bool examineMovesOther(Window& w, MoveLogProb& best_move, int& best_value, 
			     int& tried_moves, int& alpha_update, int& last_alpha_update);
      void examineMovesOther(int tree_id);
      template <Player P>
      bool testMoveOther(int tree_id, const MoveLogProb&, size_t index, bool in_pv);
#endif
    };

    /**
     * AlphaBeta の書き直し版
     */
    template <class EvalT>
    class AlphaBeta2
      : public AlphaBeta2Tree<EvalT>
    {
    public:
      typedef AlphaBeta2Tree<EvalT> base_t;
      typedef typename base_t::checkmate_t checkmate_t;
      typedef typename base_t::Window Window;
    public:
      AlphaBeta2(const NumEffectState& s, checkmate_t& checker,
		 SimpleHashTable *t, CountRecorder&);
      ~AlphaBeta2();

      /** 
       * entrance of alpha beta window search.
       *
       * see http://www.logos.t.u-tokyo.ac.jp/~gekisashi/algorithm/re_search.html.
       * rootDepth, curLimit are initialized here.
       */
      template <Player P>
      int alphaBetaSearchRoot(Window window, MoveLogProb& best_move,
			      int limit);
      static const Window fullWindow(Player P)
      {
	return Window(P, base_t::winThreshold(alt(P)), base_t::winThreshold(P));
      }
      int alphaBetaSearchRoot(Window window, MoveLogProb& best_move, int limit)
      {
	const Player P = this->state().turn();
	if (P == BLACK)
	  return alphaBetaSearchRoot<BLACK>(window, best_move, limit);
	else
	  return alphaBetaSearchRoot<WHITE>(window, best_move, limit);
      }
      int alphaBetaSearchRoot(MoveLogProb& best_move, int limit);
    
      /**
       * entrance of alpha beta iterative search.
       */
      Move computeBestMoveIteratively(int limit,
				      const int step=100, 
				      int initialLimit=600,
				      size_t nodeLimit=1600000,
				      const TimeAssigned& assign=TimeAssigned(milliseconds(60*1000)),
				      MoveWithComment *additional_info=0);
      bool isReasonableMove(Move move, int pawn_sacrifice=1);
      void setRootIgnoreMoves(const MoveVector *rim, bool prediction) 
      {
	assert(!prediction || rim);
	this->root_ignore_moves = rim;
	this->prediction_for_speculative_search = prediction;
      }

      static void showNodeDepth(std::ostream&);
      static void clearNodeDepth();
      void enableMultiPV(unsigned int width) { this->multi_pv = width; }
      const AlphaBeta2SharedRoot sharedRootInfo() const { return *(this->shared_root); }
    public:
      void setRoot(int limit);
      void makeMove(Move);
    private:
      enum PVCheckmateStatus {
	PVStable, PVThreatmateNotRecord, PVThreatmate, PVCheckmate, 
      };
      PVCheckmateStatus findCheckmateInPV(int verify_node, CArray<bool,2>& king_in_threat);
    }; // class AlphaBeta2

  } // namespace search
  typedef search::AlphaBeta2<eval::ProgressEval> AlphaBeta2ProgressEval;
  typedef search::AlphaBeta2<eval::ml::OpenMidEndingEval> AlphaBeta2OpenMidEndingEval;
} // namespace osl

#endif /* OSL_ALPHA_BETA2_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
