/**
 * defenseKing.cc
 */
#include "osl/eval/endgame/defenseKing.h"
#include "osl/eval/pieceEval.h"
#include "osl/oslConfig.h"

osl::eval::endgame::DefenseKing::Table osl::eval::endgame::DefenseKing::table;
static osl::SetUpRegister _initializer([](){ 
    osl::eval::endgame::DefenseKing::init(); 
});

// YSSの評価値を真似
// http://www32.ocn.ne.jp/~yss/book.html#SEC3
//
//         0   1   2   3   4   5   6   7   8 (Ｘ距離)
//   +8   50  50  50  50  50  50  50  50  50
//   +7   56  53  50  50  50  50  50  50  50
//   +6   64  61  55  50  50  50  50  50  50
//   +5   79  77  70  65  54  51  50  50  50
//   +4  100  99  95  87  74  58  50  50  50
//   +3  116 117 101  95  88  67  54  50  50
//   +2  131 129 124 114  90  71  59  51  50
//   +1  137 138 132 116  96  76  61  53  50
//    0  142 142 136 118  98  79  64  52  50  <--- 中心
//   -1  132 132 129 109  95  75  60  51  50
//   -2  121 120 105  97  84  66  54  50  50
//   -3   95  93  89  75  68  58  51  50  50
//   -4   79  76  69  60  53  50  50  50  50
//   -5   64  61  55  51  50  50  50  50  50
//   -6   56  52  50  50  50  50  50  50  50
//   -7   50  50  50  50  50  50  50  50  50
//   -8   50  50  50  50  50  50  50  50  50

namespace osl
{
  namespace
  {
    const CArray<int,19*9> yss_bonus = {{
      // 桂，香は一段下げるため，+9 から -9 まで取る
      50,  50,  50,  50,  50,  50,  0,  0,  0,
      50,  50,  50,  50,  50,  50,  0,  0,  0,
      56,  53,  50,  50,  50,  50,  0,  0,  0,
      64,  61,  55,  50,  50,  50,  0,  0,  0,
      79,  77,  70,  65,  54,  51,  0,  0,  0,
     100,  99,  95,  87,  74,  58,  0,  0,  0,
     116, 117, 101,  95,  88,  67,  0,  0,  0,
     131, 129, 124, 114,  90,  71,  0,  0,  0,
     137, 138, 132, 116,  96,  76,  0,  0,  0,
     142, 142, 136, 118,  98,  79,  0,  0,  0,
     132, 132, 129, 109,  95,  75,  0,  0,  0,
     121, 120, 105,  97,  84,  66,  0,  0,  0,
      95,  93,  89,  75,  68,  58,  0,  0,  0,
      79,  76,  69,  60,  53,  50,  0,  0,  0,
      64,  61,  55,  51,  50,  50,  0,  0,  0,
      56,  52,  50,  50,  50,  50,  0,  0,  0,
      50,  50,  50,  50,  50,  50,  0,  0,  0,
      50,  50,  50,  50,  50,  50,  0,  0,  0,
      50,  50,  50,  50,  50,  50,  0,  0,  0,
    }};
    /** 入玉用 */
    const CArray<int,9> yss_king_bonus = {{
      0, 0, 0, 150, 450, 900,1300,1550,1600
    }};
    inline int toEven(int value) 
    {
      if (value % 2 == 0)
	return value;
      if (value > 0)
	return value - 1;
      return value + 1;
    }
    inline int multiply(int value, int bonus)
    {
      // http://www32.ocn.ne.jp/~yss/book.html#SEC3
      // > 追記：2005年01月22日
      // > 自玉の近くの点数は高すぎるので、現在は＋を3分の1の値まで下げています
      if (bonus > 100)
	bonus = (bonus - 100)/3 + 100;
      // yss では持駒の付加価値が2割程度あるため1.2で割る
      return toEven(value * bonus / 120);
    }
    inline void adhoc_adjust(int& value, double scale)
    {
      value = toEven(static_cast<int>(value * scale));
    }
  } // anonymous namespace
} // namespace osl



void osl::eval::endgame::DefenseKing::
Table::init()
{
  // BLACKのKINGに対するBLACKの価値を書き，WHITEの駒の価値は最後に反転させる．
  data.fill(0);
  for (int king_x=1; king_x<=9; ++king_x)
  {
    for (int king_y=1; king_y<=9; ++king_y)
    {
      const Square king(king_x, king_y);
      for (int p=PTYPE_PIECE_MIN; p<=PTYPE_MAX; ++p)
      {
	const Ptype ptype = static_cast<Ptype>(p);
	assert(isPiece(ptype));
	const Ptype basic_type = unpromote(ptype);

	const int ptype_value
	  = Ptype_Eval_Table.value(newPtypeO(BLACK,ptype));
	// 持駒
	const int stand_bonus
	  = (isMajorBasic(basic_type) 
	     ? Ptype_Eval_Table.value(newPtypeO(BLACK,PAWN)) 
	     : 0);
	valueOf(king, BLACK, Square::STAND(), ptype) = ptype_value
	  + stand_bonus;
	if (ptype == KNIGHT || ptype == LANCE)
	{
	  adhoc_adjust(valueOf(king, BLACK, Square::STAND(), ptype), 0.85);
	}
	// 盤上
	if (basic_type == KING)
	{
	  // 玉は入玉を考慮
	  valueOf(king, BLACK, king, ptype) 
	    = ptype_value + yss_king_bonus[9-king_y];
	  continue;
	}
	      
	for (int defense_x=1; defense_x<=9; ++defense_x)
	{
	  const bool near_edge 
	    = (((defense_x < 3) && (defense_x < king_x))
	       || ((defense_x > 7) && (defense_x > king_x)));
	  const int relative_x = abs(king_x - defense_x);
	  for (int defense_y=1; defense_y<=9; ++defense_y)
	  {
	    const Square defender(defense_x, defense_y);
	    // 馬以外の大駒は0.9
	    if (isMajorBasic(basic_type) && ptype != PBISHOP)
	    {
	      int black_bonus;
	      // 入玉後の大駒は大事
	      if (king_y < 4)
		black_bonus = 105;
	      else if (king_y == 4)
		black_bonus = 100;
	      // 1段目と2段目の玉に対する2段目の飛車は 0.95 あげる
	      else if (basic_type == ROOK && defense_y == 8 && king_y >= 8)
		black_bonus = 95;
	      else
		black_bonus = 90;

	      valueOf(king, BLACK, defender, ptype) =
		multiply(ptype_value, black_bonus);
	      continue;
	    }
	    int relative_y_black_king = king_y - defense_y;
	    if (ptype == KNIGHT || ptype == LANCE)
	    {
	      // 桂香は一段遠くから評価
	      relative_y_black_king++;
	    }
	    else if (ptype == PAWN)
	    {
	      // 歩は玉の真上の段とその上の段のボーナスを交換する
	      if (relative_y_black_king == 1)
	      {
		relative_y_black_king = 2;
	      }
	      else if (relative_y_black_king == 2)
	      {
		relative_y_black_king = 1;
	      }
	    }

	    int bonus_black_king
	      = yss_bonus[relative_x + (-relative_y_black_king+9)*9];
	    // 端の方にはボーナスをあげない (自玉の近くの端歩以外
	    if (near_edge || ptype == LANCE)
	    {
	      if (!(ptype == PAWN &&
		    (defense_x == 1 || defense_x == 9) &&
		    defense_y < king_y))
		bonus_black_king = std::min(100, bonus_black_king);
	    }
	    if (ptype == KNIGHT)
	    {
	      if ((defense_x == 2 || defense_x == 8) &&
		  (defense_y == 1 || defense_y == 9))
	      {
	      }
	      else
	      {
		bonus_black_king = std::min(100, bonus_black_king);
	      }
	    }

	    int black_defense =  multiply(ptype_value,bonus_black_king);
	    const bool near_center 
	      = abs(defense_x-king_x)==1 &&
	      (((king_x < 3) && (defense_x > king_x))
	       || ((king_x > 7) && (defense_x < king_x)));
	    if(defense_x==king_x || near_center){
	      if(defense_y==king_y-1){
		if(king_y==9 && (king_x==1 || king_x==9))
		  black_defense+=600;
		else if(defense_x!=king_x && king_y==8 && (king_x==2 || king_x==8))
		  black_defense+=200;
		else
		  black_defense+=400;
	      }
	    }
	    valueOf(king, BLACK, defender, ptype) = black_defense;
	  }
	}
      }
    }
  }
  // 1つしか動けない香は歩と同じ点数に
  for (int king_x=1; king_x<=9; ++king_x)
  {
    for (int king_y=1; king_y<=9; ++king_y)
    {
      const Square king(king_x, king_y);
      for (int x=1; x<=9; ++x)
      {
	const Square b(x,2);
	valueOf(king, BLACK, b, LANCE) = valueOf(king, BLACK, b, PAWN);
	adhoc_adjust(valueOf(king, BLACK, Square(x, 1), GOLD), 0.5);
	adhoc_adjust(valueOf(king, BLACK, Square(x, 1), SILVER), 0.5);
	adhoc_adjust(valueOf(king, BLACK, Square(x, 1), PSILVER), 0.5);
      }

      if (king_x < 6)
      {
	valueOf(king, BLACK, Square(9, 9), LANCE) = 0;
	valueOf(king, BLACK, Square(9, 8), LANCE) = 0;
	valueOf(king, BLACK, Square(9, 7), LANCE) = 0;
	valueOf(king, BLACK, Square(8, 9), KNIGHT) = 0;

	valueOf(king, BLACK, Square(9, 1), GOLD) = 0;
	valueOf(king, BLACK, Square(9, 1), SILVER) = 0;
	valueOf(king, BLACK, Square(9, 1), PSILVER) = 0;
	valueOf(king, BLACK, Square(8, 1), GOLD) = 0;
	valueOf(king, BLACK, Square(8, 1), SILVER) = 0;
	valueOf(king, BLACK, Square(8, 1), PSILVER) = 0;
      }

      if (king_x > 4)
      {
	valueOf(king, BLACK, Square(1, 9), LANCE) = 0;
	valueOf(king, BLACK, Square(1, 8), LANCE) = 0;
	valueOf(king, BLACK, Square(1, 7), LANCE) = 0;
	valueOf(king, BLACK, Square(2, 9), KNIGHT) = 0;

	valueOf(king, BLACK, Square(1, 1), GOLD) = 0;
	valueOf(king, BLACK, Square(1, 1), PSILVER) = 0;
	valueOf(king, BLACK, Square(1, 1), SILVER) = 0;
	valueOf(king, BLACK, Square(2, 1), GOLD) = 0;
	valueOf(king, BLACK, Square(2, 1), SILVER) = 0;
	valueOf(king, BLACK, Square(2, 1), PSILVER) = 0;
      }
    }
  }

  adhoc_adjust(valueOf(Square(1,1), BLACK, Square::STAND(), BISHOP), 0.95);
  adhoc_adjust(valueOf(Square(9,1), BLACK, Square::STAND(), BISHOP), 0.95);

  // BLACK/WHITE 反転
  for (int king_x=1; king_x<=9; ++king_x) {
    for (int king_y=1; king_y<=9; ++king_y) {
      const Square king_b(king_x, king_y);
      const Square king_w = king_b.rotate180();
      
      for (int p=PTYPE_PIECE_MIN; p<=PTYPE_MAX; ++p) {
	const Ptype ptype = static_cast<Ptype>(p);
	assert(isPiece(ptype));

	// 持駒
	valueOf(king_w, WHITE, Square::STAND(), ptype)
	  = - valueOf(king_b, BLACK, Square::STAND(), ptype);

	// 盤上
	for (int defense_x=1; defense_x<=9; ++defense_x) {
	  for (int defense_y=1; defense_y<=9; ++defense_y) {
	    const Square defense_b(defense_x, defense_y); // blackへのblackの防御
	    const Square defense_w = defense_b.rotate180();

	    valueOf(king_w, WHITE, defense_w, ptype)
	      = - valueOf(king_b, BLACK, defense_b, ptype);
	  }
	}
      }
    }
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
