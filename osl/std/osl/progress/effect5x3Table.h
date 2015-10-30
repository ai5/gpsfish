/* effect5x3Table.h
 */
#ifndef _PROGRESS_EFFECT5X3_TABLE_H
#define _PROGRESS_EFFECT5X3_TABLE_H

#include "osl/basic_type.h"
#include "osl/container.h"
#include "osl/bits/offset32.h"
#include <iosfwd>

namespace osl
{
  namespace progress
  {
    /**
     * 5x3領域への長い利きの始まりと終わり．
     * offset - 利きの方向が0の時は利きが無いことを示す
     * minIndex - 利きが領域に入るindex
     * maxIndex - 利きが領域から出る手前のindex
     */
    struct LongEffect
    {
      Offset offset;
      unsigned short minIndex;
      unsigned short maxIndex;
    };
    /**
     * 長い利きの方向は高々4なので，長さ4のCArrayで表現．
     */
    typedef CArray<LongEffect,4> LongEffect4;

    /**
     * Effect5x3の差分計算で使うテーブル.
     */
    class Effect5x3Table
    {
    public:
      enum {
	StandPAWN=1,
	StandLANCE=4,
	StandKNIGHT=8,
	StandSILVER=8,
	StandGOLD=8,
	StandBISHOP=12,
	StandROOK=12,
      };
    private:
      CArray<unsigned int,PTYPE_SIZE> onStand;
      CArray2d<unsigned int,PTYPEO_SIZE,Offset32::SIZE> shortEffect;
      CArray2d<LongEffect4,PTYPEO_SIZE,Offset32::SIZE> longEffect;
      CArray2d<LongEffect,8,Offset32::SIZE> blockEffect;
      CArray3d<unsigned int,2,Square::SIZE,5*3> attackEffect;
      CArray3d<unsigned int,2,Square::SIZE,5*3> defenseEffect;
      void setupOnStand();
      void setupShortEffect();
      void setupLongEffect();
      void setupBlockEffect();
      void setupAttackEffect();
      void setupDefenseEffect();
    public:
      void init();
      /**
       * 持駒のPtypeごとの寄与を表すテーブルの参照.
       * 以下の重みで計算
       * PAWN 1
       * LANCE 4
       * KNIGHT,SILVER,GOLD 8
       * BISHOP,ROOK 12
       * @param ptype - 持駒のptype
       */
      unsigned int piecesOnStand(Ptype ptype) const
      {
	return onStand[ptype];
      }
      /**
       * 5x3領域への短い利きの数の計算.
       * ただし8倍したものを返す
       * @param ptypeO - 利きをつけようとする駒のptypeO
       * @param o32 - (to - from)のOffset32版
       */
      unsigned int countShortEffect(PtypeO ptypeO,Offset32 offset32) const
      {
	return shortEffect[ptypeO-PTYPEO_MIN][offset32.index()];
      }
      /**
       * 5x3領域へのPtypeOを限定した長い利きの計算.
       * ただし8倍したものを返す
       * @param ptypeO - 利きをつけようとする駒のptypeO
       * @param o32 - (to - from)のOffset32版
       */
      LongEffect4 const& getLongEffect(PtypeO ptypeO,Offset32 offset32) const
      {
	return longEffect[ptypeO-PTYPEO_MIN][offset32.index()];
      }
      /**
       * 5x3領域へのdirectionを限定した長い利きの計算.
       * @param d - 長い利きの方向
       * @param o32 - (to - from)のOffset32版
       */
      LongEffect const& getBlockEffect(Direction d,Offset32 offset32) const
      {
	assert(d<8);
	return blockEffect[d][offset32.index()];
      }
      unsigned int getAttackEffect(Player pl,Square pos,int x,int y) const
      {
	assert(pos.isOnBoard() && 0<=x && x<5 && 0<= y && y<3);
	return attackEffect[pl][pos.index()][x*3+y];
      }
      unsigned int getDefenseEffect(Player pl,Square pos,int x,int y) const
      {
	assert(pos.isOnBoard() && 0<=x && x<5 && 0<= y && y<3);
	return defenseEffect[pl][pos.index()][x*3+y];
      }
    };
    extern Effect5x3Table Effect5x3_Table;
    std::ostream& operator<<(std::ostream& os,LongEffect const& longEffect);
  }
}
#endif /* _PROGRESS_EFFECT5X3_TABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
