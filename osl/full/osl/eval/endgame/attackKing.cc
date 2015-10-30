/**
 * attackKing.cc
 */
#include "osl/eval/endgame/attackKing.h"
#include "osl/eval/pieceEval.h"
#include "osl/oslConfig.h"

osl::eval::endgame::AttackKing::Table osl::eval::endgame::AttackKing::table;
static osl::SetUpRegister _initializer([](){ 
    osl::eval::endgame::AttackKing::init(); 
});

// YSSの評価値を真似
// http://www32.ocn.ne.jp/~yss/book.html#SEC3
//
// 先手の玉に対する後手の駒の相対位置による増減割合。Ｘ軸については対称である。
//        0   1   2   3   4   5   6   7   8 (Ｘ距離)
// 
//   +8   50  50  50  50  50  50  50  50  50
//   +7   50  50  50  50  50  50  50  50  50
//   +6   62  60  58  52  50  50  50  50  50
//   +5   80  78  72  67  55  51  50  50  50
//   +4  100  99  95  87  78  69  50  50  50
//   +3  140 130 110 100  95  75  54  50  50
//   +2  170 160 142 114  98  80  62  55  50  <--- ２段真上を最大とする。
//   +1  170 165 150 121  94  78  58  52  50
//    0  170 145 137 115  91  75  57  50  50  <--- 中心。左隅(0 0)に王がいるとする
//   -1  132 132 129 102  84  71  51  50  50
//   -2  100  97  95  85  70  62  50  50  50
//   -3   90  85  80  68  60  53  50  50  50
//   -4   70  66  62  55  52  50  50  50  50
//   -5   54  53  51  50  50  50  50  50  50
//   -6   50  50  50  50  50  50  50  50  50
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
         50,  50,  50,  50,  50,  50,  0,  0,  0, 
         62,  60,  58,  52,  50,  50,  0,  0,  0, 
         80,  78,  72,  67,  55,  51,  0,  0,  0, 
        100,  99,  95,  87,  78,  69,  0,  0,  0, 
        140, 130, 110, 100,  95,  75,  0,  0,  0, 
        170, 160, 142, 114,  98,  80,  0,  0,  0,
        170, 165, 150, 121,  94,  78,  0,  0,  0, 
        170, 145, 137, 115,  91,  75,  0,  0,  0,
        132, 132, 129, 102,  84,  71,  0,  0,  0, 
        100,  97,  95,  85,  70,  62,  0,  0,  0, 
         90,  85,  80,  68,  60,  53,  0,  0,  0, 
         70,  66,  62,  55,  52,  50,  0,  0,  0, 
         54,  53,  51,  50,  50,  50,  0,  0,  0, 
         50,  50,  50,  50,  50,  50,  0,  0,  0, 
         50,  50,  50,  50,  50,  50,  0,  0,  0, 
         50,  50,  50,  50,  50,  50,  0,  0,  0,
         50,  50,  50,  50,  50,  50,  0,  0,  0, 
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
      // yss では持駒の付加価値が2割程度あるため1.2で割る
      return toEven(value * bonus / 120); 
    }
    inline void adhoc_adjust(int& value, double scale)
    {
      value = toEven(static_cast<int>(value * scale));
    }
  } // anonymous namespace
} // namespace osl

void osl::eval::endgame::AttackKing::
Table::adhoc_edge_king_1(const Player player,
			 const Square king,
			 const Square attack)
{
  for (int ptype = PPAWN; ptype <= ROOK; ptype++)
  {
    if (ptype != KING)
    {
      adhoc_adjust(valueOf(king, player, attack,
			   static_cast<osl::Ptype>(ptype)), 1.25);
    }
  }
}

void osl::eval::endgame::AttackKing::
Table::adhoc_edge_king_2(const Player player,
			 const Square king,
			 const Square attack)
{
  adhoc_adjust(valueOf(king, player, attack,
		       GOLD), 1.25);
  adhoc_adjust(valueOf(king, player, attack,
		       SILVER), 1.25);
  for (int ptype = PPAWN; ptype <= PSILVER; ptype++)
  {
    adhoc_adjust(valueOf(king, player, attack,
			 static_cast<osl::Ptype>(ptype)), 1.25);
  }
}

void osl::eval::endgame::AttackKing::
Table::init()
{
  // WHITEのKINGに対するBLACKの価値を書き，最後に反転させる．
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
	valueOf(king, WHITE, Square::STAND(), ptype) = +ptype_value
	  + stand_bonus;
	if (ptype == KNIGHT || ptype == LANCE)
	  adhoc_adjust(valueOf(king, WHITE, Square::STAND(), ptype), 0.85);
	if (ptype == GOLD || ptype == SILVER)
	  adhoc_adjust(valueOf(king, WHITE, Square::STAND(), ptype), 1.1);

	// 盤上
	for (int attack_x=1; attack_x<=9; ++attack_x)
	{
	  const int relative_x = abs(king_x - attack_x);
	  for (int attack_y=1; attack_y<=9; ++attack_y)
	  {
	    const Square attacker(attack_x, attack_y);
	    if (basic_type == KING)
	    {
	      // 玉はそのまま
	      valueOf(king, WHITE, attacker, ptype) = ptype_value;
	      continue;
	    }

	    const int relative_y_white_king 
	      = (((ptype == KNIGHT)||(ptype == LANCE))
		 ? attack_y - king_y - 1 // 桂香は一段遠くから評価
		 : attack_y - king_y);

	    const int bonus_white_king
	      = yss_bonus[relative_x + (-relative_y_white_king+9)*9];

	    const int black_attack =  multiply(ptype_value,bonus_white_king);

	    if (isMajorBasic(basic_type))
	    {
	      // 大駒は攻撃に利いていれば max を取る
	      const int relative_y = abs(king_y - attack_y);
	      const int diff1 = king_x + king_y - (attack_x + attack_y);
	      const int diff2 = king_x - king_y - (attack_x - attack_y);

	      if ((basic_type == ROOK &&
		   relative_y <= 1) ||
		  (basic_type == BISHOP &&
		   ((-1 <= diff1 && diff1 <= 1) ||
		    (-1 <= diff2 && diff2 <= 1))))
	      {
		valueOf(king, WHITE, attacker, ptype) 
		  = EvalTraits<BLACK>::max(ptype_value, black_attack);
		// 玉と同じ筋の方のボーナスを高くする
		if (relative_y == 0)
		{
		  adhoc_adjust(valueOf(king, WHITE, attacker, ptype),
			       0.98);
		}
	      }
	      else
	      {
		// その他は 90 に
		valueOf(king, WHITE, attacker, ptype) 
		  = multiply(ptype_value, 90);
	      }
	      continue;
	    }
	    //成れない歩と桂は減点
	    if ((ptype == PAWN && attack_y >= 5) ||
		(ptype == KNIGHT && attack_y >= 6))
	    {
	      int new_black_attack = black_attack;
	      adhoc_adjust(new_black_attack, 0.95);
	      valueOf(king, WHITE, attacker, ptype) = new_black_attack;
	    }
	    else
	      valueOf(king, WHITE, attacker, ptype) = black_attack;
	  }
	}
      }
    }
  }
  for (int king_x=1; king_x<=9; ++king_x)
  {
    for (int king_y=1; king_y<=9; ++king_y)
    {
      const Square king(king_x, king_y);
      // 1つしか動けない香は歩と同じ点数に
      for (int x=1; x<=9; ++x)
      {
	const Square b(x,2);
	valueOf(king, WHITE, b, LANCE) = valueOf(king, WHITE, b, PAWN);
      }
      if (king_y <= 7)
      {
	adhoc_adjust(valueOf(king, WHITE, Square(king_x, king_y + 2), PAWN),
		     1.25);
      }
      // 1段目の金類は価値を半分に
      for (int x=1; x<=9; ++x)
      {
	const Square b(x,1);
	if (x <= 2 || x >= 8)
	{
	  adhoc_adjust(valueOf(king, WHITE, b, GOLD),    0.25);
	  adhoc_adjust(valueOf(king, WHITE, b, PSILVER), 0.25);
	}
	else
	{
	  adhoc_adjust(valueOf(king, WHITE, b, GOLD),    0.5);
	  adhoc_adjust(valueOf(king, WHITE, b, PSILVER), 0.5);
	}
	adhoc_adjust(valueOf(king, WHITE, b, PKNIGHT), 0.5);
	adhoc_adjust(valueOf(king, WHITE, b, PLANCE),  0.5);
	adhoc_adjust(valueOf(king, WHITE, b, PPAWN),   0.5);
      }
      // 1段玉に対する3段目の駒は価値を1.25倍に
      if (king_y == 1)
      {
	for (int x = 1; x <= 9; ++x) {
	  const Square three(x, 3);
	  for (int ptype = PPAWN; ptype <= ROOK; ptype++)
	  {
	    if (ptype != KING && ptype != ROOK && ptype != PROOK)
	      adhoc_adjust(valueOf(king, WHITE, three,
				   static_cast<osl::Ptype>(ptype)), 1.25);
	  }
	}
      }
      if (king_y <= 2)
      {
	for (int x = std::max(king_x - 1, 2);
	     x <= std::min(king_x + 1, 8); x++)
	{
	  adhoc_adjust(valueOf(king, WHITE, Square(x, 4), PAWN),
		       1.25);
	}
      }
      // 端玉に対するボーナス
      if (king_x == 1 || king_x == 9)
      {
	int next = king_x == 1 ? 2 : 8;
	adhoc_edge_king_1(WHITE, king, Square(next, king_y));
	if (king_y < 9)
	{
	  adhoc_edge_king_1(WHITE, king, Square(next, king_y + 1));
	}
	if (king_y < 8)
	{
	  // 3段目は 1.25 * 1.25 倍されるけど気にしない
	  adhoc_edge_king_1(WHITE, king, Square(next, king_y + 2));
	}
      }
      if (king_x < 6)
      {
	valueOf(king, WHITE, Square(9, 9), LANCE) = 0;
	valueOf(king, WHITE, Square(9, 8), LANCE) = 0;
	valueOf(king, WHITE, Square(9, 7), LANCE) = 0;
	valueOf(king, WHITE, Square(8, 9), KNIGHT) = 0;

	valueOf(king, WHITE, Square(9, 1), GOLD) = 0;
	valueOf(king, WHITE, Square(9, 1), SILVER) = 0;
	valueOf(king, WHITE, Square(9, 1), PSILVER) = 0;
	valueOf(king, WHITE, Square(8, 1), GOLD) = 0;
	valueOf(king, WHITE, Square(8, 1), SILVER) = 0;
	valueOf(king, WHITE, Square(8, 1), PSILVER) = 0;
      }

      if (king_x > 4)
      {
	valueOf(king, WHITE, Square(1, 9), LANCE) = 0;
	valueOf(king, WHITE, Square(1, 8), LANCE) = 0;
	valueOf(king, WHITE, Square(1, 7), LANCE) = 0;
	valueOf(king, WHITE, Square(2, 9), KNIGHT) = 0;

	valueOf(king, WHITE, Square(1, 1), GOLD) = 0;
	valueOf(king, WHITE, Square(1, 1), PSILVER) = 0;
	valueOf(king, WHITE, Square(1, 1), SILVER) = 0;
	valueOf(king, WHITE, Square(2, 1), GOLD) = 0;
	valueOf(king, WHITE, Square(2, 1), SILVER) = 0;
	valueOf(king, WHITE, Square(2, 1), PSILVER) = 0;
      }
    }
  }
  adhoc_edge_king_2(WHITE, Square(1, 1), Square(3, 2));
  adhoc_edge_king_2(WHITE, Square(1, 2), Square(3, 3));
  adhoc_edge_king_2(WHITE, Square(9, 1), Square(7, 2));
  adhoc_edge_king_2(WHITE, Square(9, 2), Square(7, 3));

  for (int x = 1; x <= 9; x++)
  {
    for (int y = 1; y <= 9; y++)
    {
      adhoc_adjust(valueOf(Square(1,1), WHITE, Square(x, y), GOLD), 1.05);
      adhoc_adjust(valueOf(Square(9,1), WHITE, Square(x, y), GOLD), 1.05);
      adhoc_adjust(valueOf(Square(1,1), WHITE, Square(x, y), SILVER), 1.05);
      adhoc_adjust(valueOf(Square(9,1), WHITE, Square(x, y), SILVER), 1.05);
      adhoc_adjust(valueOf(Square(1,1), WHITE, Square(x, y), BISHOP), 0.95);
      adhoc_adjust(valueOf(Square(1,1), WHITE, Square(x, y), PBISHOP), 0.95);
      adhoc_adjust(valueOf(Square(9,1), WHITE, Square(x, y), BISHOP), 0.95);
      adhoc_adjust(valueOf(Square(9,1), WHITE, Square(x, y), PBISHOP), 0.95);
      for (int ptype = PPAWN; ptype <= PSILVER; ptype++)
      {
	adhoc_adjust(valueOf(Square(1, 1), WHITE, Square(x,y),
			     static_cast<osl::Ptype>(ptype)), 1.05);
	adhoc_adjust(valueOf(Square(9, 1), WHITE, Square(x,y),
			     static_cast<osl::Ptype>(ptype)), 1.05);
      }
    }
  }
  adhoc_adjust(valueOf(Square(1,1), WHITE, Square::STAND(), BISHOP), 0.95);
  adhoc_adjust(valueOf(Square(9,1), WHITE, Square::STAND(), BISHOP), 0.95);

  // BLACK/WHITE 反転
  for (int king_x=1; king_x<=9; ++king_x) {
    for (int king_y=1; king_y<=9; ++king_y) {
      const Square king_b(king_x, king_y);
      const Square king_w = king_b.rotate180();
      
      for (int p=PTYPE_PIECE_MIN; p<=PTYPE_MAX; ++p) {
	const Ptype ptype = static_cast<Ptype>(p);
	assert(isPiece(ptype));

	// 持駒
	valueOf(king_b, BLACK, Square::STAND(), ptype) 
	  = - valueOf(king_w, WHITE, Square::STAND(), ptype);

	// 盤上
	for (int attack_x=1; attack_x<=9; ++attack_x) {
	  for (int attack_y=1; attack_y<=9; ++attack_y) {
	    const Square attack_b(attack_x, attack_y); // white へのblackの攻撃
	    const Square attack_w = attack_b.rotate180();

	    valueOf(king_b, BLACK, attack_w, ptype) 
	      = - valueOf(king_w, WHITE, attack_b, ptype);
	  }
	}
      }
    }
  }
}

void osl::eval::endgame::AttackKing::
saveText(const char *filename)
{
  table.saveText(filename);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
