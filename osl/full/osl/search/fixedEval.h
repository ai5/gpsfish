/* fixedEval.h
 */
#ifndef SEARCH_FIXEDEVAL_H
#define SEARCH_FIXEDEVAL_H

#include "osl/eval/evalTraits.h"

namespace osl
{
  namespace search
  {
    class FixedEval
    {
      int draw_value;
    protected:
      ~FixedEval() {}
    public:
      FixedEval() : draw_value(0)
      {
      }
      void setDrawValue(int value) 
      {
	draw_value = value;
      }
      int drawValue() const { return draw_value; }

      /**
       * 相手の王手千日手，打歩詰.
       * 考慮対象外の手は詰より評価を下げる．
       */
      static int winByFoul(Player P) 
      {
	return eval::convert(P, EvalTraits<BLACK>::MAX_VALUE);
      }
      /**
       * 駒得するループ.
       * 考慮対象外の手は詰より評価を下げる．
       */
      static int winByLoop(Player P) 
      {
	return winByFoul(P);
      }
      /**
       * 詰による勝
       */
      static int winByCheckmate(Player P)
      {
	return eval::convert(P, EvalTraits<BLACK>::MAX_VALUE-2);
      }
      /**
       * 探索windowの下限 (負けでも更新される値)
       */
      static int minusInfty(Player P)
      {
	return winByCheckmate(alt(P));
      }
      /**
       * この値を越えれば勝. (奇数). loop勝も含める
       */
      static int winThreshold(Player P) 
      {
	return eval::convert(P, EvalTraits<BLACK>::MAX_VALUE-3);
      }
      /**
       * 探索して意味がある範囲 (偶数)
       */
      static int windowMax(Player P) 
      {
	return winByCheckmate(P);
      }
      /**
       * 必死(に見える)局面の評価値
       * Pが負けそう
       * @param limit 受を生成した閾値 大きい方が信頼できる必死
       */
      static int brinkmatePenalty(Player P, int limit)
      {
	return (winByFoul(alt(P))*3/4+eval::convert(alt(P), limit*16)) & (~1);
      }
      /**
       * 末端で詰めろがかかっている場合のペナルティ.
       * P に詰めろがかかっている場合に threatmatePenalty(P)を足す
       */
      static int threatmatePenalty(Player P)
      {
	return winByFoul(alt(P))/2;
      }
      /**
       * 勝かどうか. loop勝も含める
       */
      static int isWinValue(Player P, int val) 
      {
	return eval::notLessThan(P, val, winByCheckmate(P));
      }
    };
  } // namespace search
} // namespace osl

#endif /* SEARCH_FIXEDEVAL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
