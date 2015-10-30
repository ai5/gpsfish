/* directionTable.h
 */
#ifndef OSL_BOARD_TABLE_H
#define OSL_BOARD_TABLE_H

#include "osl/basic_type.h"
#include "osl/container.h"
#include "osl/bits/offset32.h"

namespace osl
{
  class BoardTable
  {
    CArray<Direction,Offset32::SIZE> directions;
    // 上位はoffsetが，下位3bitはdirectionが入る．　
    CArray<signed char,Offset::ONBOARD_OFFSET_SIZE> short8Offset;
    CArray<unsigned char,Offset::ONBOARD_OFFSET_SIZE> short8Dir;
    CArray<Offset, Offset32::SIZE> short_offsets;
    CArray<Offset, Offset32::SIZE> short_offsets_not_knight;
#ifndef MINIMAL
    CArray<int,Offset32Wide::SIZE> space_counts;
#endif
  public:
    static const CArray<Offset, DIRECTION_SIZE> offsets;
    static const CArray<int, DIRECTION_SIZE> dxs;
    static const CArray<int, DIRECTION_SIZE> dys;
  private:
    template<Direction Dir>
    void setDirections();
    template<Direction Dir>
    void setKnightDirections();
    void init();
  public:
    /**
     * 黒にとってのoffsetを返す
     */
    const Offset getOffsetForBlack(Direction dir) const{
      return offsets[static_cast<int>(dir)];
    }
    int getDxForBlack(Direction dir) const{
      return dxs[static_cast<int>(dir)];
    }
    int getDyForBlack(Direction dir) const{
      return dys[static_cast<int>(dir)];
    }
    template<Player P>
    const Offset getOffset(Direction dir) const{
      return getOffsetForBlack(dir)*sign(P);
    }
    const Offset getOffset(Player pl,Direction dir) const{
      if (pl==BLACK)
	return getOffset<BLACK>(dir);
      else
	return getOffset<WHITE>(dir);
    }

    /**
     * next position from pos for player P.
     * 答えが isOnBoard とは限らない
     */
    const Square nextSquare(Player P, Square pos, Direction dr) const
    {
      assert(pos.isOnBoard());
      const Offset offset = getOffset(P, dr);
      return pos + offset;
    }

    BoardTable();
    /** @param P どちらのPlayerにとっての方向かを指定 */
    template <Player P>
    Direction getLongDirection(Offset32 offset32) const
    {
      assert(offset32.isValid());
      const Offset32 blackOffset32 = offset32.blackOffset32<P>();
      Direction ret=directions[blackOffset32.index()];
      assert(isValid(ret));
      return ret;
    }
    Direction getLongDirection(Player P, Offset32 offset32) const
    {
      if (P == BLACK) 
	return getLongDirection<BLACK>(offset32);
      else
	return getLongDirection<WHITE>(offset32);
    }
    /** @param P どちらのPlayerにとっての方向かを指定 */
    template <Player P>
    Direction getLongDirection(Square from, Square to) const
    {
      return getLongDirection<P>(Offset32(to,from));
    }
#ifndef MINIMAL
    /**
     * fromとtoが長い利きを持つ位置にある時，間のマスの数を求める
     * 一致している時は0
     * となりも0
     * 関係ない時は-1
     */
    int spaceCounts(Square from,Square to) const
    {
      Offset32Wide offset32(from,to);
      return space_counts[offset32.index()];
    }
#endif
    /**
     * Longの利きの可能性のあるoffsetの場合は, 反復に使う offsetを
     * Shortの利きのoffsetの場合はそれ自身を返す.
     * 利きの可能性のない場合は0を返す
     */
    const Offset getShortOffset(Offset32 offset32) const{
      assert(offset32.isValid());
      return short_offsets[offset32.index()];
    }
    /**
     * Longの利きの可能性のあるoffsetの場合は, 反復に使う offsetを
     * Knight以外のShortの利きのoffsetの場合はそれ自身を返す.
     * Knightの利き, 利きの可能性のない場合は0を返す
     */
    const Offset getShortOffsetNotKnight(Offset32 offset32) const{
      assert(offset32.isValid());
      return short_offsets_not_knight[offset32.index()];
    }
    /**
     * 8方向にいない場合も適当なものを返す．
     */
    Offset getShort8OffsetUnsafe(Square from,Square to) const{
      int i=(int)(to.uintValue())-(int)(from.uintValue())-Offset::ONBOARD_OFFSET_MIN;
      return Offset::makeDirect(short8Offset[i]);
    }
    /**
     * 8方向にいない場合も適当なものを返す．
     */
    template<Player P>
    Direction getShort8Unsafe(Square from,Square to) const{
      if(P==BLACK)
	return static_cast<Direction>(short8Dir[(int)(to.uintValue())-(int)(from.uintValue())-Offset::ONBOARD_OFFSET_MIN]);
      else
	return static_cast<Direction>(short8Dir[(int)(from.uintValue())-(int)(to.uintValue())-Offset::ONBOARD_OFFSET_MIN]);
    }
    Direction getShort8Unsafe(Player P, Square from,Square to) const{
      if(P==BLACK)
	return getShort8Unsafe<BLACK>(from, to);
      else
	return getShort8Unsafe<WHITE>(from, to);
    }
    template<Player P>
    Direction getShort8(Square from,Square to) const{
      assert(from.isOnBoard() && to.isOnBoard());
      assert(from.x()==to.x() || from.y()==to.y() || 
	     abs(from.x()-to.x())==abs(from.y()-to.y()));
      return getShort8Unsafe<P>(from,to);
    }

    template<Player P>
    Direction getShort8(Square from,Square to,Offset& o) const{
      assert(from.isOnBoard() && to.isOnBoard());
      assert(from.x()==to.x() || from.y()==to.y() || 
	     abs(from.x()-to.x())==abs(from.y()-to.y()));
      int i=(int)(to.uintValue())-(int)(from.uintValue())-Offset::ONBOARD_OFFSET_MIN;
      o=Offset::makeDirect(short8Offset[i]);
      Direction d=static_cast<Direction>(short8Dir[i]);
      if(P==BLACK)
	return d;
      else
	return inverse(d);
    }
    /**
     * p0, p1の間にtがあるかどうか.
     * (t,p0), (t,p1)のどちらかが8方向である時にのみ呼び出すこと
     * 他方も8方向でないとしたらknightの関係
     */
    bool isBetween(Square t,Square p0,Square p1) const
    {
      int i1=(int)(t.uintValue())-(int)(p0.uintValue())-Offset::ONBOARD_OFFSET_MIN;
      int i2=(int)(p1.uintValue())-(int)(t.uintValue())-Offset::ONBOARD_OFFSET_MIN;
      assert(short8Dir[i1]!=DIRECTION_INVALID_VALUE || short8Dir[i2]!=DIRECTION_INVALID_VALUE);
      return short8Dir[i1]==short8Dir[i2];
    }
    bool isBetweenSafe(Square t,Square p0,Square p1) const
    {
      if (getShortOffsetNotKnight(Offset32(t, p0)).zero())
	return false;
      return isBetween(t, p0, p1);
    }
  };

  extern const BoardTable Board_Table;
} // namespace osl


#endif /* OSL_BOARD_TABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
