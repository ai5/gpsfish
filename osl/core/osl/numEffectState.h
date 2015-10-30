/* numEffectState.h
 */
#ifndef OSL_NUM_EFFECT_STATE_H
#define OSL_NUM_EFFECT_STATE_H

#include "osl/bits/numSimpleEffect.h"
#include "osl/mobility/kingMobility.h"
#include "osl/bits/align16New.h"

namespace osl
{
  namespace checkmate
  {
    class King8Info;
  }
  class MoveVector;
    class NumEffectState;
    /**
     * 駒番に依存した局面（インスタンス）比較をする.
     * なお、駒番に非依存な局面比較をしたい場合は、osl::record::CompactBoardや
     * osl::hash::HashKeyを用いる.
     */
    bool operator==(const NumEffectState& st1, const NumEffectState& st2);

    /**
     * 利きを持つ局面
     * - effects (NumSimpleEffectTable) 利き
     * - pieces_onboard (PieceMask) 盤上にある駒
     */
    class NumEffectState : public SimpleState
#if OSL_WORDSIZE == 32
			 , public misc::Align16New
#endif
    {
      effect::NumSimpleEffectTable effects;
      CArray<PieceMask,2> pieces_onboard;
      /** 成駒一覧 */
      PieceMask promoted;
      CArray<PieceMask,2> pin_or_open;
      KingMobility king_mobility;
      CArray<uint64_t,2> king8infos;

      friend bool operator==(const NumEffectState& st1,const NumEffectState& st2);
      typedef NumEffectState state_t;
    public:
      // ----------------------------------------------------------------------
      // 0. 将棋以外の操作
      // ----------------------------------------------------------------------
      explicit NumEffectState(const SimpleState& st=SimpleState(HIRATE));
      ~NumEffectState();
      /** 主要部分を高速にコピーする. 盤の外はコピーされない*/
      void copyFrom(const NumEffectState& src);
      void copyFrom(const SimpleState& src);
      bool isConsistent(bool showError=true) const;
      /** 局面更新に関する一貫性をテスト */
      bool isConsistent(const NumEffectState& prev, Move moved, bool show_error=true) const;
      void showEffect(std::ostream& os) const;


      // ----------------------------------------------------------------------
      // 1. 盤面全体の情報
      // ----------------------------------------------------------------------
      const PieceMask& piecesOnBoard(Player p) const { return pieces_onboard[p]; }
      const PieceMask promotedPieces() const { return promoted; }
      const PieceMask pin(Player king) const
      {
	return pin_or_open[king]&piecesOnBoard(king);
      }
      /** attack の駒で動くと開き王手になる可能性がある集合 */
      const PieceMask checkShadow(Player attack) const
      {
	return pin_or_open[alt(attack)]&piecesOnBoard(attack);
      }
      PieceMask pinOrOpen(Player king) const
      {
	return pin_or_open[king];
      }
      uint64_t Iking8Info(Player king) const
      {
	return king8infos[king];
      }
      const checkmate::King8Info
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      king8Info(Player king) const;
      /** Pの玉が王手状態 */
      bool inCheck(Player P) const 
      {
	const Square king = kingSquare(P);
#ifdef ALLOW_KING_ABSENCE
	if (king.isPieceStand())
	  return false;
#endif
	return hasEffectAt(alt(P), king);
      }
      /** 手番の玉が王手状態 */
      bool inCheck() const { return inCheck(turn()); }
      /**
       * target の王に合駒可能でない王手がかかっているかどうか.
       * - 両王手 => 真
       * - unblockable な利きだけ => 真
       * - blockable な利きだけ => 偽
       * - 王手でない => 偽
       * 2014/03
       */
      bool inUnblockableCheck(Player target) const {
	const Square king_position = kingSquare(target);
	Piece attacker_piece;
	if (hasEffectAt(alt(target), king_position, attacker_piece)) {
	  if (attacker_piece == Piece::EMPTY())
	    return true;	// multiple pieces
	  // sigle check
	  const Square from = attacker_piece.square();
	  const EffectContent effect
	    = Ptype_Table.getEffect(attacker_piece.ptypeO(), 
				   from, king_position);
	  return effect.hasUnblockableEffect();
	}
	// no check
	return false;
      }

      const EffectedNumTable& longEffectNumTable() const
      {
	return effects.effectedNumTable;
      }

      /** pl からの利きが(1つ以上)ある駒一覧 */
      const PieceMask effectedMask(Player pl) const
      {
	return effects.effected_mask[pl];
      }
      /** 前の指手でeffectedMask(pl)が変化したか.
       * 取られた駒は現在の実装ではリストされないようだ.
       */
      const PieceMask effectedChanged(Player pl) const
      {
	return effects.effected_changed_mask[pl];
      }
      bool hasChangedEffects() const {
	return ! effects.changedEffects(BLACK).isInvalid();
      }
      const BoardMask changedEffects(Player pl) const{
	assert(hasChangedEffects());
	return effects.changedEffects(pl);
      }
      const BoardMask changedEffects() const{
	BoardMask ret = changedEffects(BLACK);
	return ret |= changedEffects(WHITE);
      }
      const NumBitmapEffect changedPieces() const{
	return effects.changedPieces();
      }
      template <Ptype PTYPE> bool longEffectChanged() const 
      {
	return changedPieces().template hasLong<PTYPE>();
      }
      template <Ptype PTYPE> bool anyEffectChanged() const 
      {
	return changedPieces().template hasAny<PTYPE>();
      }

      /** 取られそうなPの駒で価値が最大のもの */
      const Piece findThreatenedPiece(Player P) const;

      // ----------------------------------------------------------------------
      // 2. 駒に関する情報
      // ----------------------------------------------------------------------
      bool isOnBoardNum(int num) const
      {
	return piecesOnBoard(BLACK).test(num) || piecesOnBoard(WHITE).test(num);
      }

      Square mobilityOf(Direction d,int num) const
      {
	return effects.mobilityTable.get(d,num);
      }
      Square mobilityOf(Direction d, Piece p) const 
      {
	return mobilityOf(d, p.number());
      }
      Square kingMobilityAbs(Player p, Direction d) const
      {
	return Square::makeDirect(king_mobility[p][d]);
      }
      /** 
       * 玉がd方向にどこまで動けるかを返す
       * @param p 注目する玉のプレイヤ
       * @param d piece からみた向き
       */
      Square kingMobilityOfPlayer(Player p, Direction d) const
      {
	if (p == BLACK)
	  d = inverse(d);
	return kingMobilityAbs(p, d);
      }
      /**
       * pinされた駒がPのKingから見てどの方向か?
       * Pから見たdirectionを返す
       */
      template<Player P>
      Direction pinnedDir(Piece p) const
      {
	assert(p.owner() == P);
	assert(pinOrOpen(P).test(p.number()));
	Square king=kingSquare<P>();
	return Board_Table.getShort8<P>(p.square(),king);
      }
      Direction pinnedDir(Piece p) const
      {
	if (p.owner() == BLACK)
	  return pinnedDir<BLACK>(p);
	else
	  return pinnedDir<WHITE>(p);
      }
      /**
       * pinされた駒pがtoに動けるか?
       * pinに関係がなければtoへ動けるという前提
       */
      template<Player P>
      bool pinnedCanMoveTo(Piece p,Square to) const
      {
	assert(p.owner() == P);
	Direction d=pinnedDir<P>(p);
	Square from=p.square();
	return primDir(d)==primDirUnsafe(Board_Table.getShort8Unsafe<P>(from,to));
      }
      bool pinnedCanMoveTo(Piece p,Square to) const
      {
	if (p.owner() == BLACK)
	  return pinnedCanMoveTo<BLACK>(p, to);
	else
	  return pinnedCanMoveTo<WHITE>(p, to);
      }
      /**
       * Pのpinされた駒から，そのpinの原因となっている長い利きを持つ駒を得る．
       */
      template<Player P>
      Piece pinAttacker(Piece pinned) const
      {
	assert(pinned.owner() == P);
	assert(pinOrOpen(P).test(pinned.number()));
	Direction d=pinnedDir<P>(pinned);
	int attacker_num=longEffectNumTable()[pinned.number()][(P==BLACK ? d : inverseUnsafe(d))];
	return pieceOf(attacker_num);
      }
      Piece pinAttacker(Piece pinned) const
      {
	if (pinned.owner() == BLACK)
	  return pinAttacker<BLACK>(pinned);
	else
	  return pinAttacker<WHITE>(pinned);
      }
      // ----------------------------------------------------------------------
      // 3. あるSquareへの利き
      // ----------------------------------------------------------------------
      const NumBitmapEffect effectSetAt(Square sq) const
      {
	return effects.effectSetAt(sq);
      }
      /**
       * 利きの数を数える. 
       * targetが盤をはみ出してはいけない
       */
      int countEffect(Player player,Square target) const 
      {
	assert(target.isOnBoard());
	return effectSetAt(target).countEffect(player);
      }
      /**
       * 利きの数を数える. 
       * targetが盤をはみ出してはいけない
       * @param pins この駒の利きは数えない
       */
      int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      countEffect(Player player,Square target, PieceMask pins) const 
      {
	assert(target.isOnBoard());
	const NumBitmapEffect effect = effectSetAt(target);
	const int all = effect.countEffect(player);
	pins &= effect;
	return all - pins.countBit();
      }

      // ----------------------------------------------------------------------
      // 3.1 集合を返す
      // ----------------------------------------------------------------------
      template <Ptype PTYPE>
      const mask_t allEffectAt(Player P, Square target) const 
      {
	return effectSetAt(target).template selectBit<PTYPE>() & piecesOnBoard(P).template getMask<PTYPE>();
      }
      const mask_t allEffectAt(Player P, Ptype ptype, Square target) const;
      template <Ptype PTYPE> const mask_t longEffectAt(Square target) const 
      {
	return effectSetAt(target).selectLong<PTYPE>() >> 8;
      }
      template <Ptype PTYPE> const mask_t longEffectAt(Square target, Player owner) const 
      {
	return longEffectAt<PTYPE>(target) & piecesOnBoard(owner).getMask(1);
      }
      const mask_t longEffectAt(Square target) const 
      {
	return effectSetAt(target).selectLong() >> 8;
      }
      const mask_t longEffectAt(Square target, Player owner) const 
      {
	return longEffectAt(target) & piecesOnBoard(owner).getMask(1);
      }

      // ----------------------------------------------------------------------
      // 3.2 bool を返す
      // ----------------------------------------------------------------------
      /** 
       * 対象とするマスにあるプレイヤーの利きがあるかどうか.
       * @param player 攻撃側
       * @param target 対象のマス
       */
      template<Player P>
      bool hasEffectAt(Square target) const {
	assert(target.isOnBoard());
	mask_t mask=effectSetAt(target).getMask(1);
	mask&=NumBitmapEffect::playerEffectMask<P>();
	return !mask.none();
      }
      /** 
       * 対象とするマスにあるプレイヤーの利きがあるかどうか.
       * @param player 攻撃側
       * @param target 対象のマス
       */
      bool hasEffectAt(Player player,Square target) const {
	assert(target.isOnBoard());
	mask_t mask=effectSetAt(target).getMask(1);
	mask&=NumBitmapEffect::playerEffectMask(player);
	return !mask.none();
      }
    
      /**
       * あるマスにPTYPEの長い利きがあるかどうか.
       */
      template <Ptype PTYPE>
      bool hasLongEffectAt(Player P, Square to) const {
	static_assert((PTYPE == LANCE || PTYPE == BISHOP || PTYPE == ROOK), "ptype");
	return longEffectAt<PTYPE>(to, P).any();
      }

      /**
       * target に ptype の利きがあるか? 成不成を区別しない
       */
      template <Ptype PTYPE>
      bool hasEffectByPtype(Player attack, Square target) const
      {
	return allEffectAt<PTYPE>(attack, target).any();
      }
      /**
       * target に ptype の利きがあるか? 成不成を区別
       */
      template <Ptype PTYPE>
      bool hasEffectByPtypeStrict(Player attack, Square target) const
      {
	mask_t mask=allEffectAt<PTYPE>(attack, target);
	if (isPromoted(PTYPE)) 
	  mask &= promoted.getMask<PTYPE>();
	else
	  mask &= ~(promoted.getMask<PTYPE>());
	return mask.any();
      }
      /**
       * あるマスにあるDirectionでの長い利きがあるかどうか.
       * 64bit版対応済み
       */
      template<Direction Dir,Player P>
      bool hasEffectInDirection(Square to) const {
	static_assert((DirectionTraits<Dir>::isLong), "Dir");
	const PieceMask& pieces_onboard=piecesOnBoard(P);
	mask_t mask1=pieces_onboard.getMask(1);
	mask1 &= ((PtypeDirectionTraits<LANCE,Dir>::canMove 
		   ? mask_t::makeDirect(PtypeFuns<LANCE>::indexMask) 
		   : mask_t::makeDirect(0)) 
		  | (PtypeDirectionTraits<BISHOP,Dir>::canMove 
		     ?  mask_t::makeDirect(PtypeFuns<BISHOP>::indexMask) 
		     :  mask_t::makeDirect(0))
		  | (PtypeDirectionTraits<ROOK,Dir>::canMove 
		     ? mask_t::makeDirect(PtypeFuns<ROOK>::indexMask)
		     : mask_t::makeDirect(0)));
	mask1 <<= 8;
	// 短い利きを排除
	mask1&=effectSetAt(to).getMask(1)& NumBitmapEffect::longEffectMask();
	while (mask1.any())
	{
	  int num=mask1.takeOneBit()+NumBitmapEffect::longToNumOffset;
	  Square from = pieceOf(num).square();
	  Direction dir=Board_Table.getLongDirection<BLACK>(Offset32(to,from));
	  if (dir==DirectionPlayerTraits<Dir,P>::directionByBlack)
	    return true;
	}
	return false;
      }
      /** 
       * 対象とするマスにあるプレイヤーの(ただしある駒以外)利きがあるかどうか.
       * @param player 攻撃側
       * @param piece 攻撃側の駒
       * @param target 対象のマス
       */
      bool hasEffectNotBy(Player player,Piece piece,Square target) const {
	assert(piece.owner()==player);
	PieceMask pieces_onboard=piecesOnBoard(player);
	int num=piece.number();
	pieces_onboard.reset(num);
	return (pieces_onboard&effectSetAt(target)).any();
      }
      /**
       * {pinされている駒, 玉以外}からの利きがある.
       */
      bool hasEffectByNotPinnedAndKing(Player pl,Square target) const{
	assert(target.isOnBoard());
	PieceMask m=piecesOnBoard(pl)& ~pinOrOpen(pl) & effectSetAt(target);
	m.clearBit<KING>();
	return m.any();
      }
      /**
       * pinされている駒以外からの利きがある.
       */
      bool hasEffectByNotPinned(Player pl,Square target) const{
	assert(target.isOnBoard());
	PieceMask m=piecesOnBoard(pl)& ~pinOrOpen(pl) & effectSetAt(target);
	return m.any();
      }
      /** 
       * 二つ以上の駒から利きがある.
       * そもそも利きがない場合も呼ばれる
       * @param player 攻撃側
       * @param target 対象のマス
       */
      bool hasMultipleEffectAt(Player player,Square target) const 
      {
	mask_t mask=effectSetAt(target).getMask(1);
	mask&=NumBitmapEffect::playerEffectMask(player);
	return NumBitmapEffect::playerEffect(player).getMask(1) < mask;
      }

      /** 
       * 駒attack が target に利きを持つか (旧hasEffectToと統合)
       * @param target 対象のマス
       */
      bool hasEffectByPiece(Piece attack, Square target) const 
      {
	assert(attack.isPiece());
	assert(target.isOnBoard());
	return effectSetAt(target).test(attack.number());
      }


      /**
       * attackerにptypeoの駒がいると仮定した場合にtargetに利きがあるかどうか
       * を stateをupdateしないで確かめる.
       * targetSquareは空白でも良い
       * 盤上の駒を動かす事を検討しているときはに，
       * 自分自身の陰に入って利かないと見なされることがある
       */
      bool hasEffectIf(PtypeO ptypeo,Square attacker,
			   Square target) const
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      {
	Offset32 offset32=Offset32(target,attacker);
	EffectContent effect=Ptype_Table.getEffect(ptypeo,offset32);
	if (! effect.hasEffect()) 
	  return false;
	if (effect.hasUnblockableEffect())
	  return true;
	assert(Board_Table.getShortOffset(offset32) == effect.offset());
	return this->isEmptyBetween(attacker,target,effect.offset());
      }
      /**
       * 
       */
      template<Player P>
      bool
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      hasEffectByWithRemove(Square target,Square removed) const;
    
      bool hasEffectByWithRemove(Player player, Square target,Square removed) const{
	if (player==BLACK)
	  return hasEffectByWithRemove<BLACK>(target,removed);
	else
	  return hasEffectByWithRemove<WHITE>(target,removed);
      }


      // ----------------------------------------------------------------------
      // 3.3 pieceを探す
      // ----------------------------------------------------------------------
      /** return a piece s.t. owner == attack, ptype == PTYPE, has effect on target.  return Piece::EMPTY() otherwise */
      template <Ptype PTYPE>
      const Piece findAttackAt(Player attack, Square target) const
      {
	mask_t mask=allEffectAt<PTYPE>(attack, target);
	if (mask.none())
	  return Piece::EMPTY();
	return pieceOf(mask.takeOneBit()+PtypeFuns<PTYPE>::indexNum*32);
      }
      template <Ptype PTYPE>
      const Piece findAttackAtStrict(Player attack, Square target) const
      {
	mask_t mask=allEffectAt<PTYPE>(attack, target);
	if (isPromoted(PTYPE)) 
	  mask &= promoted.getMask<PTYPE>();
	else
	  mask &= ~(promoted.getMask<PTYPE>());
	if (mask.none())
	  return Piece::EMPTY();
	return pieceOf(mask.takeOneBit()+PtypeFuns<PTYPE>::indexNum*32);
      }
      /** 
       * pieceのd方向から長い利きがある場合にその駒を返す。
       * @param d piece からみた向き
       */
      const Piece findLongAttackAt(Player owner, int piece, Direction d) const
      {
	assert(pieceOf(piece).isOnBoardByOwner(owner));
	if (owner == BLACK)
	  d = inverse(d);
	const int num = effects.effectedNumTable[piece][d];
	if (num == EMPTY_NUM)
	  return Piece::EMPTY();
	return pieceOf(num);
      }
      const Piece findLongAttackAt(Player owner, Piece piece, Direction d) const
      {
	assert(piece.isPiece());
	assert(piece.owner() == owner);
	return findLongAttackAt(owner, piece.number(), d);
      }
      const Piece findLongAttackAt(Piece piece, Direction d) const
      {
	assert(piece.isPiece());
	return findLongAttackAt(piece.owner(), piece, d);
      }
      const Piece findLongAttackAt(Square square, Direction d) const
      {
	return findLongAttackAt(pieceOnBoard(square), d);
      }
      /**
       * 利きの中から安そうな駒を選ぶ
       */
      const Piece selectCheapPiece(PieceMask effect) const;
      /**
       * @param P - 利きをつけている側のプレイヤ
       * @param square - 調査する場所
       * @return 利きを付けている中で安そうな駒 (複数の場合でもEMPTYにはしない)
       */
      const Piece findCheapAttack(Player P, Square square) const 
      {
	return selectCheapPiece(piecesOnBoard(P) & effectSetAt(square));
      }
      /**
       * @param P - 利きをつけている側のプレイヤ
       * @param square - 調査する場所
       * @return 利きを付けている中で安そうな駒 (複数の場合でもEMPTYにはしない)
       */
      const Piece findCheapAttackNotBy(Player P, Square square, const PieceMask& ignore) const 
      {
	PieceMask pieces = piecesOnBoard(P);
	pieces &= ~ignore;
	return selectCheapPiece(pieces & effectSetAt(square));
      }
      const Piece findAttackNotBy(Player P, Square square, const PieceMask& ignore) const 
      {
	PieceMask pieces = piecesOnBoard(P);
	pieces &= ~ignore;
	pieces &= effectSetAt(square);
	if (pieces.none())
	  return Piece::EMPTY();
	return pieceOf(pieces.takeOneBit());
      }
      /**
       * 王手駒を探す
       * @return 王手かどうか
       * @param attack_piece
       * 一つの駒による王手の場合はattck_pieceにその駒を入れる
       * 複数の駒による王手の場合はPiece::EMPTY()を入れる
       * @param P(template) 玉
       */
      template<Player P>
      bool findCheckPiece(Piece& attack_piece) const
      {
	return hasEffectAt<alt(P)>(kingSquare(P),attack_piece);
      }
      bool hasEffectAt(Player P, Square target,Piece& attackerPiece) const 
      {
	if (P == BLACK)
	  return hasEffectAt<BLACK>(target, attackerPiece);
	else
	  return hasEffectAt<WHITE>(target, attackerPiece);
      }
      /**
       * @param P(template) - 利きをつけている側のプレイヤ
       * @param target - 利きをつけられた場所
       * @param attackerPiece - multiple attackの場合はPiece::EMPTY()
       *        そうでないなら利きをつけている駒を返す
       */
      template<Player P>
      bool hasEffectAt(Square target,Piece& attackerPiece) const {
	attackerPiece=Piece::EMPTY();
	const PieceMask& pieceMask=piecesOnBoard(P)&effectSetAt(target);
	mask_t mask=pieceMask.getMask(0);
	if (mask.none()) return false;
	/**
	 * mask|=0x8000000000000000ll;
	 * if((mask&(mask-1))!=0x8000000000000000ll) なら1つのif文で済む
	 */
	if (mask.hasMultipleBit())
	  return true;
	int num=mask.bsf();
	attackerPiece=pieceOf(num);
	return true;
      }

      // ----------------------------------------------------------------------
      // 4. 指手の検査・生成・適用
      // ----------------------------------------------------------------------
      // --- 2014/03
      bool isSafeMove(Move move) const;
      bool isCheck(Move move) const;
      bool isPawnDropCheckmate(Move move) const;
      bool isDirectCheck(Move move) const;
      bool isOpenCheck(Move move) const;
      // ---

      /**
       * 合法手かどうかを簡単に検査する．局面に依存するチェックのみ．
       * ルール上指せない手である可能性がある場合は，isValidMove を用いる．
       *
       * Pをtemplate引数にできると少しは速くなりそう
       * 局面に依存する検査でも，玉の素抜きや王手を防いでいるか，
       * 千日手，打歩詰かどうかは検査しない．
       * @param showError(template) - falseを返す時には何か表示する.
       * @param move - 手
       */
      template <bool show_error>
      bool isAlmostValidMove(Move move) const;
      bool isAlmostValidMove(Move move,bool show_error=true) const;

      /**
       * 全ての合法手を生成する. 玉の素抜きや打歩詰の確認をする．
       * ただし, 打歩詰め絡み以外では有利にはならない手
       * （Move::ignoredUnpromote）は生成しない.
       */
      void generateLegal(MoveVector&) const;
      /**
       * 打歩詰め絡み以外では有利にはならない手も含め, 全ての合法手を生成す 
       * る（Move::ignoredUnpromoteも生成する）. 玉の素抜きや打歩詰の確認
       * をする．
       */
      void generateWithFullUnpromotions(MoveVector&) const;
      /** 自殺を含めてすべての手を生成 */
      void generateAllUnsafe(MoveVector&) const;

      void makeMove(Move move);
      void makeMovePass()
      {
	changeTurn();
	effects.clearChangedEffects();
	effects.clearEffectedChanged();
      }

      template <class Function>
      void makeUnmakePass(Function &f)
      {
	changeTurn();
	f(Square::STAND());
	changeTurn();
      }
      template <class Function>
      void makeUnmakeMove(Move move, Function &f)
      {
	if (move.player() == BLACK)
	  makeUnmakeMove(Player2Type<BLACK>(), move, f);
	else
	  makeUnmakeMove(Player2Type<WHITE>(), move, f);
      }
      template <Player P, class Function>
      void makeUnmakeMove(Player2Type<P> player, Move move, Function &f)
      {
	if (move.isPass())
	  return makeUnmakePass(f);
	assert(move.isValid());
	assert(isAlmostValidMove(move));
	assert(P == move.player());
	assert(P == turn());
	Square from=move.from();
	Square to=move.to();
	if (from.isPieceStand())
	{
	  assert(pieceAt(to) == Piece::EMPTY());
	  doUndoDropMove(player,to,move.ptype(),f);
	}
	else
	{
	  assert(pieceAt(from) != Piece::EMPTY());
	  Piece captured=pieceAt(to);
	  if (captured != Piece::EMPTY())
	  {
	    doUndoCaptureMove(player,from,to,captured,move.promoteMask(),f);
	  }
	  else
	  {
	    doUndoSimpleMove(player,from,to,move.promoteMask(),f);
	  }
	}
      }
      bool wasCheckEvasion(Move last_move) const;
      // ----------------------------------------------------------------------
      // 5. forEachXXX
      // ----------------------------------------------------------------------
      /** T は isBasic でなくても動くが unpromote(T) の結果と同じ.
       */ 
      template<Player P,Ptype T,typename F>
      void forEachOnBoard(F& func) const {
	mask_t onMask=piecesOnBoard(P).template selectBit<T>() ;    
	while (onMask.any())
	{
	  int num=onMask.takeOneBit()+((PtypeFuns<T>::indexNum)<<5);
	  Piece p = pieceOf(num);
	  func(p);
	}
      }
      /** T の成不成を区別
       */ 
      template<Player P,Ptype T,typename F>
      void forEachOnBoardPtypeStrict(F& func) const 
      {
	mask_t mask=piecesOnBoard(P).template selectBit<T>() ;    
	if (isPromoted(T)) 
	  mask &= promoted.getMask<T>();
	else
	  mask &= ~(promoted.getMask<T>());
	while (mask.any())
	{
	  int num=mask.takeOneBit()+((PtypeFuns<T>::indexNum)<<5);
	  func(pieceOf(num));
	}
      }
    private:
      template<Player P,class Action>
      void forEachEffect(const PieceMask& pieces, Square sq,Action & action) const
      {
#if OSL_WORDSIZE == 64
	mask_t mask=pieces.getMask(0);
	while (mask.any())
	{
	  const int num=mask.takeOneBit();
	  action.template doAction<P>(pieceOf(num),sq);
	}
#elif OSL_WORDSIZE == 32
	mask_t mask0=pieces.getMask(0);
	while (mask0.any())
	{
	  const int num=mask0.takeOneBit();
	  action.template doAction<P>(pieceOf(num),sq);
	}
	mask_t mask1=pieces.getMask(1);
	while (mask1.any())
	{
	  const int num=mask1.takeOneBit()+32;
	  action.template doAction<P>(pieceOf(num),sq);
	}
#endif
      }      
    public:
      /** 
       * sq への利きを持つ各駒に関して処理を行う.
       * @param action たとえば AlwaysMoveAction
       */
      template<Player P,class Action>
      void forEachEffect(Square sq,Action & action) const
      {
	const PieceMask pieceMask=piecesOnBoard(P)&effectSetAt(sq);
	forEachEffect<P,Action>(pieceMask, sq, action);
      }
      /** 
       * sq にある駒を取る move を生成して action の member を呼び出す.
       * @param pin 無視する駒
       * @param action たとえば AlwaysMoveAction
       */
      template<Player P,class Action>
      void forEachEffect(Square sq,Action & action,const PieceMask& pin) const
      {
	PieceMask pieceMask=piecesOnBoard(P)&effectSetAt(sq);
	pieceMask &= ~pin;
	forEachEffect<P,Action>(pieceMask, sq, action);
      }

      /** 
       * sq に移動する move を生成して action の member を呼び出す
       * @param action たとえば AlwayMoveAction
       * @param piece  これ以外の駒を使う
       */
      template<Player P,class Action>
      void forEachEffectNotBy(Square sq,Piece piece,Action & action) const {
	PieceMask pieces=piecesOnBoard(P)&effectSetAt(sq);
	pieces.reset(piece.number());
	forEachEffect<P,Action>(pieces, sq, action);
      }

    private:
      template<Player P,Ptype Type,class Action,Direction Dir>
      void forEachEffectOfPieceDir(Square, Action&, Int2Type<false>) const {}
      template<Player P,Ptype Type,class Action,Direction Dir>
      void forEachEffectOfPieceDir(Square pieceSquare,Action & action,Int2Type<true>) const {
	const Offset offset=DirectionPlayerTraits<Dir,P>::offset();
	action.template doAction<P>(this->pieceAt(pieceSquare),pieceSquare+offset);
      }

      template<Player P,Ptype Type,class Action,Direction Dir>
      void forEachEffectOfPieceDir(Square pieceSquare,Action & action) const {
	forEachEffectOfPieceDir<P,Type,Action,Dir>(pieceSquare,action,Int2Type<(PtypeTraits<Type>::moveMask & DirectionTraits<Dir>::mask)!=0>());
      }
      template<Player P,Ptype Type,class Action,Direction Dir>
      void forEachEffectOfPieceLongDir(Square, Action&, Int2Type<false>) const {}
      template<Player P,Ptype Type,class Action,Direction Dir>
      void forEachEffectOfPieceLongDir(Square pieceSquare,Action & action,Int2Type<true>) const {
	Piece piece=this->pieceAt(pieceSquare);
	const Offset offset=DirectionPlayerTraits<Dir,P>::offset();
	assert(offset.intValue() != 35);
	Square sq=pieceSquare+offset;
	for (;this->pieceAt(sq).isEmpty();sq+=offset)
	  action.template doAction<P>(piece,sq);
	action.template doAction<P>(piece,sq);
      }

      template<Player P,Ptype Type,class Action,Direction Dir>
      void forEachEffectOfPieceLongDir(Square pieceSquare,Action & action) const {
	forEachEffectOfPieceLongDir<P,Type,Action,Dir>(pieceSquare,action,Int2Type<(PtypeTraits<Type>::moveMask & DirectionTraits<Dir>::mask)!=0>());
      }
    public:
      /**
       * pieceSquareにある駒によって利きを受けるすべてのsquare
       * (空白含む)について
       * actionを実行する
       *
       * SimpleState より移植: ImmediateAddSupportがなければ消せる
       */
      template<Player P,Ptype Type,class Action>
      void forEachEffectOfPiece(Square pieceSquare,Action & action) const;
      template<class Action>
      void forEachEffectOfPiece(Piece piece,Action & action) const;

      /**
       * PtypeO が Square にいると仮定した時にの利きを列挙.
       * 盤面が実際と違うと長い利きが不正確になる
       * @param InterestEmpty 空白のマスに興味があるか
       */
      template <class Function, bool InterestEmpty>
      void forEachEffectOfPtypeO(Square, PtypeO, Function& f) const;
      template <Player P, class Function, bool InterestEmpty>
      void forEachEffectOfPtypeO(Square, Ptype, Function& f) const;
    
      /**
       * 玉の素抜きなしに合法手でtargetに移動可能かを判定
       * @param king 玉 (玉で取る手は考えない)
       * @return 移動可能な駒があれば，安全な駒を一つ．なければ Piece::EMPTY()
       * @see osl::move_classifier::PawnDropCheckmate
       */
      template <Player P>
      Piece safeCaptureNotByKing(Square target, Piece king) const;
      Piece safeCaptureNotByKing(Player P, Square target) const {
	const Piece king = kingPiece(P);
	if (P == BLACK)
	  return this->safeCaptureNotByKing<BLACK>(target, king);
	else
	  return this->safeCaptureNotByKing<WHITE>(target, king);
      }
      /**
       * forEachEffect の Player のtemplate 引数を通常の引数にしたバージョン
       * @param P 探す対象の駒の所有者
       * @param pos に利きのある駒を探す
       */
      template <class Action>
      void forEachEffect(Player P, Square pos, Action& a) const {
	if (P == BLACK)
	  this->template forEachEffect<BLACK>(pos,a);
	else
	  this->template forEachEffect<WHITE>(pos,a);
      }
      /**
       * target に利きのあるPieceをoutに格納する
       */
      void findEffect(Player P, Square target, PieceVector& out) const;

    private:
      void doSimpleMove(Square from, Square to, int promoteMask);
      void doDropMove(Square to,Ptype ptype);
      void doCaptureMove(Square from, Square to, Piece target,int promoteMask);

      template <Player P, class Function>
      void doUndoSimpleMove(Player2Type<P> player,
			    Square from, Square to, int promoteMask,Function& func);
      template <Player P>
      void prologueSimple(Player2Type<P>, Square from, Square to, int promoteMask,
			  Piece& oldPiece, int& num, 
			  PtypeO& oldPtypeO, PtypeO& new_ptypeo,
			  CArray<PieceMask,2>& pin_or_open_backup,
			  KingMobility& king_mobility_backup,
			  PieceMask& promoted_backup,
			  CArray<PieceMask,2>& effected_mask_backup,
			  CArray<PieceMask,2>& effected_changed_mask_backup,
			  CArray<uint64_t,2>& king8infos_backup,
			  MobilityTable &mobility_backup
	);
      void epilogueSimple(Square from, Square to, Piece oldPiece, 
			  int num, PtypeO oldPtypeO, PtypeO newPtypeO,
			  const CArray<PieceMask,2>& pin_or_open_backup,
			  const KingMobility& king_mobility_backup,
			  const PieceMask& promoted_backup,
			  const CArray<PieceMask,2>& effected_mask_backup,
			  const CArray<PieceMask,2>& effected_changed_mask_backup,
			  const CArray<uint64_t,2>& king8infos_backup,
			  const MobilityTable &mobility_backup
	);
      template <Player P, class Function>
      void doUndoDropMove(Player2Type<P> player,
			  Square to, Ptype ptype, Function& func);
      template <Player P>
      void prologueDrop(Player2Type<P>, Square to, Ptype ptype,
			Piece& oldPiece, int& num, PtypeO& ptypeO, 
			int& numIndex, mask_t& numMask,
			CArray<PieceMask,2>& pin_or_open_backup,
			KingMobility& king_mobility_backup,
			CArray<PieceMask,2>& effected_mask_backup,
			CArray<PieceMask,2>& effected_changed_mask_backup,
			CArray<uint64_t,2>& king8infos_backup,
			MobilityTable &mobility_backup);
      template<Player P>
      void epilogueDrop(Player2Type<P>, Square to, Ptype ptype, Piece oldPiece, 
			int num, PtypeO ptypeO, int numIndex, mask_t numMask,
			const CArray<PieceMask,2>& pin_or_open_backup,
			const KingMobility& king_mobility_backup,
			const CArray<PieceMask,2>& effected_mask_backup,
			const CArray<PieceMask,2>& effected_changed_mask_backup,
			const CArray<uint64_t,2>& king8infos_backup,
			const MobilityTable &mobility_backup);
      template <Player P, class Function>
      void doUndoCaptureMove(Player2Type<P> player, Square from,Square to, 
			     Piece target, int promoteMask,Function& func);

      template<Player P>
      void prologueCapture(Player2Type<P>, Square from, Square to, Piece target, 
			   int promoteMask,
			   Piece& oldPiece, PtypeO& oldPtypeO, PtypeO& capturePtypeO, 
			   PtypeO& new_ptypeo, int& num0, int& num1, 
			   int& num1Index, mask_t& num1Mask,
			   CArray<PieceMask,2>& pin_or_open_backup,
			   KingMobility& king_mobility_backup,
			   PieceMask& promoted_backup,
			   CArray<PieceMask,2>& effected_mask_backup,
			   CArray<PieceMask,2>& effected_changed_mask_backup,
			   CArray<uint64_t,2>& king8infos_backup,
			   MobilityTable &mobility_backup);

      template<Player P>
      void epilogueCapture(Player2Type<P>, Square from, Square to, Piece target, 
			   Piece oldPiece, PtypeO oldPtypeO, PtypeO capturePtypeO, 
			   PtypeO newPtypeO, int num0, int num1, 
			   int num1Index, mask_t num1Mask,
			   const CArray<PieceMask,2>& pin_or_open_backup,
			   const KingMobility& king_mobility_backup,
			   const PieceMask& promoted_backup,
			   const CArray<PieceMask,2>& effected_mask_backup,
			   const CArray<PieceMask,2>& effected_changed_mask_backup,
			   const CArray<uint64_t,2>& king8infos_backup,
			   const MobilityTable &mobility_backup);
      // 
      template<Direction DIR>
      void makePinOpenDir(Square target,
			  PieceMask& pins, PieceMask const& onBoard,Player defense)
      {
	const Offset offset = DirectionTraits<DIR>::blackOffset();
	Square sq=target-offset;
	int num;
	while(Piece::isEmptyNum(num=pieceAt(sq).number()))
	  sq-=offset;
	king_mobility[defense][DIR]=static_cast<unsigned char>(sq.uintValue());
	if(Piece::isEdgeNum(num)) return;
	int num1=longEffectNumTable()[num][DIR];
	if(Piece::isPieceNum(num1) && onBoard.test(num1)){
	  pins.set(num);
	}
      }
      void recalcPinOpen(Square changed, Direction &lastDir, Player defense)
      {
	Square target=kingSquare(defense);
#ifdef ALLOW_KING_ABSENCE
	if (target.isPieceStand())
	  return;
#endif
	const Direction longD=Board_Table.getLongDirection<BLACK>(changed,target);
	if(!isLong(longD) || (lastDir!=UL && longD==lastDir)) return;
	lastDir=longD;
	Direction shortD=longToShort(longD);
	{
	  // reset old pins
	  Square oldPos=Square::makeDirect(king_mobility[defense][shortD]);
	  int oldNum=pieceAt(oldPos).number();
	  if(Piece::isPieceNum(oldNum))
	    pin_or_open[defense].reset(oldNum);
	}
	const Offset offset = Board_Table.getOffsetForBlack(longD);
	Square sq=target-offset;
	int num;
	while(Piece::isEmptyNum(num=pieceAt(sq).number()))
	  sq-=offset;
	king_mobility[defense][shortD]=static_cast<unsigned char>(sq.uintValue());
	if(Piece::isEdgeNum(num)) return;
	int num1=longEffectNumTable()[num][shortD];
	if(Piece::isPieceNum(num1) && piecesOnBoard(alt(defense)).test(num1)){
	  pin_or_open[defense].set(num);
	}
      }
      PieceMask makePinOpen(Square target,Player defense);
      void makePinOpen(Player defense);
      template<Player P>
      void makeKing8Info();
    };

    inline bool operator!=(const NumEffectState& s1, const NumEffectState& s2)
    {
      return !(s1==s2);
    }  
} // namespace osl

template <osl::Player P, typename Function>
void osl::NumEffectState::
doUndoSimpleMove(Player2Type<P> player,
		 Square from, Square to, int promoteMask, Function& func)
{
  Piece oldPiece;
  int num;
  PtypeO oldPtypeO, newPtypeO;
  CArray<PieceMask,2> pin_or_open_backup;
  KingMobility king_mobility_backup;
  PieceMask promoted_backup;
  CArray<PieceMask,2> effected_mask_backup;
  CArray<PieceMask,2> effected_changed_mask_backup;
  CArray<uint64_t,2> king8infos_backup;
  MobilityTable mobility_backup;
  prologueSimple(player, from, to, promoteMask, oldPiece, num, oldPtypeO, newPtypeO, 
		 pin_or_open_backup, 
		 king_mobility_backup,
		 promoted_backup,
		 effected_mask_backup, effected_changed_mask_backup,
		 king8infos_backup,
		 mobility_backup);
  if (promoteMask!=0 && num < PtypeTraits<PAWN>::indexLimit)
  {
    clearPawn(P,from);
    changeTurn();
    func(to);
    changeTurn();
    setPawn(P,from);
  }
  else
  {
    changeTurn();
    func(to);
    changeTurn();
  }
  epilogueSimple(from, to, oldPiece, num, oldPtypeO, newPtypeO, 
		 pin_or_open_backup, 
		 king_mobility_backup,
		 promoted_backup, effected_mask_backup, effected_changed_mask_backup,
		 king8infos_backup,
		 mobility_backup);
}

template <osl::Player P, typename Function>
void osl::NumEffectState::doUndoDropMove(Player2Type<P> player,
				    Square to, Ptype ptype, Function& func)
{
  Piece oldPiece;
  PtypeO ptypeO;
  int num, numIndex;
  mask_t numMask;
  CArray<PieceMask,2> pin_or_open_backup;
  KingMobility king_mobility_backup;
  CArray<PieceMask,2> effected_mask_backup;
  CArray<PieceMask,2> effected_changed_mask_backup;
  CArray<uint64_t,2> king8infos_backup;
  MobilityTable mobility_backup;
  prologueDrop(player, to, ptype, oldPiece, num, ptypeO, numIndex, numMask, 
	       pin_or_open_backup, king_mobility_backup,
	       effected_mask_backup,effected_changed_mask_backup,
	       king8infos_backup,
	       mobility_backup);
  if (ptype==PAWN)
  {
    setPawn(P,to);
    changeTurn();
    func(to);
    changeTurn();
    clearPawn(P,to);
  }
  else
  {
    changeTurn();
    func(to);
    changeTurn();
  }
  epilogueDrop(player, to, ptype, oldPiece, num, ptypeO, numIndex, numMask, 
	       pin_or_open_backup, king_mobility_backup,
	       effected_mask_backup,effected_changed_mask_backup,
	       king8infos_backup,
	       mobility_backup);
}

template <osl::Player P, typename Function>
void osl::NumEffectState::doUndoCaptureMove(Player2Type<P> player,
				       Square from,Square to, Piece target, 
				       int promoteMask,Function& func)
{
  Piece oldPiece;
  PtypeO oldPtypeO, capturePtypeO, newPtypeO;
  int num0, num1, num1Index;
  mask_t num1Mask;
  CArray<PieceMask,2> pin_or_open_backup;
  KingMobility king_mobility_backup;
  PieceMask promoted_backup;
  CArray<PieceMask,2> effected_mask_backup;
  CArray<PieceMask,2> effected_changed_mask_backup;
  CArray<uint64_t,2> king8infos_backup;
  MobilityTable mobility_backup;
  prologueCapture(player, from, to, target, promoteMask, oldPiece, oldPtypeO, 
		  capturePtypeO, newPtypeO, num0, num1, num1Index,num1Mask, 
		  pin_or_open_backup, king_mobility_backup,
		  promoted_backup,
		  effected_mask_backup, effected_changed_mask_backup,
		  king8infos_backup,
		  mobility_backup);

  changeTurn();
  const Ptype capturePtype=target.ptype();
  if (capturePtype==PAWN)
  {
    clearPawn(alt(P),to);
    if (promoteMask!=0 && num0<PtypeTraits<PAWN>::indexLimit)
    {
      clearPawn(P,from);
      func(to);
      setPawn(P,from);
    }
    else
    {
      func(to);
    }
    setPawn(alt(P),to);
  }
  else if (promoteMask!=0 && num0<PtypeTraits<PAWN>::indexLimit)
  {
    clearPawn(P,from);
    func(to);
    setPawn(P,from);
  }
  else
  {
    func(to);
  }
  changeTurn();

  epilogueCapture(player, from, to, target, oldPiece, oldPtypeO, capturePtypeO, newPtypeO, 
		  num0, num1, num1Index,num1Mask, 
		  pin_or_open_backup, king_mobility_backup,
		  promoted_backup,effected_mask_backup, effected_changed_mask_backup,
		  king8infos_backup,
		  mobility_backup);
}

template <class Action>
void osl::NumEffectState::
forEachEffectOfPiece(Piece piece,Action & action) const 
{
  Square pieceSquare = piece.square();
  switch ((int)piece.ptypeO()) {
  case NEW_PTYPEO(WHITE,PAWN): forEachEffectOfPiece<WHITE,PAWN,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,LANCE): forEachEffectOfPiece<WHITE,LANCE,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,KNIGHT): forEachEffectOfPiece<WHITE,KNIGHT,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,SILVER): forEachEffectOfPiece<WHITE,SILVER,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,PPAWN): forEachEffectOfPiece<WHITE,PPAWN,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,PLANCE): forEachEffectOfPiece<WHITE,PLANCE,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,PKNIGHT): forEachEffectOfPiece<WHITE,PKNIGHT,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,PSILVER): forEachEffectOfPiece<WHITE,PSILVER,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,GOLD): forEachEffectOfPiece<WHITE,GOLD,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,BISHOP): forEachEffectOfPiece<WHITE,BISHOP,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,PBISHOP): forEachEffectOfPiece<WHITE,PBISHOP,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,ROOK): forEachEffectOfPiece<WHITE,ROOK,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,PROOK): forEachEffectOfPiece<WHITE,PROOK,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(WHITE,KING): forEachEffectOfPiece<WHITE,KING,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,PAWN): forEachEffectOfPiece<BLACK,PAWN,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,LANCE): forEachEffectOfPiece<BLACK,LANCE,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,KNIGHT): forEachEffectOfPiece<BLACK,KNIGHT,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,SILVER): forEachEffectOfPiece<BLACK,SILVER,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,PPAWN): forEachEffectOfPiece<BLACK,PPAWN,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,PLANCE): forEachEffectOfPiece<BLACK,PLANCE,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,PKNIGHT): forEachEffectOfPiece<BLACK,PKNIGHT,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,PSILVER): forEachEffectOfPiece<BLACK,PSILVER,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,GOLD): forEachEffectOfPiece<BLACK,GOLD,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,BISHOP): forEachEffectOfPiece<BLACK,BISHOP,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,PBISHOP): forEachEffectOfPiece<BLACK,PBISHOP,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,ROOK): forEachEffectOfPiece<BLACK,ROOK,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,PROOK): forEachEffectOfPiece<BLACK,PROOK,Action>(pieceSquare,action); break;
  case NEW_PTYPEO(BLACK,KING): forEachEffectOfPiece<BLACK,KING,Action>(pieceSquare,action); break;
  default: assert(0);
  }
}

template <osl::Player P, osl::Ptype Type, class Action>
void osl::NumEffectState::
forEachEffectOfPiece(Square pieceSquare,Action & action) const 
{
  forEachEffectOfPieceDir<P,Type,Action,UL>(pieceSquare,action);
  forEachEffectOfPieceDir<P,Type,Action,U>(pieceSquare,action);
  forEachEffectOfPieceDir<P,Type,Action,UR>(pieceSquare,action);
  forEachEffectOfPieceDir<P,Type,Action,L>(pieceSquare,action);
  forEachEffectOfPieceDir<P,Type,Action,R>(pieceSquare,action);
  forEachEffectOfPieceDir<P,Type,Action,DL>(pieceSquare,action);
  forEachEffectOfPieceDir<P,Type,Action,D>(pieceSquare,action);
  forEachEffectOfPieceDir<P,Type,Action,DR>(pieceSquare,action);
  forEachEffectOfPieceDir<P,Type,Action,UUL>(pieceSquare,action);
  forEachEffectOfPieceDir<P,Type,Action,UUR>(pieceSquare,action);
  forEachEffectOfPieceLongDir<P,Type,Action,LONG_UL>(pieceSquare,action);
  forEachEffectOfPieceLongDir<P,Type,Action,LONG_U>(pieceSquare,action);
  forEachEffectOfPieceLongDir<P,Type,Action,LONG_UR>(pieceSquare,action);
  forEachEffectOfPieceLongDir<P,Type,Action,LONG_L>(pieceSquare,action);
  forEachEffectOfPieceLongDir<P,Type,Action,LONG_R>(pieceSquare,action);
  forEachEffectOfPieceLongDir<P,Type,Action,LONG_DL>(pieceSquare,action);
  forEachEffectOfPieceLongDir<P,Type,Action,LONG_D>(pieceSquare,action);
  forEachEffectOfPieceLongDir<P,Type,Action,LONG_DR>(pieceSquare,action);
}

#include "osl/bits/pieceStand.h"
#endif /* OSL_NUM_EFFECT_STATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
