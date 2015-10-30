/* simpleState.h
 */
#ifndef OSL_SIMPLE_STATE_H
#define OSL_SIMPLE_STATE_H

#include "osl/basic_type.h"
#include "osl/bits/ptypeTable.h"
#include "osl/bits/boardTable.h"
#include "osl/bits/ptypeTraits.h"
#include "osl/bits/pieceMask.h"
#include "osl/bits/bitXmask.h"
#include "osl/bits/effectContent.h"
#include "osl/container.h"


#include <iosfwd>

namespace osl
{
  enum Handicap{
    HIRATE,
    //    KYOUOCHI,
    //    KAKUOCHI,
  };
    class SimpleState;
    std::ostream& operator<<(std::ostream& os,const SimpleState& state);
    /**
     * 盤上の駒のみを比較する（持ち駒は見ない）.
     * なお、駒番に非依存な局面比較をしたい場合は、osl::record::CompactBoardや
     * osl::hash::HashKeyを用いる.
     */
    bool operator==(const SimpleState& st1,const SimpleState& st2);

    class SimpleState
    {
    private:
      friend std::ostream& operator<<(std::ostream& os,const SimpleState& state);
      friend bool operator==(const SimpleState& st1,const SimpleState& st2);
      typedef SimpleState state_t;
    public:
      static const bool hasPawnMask=true;
    protected:
      CArray<Piece,Square::SIZE> board
#ifdef __GNUC__
      __attribute__((aligned(16)))
#endif
	;
      /**
       * 全てのpieceが登録されている
       */
      CArray<Piece,Piece::SIZE> pieces
#ifdef __GNUC__
      __attribute__((aligned(16)))
#endif
	;
      CArray<PieceMask,2> stand_mask;
      CArray<BitXmask,2> pawnMask;
      CArray<CArray<char,PTYPE_SIZE-PTYPE_BASIC_MIN>,2> stand_count;

      /** 手番 */
      Player player_to_move;
      PieceMask used_mask;
    public:
      // 生成に関するもの
      explicit SimpleState();
      explicit SimpleState(Handicap h);
      // public継承させるには，virtual destructorを定義する．
      virtual ~SimpleState();
      /** 盤面が空の状態に初期化 */
      void init();
      /** ハンディに応じた初期状態に初期化 */
      void init(Handicap h);
      // private:
      void initPawnMask();
    public:
      const Piece pieceOf(int num) const{
	return pieces[num];
      }
      void setPieceOf(int num,Piece p) {
	pieces[num]=p;
      }
      template<Player P>
      const Piece kingPiece() const{
	return pieceOf(KingTraits<P>::index);
      }
      const Piece kingPiece(Player P) const{
	assert(isValid(P));
	if (P==BLACK)
	  return kingPiece<BLACK>();
	else
	  return kingPiece<WHITE>();
      }
      template<Player P>
      Square kingSquare() const{
	return kingPiece<P>().square();
      }
      Square kingSquare(Player player) const{
	assert(isValid(player));
	if (player==BLACK)
	  return kingSquare<BLACK>();
	else
	  return kingSquare<WHITE>();
      }
      template <Ptype PTYPE>
      static int nthLimit() {
	return PtypeTraits<PTYPE>::indexLimit - PtypeTraits<PTYPE>::indexMin;
      }
      /**
       * unpromote(PTYPE)のn番目の駒を帰す.  
       * 
       * 駒番号に依存するので順番は不定.
       */
      template <Ptype PTYPE>
      const Piece nth(int n) const {
	assert(0 <= n && n < nthLimit<PTYPE>());
	return pieceOf(PtypeTraits<PTYPE>::indexMin+n);
      }

      void setBoard(Square sq,Piece piece)
      {
	board[sq.index()]=piece;
      }
    protected:
      PieceMask& standMask(Player p) {
	return stand_mask[p];
      }
    public:
      const PieceMask& standMask(Player p) const {
	return stand_mask[p];
      }
      const PieceMask& usedMask() const {return used_mask;}
      bool isOffBoard(int num) const{
	return standMask(BLACK).test(num) 
	  || standMask(WHITE).test(num);
      }
      // protected:
      /** (internal) */
      void clearPawn(Player pl,Square sq){
	pawnMask[pl].clear(sq);
      }
      /** (internal) */
      void setPawn(Player pl,Square sq){
	pawnMask[pl].set(sq);
      }
    public:      
      bool isPawnMaskSet(Player player, int x) const
      {
	return pawnMask[player].isSet(x);
      }

      template<Player P>
      bool isPawnMaskSet(int x)const {return isPawnMaskSet(P,x); }

      /** xの筋に歩を打てる */
      bool canDropPawnTo(Player player, int x) const
      {
	return hasPieceOnStand<PAWN>(player) && ! isPawnMaskSet(player, x);
      }

      void setPiece(Player player,Square sq,Ptype ptype);
      void setPieceAll(Player player);

      /**
       * @param sq は isOnboardを満たす Square の12近傍(8近傍+桂馬の利き)
       * ! isOnBoard(sq) の場合は PIECE_EDGE を返す
       */
      const Piece pieceAt(Square sq) const { return board[sq.index()];}
      const Piece operator[](Square sq) const { return pieceAt(sq);}
      const Piece* getPiecePtr(Square sq) const { return &board[sq.index()];}
      const Piece pieceOnBoard(Square sq) const
      {
	assert(sq.isOnBoard());
	return pieceAt(sq);
      }

      bool isOnBoard(int num) const {
	return pieceOf(num).isOnBoard();
      }
      /**
       * 持駒の枚数を数える
       */
      int countPiecesOnStand(Player pl,Ptype ptype) const {
	assert(isBasic(ptype));
	return stand_count[pl][ptype-PTYPE_BASIC_MIN];
      }
      /** 後方互換 */
      template <Ptype Type>
      int countPiecesOnStand(Player pl) const {
	return countPiecesOnStand(pl, Type);
      }
      bool hasPieceOnStand(Player player,Ptype ptype) const{
	return countPiecesOnStand(player, ptype)!=0;
      }
      template<Ptype T>
      bool hasPieceOnStand(Player P) const {
	return countPiecesOnStand(P, T);
      }
    private:
      int countPiecesOnStandBit(Player pl,Ptype ptype) const {
	return (standMask(pl).getMask(0)
		& Ptype_Table.getMaskLow(ptype)).countBit();
      }
    public:
      /**
       * diff方向にあるPiece を求める. 
       * @return 盤外ならPTYPE_EDGE
       */
      Piece nextPiece(Square cur, Offset diff) const
      {
	assert(! diff.zero());
	cur += diff;
	while (pieceAt(cur) == Piece::EMPTY())
	  cur += diff;
	return pieceAt(cur);
      }
    
      void setTurn(Player player) {
	player_to_move=player;
      }
      Player turn() const{
	return player_to_move;
      }
      /**
       * 手番を変更する
       */
      void changeTurn() {
	player_to_move = alt(player_to_move);
      }
      // check
      bool isConsistent(bool show_error=true) const;
      /** エラー表示をするかどうかをtemplateパラメータにした高速化版 */
      template <bool show_error>
      bool isAlmostValidMove(Move move) const;
      /**
       * 合法手かどうかを簡単に検査する．局面に依存するチェックのみ．
       * ルール上指せない手である可能性がある場合は，isValidMove を用いる．
       *
       * 局面に依存する検査でも，玉の素抜きや王手を防いでいるか，
       * 千日手，打歩詰かどうかは検査しない．
       */
      bool isAlmostValidMove(Move move,bool show_error=true) const;
      /**
       * 合法手かどうかを検査する．
       * isValidMoveByRule, isAlmostValidMove をおこなう．
       * 玉の素抜きや王手を防いでいるか，
       * 千日手，打歩詰かどうかは検査しない．
       */
      bool isValidMove(Move move,bool show_error=true) const;
    protected:
      template <bool show_error> bool isAlmostValidDrop(Move move) const;
      template <bool show_error> bool testValidityOtherThanEffect(Move move) const;
    public:
      /**
       * 盤面以外の部分の反則のチェック
       *
       */
      static bool isValidMoveByRule(Move move,bool show_error);

      /**
       * @param from - マスの位置
       * @param to - マスの位置
       * @param offset - fromからtoへのshort offset
       * fromとtoがクイーンで利きがある位置関係にあるという前提
       * で，間が全部空白かをチェック
       * @param pieceExistsAtTo - toに必ず駒がある (toが空白でも動く)
       */
      bool isEmptyBetween(Square from, Square to,Offset offset,bool pieceExistsAtTo=false) const
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      {
	assert(from.isOnBoard());
	assert(! offset.zero());
	assert(offset==Board_Table.getShortOffset(Offset32(to,from)));
	Square sq=from+offset;
	for (; pieceAt(sq).isEmpty(); sq+=offset) {
	  if (!pieceExistsAtTo && sq==to) 
	    return true;
	}
	return sq==to;
      
      }
      /**
       * @param from - マスの位置
       * @param to - マスの位置
       * fromとtoがクイーンで利きがある位置関係にあるという前提
       * で，間が全部空白かをチェック
       */
      bool
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      isEmptyBetween(Square from, Square to,bool noSpaceAtTo=false) const{
	assert(from.isOnBoard());
	Offset offset=Board_Table.getShortOffset(Offset32(to,from));
	assert(! offset.zero());
	return isEmptyBetween(from,to,offset,noSpaceAtTo);
      }

      /** dump: 自分を cerr に表示する。abort 前などにデバッグに使う */
      bool dump() const;
      /**
       * from で表現されたPieceをnew_ownerの持駒にした局面を作る.
       */
      const SimpleState emulateCapture(Piece from, Player new_owner) const;

      /**
       * from からto に ptypeの持駒を一枚渡した局面を作る.
       */
      const SimpleState emulateHandPiece(Player from, Player to, Ptype ptype) const;
      const SimpleState rotate180() const;
      const SimpleState flipHorizontal() const;
    };  

} // namespace osl

#endif /* OSL_SIMPLE_STATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
