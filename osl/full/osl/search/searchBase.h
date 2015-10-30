/* searchBase.h
 */
#ifndef OSL_SEARCHBASE_H
#define OSL_SEARCHBASE_H

#include "osl/search/searchRecorder.h"
#include "osl/search/searchTable.h"
#include "osl/search/realizationProbability.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/search/searchWindow.h"
#include "osl/search/fixedEval.h"
#include "osl/hash/hashCollision.h"
#include "osl/hashKey.h"
#include "osl/numEffectState.h"
#include <cassert>
namespace osl
{
  namespace search
  {
    class SimpleHashTable;

    /**
     * MTDF と SearchFramework に共通の
     * 小物のメソッド.
     */
    template<typename Eval, 
	     typename Table, typename Recorder, typename Probabilities>
    struct SearchBase : protected FixedEval
    {
      // types
      typedef Eval eval_t;
    
      typedef Probabilities Probabilities_t;
    protected:
      Recorder& recorder;
      Table *table;		// acquaintance
    public:
      SearchBase(Recorder& r, Table *t) 
	: recorder(r), table(t)
      {
	assert(table);
	assert(winThreshold(BLACK) > eval_t::infty());
      }
      virtual ~SearchBase() {}
      virtual bool abort(Move) const { return false; }

      /** 
       * テーブルの指手の正しさを確かめる
       *
       * ついでに実現確率のチェックもする
       * HashKey のconflict があるとtable から牽いた move が不正であることがある
       * conflict で turn の違いがあると isAlmostValidMove ではチェックできない
       * ことに注意
       */
      bool validTableMove(const NumEffectState& state,
			  const MoveLogProb& move, int limit) const
      {
	if ((limit < move.logProb()) || (! move.validMove()))
	  return false;
	if (! move.isNormal())
	  return false;
	const bool valid
	  = ((move.player() == state.turn())
	     && state.isAlmostValidMove<false>(move.move()));
	if (! valid)
	{
	  // 本来ここに来てはいけない
	  recorder.recordInvalidMoveInTable(state, move, limit);
	  throw hash::HashCollision();
	}
	return valid;
      }
    private:
      void recordCheckmateResult(Player P, SimpleHashRecord *record,
				 int val, Move move) const
      {
	assert(isWinValue(P,val) || isWinValue(alt(P),val));
	const int limit = SearchTable::CheckmateSpecialDepth;
	const MoveLogProb best_move(move, 101);
	recorder.tableStoreLowerBound(P, best_move, val, limit);
	recorder.tableStoreUpperBound(P, best_move, val, limit);
	record->setAbsoluteValue(best_move.move(), val,
				 SearchTable::CheckmateSpecialDepth);
      }
    public:
      /** 詰将棋の見つけた勝 */
      void recordWinByCheckmate(Player P, SimpleHashRecord *record,
				Move check_move) const
      {
	assert(record);
	recordCheckmateResult(P, record, winByCheckmate(P), check_move);
      }
      /** 詰将棋の見つけた敗 */
      void recordLoseByCheckmate(Player P, SimpleHashRecord *record) const
      {
	assert(record);
	recordCheckmateResult(P, record, winByCheckmate(alt(P)), Move::INVALID());
      }

      using FixedEval::isWinValue;
      using FixedEval::winByCheckmate;
      using FixedEval::winByFoul;
      using FixedEval::winByLoop;
      using FixedEval::minusInfty;
      using FixedEval::drawValue;
    
    };
  } // namespace search
} // namespace osl


#endif /* OSL_SEARCHBASE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
