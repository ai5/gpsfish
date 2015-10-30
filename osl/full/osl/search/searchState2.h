/* searchState2.h
 */
#ifndef OSL_SEARCHSTATE2_H
#define OSL_SEARCHSTATE2_H

#include "osl/search/killerMoveTable.h"
#include "osl/search/bigramKillerMove.h"
#include "osl/search/historyTable.h"
#include "osl/search/firstMoveThreatmate.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/checkmate/dualDfpn.h"
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/effect_util/neighboring8Direct.h"
#include "osl/numEffectState.h"
#include "osl/hashKey.h"
#include "osl/repetitionCounter.h"
#include "osl/container/moveStack.h"

#include <boost/utility.hpp>

namespace osl
{
  namespace search
  {
    /**
     * SimpleHashRecord* のstack. 
     * 先頭要素はrootを意味する。
     */
    class RecordStack2
    {
      static const int SEARCH_DEPTH_MAX = 64;
      FixedCapacityVector<SimpleHashRecord*, SEARCH_DEPTH_MAX> data;
    public:
      RecordStack2();
      void clear();
      void push(SimpleHashRecord *r) { data.push_back(r); }
      void pop() { assert(size() > 1); data.pop_back(); }

      SimpleHashRecord* lastRecord(unsigned int n=0) const
      {
	assert(size() > n);
	return data[size()-1-n];
      }
      SimpleHashRecord* rootRecord() const
      {
	assert(! empty());
	return data[0];
      }
      void setRootRecord(SimpleHashRecord *root) { data[0] = root; }
      void setLastRecord(SimpleHashRecord *r) { 
	assert(size() > 0);	// 1 for root
	data[size()-1] = r;
      }

      size_t size() const { return data.size(); }
      bool empty() const { return data.empty(); }
      bool hasLastRecord(unsigned int n=0) const
      {
	return size() > n;
      }

      void dump() const;
    };

    class Worker;
    /**
     * 並列探索をする場合に共有されるもの
     */
    struct SearchState2Shared : boost::noncopyable
    {
      BigramKillerMove bigram_killers;
      KillerMoveTable killer_moves;
      HistoryTable history_table;

      SearchState2Shared();
      ~SearchState2Shared();
    };
#define search_assert(x) assert(((x) || (SearchState2Core::abort())))
#define search_assert2(x, m) assert(((x) || (SearchState2Core::abort(m))))
    struct AlphaBeta2ParallelCommon;
    /**
     */
    class SearchState2Core
#if OSL_WORDSIZE == 32
      : public misc::Align16New
#endif
    {
      friend struct AlphaBeta2ParallelCommon;
    public:
      enum { MaxDepth = 64 };
      typedef DualDfpn checkmate_t;
    protected:
      NumEffectState current_state, root_state;
      checkmate_t* checkmate_searcher;
#ifdef OSL_SMP
    public:
      void setCheckmateSearcher(checkmate_t *new_checkmate) {
	checkmate_searcher = new_checkmate;
      }
    private:
#endif
      PathEncoding current_path;
      MoveStack move_history;
      int root_depth;
    protected:
      RecordStack2 record_stack;
      RepetitionCounter repetition_counter;
      std::shared_ptr<SearchState2Shared> shared;
    public:
      typedef FixedCapacityVector<Move,MaxDepth> PVVector;
    protected:
      CArray<PVVector,MaxDepth> pv;
      enum NodeType { PvNode = 0, AllNode = 1, CutNode = -1, };
      CArray<NodeType,MaxDepth> node_type;
    public:
      /** beta cut in parallel search */
      volatile bool stop_tree;
#ifndef MINIMAL
      static CArray<int, MaxDepth> depth_node_count_quiesce;
#endif
      SearchState2Core(const NumEffectState& s, checkmate_t& checker);
      virtual ~SearchState2Core();
      int curDepth() const { return history().size() - root_depth; }

      /**
       * state のコピーを行う.
       *
       * this->state は探索終了後も保存されるが，探索中に exception が起こると
       * 破壊されている
       */
      virtual void setState(const NumEffectState& s);
      void setHistory(const MoveStack& h);
      bool hasLastRecord(unsigned int n=0) const
      {
	return record_stack.hasLastRecord(n);
      }
      SimpleHashRecord* lastRecord(unsigned int n=0) 
      {
	return record_stack.lastRecord(n); 
      }
      const SimpleHashRecord* lastRecord(unsigned int n=0) const
      {
	return record_stack.lastRecord(n); 
      }
      SimpleHashRecord *rootRecord()
      {
	return record_stack.rootRecord();
      }
      void setCurrentRecord(SimpleHashRecord *r)
      {
	search_assert((int)record_stack.size() == curDepth()+1);
	record_stack.setLastRecord(r);
      }
      void setRootRecord(SimpleHashRecord *root)
      {
	search_assert(record_stack.size() == 1);
	search_assert(curDepth() == 0);
	record_stack.setRootRecord(root); 
      }

      // killer move
      void setKillerMove(Move best_move)
      {
	if (best_move.isPass())
	  return;
	shared->killer_moves.setMove(curDepth(), best_move);
	const Move last_move = lastMove();
	if (! last_move.isInvalid()) {
	  search_assert(best_move.player() != last_move.player());
	  shared->bigram_killers.setMove(last_move, best_move);
	}
      }
      void getBigramKillerMoves(MoveVector& moves) const
      {
	shared->bigram_killers.getMove(state(), lastMove(), moves);
#ifdef TRI_PLY_BIGRAM_KILLERS
	if (move_history.hasLastMove(3))
	  shared->bigram_killers.getMove(state(), lastMove(3), moves);
#endif
      }
      void getKillerMoves(MoveVector& moves) const
      {
	getBigramKillerMoves(moves);
	shared->killer_moves.getMove(state(), curDepth(), moves);
      }
      const BigramKillerMove& bigramKillerMove() const { 
	return shared->bigram_killers;
      }
      void setBigramKillerMove(const BigramKillerMove& killers);
      HistoryTable& historyTable() { return shared->history_table; }
      const HistoryTable& historyTable() const { return shared->history_table; }
      // doUndo
      void pushPass()
      {
	const Move pass = Move::PASS(current_state.turn());
	current_state.changeTurn();
	current_path.pushMove(pass);
	move_history.push(pass);
      }
      void popPass()
      {
	const Move pass = Move::PASS(alt(current_state.turn()));
	current_state.changeTurn();
	current_path.popMove(pass);
	move_history.pop();
      }
    private:
      /**
       * ApplyMoveの前に行うこと
       */
      void pushBeforeApply(Move move)
      {
	move_history.push(move);
	record_stack.push(0);
	assert(curDepth() > 0);
	node_type[curDepth()] = static_cast<NodeType>(-node_type[curDepth()-1]);
      }
      struct Updator
      {
	const HashKey& new_hash;
	SearchState2Core *state;
	Updator(const HashKey& h, SearchState2Core *s)
	  : new_hash(h), state(s)
	{
	}
	void update()
	{
	  state->updateRepetitionCounterAfterMove(new_hash);
	}
      };
      template <class Function>
      struct UpdateWrapper : public Updator
      {
	Function& function;
	UpdateWrapper(const HashKey& h, SearchState2Core *s, Function& f)
	  : Updator(h, s), function(f)
	{
	}
	void operator()(Square to)
	{
	  update();
	  function(to);
	}
      };
      friend struct Updator;
      /**
       * pushBeforeApply の後，ApplyMoveの中，Functionを呼ぶ前に呼ばれる
       */
      void updateRepetitionCounterAfterMove(const HashKey& new_hash)
      {
	repetition_counter.push(new_hash, current_state);
      }
      /**
       * ApplyMoveの後に行うこと
       */
      void popAfterApply()
      {
	record_stack.pop();
	repetition_counter.pop();
	move_history.pop();
      }
#ifndef NDEBUG
      void makeMoveHook(Move);
#endif
    public:
      /**
       * まともなdoUndo
       */
      template <Player P, class Function>
      void doUndoMoveOrPass(const HashKey& new_hash,
			    Move move, Function& f)
      {
	pushBeforeApply(move);
	UpdateWrapper<Function> wrapper(new_hash, this, f);
	current_path.pushMove(move);
	current_state.makeUnmakeMove(Player2Type<P>(), move, wrapper);
	current_path.popMove(move);
	popAfterApply();
      }
      void makeMove(Move move)	// for debug
      {
	HashKey new_hash = currentHash().newHashWithMove(move);
	pushBeforeApply(move);
	current_state.makeMove(move);
	updateRepetitionCounterAfterMove(new_hash);
      }
      
      const Move lastMove(int i=1) const { return move_history.lastMove(i); }
      const MoveStack& history() const { return move_history; }
      const RecordStack2& recordHistory() const { return record_stack; }
      const PathEncoding& path() const { return current_path; }
      const NumEffectState& state() const { return current_state; }
      const NumEffectState& rootState() const { return root_state; }
      void restoreRootState();
      const checkmate_t& checkmateSearcher() const { return *checkmate_searcher; }
      const RepetitionCounter& repetitionCounter() const { 
	return repetition_counter;
      }
      const HashKey& currentHash() const
      {
	return repetition_counter.history().top();
      }

      /**
       * 軽量化版 doUndo 千日手情報や, hash を更新しない
       */
      template <Player P, class Function>
      void doUndoMoveLight(Move move, Function& f)
      {
	current_path.pushMove(move);
	current_state.makeUnmakeMove(Player2Type<P>(), move, f);
	current_path.popMove(move);
      }

      template <Player P>
      bool isLosingState(int node_limit)
      {
	search_assert(P == path().turn());
	const bool lose = 
	  checkmate_searcher->isLosingState<P>
	  (node_limit, current_state, currentHash(), path(), lastMove());
	return lose;
      }
    public:
      template <Player P>
      static bool isWinningState(checkmate_t& search, NumEffectState& state,
				 const HashKey& key, PathEncoding path,
				 int node_limit, Move& checkmate_move, 
				 Move last_move, bool
#ifdef OSL_DFPN_SMP_SEARCH
 parallel
#endif
				 =false
	)
      {
	assert(P == path.turn());
#ifdef OSL_DFPN_SMP_SEARCH
	if (parallel)
	  return search.isWinningStateParallel<P>
	    (node_limit, state, key, path, checkmate_move, last_move);
#endif
	const bool win = search.isWinningState<P>
	  (node_limit, state, key, path, checkmate_move, last_move);
	return win;
      }
      static bool isWinningState(checkmate_t& search, NumEffectState& state,
				 const HashKey& key, PathEncoding path,
				 int node_limit, Move& checkmate_move, 
				 Move last_move, bool parallel=false)
      {
	if (state.turn() == BLACK)
	  return isWinningState<BLACK>
	    (search, state, key, path, node_limit, checkmate_move, last_move, parallel);
	else
	  return isWinningState<WHITE>
	    (search, state, key, path, node_limit, checkmate_move, last_move, parallel);
      }
      template <Player P>
      bool isWinningState(int node_limit, Move& checkmate_move, bool parallel=false)
      {
	search_assert(P == path().turn());
	return isWinningState<P>(*checkmate_searcher, current_state, currentHash(),
				 path(), node_limit, checkmate_move, lastMove(), parallel);
      }
      /** FixedDepthSearcher を呼ぶ */
      template <Player P>
      bool isWinningStateShort(int depth, Move& checkmate_move)
      {
	checkmate::FixedDepthSearcher searcher(current_state);
	const ProofDisproof pdp=searcher.hasCheckmateMove<P>(depth, checkmate_move);
	return pdp.isCheckmateSuccess();
      }
      /**
       * P の手番でPの玉に詰めろがかかっているかどうか
       */
      template <Player P>
      bool isThreatmateState(int node_limit, Move& threatmate_move, bool 
#ifdef OSL_DFPN_SMP_SEARCH
			     parallel
#endif
			     =false)
      {
	search_assert(P == path().turn());
	current_state.changeTurn();
	HashKey threatmate_hash = currentHash();
	threatmate_hash.changeTurn();
	const PathEncoding threatmate_path(current_state.turn());
	const Player Opponent = alt(P);
	bool threatmate;
#ifdef OSL_DFPN_SMP_SEARCH
	if (parallel)
	  threatmate = checkmate_searcher->template isWinningStateParallel<Opponent>
	    (node_limit, current_state, 
	     threatmate_hash, threatmate_path, threatmate_move, Move::PASS(P));
	else
#endif
	threatmate
	  = checkmate_searcher->template isWinningState<Opponent>
	  (node_limit, current_state, 
	   threatmate_hash, threatmate_path, threatmate_move, Move::PASS(P));
	current_state.changeTurn();
	return threatmate;
      }
      template <Player P>
      bool isThreatmateStateShort(int depth, Move& threatmate_move)
      {
	search_assert(P == path().turn());
	current_state.changeTurn();

	const Player Opponent = alt(P);

	checkmate::FixedDepthSearcher searcher(current_state);
	const ProofDisproof pdp
	  = searcher.hasCheckmateMove<Opponent>(depth, threatmate_move);

	current_state.changeTurn();
	return pdp.isCheckmateSuccess();
      }
      bool abort() const;
      virtual bool abort(Move) const;

      bool tryThreatmate() const 
      {
	const Move last_move = lastMove();
	if (! last_move.isNormal())
	  return false;
	const Square king = state().kingSquare(state().turn());
	if (curDepth() == 1)
	  return FirstMoveThreatmate::isMember(last_move, king);
	return Neighboring8Direct::hasEffect
	  (state(), last_move.ptypeO(), last_move.to(), king);

      }

      void makePV(Move m) 
      {
	const int depth = curDepth();
	makePV(pv[depth], m, pv[depth+1]);
      }
      void initPV() 
      {
	const int depth = curDepth();
	pv[depth].clear();
      }
      void makePV(PVVector& parent, Move m, PVVector& pv) const;
      /** turn の側が連続王手で詰ろを逃れている回数 */
      int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      countCheckAfterThreatmate(Player turn, int depth=1) const
      {
	assert(((depth % 2) && state().turn() == turn)
	       || ((depth % 2) == 0 && state().turn() != turn));
	int result = 0;
	for (int i=depth;; i+=2) {
	  if (! hasLastRecord(i) || ! lastRecord(i))
	    break;
	  if (lastRecord(i)->qrecord.threatmate.status(turn).status()
	      != ThreatmateState::CHECK_AFTER_THREATMATE)
	    break;
	  ++result;
	}
	return result;
      }
      int countCheckAfterThreatmateSacrifice(Player turn, int depth=1) const
      {
	assert(((depth % 2) && state().turn() == turn)
	       || ((depth % 2) == 0 && state().turn() != turn));
	assert(depth > 0);
	int result = 0;
	for (int i=depth;; i+=2) {
	  if (! hasLastRecord(i) || ! lastRecord(i))
	    break;
	  if (lastRecord(i)->qrecord.threatmate.status(turn).status()
	      != ThreatmateState::CHECK_AFTER_THREATMATE)
	    break;
	  if (lastMove(i-1).isCapture())
	    ++result;
	}
	return result;
      }
    };

#undef search_assert    
#undef search_assert2
#define search_assert(x) assert((x) || SearchState2Core::abort())
#define search_assert2(x, m) assert((x) || SearchState2Core::abort(m))
    /**
     * SearchFramework のうち，template parameter を含まない部分.
     */
    class SearchState2 : public SearchState2Core
    {
    public:
      /** 再探索や，指手生成でより確率の高い手があったときに無視する範囲 */
      static const int ReSearchLimitMargin = 80;
    protected:
      int root_limit;
      int cur_limit;
    public:
      SearchState2(const NumEffectState& s, checkmate_t& checker);
      virtual ~SearchState2();

      void setState(const NumEffectState& s);
      void setKillerMove(Move best_move)
      {
	if (best_move.isPass())
	  return;
	SearchState2Core::setKillerMove(best_move);
      }
      
      int curLimit() const { return cur_limit; }

      bool abort(Move) const;

    protected:
      /**
       * root で limitを閾値に探索を始めることを設定
       */
      void setRoot(int limit)
      {
	root_limit = limit;
	cur_limit = limit;
      }
      void addLimit(int limit) { cur_limit += limit; search_assert(cur_limit >= 0); }
      void subLimit(int limit) { cur_limit -= limit; search_assert(cur_limit >= 0); }

      /** 王手の捨て駒の連続を2ループまで数える 
       * @param history_max これ以上を逆上らない
       */
      int countSacrificeCheck2(int history_max) const;
      /** debug 用途 */
      void checkPointSearchAllMoves();
    };
  } // namespace search
} // namespace osl

#undef search_assert
#undef search_assert2

#endif /* OSL_SEARCHSTATE2_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
