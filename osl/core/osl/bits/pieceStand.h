/* pieceStand.h
 */
#ifndef OSL_PIECESTAND_H
#define OSL_PIECESTAND_H
#include "osl/basic_type.h"
#include "osl/container.h"
#include <iosfwd>
#include <cassert>
namespace osl
{
  class SimpleState;
  /**
   * 片方の手番の持駒の枚数を記録するクラス.
   * - bit field を使うべきか迷う
   * - 一応 king を持駒にして良いことにしておく
   * レイアウト 長さ:index
   * -  reserved : 1;31
   * -  carry    : 1;
   * -  KING     : 2;28
   * -  carry    : 1;
   * -  GOLD     : 3;24
   * -  carry    : 1;
   * -  PAWN     : 5;18
   * -  carry    : 1;
   * -  LANCE    : 3;14
   * -  carry    : 1;
   * -  KNIGHT   : 3;10
   * -  carry    : 1;
   * -  SILVER   : 3; 6
   * -  carry    : 1;
   * -  BISHOP   : 2; 3
   * -  carry    : 1; 
   * -  ROOK     : 2; 0
   *
   * == を軽くするために carry off の状態を基本とする
   */
  class PieceStand
  {
  public:
    /** 持駒の表示で良く使われる順番. KINGや成駒は -1 */
    static const CArray<Ptype,7> order;
    static const unsigned int carryMask = 0x48822224;
  private:
    static const CArray<unsigned char,PTYPE_MAX+1> shift;
    static const CArray<unsigned char,PTYPE_MAX+1> mask;
    mutable unsigned int flags;
  public:
    explicit PieceStand(unsigned int value=0) : flags(value)
    {
    }
    explicit PieceStand(Player, const SimpleState&);
    PieceStand(int pawnCount, int lanceCount, 
	       int knightCount, int silverCount,
	       int goldCount, int bishopCount,
	       int rookCount, int kingCount) 
      : flags(0)
    {
      add(PAWN, pawnCount);
      add(LANCE, lanceCount);
      add(KNIGHT, knightCount);
      add(SILVER, silverCount);
      add(GOLD, goldCount);
      add(BISHOP, bishopCount);
      add(ROOK, rookCount);
      add(KING, kingCount);
    }

    void add(Ptype type, unsigned int num=1)
    {
      assert(isBasic(type));
      assert(num == (num & mask[type]));
      flags += (num << (shift[type]));
      assert(testCarries() == 0);	// overflow 検出
    }    
    void sub(Ptype type, unsigned int num=1)
    {
      assert(isBasic(type));
      assert(num == (num & mask[type]));
      assert(get(type) >= num);
      flags -= (num << (shift[type]));
    }

    /**
     * 加算可能なら加える.
     * 速度が必要なところでは使ってないので .cc に移動．
     */
    void tryAdd(Ptype type);
    bool canAdd(Ptype type) const;
    /**
     * 1枚以上持っていれば減らす
     */
    void trySub(Ptype type)
    {
      if (get(type))
	sub(type);
    }

    /**
     * 一種類の駒しかない
     */
    bool atMostOneKind() const;

    /**
     * pieceStand同士の加算，減算.
     * 足して良いのは，carry が立っていないpiecestandで
     * かつ，含まれる駒が高々1つ
     */
    void addAtmostOnePiece(PieceStand const& ps){
#ifndef NDEBUG
      const PieceStand copy(*this);
#endif
      assert(! ps.testCarries());
      assert(ps.atMostOneKind());
      flags += ps.getFlags();
      assert(carryUnchangedAfterAdd(copy, ps));
    }

    void subAtmostOnePiece(PieceStand const& ps){
#ifndef NDEBUG
      const PieceStand copy(*this);
#endif
      assert(! ps.testCarries());
      assert(ps.atMostOneKind());
      flags -= ps.getFlags();
      assert(carryUnchangedAfterSub(copy, ps));
    }
  private:
    bool carryUnchangedAfterAdd(const PieceStand& original, const PieceStand& other) const;
    bool carryUnchangedAfterSub(const PieceStand& original, const PieceStand& other) const;
  public:
    unsigned int get(Ptype type) const
    {
      return (flags >> (shift[type])) & mask[type];
    }
    void carriesOff() const { flags &= (~carryMask); }
    void carriesOn()  const { flags |= carryMask; }
    unsigned int testCarries() const { return (flags & carryMask); }
    bool isSuperiorOrEqualTo(PieceStand other) const
    {
      carriesOn();
      other.carriesOff();
      const bool result = (((flags - other.flags) & carryMask) == carryMask);
      carriesOff();
      return result;
    }
    /**
     * this と other が BLACK の持駒と考えた時に，
     * this の方が同じか沢山持っていれば真.
     */
    template <Player P>
    bool hasMoreThan(PieceStand other) const
    {
      if (P == BLACK)
	return isSuperiorOrEqualTo(other);
      else
	return other.isSuperiorOrEqualTo(*this);
    }
    bool hasMoreThan(Player P, PieceStand other) const
    {
      if (P == BLACK)
	return hasMoreThan<BLACK>(other);
      else
	return hasMoreThan<WHITE>(other);
    }
    unsigned int getFlags() const { return flags; }
    /** どれかの駒を一枚でも持っている */
    bool any() const { return flags; }
    /**
     * 種類毎に this と other の持駒の多い方を取る
     */
    const PieceStand max(PieceStand other) const
    {
      // other以上の数持っているptypeに対応するcarryが1になる．
      const unsigned int mask0 = ((flags|carryMask)-other.flags) & carryMask;
      // ROOK BISHOP KING用のMASKを作る
      unsigned int my_mask = mask0-((mask0&0x40000024)>>2);
      // GOLD SILVER KNIGHT LANCE用のMASKを作る
      my_mask -= (mask0&0x08022200)>>3;
      // PAWN用のMASKのみ残す
      my_mask -= (mask0&0x00800000)>>5;
      // my_mask が1のptypeの数は自分から，0のptypeはotherのところの値を
      return PieceStand((flags&my_mask)|(other.flags&~my_mask));
    }     
    /**
     * 種類毎に this と other の持駒の多い方を取る (max のalternative)
     */
    const PieceStand max2(PieceStand other) const
    {
      // other以上の数持っているptypeに対応するcarryが1になる．
      const unsigned int diff0=((flags|carryMask)-other.flags);
      const unsigned int mask0=diff0&carryMask;

      // ROOK BISHOP KING GOLD SILVER KNIGHT LANCE用のMASKを作る
      const unsigned int mask02=(mask0&0x40000024u)+(mask0&0x48022224u);
      unsigned int my_mask=mask0-(mask02>>3);

      // PAWN用のMASKのみ残す
      my_mask -= (mask0&0x00800000)>>5;
      // my_mask が1のptypeの数は自分から，0のptypeはotherのところの値を
      return PieceStand((other.flags+(diff0&my_mask))&~carryMask);
    }     

    const PieceStand nextStand(Player pl, Move move) const
    {
      assert(move.isNormal());
      PieceStand result = *this;
      if (move.player() == pl)
      {
	if (const Ptype ptype = move.capturePtype())
	{
	  result.add(unpromote(ptype));
	}
	else if (move.isDrop())
	{
	  const Ptype ptype = move.ptype();
	  assert(get(ptype));
	  result.sub(ptype);
	}
      }
      return result;
    }
    const PieceStand nextStand(Move move) const
    {
      return nextStand(move.player(), move);
    }
    const PieceStand previousStand(Player pl, Move move) const
    {
      assert(move.isNormal());
      PieceStand result = *this;
      if (move.player() == pl)
      {
	if (const Ptype ptype = move.capturePtype())
	{
	  const Ptype before = unpromote(ptype);
	  assert(get(before));
	  result.sub(before);
	}
	else if (move.isDrop())
	{
	  const Ptype ptype = move.ptype();
	  result.add(ptype);
	}
      }
      return result;
    }
    const PieceStand previousStand(Move move) const
    {
      return previousStand(move.player(), move);
    }
  };

  inline bool operator==(PieceStand l, PieceStand r)
  {
    assert(! l.testCarries());
    assert(! r.testCarries());
    return l.getFlags() == r.getFlags();
  }
  inline bool operator!=(PieceStand l, PieceStand r)
  {
    return ! (l == r);
  }
  inline bool operator<(PieceStand l, PieceStand r)
  {
    assert(! l.testCarries());
    assert(! r.testCarries());
    return l.getFlags() < r.getFlags();
  }
  std::ostream& operator<<(std::ostream&, PieceStand l);

  struct PieceStandIO
  {
    /**
     * 持駒の数を空白区切で出力する. 数値処理用途
     */
    static std::ostream& writeNumbers(std::ostream&, const PieceStand& stand);
    static std::istream& readNumbers(std::istream&, PieceStand& stand);
  };
} // namespace osl

#endif /* OSL_PIECESTAND_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
