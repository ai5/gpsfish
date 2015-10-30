/* searchRecorder.h
 */
#ifndef _MTDF_RECORDER_H
#define _MTDF_RECORDER_H

#include "osl/numEffectState.h"
#include "osl/misc/lightMutex.h"
#include <iosfwd>
namespace osl
{
  class MoveLogProb;
  namespace search
  {
    /**
     * recorder of MTDF/AlphaBeta
     *
     * destructor はvirtualにすることで管理を容易にする．
     * method はvirtualに*しない*ことで，キャストしたら上位クラスとして
     * 振る舞うようにする．
     */
    class CountRecorder
    {
      size_t node_count;
      size_t quiescence_count;
      size_t checkmate_count;
#ifdef OSL_SMP
      mutable LightMutex mutex;
#endif
    public:
      CountRecorder();
      virtual ~CountRecorder();

      /** 探索の途中終了で使えるように必ず数える */
      void addNodeCount(int count=1) { 
#if (defined OSL_SMP) && (defined OSL_USE_RACE_DETECTOR)
	SCOPED_LOCK(lk,mutex);
#endif
	node_count += count; 
      }
      void addQuiescenceCount(int count=1) { 
#if (defined OSL_SMP) && (defined OSL_USE_RACE_DETECTOR)
	SCOPED_LOCK(lk,mutex);
#endif
	quiescence_count += count; 
      }
      void addCheckmateCount(int count=1) { 
#ifdef OSL_SMP
	SCOPED_LOCK(lk,mutex);
#endif
	checkmate_count += count; 
      }
      void setCheckmateCount(int count) { 
#ifdef OSL_SMP
	SCOPED_LOCK(lk,mutex);
#endif
	checkmate_count = count; 
      }

      void resetNodeCount();
      size_t nodeCount() const { 
#if (defined OSL_SMP) && (defined OSL_USE_RACE_DETECTOR)
	SCOPED_LOCK(lk,mutex);
#endif
	return node_count; 
      }
      size_t quiescenceCount() const { 
#if (defined OSL_SMP) && (defined OSL_USE_RACE_DETECTOR)
	SCOPED_LOCK(lk,mutex);
#endif
	return quiescence_count; 
      }
      size_t checkmateCount() const { 
#if (defined OSL_SMP) && (defined OSL_USE_RACE_DETECTOR)
	SCOPED_LOCK(lk,mutex);
#endif
	return checkmate_count; 
      }
      size_t searchNodeCount() const 
      {
#if (defined OSL_SMP) && (defined OSL_USE_RACE_DETECTOR)
	SCOPED_LOCK(lk,mutex);
#endif
	return node_count + quiescence_count; 
      }
      size_t allNodeCount() const { 
#if (defined OSL_SMP) && (defined OSL_USE_RACE_DETECTOR)
	SCOPED_LOCK(lk,mutex);
#endif
	return node_count + quiescence_count + checkmate_count; 
      }
      double checkmateRatio() const 
      {
	const double checkmate = checkmateCount();
	const double search = searchNodeCount();
	return checkmate / (checkmate + search);
      }
      /** recordValue とセットで呼ぶ */
      void tryMove(const MoveLogProb& /*m*/, int /*last_f*/, int /*limit*/) const {}
      /** recordValue とセットで呼ぶ */
      void retryMove(const MoveLogProb& /*m*/, int /*last_f*/, int /*limit*/,
		     int /*retryCount*/) const {}
      /** tryMove とセットで呼ぶ */
      void recordValue(const MoveLogProb&, int /*val*/, bool /*better_move*/,
		       int /*limit*/) const {}

      /** 主に数の記録用 */
      void recordTopLevelLowFail(const MoveLogProb& /* best */, int /*last_f*/) const {}
      void recordTopLevelHighFail(const MoveLogProb& /*best */, int /*last_f*/) const {}

      void tableHitLowerBound(Player, int, int /*last_f*/, int /*limit*/) const {}
      void tableHitUpperBound(Player, int, int /*last_f*/, int /*limit*/) const {}

      void tableStoreLowerBound(Player, const MoveLogProb&, int, int) const {}
      void tableStoreUpperBound(Player, const MoveLogProb&, int, int) const {}
    

      void startSearch(int /*limit*/) const {}
      /** これは遅くても気にしない */
      virtual void finishSearch(Move best, double seconds_consumed,
				bool verbose) const;

      void recordInvalidMoveInTable(const SimpleState&, 
				    const MoveLogProb&, int limit) const;
      void newCategory(const char * /*name*/, int /*limit*/) const {}

      /** 詰将棋無限ループ発見用 */
      void gotoCheckmateSearch(const SimpleState&, int) const {}
      void backFromCheckmateSearch() const {}

      void reportCount(std::ostream&, double seconds) const;
      void reportCount(std::ostream&) const;
    };

    class SearchRecorder : public CountRecorder
    {
      struct Recorder;
      /** hide implementation */
      std::unique_ptr<Recorder> recorder;
    public:
      explicit SearchRecorder(const char *filename="mtdf.log");
      ~SearchRecorder();

      /** どの程度深く記録を取るか指示 */
      void setLogMargin(int margin=500);

      void tryMove(const MoveLogProb& m, int last_f, int limit) const;
      void retryMove(const MoveLogProb& m, int last_f, int limit,
		     int retryCount) const;

      void recordValue(const MoveLogProb& m, int val, bool betterMove, int limit) const;

      void tableHitLowerBound(Player p, int val, int last_f, int limit) const;
      void tableHitUpperBound(Player p, int val, int last_f, int limit) const;

      void tableStoreLowerBound(Player p, const MoveLogProb& best_move, int val, int limit) const;
      void tableStoreUpperBound(Player p, const MoveLogProb& best_move, int val, int limit) const;

      void recordTopLevelLowFail(const MoveLogProb& /* best */, int last_f) const;
      void recordTopLevelHighFail(const MoveLogProb& best_move, int last_f) const;

      void startSearch(int limit) const;
      void finishSearch(Move best_move, double seconds_consumed, bool verbose) const;

      void newCategory(const char *name, int limit) const;

      void gotoCheckmateSearch(const SimpleState&, int nodeLimit) const;
      void backFromCheckmateSearch() const;

      /** ログにメッセージを混ぜたいときに使う */
      std::ostream& stream() const;
    };
  } // namespace search

  using search::CountRecorder;
  using search::SearchRecorder;
} // namespace osl


#endif /* _MTDF_RECORDER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
