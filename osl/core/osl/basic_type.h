#ifndef OSL_BASIC_TYPE_H
#define OSL_BASIC_TYPE_H
#include "osl/config.h"
#include <type_traits>
#include <cassert>
#include <iosfwd>
namespace osl{
  enum Player{
    BLACK=0,
    WHITE= -1
  };
  
  constexpr Player alt(Player player){
    return static_cast<Player>(-1-static_cast<int>(player));
  }
  constexpr int playerToIndex(Player player){
    return -static_cast<int>(player);
  }
  constexpr Player indexToPlayer(int n) {
    // assert(n == 0 || n == 1)
    return static_cast<Player>(-n);
  }
  constexpr int sign(Player player){
    // int ret=1+(static_cast<int>(player)<<1);
    // assert(ret==1 || ret== -1);
    return 1+(static_cast<int>(player)<<1);
  }
  constexpr int playerToMask(Player player){
    return static_cast<int>(player);
  }

  // These codes are intentionally DECLARED and NOT IMPLEMENTED.
  // you will get link error here if you write code such as "value += v * piece.owner() == BLACK ? 1.0 : -1.0;"
  int operator+(Player, int);	int operator+(int, Player);	
  int operator-(Player, int);	int operator-(int, Player);	
  int operator*(Player, int);	int operator*(int, Player);	
  int operator/(Player, int);	int operator/(int, Player);	
  
  /**
   * cast等で作られたplayerが正しいかどうかを返す
   */
  bool isValid(Player player);
#if 0    
  template<Player P>
  struct PlayerTraits;
  
  template<>
  struct PlayerTraits<BLACK>{
    static const int offsetMul=1;
    static const int index=0;
    static const int mask=0;
    static const Player opponent=WHITE;
  };
  
  template<>
  struct PlayerTraits<WHITE>{
    static const int offsetMul=-1;
    static const int index=1;
    static const int mask= -1;
    static const Player opponent=BLACK;
  };
#endif
  std::ostream& operator<<(std::ostream& os,Player player);

  namespace misc
  {
// Int2Type by LOKI
    template<int v>
    struct Int2Type{ enum { value=v }; }; 

    template<typename T>
    struct Type2Type{}; 

    template<Player P>
    struct Player2Type{ enum { value=P }; }; 

    struct EmptyType{};
  } // namespace misc
  using misc::Int2Type;
  using misc::Player2Type;

  /** 駒の種類を4ビットでコード化する */
  enum Ptype
  {
    PTYPE_EMPTY=0,
    PTYPE_EDGE=1,
    PPAWN=2,
    PLANCE=3,
    PKNIGHT=4,
    PSILVER=5,
    PBISHOP=6,
    PROOK=7,
    KING=8,
    GOLD=9,
    PAWN=10,
    LANCE=11,
    KNIGHT=12,
    SILVER=13,
    BISHOP=14,
    ROOK=15,

    PTYPE_MIN=0,
    PTYPE_BASIC_MIN=KING,
    PTYPE_PIECE_MIN=2,
    PTYPE_MAX=15,
  };
  const int PTYPE_SIZE=PTYPE_MAX-PTYPE_MIN+1;
  
  std::istream& operator>>(std::istream& is, Ptype& ptype);
  std::ostream& operator<<(std::ostream& os,const Ptype ptype);
  
  /**
   * int等からcastして作ったptypeが，正しい範囲に入っているかどうかのチェック
   */
  bool isValid(Ptype ptype);

  /**
   * ptypeが空白やEDGEでないかのチェック
   */
  constexpr bool isPiece(Ptype ptype)
  {
    // assert(isValid(ptype));
    return static_cast<int>(ptype)>=PTYPE_PIECE_MIN;
  }
  /**
   * ptypeが基本型(promoteしていない)かのチェック
   */
  inline bool isBasic(Ptype ptype)
  {
    assert(isValid(ptype));
    return static_cast<int>(ptype)>PROOK;
  }
  
  /**
   * ptypeがpromote後の型かどうかのチェック
   */
  inline bool isPromoted(Ptype ptype)
  {
    assert(isPiece(ptype));
    return static_cast<int>(ptype)<KING; 
  }

  /**
   * ptypeがpromote可能な型かどうかのチェック
   * promote済みの場合はfalseを返す
   */
  inline bool canPromote(Ptype ptype)
  {
    assert(isPiece(ptype));
    return static_cast<int>(ptype)>GOLD; 
  }
  
  /** 
   * ptypeがpromote後の型の時に，promote前の型を返す．
   * promoteしていない型の時はそのまま返す
   */
  inline Ptype unpromote(Ptype ptype)
  {
    assert(isPiece(ptype));
    Ptype ret=static_cast<Ptype>(static_cast<int>(ptype)|8); 
    assert(isPiece(ret));
    return ret;
  }
  constexpr Ptype unpromoteSafe(Ptype ptype)
  {
    return (! isPiece(ptype)) ? ptype : unpromote(ptype);
  }
  
  /** 
   * promote可能なptypeに対して，promote後の型を返す
   * promote不可のptypeを与えてはいけない．
   */
  inline Ptype promote(Ptype ptype)
  {
    assert(canPromote(ptype));
    Ptype ret=static_cast<Ptype>(static_cast<int>(ptype)-8); 
    assert(isPiece(ret));
    return ret;
  }

  inline bool isMajorBasic(Ptype ptype)
  {
    return ptype >= 14;
  }
  inline bool isMajor(Ptype ptype)
  {
    assert(isPiece(ptype));
    return isMajorBasic(unpromote(ptype));
  }
  inline bool isMajorNonPieceOK(Ptype ptype)
  {
    return (static_cast<int>(ptype)|8)>=14;
  }
  
  /**
   * Player + Ptype [-15, 15] 
   * PtypeO の O は Owner の O
   */
  enum PtypeO {
    PTYPEO_MIN= PTYPE_EMPTY-16,
    PTYPEO_MAX= 15,
  };
  
#define NEW_PTYPEO(player,ptype) static_cast<PtypeO>(static_cast<int>(ptype)-(16&static_cast<int>(player)))
  inline unsigned int ptypeOIndex(PtypeO ptypeo)
  {
    const int result = ptypeo - PTYPEO_MIN;
    assert(result >= 0);
    return result;
  }
  inline PtypeO newPtypeO(Player player,Ptype ptype)
  {
    return static_cast<PtypeO>(static_cast<int>(ptype)-(16&static_cast<int>(player)));
  }
  
  
  inline Ptype getPtype(PtypeO ptypeO)
  {
    return static_cast<Ptype>(static_cast<int>(ptypeO)& 15);
  }
  
  /** pieceをpromoteさせる. promote不可のptypeを与えてはいけない．*/
  inline PtypeO promote(PtypeO ptypeO)
  {
    assert(canPromote(getPtype(ptypeO)));
    PtypeO ret=static_cast<PtypeO>(static_cast<int>(ptypeO)-8); 
    assert(isPiece(getPtype(ret)));
    return ret;
  }
  
  /** pieceを引数次第でpromoteさせる */
  inline PtypeO promoteWithMask(PtypeO ptypeO,int promoteMask)
  {
    assert(promoteMask==0 || promoteMask==0x800000);
    PtypeO ret=static_cast<PtypeO>(static_cast<int>(ptypeO)-(promoteMask>>20)); 
    return ret;
  }
  
  /** pieceをunpromoteさせる.  promoteしていないptypeを与えてもよい */
  inline PtypeO unpromote(PtypeO ptypeO)
  {
    return static_cast<PtypeO>(static_cast<int>(ptypeO)|8); 
  }

  bool isValidPtypeO(int ptypeO);
  
  /**
   * EMPTY, EDGEではない
   */
  inline bool isPiece(PtypeO ptypeO)
  {
    assert(isValidPtypeO(ptypeO));
    return isPiece(getPtype(ptypeO));
  }

  inline Player getOwner(PtypeO ptypeO)
  {
    assert(isPiece(ptypeO));
    return static_cast<Player>(static_cast<int>(ptypeO)>>31);
  }
  

  /** unpromoteすると共に，ownerを反転する． */
  inline PtypeO captured(PtypeO ptypeO)
  {
    assert(isPiece(ptypeO));
    return static_cast<PtypeO>((static_cast<int>(ptypeO)|8)^(~15));
  }
  
  /** owner を反転する */
  inline PtypeO alt(PtypeO ptypeO)
  {
    assert(isPiece(ptypeO));
    return static_cast<PtypeO>(static_cast<int>(ptypeO)^(~15));
  }

  /** 
   * Pieceの時にはowner を反転する 
   * 
   */
  inline PtypeO altIfPiece(PtypeO ptypeO)
  {
    int v=static_cast<int>(ptypeO);
    return static_cast<PtypeO>(v^((1-(v&15))&~15));
  }

  inline bool canPromote(PtypeO ptypeO)
  {
    return canPromote(getPtype(ptypeO));
  }


  /**
   * ptypeOが promote済みかどうか
   */
  inline bool isPromoted(PtypeO ptypeO)
  {
    assert(isValidPtypeO(ptypeO));
    return isPromoted(getPtype(ptypeO));
  }


  const PtypeO PTYPEO_EMPTY=newPtypeO(BLACK,PTYPE_EMPTY);
  const PtypeO PTYPEO_EDGE __attribute__((unused)) = newPtypeO(WHITE,PTYPE_EDGE);
  
  std::ostream& operator<<(std::ostream& os,const PtypeO ptypeO);
  
  const int PTYPEO_SIZE=PTYPEO_MAX-PTYPEO_MIN+1;

  enum Direction{
    SHORT_DIRECTION_MIN=0,
    SHORT8_DIRECTION_MIN=0,
    UL=0,
    U=1,
    UR=2,
    L=3,
    R=4,
    DL=5,
    D=6,
    DR=7,
    SHORT8_DIRECTION_MAX=7,
    UUL=8,
    UUR=9,
    LONG_DIRECTION_MIN=10,
    LONG_UL=10,
    LONG_U=11,
    LONG_UR=12,
    LONG_L=13,
    LONG_R=14,
    LONG_DL=15,
    LONG_D=16,
    LONG_DR=17,
    LONG_DIRECTION_MAX=17,
    DIRECTION_MIN=0,
    SHORT_DIRECTION_MAX=9,
    SHORT_DIRECTION_SIZE=10,
    DIRECTION_MAX=17,
    DIRECTION_INVALID_VALUE=18,
    DIRECTION_SIZE=18
  };
  
  constexpr bool isShort(Direction d){
    return d<=SHORT_DIRECTION_MAX;
  }

  constexpr bool isShort8(Direction d){
    return d<=SHORT8_DIRECTION_MAX;
  }

  constexpr bool isLong(Direction d){
    return d>=LONG_DIRECTION_MIN;
  }

  constexpr Direction inverseUnsafe(Direction d){
    return static_cast<Direction>(7 - d);
  }

  constexpr Direction inverse(Direction d){
    //assert(isShort8(d))
    return inverseUnsafe(d);
  }

  /**
   * 8方向について，primitiveな4方向を求める
   */
  constexpr Direction primDir(Direction d){
    //assert(isShort8(d))
    return (d<4) ? d : inverse(d);
  }
  /**
   * 8方向について，primitiveな4方向を求める
   * dとしてknight, INVALIDなども来る
   */
  constexpr Direction primDirUnsafe(Direction d){
    return (d<4) ? d : inverseUnsafe(d);
  }

  bool isValid(Direction d);
  
  constexpr Direction longToShort(Direction d){
    //assert(isLong(d))
    return static_cast<Direction>(static_cast<int>(d)-LONG_UL);
  }
  
  /**
   * 引数に longDirを与えてはいけない
   */
  constexpr Direction shortToLong(Direction d){
    //assert(isShort(d))
    return static_cast<Direction>(static_cast<int>(d)+LONG_UL);
  }

  constexpr int dirToMask(Direction dir){
    return (1<<static_cast<int>(dir));
  }
  
  std::ostream& operator<<(std::ostream& os,const Direction d);

  /**
   * 座標.
   *        盤面のインデックス
   * X, Yも1-9の範囲で表す 
   * Xは右から数える．Yは上から数える
   * なお駒台は0
   * <pre>
   * (A0)  ......................... (00)
   * (A1)  ......................... (01)
   * (A2) 92 82 72 62 52 42 32 22 12 (02)
   * (A3) 93 83 73 63 53 43 33 23 13 (03)
   * (A4) 94 84 74 64 54 44 34 24 14 (04)
   * (A5) 95 85 75 65 55 45 35 25 15 (05)
   * (A6) 96 86 76 66 56 46 36 26 16 (06)
   * (A7) 97 87 77 67 57 47 37 27 17 (07)
   * (A8) 98 88 78 68 58 48 38 28 18 (08)
   * (A9) 99 89 79 69 59 49 39 29 19 (09)
   * (AA) 9A 8A 7A 6A 5A 4A 3A 2A 1A (0A)
   * (AB) ...........................(0B)
   * (AC) ...........................(0C)
   * (AD) ...........................(0D)
   * (AE) ...........................(0E)
   * (AF) ...........................(0F) 
   * </pre>
   */
  class Square;
  bool operator==(Square l, Square r);
  /**
   * 座標の差分
   */
  class Offset
  {
  public:
    enum {
      OFFSET_MIN=-0x100,
      ONBOARD_OFFSET_MIN=-0x88,
      OFFSET_ZERO=0,
      ONBOARD_OFFSET_MAX=0x88,
      OFFSET_MAX=0x100,
      ONBOARD_OFFSET_SIZE=0x88*2+1
    };
    static const int BOARD_HEIGHT=16;
  private:
    int offset;
    explicit Offset(int o) : offset(o)
    {
    }
  public:
    static const Offset makeDirect(int value) { return Offset(value); }
    int intValue() const { return offset; }
  public:
    static int makeOffset(int dx,int dy) { return dx*BOARD_HEIGHT + dy; }
    Offset(int dx,int dy) : offset(makeOffset(dx,dy))
    {
    }
    Offset(Player, Direction);
    Offset() : offset(OFFSET_ZERO)
    {
    }
    template <Player, Direction>
    static Offset make();	// defined in directionTraits.h
    static const Offset ZERO() { return Offset(OFFSET_ZERO); }
    int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
    dx() const;
    int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
    dy() const;
    unsigned int index() const { return offset - OFFSET_MIN; }

    Offset& operator+=(Offset other)
    {
      offset += other.offset;
      return *this;
    }
    Offset& operator-=(Offset other){
      offset -= other.offset;
      return *this;
    }
    const Offset operator+(Offset other) const 
    {
      Offset result(*this);
      return result += other;
    }
    const Offset operator-(const Offset other) const
    {
      Offset result(*this);
      return result -= other;
    }
    const Offset operator*(const int mult) const {
      return static_cast<Offset>(static_cast<int>(offset)*mult);
    }
    const Offset operator-() const { return Offset(-offset); }
    /**
     * Player P からみた offset を黒番のものに変更する
     */
    template <Player P>
    const Offset blackOffset() const { return (P==BLACK) ? *this : -(*this); }

    bool zero() const { return offset == OFFSET_ZERO; }
  };

  /**
   * @obsolete
   */
  inline Offset newOffset(int dx,int dy){
    return Offset(dx,dy);
  }

  inline bool operator==(Offset l, Offset r)
  {
    return l.intValue() == r.intValue();
  }
  inline bool operator!=(Offset l, Offset r)
  {
    return ! (l == r);
  }
  inline bool operator<(Offset l, Offset r)
  {
    return l.intValue() < r.intValue();
  }
  

  std::ostream& operator<<(std::ostream&, Offset);
}
#include "bits/directionTraits.h"
namespace osl
{
  class Square
  {
    unsigned int square;
    explicit Square(int p) : square(p)
    {
    }
  public:
    static const Square makeDirect(int value) { return Square(value); }
    unsigned int uintValue() const { return square; }
    enum {
      PIECE_STAND=0,
      MIN=0,
      SIZE=0x100
    };
    Square() : square(PIECE_STAND)
    {
    }
    static const Square STAND() { return Square(PIECE_STAND); }
    Square(int x, int y) : square((x*Offset::BOARD_HEIGHT)+y+1)
    {
      assert(square < SIZE);
    }
    /**
     * assertなしに作る
     */
    static const Square makeNoCheck(int x, int y) { 
      return Square((x*Offset::BOARD_HEIGHT)+y+1); 
    }
    static const Square nth(unsigned int i) { return Square(i+MIN); }
    /**
     * 将棋としてのX座標を返す. Squareの内部表現に依存しない．
     */
    int x() const { return square >> 4; }
    /**
     * 将棋としてのY座標を返す. Squareの内部表現に依存しない．
     */
    int y() const { return (square&0xf)-1; }
    /**
     * y+1を返す
     */
    int y1() const { return square&0xf; }
    unsigned int index() const { return square - MIN; }
    static unsigned int indexMax() { return SIZE - MIN; }
    int indexForOffset32() const { return square + (square&0xf0); }

    bool isPieceStand() const { return square == PIECE_STAND; }
    bool isOnBoardSlow() const;
    /**
     * 盤面上を表すかどうかの判定．
     * 1<=x() && x()<=9 && 1<=y() && y()<=9
     * Squareの内部表現に依存する．
     */
    bool isOnBoard() const { 
      return (0xffffff88&(square-0x12)&
	      ((unsigned int)((square&0x77)^0x12)+0xffffff77))==0;
    }
    /**
     * onBoardから8近傍のオフセットを足した点がedgeかどうかの判定
     * そこそこ速くなった．
     */
    bool isEdge() const { 
      assert(!isPieceStand() && 0<=x() && x()<=10 && 0<=y() && y()<=10);
      return (0x88&(square-0x12)&((square&0x11)+0xf7))!=0;
    }
    bool isValid() const;


    const Square squareForBlack(Player player) const {
      return (player == BLACK)
	? *this
	: makeDirect(Square(9,9).uintValue()+Square(1,1).uintValue()-uintValue());
    }

    /** 
     * 後手の場合は盤面を引っくり返す.
     * PIECE_STANDの場合は扱えない．
     */
    template<Player P>
    const Square squareForBlack() const{
      return squareForBlack(P);
    }

    const Square rotate180() const 
    {
      return squareForBlack<WHITE>();
    }
    const Square rotate180EdgeOK() const 
    {
      Square ret=makeDirect(Square(9,9).uintValue()+Square(1,1).uintValue()-uintValue());
      return ret;
    }
    const Square rotate180Safe() const 
    {
      if (isPieceStand())
	return *this;
      return squareForBlack<WHITE>();
    }
    const Square flipHorizontal() const
    {
      if (isPieceStand())
	return *this;
      return Square(10-x(), y());
    }

    static const Square onBoardMax(){ return Square(9,9); }
    static const Square onBoardMin(){ return Square(1,1); }
  
    /**
     * squareがONBOARD_MINとONBOARD_MAXの間にある
     */
    bool isOnBoardRegion() const {
      return static_cast<unsigned int>(index()-onBoardMin().index()) 
	<= static_cast<unsigned int>(onBoardMax().index()-onBoardMin().index());
    }

    Square& operator++() {
      square += 1;
      return *this;
    }

    static int reverseX(int x) { return 10-x; }
    static int reverseY(int y) { return 10-y; }
  public:
    template <Player P>
    static bool canPromoteY(int y) { 
      return P == BLACK ? y <= 3 : y >= 7;
    }
    template <Player P>
    bool canPromote() const{
      return canPromote(P);
    }
    bool canPromote(Player player) const 
    {
      if (player==BLACK) 
	return (uintValue()&0xf)<=4;
      else 
	return (uintValue()&0x8)!=0;
    }
    /**
     * 2つのSquare(onBoardであることが前提)が，
     * xが等しいかyが等しい
     */
    bool isULRD(Square sq) const{
      assert(isOnBoard() && sq.isOnBoard());
      unsigned int v=uintValue() ^ sq.uintValue();
      return (((v+0xefull)^v)&0x110ull)!=0x110ull;
    }
    /**
     * 2つのSquare(onBoardであることが前提)のxが等しい
     */
    bool isUD(Square sq) const{
      assert(isOnBoard() && sq.isOnBoard());
      unsigned int v=uintValue() ^ sq.uintValue();
      return (v&0xf0)==0;
    }
    /**
     * sqがPlayer Pにとって上
     */
    template<Player P>
    bool isU(Square sq) const{
      assert(isOnBoard() && sq.isOnBoard());
      unsigned int v=uintValue() ^ sq.uintValue();
      if(P==BLACK)
	return ((v|(uintValue()-sq.uintValue()))&0xf0)==0;
      else
	return ((v|(sq.uintValue()-uintValue()))&0xf0)==0;
    }
    /**
     * 2つのSquare(onBoardであることが前提)のyが等しい
     */
    bool isLR(Square sq) const{
      assert(isOnBoard() && sq.isOnBoard());
      unsigned int v=uintValue() ^ sq.uintValue();
      return (v&0xf)==0;
    }
    Square& operator+=(Offset offset) {
      square += offset.intValue();
      return *this;
    }
    Square& operator-=(Offset offset) {
      square -= offset.intValue();
      return *this;
    }
    const Square operator+(Offset offset) const {
      Square result(*this);
      return result+=offset;
    }
    const Square operator-(Offset offset) const {
      Square result(*this);
      return result-=offset;
    }
    const Offset operator-(Square other) const {
      return Offset::makeDirect(square - other.square);
    }
    template<int Y>
    bool yEq() {
      return (uintValue()&0xf)==(Y+1);
    }
    template<int Y>
    typename std::enable_if<Y!=2,bool>::type yLe() {
      return (uintValue()&0xf)<=(Y+1);
    }
    template<int Y>
    typename std::enable_if<Y==2,bool>::type yLe() {
      return (uintValue()&0xc)==0;
    }
    template<int Y>
    typename std::enable_if<Y!=7,bool>::type yGe() {
      return (uintValue()&0xf)>=(Y+1);
    }
    template<int Y>
    typename std::enable_if<Y==7,bool>::type yGe() {
      return (uintValue()&0x8)!=0;
    }
    template <Player P, Direction D>
    const Square neighbor() const {
      return *this + DirectionPlayerTraits<D,P>::offset();
    }
    template <Player P, Direction D>
    const Square back() const {
      return neighbor<alt(P),D>();
    }
    const Square neighbor(Player P, Direction D) const;
    const Square back(Player P, Direction D) const;
    bool isNeighboring8(Square to) const;
  };

  inline bool operator==(Square l, Square r)
  {
    return l.uintValue() == r.uintValue();
  }
  inline bool operator!=(Square l, Square r)
  {
    return ! (l == r);
  }
  inline bool operator<(Square l, Square r)
  {
    return l.uintValue() < r.uintValue();
  }
  inline bool operator>(Square l, Square r)
  {
    return l.uintValue() > r.uintValue();
  }
  std::ostream& operator<<(std::ostream&, Square);

  class Piece;
  inline bool operator==(Piece l, Piece r);
  const int EMPTY_NUM=0x80;
  const int EDGE_NUM=0x40;
  /**
   * 駒.
   * 駒はptypeo(-15 - 15), 番号(0-39), ポジション(0-0xff)からなる 
   * 上位16 bitでptypeo, 8bitで番号, 8bitでポジションとする．
   * 空きマスは 黒，PTYPE_EMPTY, 番号 0x80, ポジション 0
   * 盤外は     白，PTYPE_EDGE,  番号 0x40, ポジション 0
   */
  class Piece
  {
    int piece;
    Piece(int p) : piece(p)
    {
    }
  public:
    static const int SIZE=40;
    static const Piece makeDirect(int value) { return Piece(value); }
    int intValue() const { return piece; }
    static const Piece EMPTY()  { return Piece(BLACK,PTYPE_EMPTY,EMPTY_NUM,Square::STAND()); }
    static const Piece EDGE() { return Piece(WHITE,PTYPE_EDGE,EDGE_NUM,Square::STAND()); }
    static const int BitOffsetPtype=16;
    static const int BitOffsetPromote=BitOffsetPtype+3;
    static const int BitOffsetMovePromote=BitOffsetPromote+4;
    
    Piece(Player owner, Ptype ptype, int num, Square square)
      : piece((static_cast<int>(owner)<<20)
	      +(static_cast<int>(ptype)<<BitOffsetPtype)
	      +((num)<<8)+ square.uintValue())
    {
    }
    Piece() : piece(EMPTY().piece)
    {
    }
    /**
     * 玉を作る
     */
    static const Piece
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
    makeKing(Player owner, Square square);

    Ptype ptype() const {
      return static_cast<Ptype>((piece>>BitOffsetPtype)&0xf);
    }
    PtypeO ptypeO() const {
      return static_cast<PtypeO>(piece>>BitOffsetPtype);
    }

    int number() const {
      return ((piece&0xff00)>>8);
    }

    const Square square() const {
      return Square::makeDirect(piece&0xff);
    }

    Piece& operator+=(Offset offset) {
      piece += offset.intValue();
      return *this;
    }

    void setSquare(Square square) {
      piece = (piece&0xffffff00)+square.uintValue();
    }
  public:
    /**
     * piece がプレイヤーPの持ち物でかつボード上にある駒の場合は true.
     * 敵の駒だったり，駒台の駒だったり，Piece::EMPTY(), PIECE_EDGEの場合は false
     * @param P(template) - プレイヤー
     * @param piece - 
     */
    template<Player P>
    bool isOnBoardByOwner() const { return isOnBoardByOwner(P); }
    /**
     * isOnBoardByOwner の通常関数のバージョン.
     */
    bool isOnBoardByOwner(Player owner) const
    {
      if(owner==BLACK)
	return static_cast<int>(static_cast<unsigned int>(piece)&0x800000ff)>0;
      else
	return static_cast<int>((-piece)&0x800000ff)>0;
    }

    /* 成る.  PROMOTE不可なpieceに適用不可 */
    const Piece promote() const {
      assert(canPromote(ptype()));
      return Piece(piece-0x80000);
    }

    /* 成りを戻す.  PROMOTE不可なpieceに適用可  */
    const Piece unpromote() const {
      return Piece((int)piece|0x80000);
    }

    /**
     * 取られたpieceを作成. unpromoteして，Squareは0に
     * 相手の持ちものにする
     */
    const Piece captured() const {
      // return (Piece)((((int)piece|0x80000)&0xffffff00)^0xfff00000);
      // をoptimizeする
      return Piece((piece&0xfff7ff00)^0xfff80000);
    }

    const Piece promoteWithMask(int promote_mask) const {
      assert(! (isPromoted() && promote_mask));
      assert(promote_mask==0 || promote_mask==(1<<23));
      return Piece(piece - (promote_mask>>(BitOffsetMovePromote-BitOffsetPromote)));
    }

    const Piece checkPromote(bool promotep) const {
      return Piece(piece - (promotep<<19));
    }

    /**
     * promoteした駒かどうかをチェックする
     */
    bool isPromoted() const { return (piece&(1<<19))==0; }

    /**
     * promoteしていないOnBoardの駒であることのチェック
     * Lance位しか使い道がない?
     */
    bool isOnBoardNotPromoted() const{
      int mask=piece&((1<<19)|0xff);
      return mask>(1<<19);
    }
    bool isPromotedNotKingGold() const {
      assert(ptype()!=KING && ptype()!=GOLD);
      return isPromoted();
    }

    bool isEmpty() const {
      return (piece&0x8000)!=0;
    }
    static bool isEmptyNum(int num) {
      return (num&0x80)!=0;
    }
    bool isEdge() const { 
      return (piece&0x4000)!=0;
    }
    static bool isEdgeNum(int num){
      assert(!isEmptyNum(num));
       return (num&0x40)!=0;
    }
    static bool isPieceNum(int num){
      return (num&0xc0)==0;
    }
    template<Ptype T>
    bool isPtype() const{
      return (piece&0xf0000)==((T)<<BitOffsetPtype);
    }
    /**
     * あるpieceがPlayer pの持ち物でPtype ptypeであるかどうかをチェックする．
     * TはEMPTY, EDGEではない．
     */
    bool isPlayerPtype(Player pl,Ptype ptype) const{
      assert(PTYPE_PIECE_MIN<=ptype && ptype<=PTYPE_MAX);
      return (piece&0x1f0000)==(((ptype)<<BitOffsetPtype)|(pl&0x100000));
    }
    /**
     * あるpieceがPlayer pの持ち物でBASIC typeがptypeであるかどうかをチェックする．
     * TはEMPTY, EDGEではない．
     */
    bool isPlayerBasicPtype(Player pl,Ptype ptype) const{
      assert(PTYPE_PIECE_MIN<=ptype && ptype<=PTYPE_MAX);
      assert(isBasic(ptype));
      if(canPromote(ptype))
	return (piece&0x170000)==(((osl::promote(ptype))<<BitOffsetPtype)|(pl&0x100000));
      else
	return isPlayerPtype(pl,ptype);
    }
    bool isPiece() const {
      return (piece&0xc000)==0;
    }
    /**
     * pieceであることが分かっている時に，更にBlackかどうかをチェックする．
     */
    bool pieceIsBlack() const{
      assert(isPiece());
      return static_cast<int>(piece)>=0;
    }
    Player owner() const
    {
      assert(isPiece());
      return static_cast<Player>(piece>>20);
    }

  private:
  public:
    /** Player Pの駒が，thisの上に移動できるか?
     * PIECE_EMPTY 0x00008000
     * BLACK_PIECE 0x000XxxYY X>=2, YY>0
     * PIECE_EDGE  0xfff14000
     * WHITE_PIECE 0xfffXxxYY X>=2, YY>0
     * @return thisが相手の駒かEMPTYならtrue
     * @param P 手番
     */
    template<Player P>
    bool canMoveOn() const { return canMoveOn(P); }
    bool canMoveOn(Player pl) const{
      return pl == BLACK ? ((piece+0xe0000)&0x104000)==0 : piece>=0;
    }

    bool isOnBoard() const {
      assert(square().isValid());
      return ! square().isPieceStand();
    }
  };

  inline bool operator<(Piece l, Piece r)
  {
    return l.intValue() < r.intValue();
  }
  inline bool operator==(Piece l, Piece r)
  {
    return l.intValue() == r.intValue();
  }
  inline bool operator!=(Piece l, Piece r)
  {
    return ! (l == r);
  }

  std::ostream& operator<<(std::ostream& os,const Piece piece);
}

/** move 関係でつかまえ所のないエラーがでるときに定義する */
// #define MOVE_DEBUG
#ifdef MOVE_DEBUG
#  include <cassert>
#  define move_assert(x) assert(x)
#else
#  define move_assert(x) 
#endif
// 2009/12/10 以前のfromが下位にあるパターンと
// operator< を同じにしたい時に定義する．
// #define PRESERVE_MOVE_ORDER

namespace osl
{
  class SimpleState;
  /** 16bit 表現*/
  enum Move16 {
    MOVE16_NONE = 0,
  };
  /**
   * 圧縮していない moveの表現 .
   * - invalid: isInvalid 以外の演算はできない
   * - declare_win: isInvalid 以外の演算はできない
   * - pass: from, to, ptype, oldPtype はとれる．player()はとれない．
   * 
   * Pieceとpromotepをそろえる  -> 変える． 
   * 下位から 
   * 2009/12/10から
   * - to       : 8 bit 
   * - from     : 8 bit 
   * - capture ptype    : 4 bit 
   * - dummy    : 3 bit 
   * - promote? : 1 bit  
   * - ptype    : 4 bit - promote moveの場合はpromote後のもの
   * - owner    : signed 
   * 2009/12/10以前
   * - from     : 8 bit 
   * - to       : 8 bit 
   * - dummy    : 3 bit 
   * - promote? : 1 bit  
   * - capture ptype    : 4 bit 
   * - ptype    : 4 bit - promote moveの場合はpromote後のもの
   * - owner    : signed 
   */
  class Move
  {
  public:
    static const int BitOffsetPromote=Piece::BitOffsetMovePromote;  // 23
  private:
    int move;
    explicit Move(int value) : move(value)
    {
    }
    enum { 
      INVALID_VALUE = (1<<8), DECLARE_WIN = (2<<8),
      BLACK_PASS = 0, WHITE_PASS = (-1)<<28, 
    };
  public:
    int intValue() const { return move; }
    /** 駒を取らない手を [0, 16305] にmap */
    unsigned int hash() const;
    /** 一局面辺りの合法手の最大値 
     * 重複して手を生成することがある場合は，600では不足かもしれない
     */
    static const unsigned int MaxUniqMoves=600;
  private:
    void init(Square from, Square to, Ptype ptype,
	      Ptype capture_ptype, bool is_promote, Player player)
    {
      move =  (to.uintValue()
 	       + (from.uintValue()<<8)
	       + (static_cast<unsigned int>(capture_ptype)<<16)
	       + (static_cast<unsigned int>(is_promote)<<BitOffsetPromote)
	       + (static_cast<unsigned int>(ptype)<<24)
	       + (static_cast<int>(player)<<28));
    }
  public:
    Move() : move(INVALID_VALUE)
    {
    }
    /** INVALID でも PASS でもない. isValid()かどうかは分からない．*/
    bool isNormal() const { 
      // PASS や INVALID は to() が 00
      return move & 0x00ff; 
    }
    bool isPass() const { return (move & 0xffff) == 0; }
    static const Move makeDirect(int value) { return Move(value); }
    static const Move PASS(Player P) { return Move(P<<28); }
    static const Move INVALID() { return Move(INVALID_VALUE); }
    static const Move DeclareWin() { return Move(DECLARE_WIN); }
    /**
     * 移動
     */
    Move(Square from, Square to, Ptype ptype,
	 Ptype capture_ptype, bool is_promote, Player player)
    {
      move_assert(from.isValid());
      move_assert(to.isOnBoard());
      move_assert(isValid(ptype));
      move_assert(isValid(capture_ptype));
      move_assert(isValid(player));
      init(from, to, ptype, capture_ptype, is_promote, player);
      move_assert(isValid());
    }
    /**
     * drop
     */
    Move(Square to, Ptype ptype, Player player)
    {
      move_assert(to.isOnBoard());
      move_assert(isValid(ptype));
      move_assert(isValid(player));
      init(Square::STAND(), to, ptype, PTYPE_EMPTY, false, player);
      move_assert(isValid());
    }
    static const Move fromMove16(Move16, const SimpleState&);
    Move16 toMove16() const;

    const Square from() const 
    {
      assert(! isInvalid());
      move_assert(isValidOrPass());
      const Square result = Square::makeDirect((move>>8) & 0xff);
      return result;
    }
    const Square to() const {
      assert(! isInvalid());
      move_assert(isValidOrPass());
      const Square result = Square::makeDirect(move & 0xff);
      return result;
    }
    /** fromとtoをまとめて同一性の判定など */
    unsigned int fromTo() const { return move & 0xffff; }
    /**
     * pieceに使うためのmaskなので
     */
    int promoteMask() const {
      assert(isNormal());
      return (static_cast<int>(move)&(1<<BitOffsetPromote));
    }
    bool isPromotion() const { assert(isNormal()); return (move & (1<<BitOffsetPromote))!=0; }
    bool isCapture() const { assert(isNormal()); return capturePtype() != PTYPE_EMPTY; }
    bool isCaptureOrPromotion() const { return isCapture() || isPromotion(); }
    bool isDrop() const { assert(isNormal()); return from().isPieceStand(); }
    bool isPawnDrop() const {
      return isDrop() && ptype() == PAWN;
    }
      
    Ptype ptype() const {
      assert(! isInvalid());
      move_assert(isValidOrPass());
      const Ptype result = static_cast<Ptype>((move >> 24) & 0xf);
      return result;
    }
    /** 移動後のPtype, i.e., 成る手だった場合成った後 */
    PtypeO ptypeO() const {
      assert(! isInvalid());
      const PtypeO result = static_cast<PtypeO>(move >> 24);
      return result;
    }
    /** 移動前のPtypeO, i.e., 成る手だった場合成る前 */
    PtypeO oldPtypeO() const {
      assert(! isInvalid());
      const PtypeO result = static_cast<PtypeO>((move>>24)+((move >> (BitOffsetPromote-3))&8));
      return result;
    }
    /** 移動前のPtype, i.e., 成る手だった場合成る前 */
    Ptype oldPtype() const { 
      assert(! isInvalid());
      move_assert(isValidOrPass());
      const PtypeO old_ptypeo = static_cast<PtypeO>((move>>24)+((move >> (BitOffsetPromote-3))&8));
      return getPtype(old_ptypeo); 
    }
    Ptype capturePtype() const {
      assert(isNormal());
      const Ptype result = static_cast<Ptype>((move>>16)&0xf);
      return result;
    }
    PtypeO capturePtypeO() const {
      assert(isCapture());
      return newPtypeO(alt(player()), capturePtype());
    }
    PtypeO capturePtypeOSafe() const {
      if (! isCapture())
	return PTYPEO_EMPTY;
      return capturePtypeO();
    }

    Player player() const {
      assert(! isInvalid());
      const Player result = static_cast<Player>(move>>28);
      return result;
    }
    bool isValid() const;
    /** state に apply 可能でない場合にtrue */
    bool isInvalid() const { 
      return static_cast<unsigned int>(move-1) < DECLARE_WIN; 
    }
    bool isValidOrPass() const { return isPass() || isValid(); }

    Move newFrom(Square new_from) const
    {
      assert(isNormal());
      int result = static_cast<int>(intValue());
      result &= ~(0xff00);
      result += (new_from.uintValue()<<8);
      return makeDirect(result);
    }
    Move newAddFrom(Square new_from) const
    {
      assert(isNormal());
      assert(from().uintValue()==0);
      int result = static_cast<int>(intValue());
      result += (new_from.uintValue()<<8);
      return makeDirect(result);
    }
    /**
     * no capture moveからcapture moveを作る
     */
    const Move newAddCapture(Piece capture) const
    {
      assert(! isCapture());
      return makeDirect(intValue()+(capture.intValue()&0xf0000));
    }
    const Move newCapture(Piece capture) const
    {
      return makeDirect((intValue()&0xfff0ffff)+(capture.intValue()&0xf0000));
    }
    const Move newCapture(Ptype capture) const
    {
      return makeDirect((intValue()&0xfff0ffff)
			+(static_cast<int>(capture)<<Piece::BitOffsetPtype));
    }
    /**
     * promote moveからunpromote moveを作る
     */
    const Move unpromote() const {
      assert(isNormal());
      move_assert(isPromotion() && isPromoted(ptype()));
      return makeDirect(intValue()^((1<<BitOffsetPromote)^(1<<27)));
    }
    /**
     * unpromote moveからpromote moveを作る
     */
    const Move promote() const {
      assert(isNormal());
      move_assert(!isPromotion() && canPromote(ptype()));
      return makeDirect(intValue()^((1<<BitOffsetPromote)^(1<<27)));
    }
    /**
     * moveのtoをoffsetだけ変える．
     * 元のtoが0以外でも使える
     */
    inline Move newAddTo(Offset o) const{
      return makeDirect(intValue()+o.intValue());
    }
    /**
     * つくってあったmoveの雛形のsquareをsetする．
     * mのtoは0
     */
    inline Move newAddTo(Square sq) const{
      assert((intValue()&0xff)==0);
      return Move::makeDirect(intValue()+sq.uintValue());
    }
    /**
     * 作ってあったPTYPE_EMPTYのひな形のPTYPEをsetする 
     */
    inline Move newAddPtype(Ptype newPtype) const{
      assert(ptype()==PTYPE_EMPTY);
      return Move::makeDirect(intValue()
			      + (static_cast<unsigned int>(newPtype)<<24));
    }
    template<Player P>
    static bool ignoreUnpromote(Ptype ptype,Square from,Square to){
      switch(ptype){
      case PAWN: 
	return to.canPromote<P>();
      case BISHOP: case ROOK: 
	return to.canPromote<P>() || from.canPromote<P>();
      case LANCE:
	return (P==BLACK ? to.y()==2 : to.y()==8);
      default: return false;
      }
    }
    /**
     * 合法手ではあるが，打歩詰め絡み以外では有利にはならない手.  
     * TODO 遅い
     */
    template<Player P>
    bool ignoreUnpromote() const{
      assert(player()==P);
      if(isDrop()) return false;
      return ignoreUnpromote<P>(ptype(),from(),to());
    }
    bool ignoreUnpromote() const{
      if(player()==BLACK) return ignoreUnpromote<BLACK>();
      else return ignoreUnpromote<WHITE>();
    }
    /**
     * MoveをunpromoteするとcutUnpromoteなMoveになる
     */
    template<Player P>
    bool hasIgnoredUnpromote() const{
      assert(player()==P);
      if(!isPromotion()) return false;
      switch(ptype()){
      case PPAWN: 
	return (P==BLACK ? to().y()!=1 : to().y()!=9);
      case PLANCE:
	return (P==BLACK ? to().y()==2 : to().y()==8);
      case PBISHOP: case PROOK: 
	return true;
      default: return false;
      }
    }
    bool hasIgnoredUnpromote() const{
      if(player()==BLACK) return hasIgnoredUnpromote<BLACK>();
      else return hasIgnoredUnpromote<WHITE>();
    }
    const Move rotate180() const;
  };
  inline bool operator<(Move lhs, Move rhs)
  {
#ifdef PRESERVE_MOVE_ORDER
    int l=lhs.intValue();
    l=(l&0xffff0000)+((l>>8)&0xff)+((l<<8)&0xff00);
    int r=rhs.intValue();
    r=(r&0xffff0000)+((r>>8)&0xff)+((r<<8)&0xff00);
    return l<r;
#else
    return lhs.intValue() < rhs.intValue();
#endif
  }
  inline bool operator==(Move lhs, Move rhs)
  {
    return lhs.intValue() == rhs.intValue();
  }
  inline bool operator!=(Move lhs, Move rhs)
  {
    return ! (lhs == rhs);
  }

  std::ostream& operator<<(std::ostream& os, Move move);
}

namespace std
{
  template <typename T> struct hash;
  template <> struct hash<osl::Move>
  {
    unsigned long operator()(osl::Move m) const { return m.intValue(); }
  };
} // namespace stl
#endif /* OSL_BASIC_TYPE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
