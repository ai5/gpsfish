/* quiescenceSearch2.h
 */
#ifndef _QUIESCENCESEARCH2_H
#define _QUIESCENCESEARCH2_H

#include "osl/search/fixedEval.h"
#include "osl/search/quiescenceRecord.h"
#include "osl/search/searchState2.h"
#include "osl/eval/pieceEval.h"
#include "osl/numEffectState.h"
#include "osl/pathEncoding.h"
namespace osl
{
  namespace container
  {    
    class MoveVector;
  }
  namespace hash
  {
    class HashKey;
  }
  namespace search
  {
    class SimpleHashTable;

    /**
     * 取り合い探索 (静止探索).
     *
     * - 試すのは取る手と成る手のみ
     * - KFEND流 http://www31.ocn.ne.jp/~kfend/inside_kfend/quiescence.html
     * - 高速化のため末端の取り返しは，一直線に読む
     * - 合駒が無駄かどうかを考慮する
     * - 「逃れることができない脅威」はまだ実装していない
     * - 末端の成る手は，飛車と角だけで，取り返されない場合だけ
     *
     * TODO:
     * - 将来は，逃げる手，有効王手も候補?
     *
     */
    template <class EvalT>
    class QuiescenceSearch2 : protected FixedEval, private QSearchTraits
    {
      typedef FixedEval base_t;
      SearchState2Core& state;
      SimpleHashTable& table;
      int root_depth;
      int max_depth;
      /**
       * 探索ノード数
       */
      int node_count;
      /** rootからの深さ */
      int depthFromRoot() const 
      {
	return state.path().getDepth() - root_depth;
      }
      /** 残り深さ */
      int depth() const 
      {
	return max_depth - depthFromRoot();
      }
    public:
      typedef EvalT eval_t;
      typedef NumEffectState effect_state_t;
      using base_t::isWinValue;    
      QuiescenceSearch2(SearchState2Core& s, SimpleHashTable& t)
	: state(s), table(t), root_depth(s.path().getDepth()),
	  max_depth(QSearchTraits::MaxDepth), node_count(0)
      {
      }
      template <Player P>
      int search(eval_t& ev, Move last_move,
		 int depth=QSearchTraits::MaxDepth)
      {
	assert(last_move.player() == alt(P));
	assert(state.state().turn() == P);
	max_depth = depth;
	return searchInternal<P>(base_t::winThreshold(alt(P)), 
				 base_t::winThreshold(P), 
				 ev, last_move);
      }
      int search(Player P, eval_t& ev, Move last_move,
		 int depth=QSearchTraits::MaxDepth)
      {
	if (P == BLACK)
	  return search<BLACK>(ev, last_move, depth);
	else
	  return search<WHITE>(ev, last_move, depth);
      }
      template <Player P>
      int searchIteratively(eval_t& ev, Move last_move,
			    int depth=QSearchTraits::MaxDepth)
      {
	assert(last_move.player() == alt(P));
	assert(state.state().turn() == P);
	return searchIteratively<P>(base_t::winThreshold(alt(P)), 
				    base_t::winThreshold(P), 
				    ev, last_move, depth);
      }
      int searchIteratively(Player P, eval_t& ev, Move last_move,
			    int depth=QSearchTraits::MaxDepth)
      {
	if (P == BLACK)
	  return searchIteratively<BLACK>(ev, last_move, depth);
	else
	  return searchIteratively<WHITE>(ev, last_move, depth);
      }

      template <Player P>
      int searchIteratively(int alpha, int beta, eval_t& ev, Move last_move,
			    int depth)
      {
	assert(depth >= 2);
	int result=0;
	for (int i=2; i<=depth; i+=2)
	{
	  max_depth = i;
	  result=searchInternal<P>(alpha, beta, ev, last_move);
	}
	return result;
      }
      template <Player P>
      int search(int alpha, int beta, eval_t& ev, Move last_move,
		 int depth=QSearchTraits::MaxDepth)
      {
	max_depth = depth;
	return searchInternal<P>(alpha, beta, ev, last_move);
      }
      int search(Player P, int alpha, int beta, eval_t& ev, Move last_move, int depth){
	if (P == BLACK)
	  return search<BLACK>(alpha, beta, ev, last_move, depth);
	else
	  return search<WHITE>(alpha, beta, ev, last_move, depth);
      }
      template <Player P>
      int searchProbCut(int alpha, int beta, eval_t& ev, Move last_move);
      int searchProbCut(Player P, int alpha, int beta, eval_t& ev, Move last_move)
      {
	if (P == BLACK)
	  return searchProbCut<BLACK>(alpha, beta, ev, last_move);
	else
	  return searchProbCut<WHITE>(alpha, beta, ev, last_move);
      }

      enum EvalUpdateState { AfterUpdate, BeforeUpdate };
      template <Player P>
      int searchInternal(int alpha, int beta, eval_t& ev, Move last_move,
			 int additional_depth=0, EvalUpdateState need_eval_update=AfterUpdate);
    private:
      template <Player P, bool has_record>
      int searchMain(QuiescenceRecord *record, 
		     int alpha, int beta, eval_t& ev, Move last_move,
		     int additional_depth, EvalUpdateState& need_eval_update);
      /**
       * PTYPE を取る手を生成して examineMoves を呼ぶ
       * @return cut できるか
       */
      template <Player P, Ptype PTYPE, bool has_record>
      bool examineCapture(QuiescenceRecord *record,
			  int& curVal, MoveVector& working, int& alpha, 
			  int beta, eval_t const& ev, Piece last_piece,
			  int additional_depth);
    public:
      template <Player P, bool has_record>
      int staticValue(eval_t const& ev, int alpha, int beta, QuiescenceRecord *record);
    private:
      template <Player P>
      int passValue(int alpha, int beta, eval_t const& ev);
      int currentValueWithLastThreat(eval_t const& ev, Piece last_move_piece);
    public:
      /**
       * @return cut できるか
       * @param dont_capture TakeBack で試した手を後から重複して試さないために使う.
       *   template parameter の has_dont_capture がtrue の時だけ見る
       */
      template <Player P, bool has_record, bool has_dont_capture, MoveType move_type>
      bool examineMoves(QuiescenceRecord *record, int& curVal, 
			const Move *first, const Move *last, 
			int& alpha, int beta, eval_t const& ev,
			int additional_depth,
			Square dont_capture=Square::STAND());
      /**
       * 単純な取り返しの探索.
       * PieceEval::computeDiffAfterMoveForRP と異なり，盤面を動かす．
       * 王手を正確に判定し，利きも伸びる
       * @param attack_piece last_move が王手回避の時に王手をかけていた駒
       */
      template <Player P>
      int takeBackValue(int alpha, int beta, eval_t const& ev, Move last_move);
      template <Player P>
      bool examineTakeBack(const MoveVector& moves,
			   int& cur_val, int& alpha, int beta, eval_t const& ev);

      /**
       * 末端の取り返し用. 二つthreatを求める
       * @param threat1 最大の脅威
       * @param threat2 2番目の脅威
       * @param calm_move_only 相手の利きがあるマスにはいかない
       * @param king_attack_piece P に王手をかけている駒(のうちの一つ)
       */
      template <Player P, bool calm_move_only, bool first_nolmal_move_only>
      bool examineTakeBack2(const MoveVector& moves,
			    QuiescenceThreat& threat2, 
			    QuiescenceThreat& threat1,
			    int beta, int beta2, eval_t const& ev);
      /** 末端の取り返し用.
       * 各取れる駒毎に指手を生成して examineTakeBack2 を呼ぶ
       */
      template <Player P, Ptype PTYPE>
      bool generateAndExamineTakeBack2(MoveVector& moves, 
				       QuiescenceThreat& threat2, 
				       QuiescenceThreat& threat1, 
				       int beta1, int beta2, eval_t const& ev);

      /**
       * last_move が逃げる手で、逃げた先の取り返し、または追撃の価値を判定
       */
      template <Player P>
      int takeBackOrChase(int alpha, int beta, eval_t const& ev, Move last_move);

      template <Player P>
      int staticValueWithThreat(eval_t const& ev, int alpha, 
				QuiescenceThreat& threat1, 
				QuiescenceThreat& threat2);
      template <Player P>
      int staticValueWithThreat(eval_t const& ev)
      {
	QuiescenceThreat t1, t2;
	return staticValueWithThreat<P>(ev, base_t::winThreshold(alt(P)), 
					t1, t2);
      }
      int staticValueWithThreat(eval_t const& ev)
      {
	if (state.state().turn() == BLACK)
	  return staticValueWithThreat<BLACK>(ev);
	else
	  return staticValueWithThreat<WHITE>(ev);
      }
      int nodeCount() const { return node_count; }
      const NumEffectState& currentState() const { return state.state(); }
    };

  } // namespace search
  using search::QuiescenceSearch2;
} // namespace osl


#endif /* _QUIESCENCESEARCH2_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
