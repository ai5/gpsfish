/* pairedit.cc
 */

#include "osl/eval/ppair/piecePairRawEval.h"
#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

/**
 * @file
 * PiecePairEvalTable に人為的に手を加える
 *
 * 試しに，玉の周り5x5 端では4x4 にいる敵の駒を+100してみる
 * 歩桂香は玉より下では加点しない，代わりに上を延ばす
 * 下にいる金は半分?
 * 頂点は半分?
 *
 * 参考
 * http://www32.ocn.ne.jp/~yss/book.html#SEC3
 * 桂、香の駒は玉が実際の位置よりももう１段上にいるとして計算している（桂香では敵玉から３段上が最大となる）
 */
using namespace osl;
using namespace osl::eval;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-f read-pair-file-name] [-o write-pair-file-name] "
       << endl;
  exit(1);
}

bool verbose = false;

void adjust(PiecePairRawTable& table, 
	    Square pos1, PtypeO ptypeo1,
	    Square pos2, PtypeO ptypeo2,
	    int value)
{
  assert(pos1.isOnBoard());
  assert(pos2.isOnBoard());
  const unsigned int index1 = table.indexOf(pos1, ptypeo1);
  const unsigned int index2 = table.indexOf(pos2, ptypeo2);
  
  int val = table.valueOf(index1, index2);
  assert(val == table.valueOf(index2, index1));
  if (verbose)
    std::cerr << pos1 << ptypeo1 << " " << pos2 << ptypeo2
	      << " " << val << " => ";
  val += value;
  val = std::min(127, val);
  val = std::max(-127, val);
  if (verbose)
    std::cerr << val << "\n";
  table.valueOf(index1, index2) = val;
  table.valueOf(index2, index1) = val;
}

void adjustKingBonus(PiecePairRawTable& table, 
		     Square pos1, PtypeO ptypeo1, // king
		     Square pos2, PtypeO ptypeo2, // attacker
		     int bonus)
{
  assert(getPtype(ptypeo1) == KING);
  assert(getPtype(ptypeo2) != KING);
  assert(getOwner(ptypeo1) != getOwner(ptypeo2));
  assert((bonus > 0) ^ (getOwner(ptypeo1) == BLACK));

  adjust(table, pos1, ptypeo1, pos2, ptypeo2, bonus);
}

/** king: white, attacker: black*/
void adjustDual(PiecePairRawTable& table, 
		Square king, Square attacker, Ptype attackerType,
		int blackAttackBonus, int whiteAttackBonus)
{
  adjustKingBonus(table, king.rotate180(), newPtypeO(BLACK, KING),
	 attacker.rotate180(), newPtypeO(WHITE, attackerType),
	 whiteAttackBonus);
  adjustKingBonus(table, king, newPtypeO(WHITE, KING),
	 attacker, newPtypeO(BLACK, attackerType),
	 blackAttackBonus);
}

void adjustDual(PiecePairRawTable& table, 
		Square black, Ptype black_ptype,
		Square white, Ptype white_ptype,
		int value)
{
  adjust(table, black, newPtypeO(BLACK, black_ptype),
	 white, newPtypeO(WHITE, white_ptype), value);
  adjust(table, black.rotate180(), newPtypeO(WHITE, black_ptype),
	 white.rotate180(), newPtypeO(BLACK, white_ptype), -value);
}

void addValue(Player player, PiecePairRawTable& table,
	      Square pos1, Ptype ptype1,
	      Square pos2, Ptype ptype2,
	      int bonus)
{
  const PtypeO ptypeo1 = newPtypeO(player, ptype1);
  const PtypeO ptypeo2 = newPtypeO(player, ptype2);
  adjust(table, pos1, ptypeo1, pos2, ptypeo2, bonus);
}

void addPenalty(Player player, PiecePairRawTable& table,
		Square pos1, Ptype ptype1,
		Square pos2, Ptype ptype2,
		int bonus)
{
  assert(eval::betterThan(player, 0, bonus));
  addValue(player, table, pos1, ptype1, pos2, ptype2, bonus);
}

void addBonus(Player player, PiecePairRawTable& table,
	      Square pos1, Ptype ptype1,
	      Square pos2, Ptype ptype2,
	      int bonus)
{
  assert(eval::betterThan(player, bonus, 0));
  addValue(player, table, pos1, ptype1, pos2, ptype2, bonus);
}

void addPenaltyDual(PiecePairRawTable& table, 
		    Square pos1, Ptype ptype1,
		    Square pos2, Ptype ptype2,
		    int black_bonus)
{
  assert(black_bonus < 0);
  addPenalty(BLACK, table, pos1, ptype1, pos2, ptype2,  black_bonus);
  addPenalty(WHITE, table, pos1, ptype1, pos2, ptype2, -black_bonus);
}

void addSelfPenaltyDual(PiecePairRawTable& table, 
			Square pos, Ptype ptype,
			int black_bonus)
{
  addPenaltyDual(table, pos, ptype, pos, ptype, black_bonus);
}


int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  const char *read_pairfilename = 0;
  const char *write_pairfilename = 0;
  
  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "f:o:vh")) != EOF)
  {
    switch(c)
    {
    case 'f':	read_pairfilename = optarg;
      break;
    case 'o':	write_pairfilename = optarg;
      break;
    case 'v':	verbose = true;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag || (! read_pairfilename) || (! write_pairfilename))
    usage(program_name);

  std::unique_ptr<PiecePairRawTable> table(new PiecePairRawTable);
  table->loadFromBinaryFile(read_pairfilename);

  // 通常の5x5の範囲
  CArray2d<bool,Square::SIZE,Square::SIZE> adjusted;
  adjusted.fill(false);
  for (int king_x=1; king_x<=9; ++king_x)
  {
    for (int king_y=1; king_y<=9; ++king_y)
    {
      const Square king(king_x,king_y);
      for (int attacker_x = ((king_x==9) ? king_x-3 : king_x-2); 
	   attacker_x <= ((king_x==1) ? king_x+3 : king_x+2); ++attacker_x)
      {
	if ((attacker_x < 1) || (attacker_x > 9))
	  continue;
	for (int attacker_y = ((king_y==9) ? king_y-3 : king_y-2); 
	     attacker_y <= ((king_y==1) ? king_y+3 : king_y+2); ++attacker_y)
	{
	  if ((attacker_y < 1) || (attacker_y > 9))
	    continue;
	  const Square attacker(attacker_x,attacker_y);
	  if (king == attacker)
	    continue;
	  adjusted[king.index()][attacker.index()] = true;
	  adjustDual(*table, king, attacker, PPAWN,   100, -100);
	  adjustDual(*table, king, attacker, PLANCE,  100, -100);
	  adjustDual(*table, king, attacker, PKNIGHT, 100, -100);
	  adjustDual(*table, king, attacker, PBISHOP, 100, -100);
	  adjustDual(*table, king, attacker, PROOK,   100, -100);
	  adjustDual(*table, king, attacker, SILVER,  100, -100);
	  adjustDual(*table, king, attacker, BISHOP,  100, -100);
	  adjustDual(*table, king, attacker, ROOK,    100, -100);

	  // 金と成銀は1段目以外
	  if (attacker_y != 1)
	  {
	    const int bonus = 
	      (attacker_y >= king_y) ? 100 : 50;
	    adjustKingBonus(*table, king, newPtypeO(WHITE, KING),
			    attacker, newPtypeO(BLACK, PSILVER),
			    bonus);
	    adjustKingBonus(*table, king, newPtypeO(WHITE, KING),
			    attacker, newPtypeO(BLACK, GOLD),
			    bonus);
	  }
	  if (attacker_y != 9)
	  {
	    const int bonus = 
	      (attacker_y <= king_y) ? -100 : -50;
	    adjustKingBonus(*table, king, newPtypeO(BLACK, KING),
			    attacker, newPtypeO(WHITE, PSILVER),
			    bonus);
	    adjustKingBonus(*table, king, newPtypeO(BLACK, KING),
			    attacker, newPtypeO(WHITE, GOLD),
			    bonus);
	  }
	  // 歩は王と同列かより手前
	  if (attacker_y >= king_y)
	  {
	    adjustKingBonus(*table, king, newPtypeO(WHITE, KING),
			    attacker, newPtypeO(BLACK, PAWN),
			    100);
	  }
	  if (attacker_y <= king_y)
	  {
	    adjustKingBonus(*table, king, newPtypeO(BLACK, KING),
			    attacker, newPtypeO(WHITE, PAWN),
			    -100);
	  }
	} // attacker_y
	// 桂香は長目にとる
	for (int attacker_y = ((king_y==9) ? king_y-4 : king_y-3);
	     attacker_y <= ((king_y==1) ? king_y+4 : king_y+3); ++attacker_y)
	{
	  if ((attacker_y < 1) || (attacker_y > 9))
	    continue;
	  const Square attacker(attacker_x,attacker_y);
	  if (king == attacker)
	    continue;

	  // 王より手前, 25,45,65,85 は弊害が大きいので除く
	  if (! ((attacker_y == 5)
		 && ((attacker_x == 2)
		     || (attacker_x == 4)
		     || (attacker_x == 6)
		     || (attacker_x == 8))))
	  {
	    if ((attacker_y > king_y) && (attacker_y > 2))
	    {
	      adjustKingBonus(*table, king, newPtypeO(WHITE, KING),
			      attacker, newPtypeO(BLACK, LANCE),
			      100);
	      adjustKingBonus(*table, king, newPtypeO(WHITE, KING),
			      attacker, newPtypeO(BLACK, KNIGHT),
			      100);
	    }
	    if ((attacker_y < king_y) && (attacker_y < 8))
	    {
	      adjustKingBonus(*table, king, newPtypeO(BLACK, KING),
			      attacker, newPtypeO(WHITE, LANCE),
			      -100);
	      adjustKingBonus(*table, king, newPtypeO(BLACK, KING),
			      attacker, newPtypeO(WHITE, KNIGHT),
			      -100);
	    }
	  }
	} // attacker_y, knight, lance
      }	// attacker_x
    } // king_y
  } // king_x
  // 5x5の範囲外の補正 x+1
  for (int king_x=1; king_x<=9; ++king_x)
  {
    for (int king_y=1; king_y<=3; ++king_y)
    {
      const Square king(king_x,king_y);
      for (int rook_x=1; rook_x<=9; ++rook_x)
      {
	for (int rook_y=king_y-1; rook_y<=king_y+1; ++rook_y)
	{
	  if ((rook_y < 1) || (rook_y > 9))
	    continue;
	  const Square rook(rook_x, rook_y);
	  if (king == rook)
	    continue;
	  if (! adjusted[king.index()][rook.index()])
	  {
	    adjustDual(*table, king, rook, ROOK,   30, -30);
	    adjustDual(*table, king, rook, PROOK,  30, -30);
	  }
	}
      }
      for (int attacker_x = ((king_x==9) ? king_x-4 : king_x-3); 
	   attacker_x <= ((king_x==1) ? king_x+4 : king_x+3); ++attacker_x)
      {
	if ((attacker_x < 1) || (attacker_x > 9))
	  continue;
	for (int attacker_y = king_y; attacker_y <= ((king_y==1) ? king_y+3 : king_y+2); ++attacker_y)
	{
	  if ((attacker_y < 1) || (attacker_y > 9))
	    continue;
	  const Square attacker(attacker_x,attacker_y);
	  if (king == attacker)
	    continue;
	  if (adjusted[king.index()][attacker.index()])
	    continue;
	  adjusted[king.index()][attacker.index()] = true;
	  adjustDual(*table, king, attacker, PPAWN,   40, -40);
	  adjustDual(*table, king, attacker, PLANCE,  40, -40);	  
	  adjustDual(*table, king, attacker, PKNIGHT, 40, -40);	  
	  adjustDual(*table, king, attacker, SILVER,  40, -40);	  
	}
      }
    }
  }
  // x+2
  for (int king_x=1; king_x<=9; ++king_x)
  {
    for (int king_y=1; king_y<=3; ++king_y)
    {
      const Square king(king_x,king_y);
      for (int attacker_x = ((king_x==9) ? king_x-5 : king_x-4); 
	   attacker_x <= ((king_x==1) ? king_x+5 : king_x+4); ++attacker_x)
      {
	if ((attacker_x < 1) || (attacker_x > 9))
	  continue;
	for (int attacker_y = king_y+1; attacker_y <= ((king_y==1) ? king_y+3 : king_y+2); ++attacker_y)
	{
	  if ((attacker_y < 1) || (attacker_y > 9))
	    continue;
	  const Square attacker(attacker_x,attacker_y);
	  if (king == attacker)
	    continue;
	  if (adjusted[king.index()][attacker.index()])
	    continue;
	  adjustDual(*table, king, attacker, PPAWN,   10, -10);
	  adjustDual(*table, king, attacker, PLANCE,  10, -10);	  
	  adjustDual(*table, king, attacker, PKNIGHT, 10, -10);	  
	  adjustDual(*table, king, attacker, SILVER,  10, -10);	  
	}
      }
    }
  }
  for (int attacker_x=1; attacker_x<=9; ++attacker_x)
  {
    for (int attacker_y=1; attacker_y<=9; ++attacker_y)
    {
      const Square attacker(attacker_x,attacker_y);
      // 端の駒は色々減点
      if ((attacker_x == 1) || (attacker_x == 9))
      {
	addSelfPenaltyDual(*table, attacker, KNIGHT,  -100);
	addSelfPenaltyDual(*table, attacker, SILVER,  -100);
	addSelfPenaltyDual(*table, attacker, GOLD  ,  -100);
	addSelfPenaltyDual(*table, attacker, PPAWN ,  -100);
	addSelfPenaltyDual(*table, attacker, PLANCE,  -100);
	addSelfPenaltyDual(*table, attacker, PKNIGHT, -100);
	addSelfPenaltyDual(*table, attacker, PSILVER, -100);
      }
    }
  }
  // 捌きにくい銀
  addPenalty(BLACK, *table, Square(2,6), SILVER, Square(3,7), KNIGHT, -80);
  addPenalty(WHITE, *table, Square(8,4), SILVER, Square(7,3), KNIGHT, +80);

  addPenalty(BLACK, *table, Square(2,6), SILVER, Square(1,5), PAWN, -80);
  addPenalty(WHITE, *table, Square(8,4), SILVER, Square(9,5), PAWN, +80);

  addPenalty(BLACK, *table, Square(9,7), SILVER, Square(8,8), BISHOP, -80);
  addPenalty(BLACK, *table, Square(1,7), SILVER, Square(2,8), BISHOP, -80);
  addPenalty(WHITE, *table, Square(9,3), SILVER, Square(8,2), BISHOP, +80);
  addPenalty(WHITE, *table, Square(1,3), SILVER, Square(2,2), BISHOP, +80);
  
  // 穴熊関係
  addPenalty(BLACK, *table, Square(9,9), KING, Square(7,7), KNIGHT, -120);
  addPenalty(BLACK, *table, Square(9,9), KING, Square(9,7), KNIGHT, -120);
  addPenalty(BLACK, *table, Square(1,9), KING, Square(3,7), KNIGHT, -120);
  addPenalty(BLACK, *table, Square(1,9), KING, Square(1,7), KNIGHT, -120);
  addPenalty(WHITE, *table, Square(9,1), KING, Square(7,3), KNIGHT, +120);
  addPenalty(WHITE, *table, Square(9,1), KING, Square(9,3), KNIGHT, +120);
  addPenalty(WHITE, *table, Square(1,1), KING, Square(3,3), KNIGHT, +120);
  addPenalty(WHITE, *table, Square(1,1), KING, Square(1,3), KNIGHT, +120);

  // 玉の点数を引いておき，桂香があれば復活させる
  addPenalty(BLACK, *table, Square(9,9), KING, Square(9,9), KING, -120);
  addBonus(BLACK, *table, Square(9,9), KING, Square(9,8), LANCE, 40);
  addBonus(BLACK, *table, Square(9,9), KING, Square(8,9), KNIGHT, 80);

  addPenalty(BLACK, *table, Square(1,9), KING, Square(1,9), KING, -120);
  addBonus(BLACK, *table, Square(1,9), KING, Square(1,8), LANCE, 40);
  addBonus(BLACK, *table, Square(1,9), KING, Square(2,9), KNIGHT, 80);

  addPenalty(WHITE, *table, Square(9,1), KING, Square(9,1), KING, +120);
  addBonus(WHITE, *table, Square(9,1), KING, Square(9,2), LANCE, -40);
  addBonus(WHITE, *table, Square(9,1), KING, Square(8,1), KNIGHT, -80);

  addPenalty(WHITE, *table, Square(1,1), KING, Square(1,1), KING, +120);
  addBonus(WHITE, *table, Square(1,1), KING, Square(1,2), LANCE, -40);
  addBonus(WHITE, *table, Square(1,1), KING, Square(2,1), KNIGHT, -80);

  // 5段目の桂馬 (取られそう)
  for (int x=2; x<=8; ++x)	// 1,9はペナルティ済
  {
    // 
    const Square attacker(x,5);
    addSelfPenaltyDual(*table, attacker, KNIGHT, -100);

    // 相手の歩が遠ければ大丈夫かも．
    adjust(*table, Square(x,1), newPtypeO(WHITE,PAWN), attacker, newPtypeO(BLACK,KNIGHT), 100);
    adjust(*table, Square(x,2), newPtypeO(WHITE,PAWN), attacker, newPtypeO(BLACK,KNIGHT), 100);
    adjust(*table, Square(x,9), newPtypeO(BLACK,PAWN), attacker, newPtypeO(WHITE,KNIGHT), -100);
    adjust(*table, Square(x,8), newPtypeO(BLACK,PAWN), attacker, newPtypeO(WHITE,KNIGHT), -100);
  }

  // 敵の大駒の成駒と自陣の生の駒
  for (int promoted_x=1; promoted_x<=9; ++promoted_x)
  {
    for (int x=1; x<=9; ++x)
    {
      for (int promoted_y=1; promoted_y<=3; ++promoted_y)
      {
	const Square promoted(promoted_x, promoted_y);
	for (int y=1; y<=3; ++y)
	{
	  const Square unpromoted(x, y);
	  if (promoted == unpromoted)
	    continue;
	  adjust(*table, promoted, newPtypeO(BLACK,PROOK), 
		 unpromoted, newPtypeO(WHITE,ROOK), 100);
	  adjust(*table, promoted, newPtypeO(BLACK,PROOK), 
		 unpromoted, newPtypeO(WHITE,BISHOP), 100);
	  adjust(*table, promoted, newPtypeO(BLACK,PBISHOP), 
		 unpromoted, newPtypeO(WHITE,ROOK), 100);
	  adjust(*table, promoted, newPtypeO(BLACK,PBISHOP), 
		 unpromoted, newPtypeO(WHITE,BISHOP), 100);
	}
      }
      for (int promoted_y=7; promoted_y<=9; ++promoted_y)
      {
	const Square promoted(promoted_x, promoted_y);
	for (int y=7; y<=9; ++y)
	{
	  const Square unpromoted(x, y);
	  if (promoted == unpromoted)
	    continue;
	  adjust(*table, promoted, newPtypeO(WHITE,PROOK), 
		 unpromoted, newPtypeO(BLACK,ROOK), -100);
	  adjust(*table, promoted, newPtypeO(WHITE,PROOK), 
		 unpromoted, newPtypeO(BLACK,BISHOP), -100);
	  adjust(*table, promoted, newPtypeO(WHITE,PBISHOP), 
		 unpromoted, newPtypeO(BLACK,ROOK), -100);
	  adjust(*table, promoted, newPtypeO(WHITE,PBISHOP), 
		 unpromoted, newPtypeO(BLACK,BISHOP), -100);
	}
      }
    } // x
  }
  // 4,6段目の歩に抑え込みボーナス
  for (int x=1; x<=9; ++x)
  {
    // 
    const Square black_position(x,4);
    const Square white_position(x,6);
    addBonus(BLACK, *table, black_position, PAWN, black_position, PAWN, 100);
    addBonus(WHITE, *table, white_position, PAWN, white_position, PAWN,-100);
  }

  for (int x=1; x<=9; ++x)
  {
    // 危ない飛車
    addPenalty(BLACK, *table, Square(x,6), ROOK, Square(x,7), PAWN, -95);
    addPenalty(WHITE, *table, Square(x,4), ROOK, Square(x,3), PAWN, +95);
    // 捌きにくい銀
    if (x == 1 || x == 3 || x == 7 || x == 9) 
    {
      addPenalty(BLACK, *table, Square(x,6), SILVER, Square(x,7), PAWN, -80);
      addPenalty(WHITE, *table, Square(x,4), SILVER, Square(x,3), PAWN, +80);
    }
    // 捻り飛車失敗?
    for (int i=x-1; i<=x+1; ++i)
    {
      if (i<1 || i > 9)
	continue;
      addPenalty(BLACK, *table, Square(x,6), ROOK, Square(i,8), KING, -100);
      addPenalty(BLACK, *table, Square(x,6), ROOK, Square(i,9), KING,  -90);
      addPenalty(WHITE, *table, Square(x,4), ROOK, Square(i,2), KING, +100);
      addPenalty(WHITE, *table, Square(x,4), ROOK, Square(i,1), KING,  +90);
    }
    // 玉飛接近
    for (int y=7; y<=9; ++y)
    {
      const int wy = 10-y;
      addPenalty(BLACK, *table, Square(x,y), KING, Square(x,y-1), ROOK, -70);
      addPenalty(BLACK, *table, Square(x,y-1), KING, Square(x,y), ROOK, -70);
      addPenalty(WHITE, *table, Square(x,wy), KING, Square(x,wy+1), ROOK, +70);
      addPenalty(WHITE, *table, Square(x,wy+1), KING, Square(x,wy), ROOK, +70);
      if (x > 1)
      {
	addPenalty(BLACK, *table, Square(x,y), KING, Square(x-1,y), ROOK, -70);
	addPenalty(WHITE, *table, Square(x,wy), KING, Square(x-1,wy), ROOK, +70);
      }
      if (x > 2) 
      {
	addPenalty(BLACK, *table, Square(x,y), KING, Square(x-2,y), ROOK, -50);
	addPenalty(WHITE, *table, Square(x,wy), KING, Square(x-2,wy), ROOK, +50);
      }
      if (x < 9)
      {
	addPenalty(BLACK, *table, Square(x,y), KING, Square(x+1,y), ROOK, -70);
	addPenalty(WHITE, *table, Square(x,wy), KING, Square(x+1,wy), ROOK, +70);
      }
      if (x < 8)
      {
	addPenalty(BLACK, *table, Square(x,y), KING, Square(x+2,y), ROOK, -50);
	addPenalty(WHITE, *table, Square(x,wy), KING, Square(x+2,wy), ROOK, +50);
      }
    } // for y
  } // for x

  // 桂馬と飛車
  addPenalty(BLACK, *table, Square(1,7), ROOK, Square(2,9), KNIGHT, -70);
  addPenalty(BLACK, *table, Square(3,7), ROOK, Square(2,9), KNIGHT, -70);
  addPenalty(BLACK, *table, Square(7,7), ROOK, Square(8,9), KNIGHT, -70);
  addPenalty(BLACK, *table, Square(9,7), ROOK, Square(8,9), KNIGHT, -70);
  addPenalty(WHITE, *table, Square(1,3), ROOK, Square(2,1), KNIGHT, +70);
  addPenalty(WHITE, *table, Square(3,3), ROOK, Square(2,1), KNIGHT, +70);
  addPenalty(WHITE, *table, Square(7,3), ROOK, Square(8,1), KNIGHT, +70);
  addPenalty(WHITE, *table, Square(9,3), ROOK, Square(8,1), KNIGHT, +70);
  // 角道を防ぐ金
  for (int x=1; x<=9; ++x)
  {
    for (int y=8; y<=9; ++y) 
    {
      const Square bishop(x, y);
      if (x < 9)
      {
	const Square ul(x+1, y-1);
	addPenalty(BLACK, *table, bishop, BISHOP, ul, GOLD, -65);
	addPenalty(WHITE, *table, bishop.rotate180(), BISHOP, 
		   ul.rotate180(), GOLD, 65);
      }
      if (x > 1) 
      {
	const Square ur(x-1, y-1);
	addPenalty(BLACK, *table, bishop, BISHOP, ur, GOLD, -65);
	addPenalty(WHITE, *table, bishop.rotate180(), BISHOP, 
		   ur.rotate180(), GOLD, 65);
      }
    }
  }

  // 隣接する成駒
  const CArray<Ptype, 6> types = 
    {{ LANCE, KNIGHT, PPAWN, PLANCE, PKNIGHT, PSILVER }};
  for (int x1=1; x1<=9; ++x1)
  {
    for (int x2=x1; x2<=9; ++x2)
    {
      const int xdiff = abs(x1 - x2);
      if (xdiff > 2)
	continue;
      for (int y1=1; y1<=3; ++y1) 
      {
	const Square p1(x1,y1);
	for (int y2=y1; y2<=3; ++y2) 
	{
	  if (x1 == x2 && y1 == y2)
	    continue;
	  const Square p2(x2,y2);
	  const int py = (3-std::min(y1, y2))*10; // 3段目のほうがマシに
	  const int center = std::min(abs(5-x1), abs(5-x2)); // 端を減点
	  const int p = 0-py-center*15; // y=1:0,-80
	  if (p == 0)
	    continue;
	  for (int t1=0; t1<(int)types.size(); ++t1)
	  {
	    assert(isPiece(types[t1]));
	    if (y1 < 3 && types[t1] == KNIGHT)
	      continue;
	    if (y1 == 1 && (types[t1] == LANCE || types[t1] == PAWN))
	      continue;
	    for (int t2=0; t2<(int)types.size(); ++t2)
	    {
	      if (y2 < 3 && types[t2] == KNIGHT)
		continue;
	      if (y2 == 1 && (types[t2] == LANCE || types[t2] == PAWN))
		continue;
	      addPenalty(BLACK, *table, p1, types[t1], p2, types[t2], p);
	      addPenalty(WHITE, *table, p1.rotate180(), types[t1], 
			 p2.rotate180(), types[t2], -p);
	    }
	  }
	}
      }
    }
  }

  // 飛車先の歩を切る
  for (int x=1; x<=9; ++x)
  {
    for (int y=5; y<=7; ++y)
    {
      const Square pawn(x,y);
      for (int ry=6; ry<=9; ++ry)
      {
	if (y == ry)
	  continue;
	const Square rook(x,ry);
	const int p = -y*10-25;
	addPenalty(BLACK, *table, rook, ROOK, pawn, PAWN, p);
	addPenalty(WHITE, *table, rook.rotate180(), ROOK, 
		   pawn.rotate180(), PAWN, -p);
      }
    }
  }

  // 相手の飛車/香車を受ける
  for (int x=1; x<=9; ++x)
  {
    for (int y=3; y<=7; ++y)
    {
      const Square pawn(x,y);
      for (int ry=1; ry<y; ++ry)
      {
	const Square rook(x,ry);
	adjustDual(*table, pawn, PAWN, rook, ROOK, 90);
	adjustDual(*table, pawn, PAWN, rook, LANCE, 90);
      }
    }
  }

  // 自陣の歩
  for (int x=1; x<=9; ++x)
  {
    for (int y=8; y<=9; ++y)
    {
      const Square pawn(x,y);
      const Square pawn_white = pawn.rotate180();
#if 0
      addPenalty(BLACK, *table, pawn, PAWN, pawn, PAWN, -60);
      addPenalty(WHITE, *table, pawn_white, PAWN, pawn_white, PAWN, 60);
#endif
      // 玉に近いとさらに減点
      for (int kx=1; kx<=9; ++kx)
      {
	for (int ky=7; ky<=9; ++ky)
	{
	  const Square king(kx,ky);
	  const Square king_white = king.rotate180();
	  if (king == pawn)
	    continue;
	  const int penalty = -(8-abs(kx-x))*10;	// 0-80
	  if (penalty == 0)
	    continue;
	  addPenalty(BLACK, *table, pawn, PAWN, king, KING, penalty);
	  addPenalty(WHITE, *table, pawn_white, PAWN, king_white, KING, -penalty);

	  // 敵玉の居る筋に対するペナルティ
	  adjust(*table, pawn, newPtypeO(BLACK,PAWN), 
		 Square(kx,10-ky), newPtypeO(WHITE,KING), penalty);
	  adjust(*table, Square(x,10-y), newPtypeO(WHITE,PAWN), 
		 king, newPtypeO(BLACK,KING), -penalty);
	}
      }
    }
  }
  
  table->writeInBinaryFile(write_pairfilename);
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
