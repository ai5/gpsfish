/* majorPiece.h
 */

#ifndef EVAL_ML_MAJORPIECE_H
#define EVAL_ML_MAJORPIECE_H

#include "osl/eval/weights.h"
#include "osl/eval/midgame.h"
#include "osl/numEffectState.h"
#include <cstdlib>

namespace osl
{
  namespace eval
  {
    namespace ml
    {
      template <bool Opening, Ptype MajorBasic>
      class MajorY
      {
      private:
	static CArray<int, 18> table;
	static int index(Piece piece)
	{
	  return ((piece.owner() == BLACK) ? (piece.square().y() - 1) :
		  (9 - piece.square().y())) + (piece.isPromoted() ? 9 : 0);
	}
      public:
	enum { DIM = 18 };
	static void setUp(const Weights &weights);
	static int eval(const NumEffectState &state)
	{
	  int value = 0;
	  for (int i = PtypeTraits<MajorBasic>::indexMin;
	       i < PtypeTraits<MajorBasic>::indexLimit;
	       ++i)
	  {
	    const Piece piece = state.pieceOf(i);
	    if (piece.isOnBoard())
	      {
		if (piece.owner() == BLACK)
		  value += table[index(piece)];
	        else
		  value -= table[index(piece)];
	      }
	  }
	  return value;
	}
      };

      class RookYOpening : public MajorY<true, ROOK>
      {
      };
      class RookYEnding : public MajorY<false, ROOK>
      {
      };
      class BishopYOpening : public MajorY<true, BISHOP>
      {
      };
      class BishopYEnding : public MajorY<false, BISHOP>
      {
      };

      template <bool Opening>
      class RookPawn
      {
      public:
	enum { DIM = 1 };
	static void setUp(const Weights &weights);
	static int eval(const NumEffectState &state);
      private:
	static int weight;
      };
      class RookPawnOpening : public RookPawn<true>
      {
      };
      class RookPawnEnding : public RookPawn<false>
      {
      };

      class RookPawnY
      {
	friend class RookPawnYX;
      public:
	enum { ONE_DIM = 180, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state,
			      const CArray2d<int, 2, 9> &pawns);
      private:
	static int index(const Piece rook, const int pawn_y)
	{
	  const int rook_y =
	    (rook.owner() == BLACK ? rook.square().y() : 10 - rook.square().y());
	  return (rook_y - 1) * 10 + pawn_y + (rook.isPromoted() ? 90 : 0);
	}
	static int indexY(const Square king,
			  const Piece rook, int pawn_y)
	{
	  const int x_diff = std::abs(rook.square().x() - king.x());
	  const int rook_y =
	    (rook.owner() == BLACK ? rook.square().y() : 10 - rook.square().y());
	  return x_diff * 10 * 9 + (rook_y - 1) * 10 + pawn_y + (rook.isPromoted() ? 810 : 0);
	}
	static CArray<MultiInt, 180> table;
	static CArray<MultiInt, 1620> y_attack_table;
	static CArray<MultiInt, 1620> y_defense_table;
      };

      class RookPawnYX
      {
      public:
	enum { ONE_DIM = 1620, DIM = ONE_DIM * 2*EvalStages };
	static void setUp(const Weights &weights);
      };

      class AllMajor
      {
      public:
	enum { DIM = 1 };
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(int black_major_count)
	{
	  if (black_major_count == 4)
	    return weight;
	  else if (black_major_count == 0)
	    return -weight;

	  return MultiInt();
	}
      private:
	static MultiInt weight;
      };

      template <bool Opening>
      class MajorGoldSilverAttacked
      {
      public:
	enum { DIM = 32 };
	static void setUp(const Weights &weights);
	static int eval(const NumEffectState &state);
	static int index(const NumEffectState &state, Piece piece);
	template <Ptype PTYPE>
	static int evalOne(const NumEffectState &state);
      private:
	static CArray<int, 32> table;
      };

      class MajorGoldSilverAttackedOpening : public MajorGoldSilverAttacked<true>
      {
      };
      class MajorGoldSilverAttackedEnding : public MajorGoldSilverAttacked<false>
      {
      };

      class RookEffectBase
      {
	friend class RookEffectPiece;
      public:
	enum { ONE_DIM = 612, DIM = ONE_DIM * 2 };
	static MultiInt eval(const NumEffectState &state);
      protected:
	template<Player P>
	static MultiInt evalOne(const NumEffectState& state,
				 Square rook,
				 Square myKing,
				 Square opKing,
				 Square up,
				 Square dp,
				 Square rp,
				 Square lp,
				 bool isP);
	/**
	 * (abs_x_diff, y_diff) - 玉を原点とした時の空マスの相対位置
	 * horizontal - 飛車の横利きがある場合
	 * is_promoted - 竜の場合
	 */
	static int index(int abs_x_diff, int y_diff, bool horizontal, bool is_promoted)
	{
	  return y_diff + 8 + abs_x_diff * 17 + (horizontal ? 153 : 0) +
	    (is_promoted ? 306 : 0);
	}
	/**
	 * 黒の飛車(竜)から利きのある駒
	 * (abs_x_diff, y_diff) - 駒を基準にした玉の相対位置
	 *                        abs_x_diffは絶対値
	 * ptypeO - 駒のptypeO, 白からの場合は反転
	 * horizontal - 飛車の横利きがある場合
	 * is_promoted - 竜の場合
	 */
	static int index0(int abs_x_diff,int y_diff,
			  PtypeO ptypeO,
			  bool horizontal, bool promoted){
	  return y_diff+8+abs_x_diff*17+(ptypeO - PTYPEO_MIN) * 17 * 9 +
	    (horizontal ? 4896 : 0) + (promoted ? 9792 : 0);
	}
	/**
	 * 黒の飛車(竜)からの利きのある駒のindex
	 * {attack,defense}_{u,r,l,d} へのアクセスに使う
	 * from - 駒の位置
	 * king - 玉の位置
	 * ptypeO - 駒の種類，白からの利きの場合は反転．
	 *          (BLACK,PTYPE_EDGE)もあり得る
	 * isP - 竜の場合
	 */
	static int index1(Square king,Square from,PtypeO ptypeO,bool isP)
	{
	  int y_diff=from.y()-king.y();
	  int x_diff=from.x()-king.x();
	  return index1(x_diff,y_diff,ptypeO,isP);
	}
	/**
	 * 黒の飛車(竜)からの利きのある駒のindex
	 * {attack,defense}_{u,r,l,d} へのアクセスに使う
	 * (x_diff, y_diff) - 玉を基準に見た駒の相対位置
	 * ptypeO - 駒の種類，白からの利きの場合は反転．
	 *          (BLACK,PTYPE_EDGE)もあり得る
	 * isP - 竜の場合
	 */
	static int index1(int x_diff,int y_diff,PtypeO ptypeO,bool isP){
	  assert(-9 <= y_diff && y_diff <= 9);
	  assert(-9 <= x_diff && x_diff <= 9);
	  assert(getPtype((PtypeO)ptypeO)!=PTYPE_EMPTY);
	  int index=(ptypeO-PTYPEO_MIN)+32*((y_diff+9)+19*(x_diff+9+19*(isP ? 1 : 0)));
	  assert(0<=index && index<32*19*19*2);
	  return index;
	}
	/**
	 * 黒の飛車(竜)がある場所は空マスでないのでその分を補正するテーブル
	 * {attack,defense}_nospace へのアクセス
	 * king - 玉の位置
	 * from - 飛車(竜)の位置
	 * isP - 竜の場合
	 */
        static int index2(Square king,Square from,bool isP)
	{
	  int y_diff=from.y()-king.y();
	  int x_diff=from.x()-king.x();
	  return index2(x_diff,y_diff,isP);
	}
	/**
	 * 黒の飛車(竜)がある場所は空マスでないのでその分を補正するテーブル
	 * {attack,defense}_nospace へのアクセス
	 * (x_diff, y_diff)  - 玉を基準にしてみた飛車(竜)の相対位置
	 * isP - 竜の場合
	 */
	static int index2(int x_diff,int y_diff,bool isP){
	  assert(-9 <= y_diff && y_diff <= 9);
	  assert(-9 <= x_diff && x_diff <= 9);
	  int index=(y_diff+9)+19*(x_diff+9+19*(isP ? 1 : 0));
	  assert(0<=index && index<19*19*2);
	  return index;
	}
	static CArray<MultiInt, 612> attack_table;
	static CArray<MultiInt, 612> defense_table;
	static CArray<MultiInt, 32> piece_table;
	static CArray<MultiInt, 23104> attack_u;
	static CArray<MultiInt, 23104> attack_d;
	static CArray<MultiInt, 23104> attack_l;
	static CArray<MultiInt, 23104> attack_r;
	static CArray<MultiInt, 23104> defense_u;
	static CArray<MultiInt, 23104> defense_d;
	static CArray<MultiInt, 23104> defense_l;
	static CArray<MultiInt, 23104> defense_r;
	static CArray<MultiInt, 722> attack_nospace;
	static CArray<MultiInt, 722> defense_nospace;
      };
      class RookEffect : public RookEffectBase
      {
      public:
	static void setUp(const Weights &weights,int stage);
      };

      class RookEffectPiece
      {
      public:
	enum { DIM = 32 * EvalStages };
	static void setUp(const Weights &weights);
      };
      class RookEffectPieceKingRelative : RookEffectBase
      {
      public:
	enum { ONE_DIM = 19584, DIM = ONE_DIM * 2*EvalStages };
	static void setUp(const Weights & weights);
      };

      class RookPromoteDefense : public RookEffectBase
      {
	friend class RookPromoteDefenseRookH;
      public:
	enum { ONE_DIM = 256, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static CArray<MultiInt, 256> promote_defense_table;
	static CArray<MultiInt, 144> promote_defense_rook_table;
      };

      class RookPromoteDefenseRookH : public RookEffectBase
      {
      public:
	enum { ONE_DIM = 144, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      private:
      };

      class BishopEffectBase
      {
	friend class BishopEffectPiece;
      public:
	enum { ONE_DIM = 612, DIM = ONE_DIM * 2 };
	static MultiInt eval(const NumEffectState &state);
      protected:
	template<Player P>
	static MultiInt evalOne(const NumEffectState& state,
				 Square bishop,
				 Square myKing,
				 Square opKing,
				 Square ulp,
				 Square urp,
				 Square dlp,
				 Square drp,
				 bool isP);
	static int index(int x_diff, int y_diff, bool ur, bool promoted)
	{
	  if (x_diff<0)
	    ur = !ur;
	  return y_diff + 8 + std::abs(x_diff) * 17 + (ur ? 153 : 0) + (promoted ? 306 : 0);
	}
	static int index0(int x_diff, int y_diff,PtypeO ptypeO,bool ur, bool promoted)
	{
	  if (x_diff>0)
	    ur = !ur;
	  return -y_diff + 8 + std::abs(x_diff) * 17 + (ptypeO - PTYPEO_MIN) * 17 * 9 +
	    (ur ? 4896 : 0) + (promoted ? 9792 : 0);
	}
	/**
	 * 黒の角(馬)からの利きのある駒のindex
	 * {attack,defense}_{ul,ur,dl,dr} へのアクセスに使う
	 * from - 駒の位置
	 * king - 玉の位置
	 * ptypeO - 駒の種類，白からの利きの場合は反転．
	 *          (BLACK,PTYPE_EDGE)もあり得る
	 * isP - 馬の場合
	 */
	static int index1(Square king,Square from,PtypeO ptypeO,bool isP)
	{
	  int y_diff=from.y()-king.y();
	  int x_diff=from.x()-king.x();
	  return index1(x_diff,y_diff,ptypeO,isP);
	}
	/**
	 * 黒の角(馬)からの利きのある駒のindex
	 * {attack,defense}_{ul,ur,dl,dr} へのアクセスに使う
	 * (x_diff, y_diff) - 玉を基準に見た駒の相対位置
	 * ptypeO - 駒の種類，白からの利きの場合は反転．
	 *          (BLACK,PTYPE_EDGE)もあり得る
	 * isP - 馬の場合
	 */
	static int index1(int x_diff,int y_diff,PtypeO ptypeO,bool isP){
	  assert(-9 <= y_diff && y_diff <= 9);
	  assert(-9 <= x_diff && x_diff <= 9);
	  assert(getPtype((PtypeO)ptypeO)!=PTYPE_EMPTY);
	  int index=(ptypeO-PTYPEO_MIN)+32*((y_diff+9)+19*(x_diff+9+19*(isP ? 1 : 0)));
	  assert(0<=index && index<32*19*19*2);
	  return index;
	}
	/**
	 * 黒の角(馬)がある場所は空マスでないのでその分を補正するテーブル
	 * {attack,defense}_nospace へのアクセス
	 * king - 玉の位置
	 * from - 角(馬)の位置
	 * isP - 馬の場合
	 */
        static int index2(Square king,Square from,bool isP)
	{
	  int y_diff=from.y()-king.y();
	  int x_diff=from.x()-king.x();
	  return index2(x_diff,y_diff,isP);
	}
	/**
	 * 黒の角(馬)がある場所は空マスでないのでその分を補正するテーブル
	 * {attack,defense}_nospace へのアクセス
	 * (x_diff, y_diff)  - 玉を基準にしてみた角(馬)の相対位置
	 * isP - 馬の場合
	 */
	static int index2(int x_diff,int y_diff,bool isP){
	  assert(-9 <= y_diff && y_diff <= 9);
	  assert(-9 <= x_diff && x_diff <= 9);
	  int index=(y_diff+9)+19*(x_diff+9+19*(isP ? 1 : 0));
	  assert(0<=index && index<19*19*2);
	  return index;
	}
	static CArray<MultiInt, 612> attack_table;
	static CArray<MultiInt, 612> defense_table;
	static CArray<MultiInt, 32> piece_table;
	static CArray<MultiInt, 23104> attack_ur;
	static CArray<MultiInt, 23104> attack_ul;
	static CArray<MultiInt, 23104> attack_dr;
	static CArray<MultiInt, 23104> attack_dl;
	static CArray<MultiInt, 23104> defense_ur;
	static CArray<MultiInt, 23104> defense_ul;
	static CArray<MultiInt, 23104> defense_dr;
	static CArray<MultiInt, 23104> defense_dl;
	static CArray<MultiInt, 722> attack_nospace;
	static CArray<MultiInt, 722> defense_nospace;
      };
      class BishopEffect : public BishopEffectBase
      {
      public:
	static void setUp(const Weights &weights,int stage);
      };
      class BishopEffectPiece
      {
      public:
	enum { DIM = 32*EvalStages };
	static void setUp(const Weights &weights);
      };

      class BishopEffectPieceKingRelative : BishopEffectBase
      {
      public:
	enum { ONE_DIM = 19584, DIM = ONE_DIM * 2*EvalStages };
	static void setUp(const Weights & weights);
      };

      class BishopHead
      {
	friend class BishopHeadKingRelative;
	friend class BishopHeadX;
      public:
	enum { ONE_DIM = 32, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static int indexK(Player player, PtypeO ptypeO, int x_diff, int y_diff)
	{
	  if (player == WHITE)
	    {
	      ptypeO=(PtypeO)(static_cast<int>(ptypeO)^(~15));
	    }
	  if (player == WHITE)
	    {
	      y_diff = -y_diff;
	    }
	  return (ptypeOIndex(ptypeO) * 9 + x_diff) * 17 + y_diff + 8;
	}
	template <Player P>
	static int indexX(PtypeO ptypeO, int x)
	{
	  if (x > 5)
	  {
	    x = 10 - x;
	  }
	  if (P == WHITE)
	  {
	    ptypeO = altIfPiece(ptypeO);
	  }
	  return x - 1 + 5 * ptypeOIndex(ptypeO);
	}
	static CArray<MultiInt, 32> table;
	static CArray<MultiInt, 4896> king_table;
	static CArray<MultiInt, 160> x_table;
      };

      class BishopHeadKingRelative
      {
      public:
	enum { ONE_DIM = 4896, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class BishopHeadX
      {
      public:
	enum { ONE_DIM = 160, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class KingRookBishop
      {
      public:
	enum { ONE_DIM = 374544, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	template<Player King>
	static MultiInt evalOne(const NumEffectState &state);
	static MultiInt eval(const NumEffectState &state);
      private:
	static CArray<MultiInt, 374544> table;
	template <Player King>
	static int index(const Square king, const Piece rook, const Piece bishop)
	{
	  const int rook_x = std::abs(king.x() - rook.square().x());
	  const int bishop_x = std::abs(king.x() - bishop.square().x());
	  const int rook_y = (King == BLACK ? rook.square().y() - king.y() : king.y() - rook.square().y());
	  const int bishop_y = (King == BLACK ? bishop.square().y() - king.y() : king.y() - bishop.square().y());
	  return bishop_y + 8 + 17 * (bishop_x + 9 * (rook_y + 8 + 17 * (rook_x + 9 * ((bishop.owner() == King ? 1 : 0) + 2 * ((rook.owner() == King ? 1 : 0) + 2 * (2 * (bishop.isPromoted() ? 1 : 0) + (rook.isPromoted() ? 1 : 0)))))));
	}
      };

      class NumPiecesBetweenBishopAndKing
      {
	friend class NumPiecesBetweenBishopAndKingSelf;
	friend class NumPiecesBetweenBishopAndKingOpp;
	friend class NumPiecesBetweenBishopAndKingAll;
      public:
	static MultiInt eval(const NumEffectState &state);
      private:
	static void countBetween(const NumEffectState &state,
				 Square king, Piece bishop,
				 int &self_count, int &opp_count,
				 int &total_count);
	static CArray<MultiInt, 9> self_table;
	static CArray<MultiInt, 9> opp_table;
	static CArray<MultiInt, 9> all_table;
      };
      class NumPiecesBetweenBishopAndKingSelf
      {
      public:
	enum { ONE_DIM = 9, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class NumPiecesBetweenBishopAndKingOpp
      {
      public:
	enum { ONE_DIM = 9, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class NumPiecesBetweenBishopAndKingAll
      {
      public:
	enum { ONE_DIM = 9, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class BishopBishopPiece
      {
      public:
	enum { ONE_DIM = 64, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static int index(Ptype ptype, bool self_with_support,
			 bool opp_with_support)
	{
	  return ptype + PTYPE_SIZE * ((self_with_support ? 1 : 0) +
				       2 * (opp_with_support ? 1 : 0));
	}
	static CArray<MultiInt, 64> table;
      };

      class RookRook
      {
      public:
	enum { ONE_DIM = 800, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	template <bool SamePlayer, Player P>
	static int index(Piece rook1, Piece rook2)
	{
	  const int y1 = (rook1.isOnBoard() ? rook1.square().y() : 0);
	  const int y2 = (rook2.isOnBoard() ? rook2.square().y() : 0);
	  if (SamePlayer)
	  {
	    if (P == BLACK)
	    {
	      return y1 + 10 *
		(y2 + 10 * ((rook1.isPromoted() ? 1 : 0) + 2 *
			    ((rook2.isPromoted() ? 1 : 0) + 2 *
			     (SamePlayer ? 1 : 0))));
	    }
	    else
	    {
	      if (y1 == 0 || y2 == 0 || y1 == y2)
	      {
		return (10 - y1) % 10 + 10 *
		  ((10 - y2) % 10 + 10 * ((rook1.isPromoted() ? 1 : 0) + 2 *
					  ((rook2.isPromoted() ? 1 : 0) + 2 *
					   (SamePlayer ? 1 : 0))));
	      }
	      else
	      {
		return (10 - y2) % 10 + 10 *
		  ((10 - y1) % 10 + 10 * ((rook2.isPromoted() ? 1 : 0) + 2 *
					  ((rook1.isPromoted() ? 1 : 0) + 2 *
					   (SamePlayer ? 1 : 0))));
	      }
	    }
	  }
	  else
	  {
	    return y1 + 10 *
	      (y2 + 10 * ((rook1.isPromoted() ? 1 : 0) + 2 *
			  ((rook2.isPromoted() ? 1 : 0) + 2 *
			   (SamePlayer ? 1 : 0))));
	  }
	}
	static int index(bool same_player, bool promoted1,
			 bool promoted2, int y1, int y2)
	{
	  return y1 + 10 *
	    (y2 + 10 * ((promoted1 ? 1 : 0) + 2 *
			((promoted2 ? 1 : 0) + 2 *
			 (same_player ? 1 : 0))));
	}
	static CArray<MultiInt, 800> table;
      };

      class RookRookPiece
      {
      public:
	enum { ONE_DIM = 128, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static int index(Ptype ptype, bool self_with_support,
			 bool opp_with_support, bool vertical)
	{
	  return ptype + PTYPE_SIZE * ((self_with_support ? 1 : 0) +
				       2 * (opp_with_support ? 1 : 0)) +
	    (vertical ? PTYPE_SIZE * 2 * 2 : 0);
	}
	static CArray<MultiInt, 128> table;
      };

      class BishopStandFile5
      {
      public:
	enum { ONE_DIM = 32, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static CArray<MultiInt, 32> table;
      };

      class MajorCheckWithCapture
      {
      public:
	enum {
	  ONE_DIM = PTYPE_SIZE * 2/*bishop or rook*/ * 2 /*promotable*/,
	  DIM = ONE_DIM * EvalStages
	};
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static CArray<MultiInt, ONE_DIM> table;	
	template <Player Owner>
	static MultiInt addOne(const NumEffectState &state);
	static size_t index(Ptype ptype, bool is_rook, bool can_promote) 
	{
	  return ptype * 4 + is_rook * 2 + can_promote;
	}
      };

      class RookSilverKnight
      {
      public:
	enum {
	  ONE_DIM = 5 * 9 * 9 * 9 * 9 * 9,
	  DIM = ONE_DIM * EvalStages
	};
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static CArray<MultiInt, ONE_DIM> table;	
	static size_t index(int rook_x, int rook_y, int silver_x, int silver_y,
			    int knight_x, int knight_y)
	{
	  return knight_y + 9 * (knight_x + 9 * (silver_y + 9 * (silver_x + 9 * (rook_y + 9 * rook_x))));
	}
      };

      class BishopSilverKnight
      {
      public:
	enum {
	  ONE_DIM = 5 * 9 * 9 * 9 * 9 * 9,
	  DIM = ONE_DIM * EvalStages
	};
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static CArray<MultiInt, ONE_DIM> table;	
	static size_t index(int bishop_x, int bishop_y, int silver_x, int silver_y,
			    int knight_x, int knight_y)
	{
	  return knight_y + 9 * (knight_x + 9 * (silver_y + 9 * (silver_x + 9 * (bishop_y + 9 * bishop_x))));
	}
      };

      class AttackMajorsInBase
      {
      public:
	enum {
	  ONE_DIM = PTYPE_SIZE * PTYPE_SIZE * 2 * 2 * 2,
	  DIM = ONE_DIM * EvalStages
	};
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static CArray<MultiInt, ONE_DIM> table;	
	size_t maxActive() const { return 4; }
	static int index(Ptype support, Ptype attack, bool has_gold, 
			 bool rook_support, bool bishop_support)
	{
	  return (unpromoteSafe(support)*16 + unpromoteSafe(attack))*8+has_gold*4
	    +rook_support*2+bishop_support;
	}
	template <Player Owner>
	static void addOne(const NumEffectState &state, Piece rook, MultiInt&);
      };
    }
  }
}

#endif // EVAL_ML_MAJORPIECE_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
