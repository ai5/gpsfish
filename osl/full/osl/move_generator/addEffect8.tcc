#ifndef _MOVE_GENERATOR_ADD_EFFECT8_TCC
#define _MOVE_GENERATOR_ADD_EFFECT8_TCC
#include "osl/move_generator/addEffect8.h"
#include "osl/move_generator/pieceOnBoard.h"
#include "osl/move_generator/addEffect8Table.h"

namespace osl
{
  namespace move_generator
  {
    namespace addeffect8{
      /**
       * 5x5領域への駒のdropによる手生成.
       * BISHOPやROOKは敵の利きのない場合のみ生成
       * -> pinned以外の利きに変更(2009/12/24)
       * @param P(template) - 手番のプレイヤー
       * @param T(template) - 置こうとする駒の種類(当然basic)
       * @param state - 盤面
       * @param target - 相手の玉のposition(redundantだが)
       * @param action - 手生成のcallback
       */
      template <Player P,Ptype T,class Action>
      void generateShortDrop(NumEffectState const& state,Square target,Action& action)
      {
	Square targetForP=target.squareForBlack(P);
	for(int i=0;;i++){
	  Square pos=Add_Effect8_Table.getDropSquare(T,targetForP,i);
	  if(pos.isPieceStand()) break;
	  pos=pos.squareForBlack<P>();
	  if(state.pieceAt(pos).isEmpty() &&
	     (T!=PAWN || !state.isPawnMaskSet<P>(pos.x()))){
	    if((T==BISHOP || T==ROOK) && 
	       state.hasEffectByNotPinnedAndKing(alt(P),pos)) continue;
	    action.dropMove(pos,T,P);
	  }
	}
      }

      /**
       * 方向を決めて，long dropを作成する．
       * @param P(template) - 手番のプレイヤー
       * @param T(template) - 置こうとする駒の種類(当然basic)
       * @param state - 盤面
       * @param to - 利きを追加する点
       * @param from - 置き始める点
       * @param offset - 次に置く点へのoffset
       * @param action - 手生成のcallback
       */
      template <Player P,Ptype T,class Action>
      void generateLongDropWithOffset(NumEffectState const& state,Square to,Square from, Offset offset,int countMax,Action& action){
	const Player altP=alt(P);
	int count=0;
	for(;;){
	  Piece p;
	  for(;(p=state.pieceAt(from)).isEmpty();from+=offset){
	    if(T==LANCE)
	      action.dropMove(from,T,P);
	    else if(!state.hasEffectAt<altP>(from)){
	      action.dropMove(from,T,P);
	      if(++count>=countMax) return;
	    }
	  }
	  if(!p.isOnBoardByOwner<P>()) return;
	  if(!Ptype_Table.getEffect(p.ptypeO(),from,to).hasEffect()) return;
	  from+=offset;
	}
      }
      
      /**
       * 長い利きを持つ駒のdropによる手生成.
       * 追加利きも生成する.
       * @param P(template) - 手番のプレイヤー
       * @param T(template) - 置こうとする駒の種類(当然basic)
       * @param state - 盤面
       * @param target - 相手の玉のposition(redundantだが)
       * @param action - 手生成のcallback
       */
      template <Player P,Ptype T,class Action>
      void generateLongDrop(NumEffectState const& state,Square target,Action& action)
      {
	static_assert(T==ROOK || T==BISHOP || T==LANCE, "");
	// 8近傍以内へのdropはshort扱いで作る．
	generateShortDrop<P,T,Action>(state,target,action);
	Square targetForP=target.squareForBlack(P);
	// 8近傍に駒がある時(王手にならない)に直接方向の利き
	for(int i=0;;i++){
	  Offset offset=Add_Effect8_Table.getLongDropDirect(T,targetForP,i);
	  if(offset.zero()) break;
	  offset=offset.blackOffset<P>();
	  Square pos=target+offset;
	  assert(pos.isOnBoard());
	  Square from=pos+offset;
	  if(state.pieceAt(pos).isEmpty()){
	    Piece p=state.pieceAt(from);
	    // 一つ置いて短い利きの時のみOK
	    if(!p.isOnBoardByOwner<P>() ||
	       !state.hasEffectByPiece(p,pos)) continue;
	    from+=offset;
	  }
	  generateLongDropWithOffset<P,T,Action>(state,pos,from,offset,2,action);
	}
	for(int i=0;;i++){
	  PO po=Add_Effect8_Table.getLongDropSquare(T,targetForP,i);
	  if(po.first.isPieceStand()) break;
	  Square pos=po.first.squareForBlack<P>();
	  Offset offset=po.second.blackOffset<P>();
	  assert(!offset.zero());
	  generateLongDropWithOffset<P,T,Action>(state,pos-offset,pos,offset,2,action);
	}
	if(T==BISHOP){
	  for(int i=0;;i++){
	    POO poo=Add_Effect8_Table.getLongDrop2Square(T,targetForP,i);
	    if(poo.first.isPieceStand()) break;
	    Square pos=poo.first.squareForBlack<P>();
	    Offset offset1=poo.second.first.blackOffset<P>();
	    assert(!offset1.zero());
	    Offset offset2=poo.second.second.blackOffset<P>();
	    assert(!offset2.zero());
	    Piece p=state.pieceAt(pos);
	    if(p.isEmpty()){
	      generateLongDropWithOffset<P,T,Action>(state,pos-offset1,
						     pos,offset1,2,action);
	      generateLongDropWithOffset<P,T,Action>(state,pos-offset2,
						     pos+offset2,offset2,1,
						     action);
	    }
	    else if(p.isOnBoardByOwner<P>()){
	      if(state.hasEffectByPiece(p,pos-offset1)){
		generateLongDropWithOffset<P,T,Action>(state,pos-offset1,
						       pos+offset1,offset1,
						       2,action);
	      }
	      if(state.hasEffectByPiece(p,pos-offset2)){
		generateLongDropWithOffset<P,T,Action>(state,pos-offset2,
						       pos+offset2,offset2,
						       2,action);
	      }
	    }
	  }
	}
      }
      /**
       * unblockableな動きでunblockableな利きをつける手生成.
       * @param P(template) - 攻撃側のプレイヤー
       * @param T(template) - 攻撃側の駒の種類
       * @param state - 局面
       * @param from - 攻撃しようとする駒の位置
       * @param target - 攻撃しようとするマス
       */
      template <Player P,Ptype T,class Action>
      void generateShortMove(NumEffectState const& state,Piece attacker,
			     Square target, Action& action)
      {
	Square from=attacker.square();
	Offset32 o32=Offset32(from,target).blackOffset32<P>();
	for(int i=0;;i++){
	  Offset o=Add_Effect8_Table.getShortMoveOffset(false,T,o32,i);
	  if(o.zero()) break;
	  Square pos=target+o.blackOffset<P>();
	  if((T!=KNIGHT && pos.isEdge()) ||
	     (T==KNIGHT && !pos.isOnBoard())) continue;
	  if((T==PAWN ? pos.canPromote<P>() :
	      (PtypeTraits<T>::isBasic 
	       && !Ptype_Table.canDropTo(P,T,pos)
	       // !PtypePlayerTraits<T,P>::canDropTo(pos)
		)))
	    continue;
	  Piece p=state.pieceAt(pos);
	  if(p.isEmpty()){
	    if(!state.pinOrOpen(P).test(attacker.number()) ||
	       state.pinnedCanMoveTo<P>(attacker,pos))
	      action.simpleMove(from,pos,T,false,P);
	  }
	}
      }

      /**
       * fromにプレイヤーPの駒がある．
       * toは玉の8近傍でPがfromから長い利きを持ちうるマスだとする．
       * 味方の駒で8近傍への利きをblockしている駒(味方か敵かを問わず)があるかどうかをチェックし，
       * あるならblockerに入れてtrueを返す
       */
      template<Player P>
      bool findBlocker(NumEffectState const& state,Piece attacker, Square target, Square from,Square& pos,Piece& blocker, Offset offset)
      {
	if(state.hasEffectByPiece(attacker,pos)){
	  blocker=state.pieceAt(pos);
	  if(!blocker.isEmpty()) return blocker.isOnBoardByOwner<P>();
	  pos-=offset;
	  if(abs(pos.x()-target.x())>1 || abs(pos.y()-target.y())>1)
	    return false;
	  blocker=state.pieceAt(pos);
	  return blocker.isOnBoardByOwner<P>();
	}
	else{
	  // TODO effectedNumTableを使える?
	  for(Square pos1=from+offset;pos1!=pos;pos1+=offset){
	    Piece p=state.pieceAt(pos1);
	    if(!p.isEmpty()){
	      if(!p.isOnBoardByOwner<P>()) return false;
	      if(!Ptype_Table.getEffect(p.ptypeO(),pos1,pos).hasEffect()){
		blocker=p;
		for(pos1+=offset;pos1!=pos;pos1+=offset){
		  p=state.pieceAt(pos1);
		  if(!p.isEmpty()){
		    if(!p.isOnBoardByOwner<P>() ||
		       !Ptype_Table.getEffect(p.ptypeO(),pos1,pos).hasEffect()) return false;
		  }
		}
		pos=blocker.square();
		return true;
	      }
	      if(pos1==pos-offset &&
		 (pos+offset).isOnBoard() &&
		 !Ptype_Table.getEffect(p.ptypeO(),pos1,pos+offset).hasEffect()){
		blocker=p;
		pos=blocker.square();
		return true;
	      }
	      return false;
	    }
	  }
	  return false;
	}
      }
      /**
       * fromにプレイヤーPの種類Tの駒があったら利きがある盤面かどうか.
       * 追加も含める.
       * 間が全部空白なら利きがあるのが前提．
       * @param P(template) - 利きをつけるプレイヤー
       * @param T - 駒の種類(ROOK, BISHOP, LANCEのいずれか)
       * @param state - 盤面の状態
       * @param from - 駒があると仮定するマス
       * @param to - 利きの有無を判定したいマス
       * @param blocker - 利きがない時にブロックしている駒を返す
       * @param offset - toからfromへのoffset(redundant)
       */
      template<Player P,Ptype T>
      bool canAddLongEffect(NumEffectState const& state,Square from,Square to,Piece& blocker,Offset offset)
      {
	for(Square pos=to+offset;pos!=from;pos+=offset){
	  Piece p=state.pieceAt(pos);
	  if(!p.isEmpty()){
	    if(!p.isOnBoardByOwner<P>() ||
	       !Ptype_Table.getEffect(p.ptypeO(),pos,to).hasEffect()){
	      blocker=p;
	      assert(!blocker.isEdge());
	      return false;
	    }
	  }
	}
	return true;
      }
      /**
       * fromにプレイヤーPの種類Tの駒があったら利きがある盤面かどうか.
       * 追加も含める.
       * 間が全部空白なら利きがあるのが前提．
       * @param P(template) - 利きをつけるプレイヤー
       * @param T - 駒の種類(ROOK, BISHOP, LANCEのいずれか)
       * @param state - 盤面の状態
       * @param from - 駒があると仮定するマス
       * @param to - 利きの有無を判定したいマス
       * @param blocker - 利きがない時にブロックしている駒を返す
       */
      template<Player P,Ptype T>
      bool canAddLongEffect(NumEffectState const& state,Square from,Square to,Piece& blocker)
      {
	Offset offset=Board_Table.getShortOffset(Offset32(from,to));
	return canAddLongEffect<P,T>(state,from,to,blocker,offset);
      }
      /**
       * 長い利きも持つ駒の手生成.
       * @param P(template) - 攻撃側のプレイヤー
       * @param T(template) - 攻撃側の駒の種類
       * @param isPromotion(template) - 成っての攻撃(成れるかは内部でチェック)
       * @param state - 局面
       * @param attacker - 攻撃しようとする駒
       * @param from - 攻撃しようとする駒の位置
       * @param target - 攻撃しようとするマス
       */
      template <Player P,Ptype T,class Action>
      void generateLongMove(NumEffectState const& state,Piece attacker,
			    Square target, Action& action)
      {
	Square from=attacker.square();
	// 飛車，角はpromoteできる時はpromoteする手を生成済み
	if((T==ROOK || T==BISHOP) && from.canPromote<P>())
	  return;
	Offset32 o32=Offset32(from,target).blackOffset32<P>();
	// 間の味方の駒のopen
	OffsetPair op=Add_Effect8_Table.getBetweenOffset(T,o32);
	if(!op.first.zero()){
	  Square pos=target+op.first.blackOffset<P>();
	  Offset offset=op.second.blackOffset<P>();
	  if(pos.isOnBoard()){
	    Piece blocker=Piece::EMPTY();
	    if(findBlocker<P>(state,attacker,target,from,pos,blocker,offset)){
	      Direction d=longToShort(Board_Table.getLongDirection<BLACK>(Offset32(pos,from).blackOffset32<P>()));
//		std::cerr << "blocker=" << blocker << ",d=" << d << std::endl;
	      PieceOnBoard<Action,true>::template generate<P,true>(
		state,blocker,action,1<<primDir(d));
	    }
	  }
	}
	if(T==LANCE) return;
	// 移動後に短い利きで迫る
	// promoteなし
	for(int i=0;;i++){
	  Offset o=Add_Effect8_Table.getShortMoveOffset(false,T,o32,i);
	  if(o.zero()) break;
	  Square pos=target+o.blackOffset<P>();
	  if(!pos.isOnBoard()) continue;
	  if ((!canPromote(T) || !pos.canPromote<P>()) &&
	      state.hasEffectByPiece(attacker,pos) &&
	      state.pieceAt(pos).isEmpty()){
	    if(!state.pinOrOpen(P).test(attacker.number()) ||
	       state.pinnedCanMoveTo<P>(attacker,pos))
	      action.simpleMove(from,pos,T,false,P);
	  }
	}
	// 移動後に長い利きで迫る
	for(int i=0;;i++){
	  OffsetPair op=Add_Effect8_Table.getLongMoveOffset(T,o32,i);
	  if(op.first.zero()) break;
	  Square pos1=target+op.first.blackOffset<P>();
	  Square pos2=target+op.second.blackOffset<P>();
	  Piece blocker=Piece::EMPTY();
	  if(pos1.isOnBoard() && pos2.isOnBoard() &&
	     state.pieceAt(pos1).isEmpty() &&
	     (!canPromote(T) || !pos1.canPromote<P>()) &&
	     state.hasEffectByPiece(attacker,pos1) &&
	     canAddLongEffect<P,T>(state,pos1,pos2,blocker)){
	    if(!state.pinOrOpen(P).test(attacker.number()) ||
	       state.pinnedCanMoveTo<P>(attacker,pos1))
	      action.simpleMove(from,pos1,T,false,P);
	  }
	}
      }

      /**
       * promote可能な足の短い駒による利きの生成用 Functor
       */
      template <Player P,Ptype T,class Action>
      class ShortPieceAction
      {
	NumEffectState const& state;
	Square target;
	Action& action;
      public:
	ShortPieceAction(NumEffectState const& s,Square p,Action& a)
	  :state(s),target(p),action(a)
	{}
	/**
	 * forEachOnBoardから呼ばれる
	 */
	void operator()(Piece p)
	{
	  if (p.isPromotedNotKingGold())
	  {
	    generateShortMove<P,PtypeFuns<T>::promotePtype,Action>(state,p,target,action);
	  }
	  else
	  {
	    generateShortMove<P,T,Action>(state,p,target,action);
	  }  
	}
      };

      /**
       * Goldによる利きの生成用 Functor
       */
      template <Player P,class Action>
      class GoldAction
      {
	NumEffectState const& state;
	Square target;
	Action& action;
      public:
	GoldAction(NumEffectState const& s,Square p,Action& a)
	  :state(s),target(p),action(a)
	{}
	/**
	 * forEachOnBoardから呼ばれる
	 */
	void operator()(Piece p)
	{
	  generateShortMove<P,GOLD,Action>(state,p,target,action);
	}
      };

      /**
       * promote可能な足の長い駒による利きの生成用 Functor
       */
      template <Player P,Ptype T,class Action>
      class LongPieceAction
      {
	NumEffectState const& state;
	Square target;
	Action& action;
      public:
	LongPieceAction(NumEffectState const& s,Square p,Action& a)
	  :state(s),target(p),action(a)
	{}
	/**
	 * forEachOnBoardから呼ばれる
	 */
	void operator()(Piece p)
	{
	  if (p.isPromotedNotKingGold())
	  {
	    if (T==LANCE)
	      generateShortMove<P,PtypeFuns<T>::promotePtype,Action>(state,p,target,action);
	    else
	      generateLongMove<P,PtypeFuns<T>::promotePtype,Action>(state,p,target,action);
	  }
	  else
	  {
	    generateLongMove<P,T,Action>(state,p,target,action);
	  }  
	}
      };

      template<Player P,Ptype T,class Action>
      void generateShort(const NumEffectState& state,Square target,
			 Action& action)
      {
	static_assert((PtypeTraits<T>::isBasic), "");
	static_assert((PtypeTraits<T>::canPromote), "");
	typedef ShortPieceAction<P,T,Action> action_t;
	action_t gkAction(state,target,action);
	state.template forEachOnBoard<P,T,action_t>(gkAction);
	/** drop move */
	if (state.template hasPieceOnStand<T>(P)){
	  generateShortDrop<P,T,Action>(state,target,action);
	}
      }

      template<Player P,Ptype T,class Action>
      void generateLong(const NumEffectState& state,Square target,
			Action& action)
      {
	static_assert((PtypeTraits<T>::isBasic), "");
	static_assert((PtypeTraits<T>::canPromote), "");
	typedef LongPieceAction<P,T,Action> action_t;
	action_t gkAction(state,target,action);
	state.template forEachOnBoard<P,T,action_t>(gkAction);
      }

      template<Player P,class Action>
      void generateGold(const NumEffectState& state,Square target,
			Action& action)
      {
	typedef GoldAction<P,Action> action_t;
	action_t gkAction(state,target,action);
	state.template forEachOnBoard<P,GOLD,action_t>(gkAction);
	/** drop move */
	if (state.template hasPieceOnStand<GOLD>(P)){
	  generateShortDrop<P,GOLD,Action>(state,target,action);
	}
      }
    }

    template<Player P>
    template<class Action>
    void AddEffect8<P>::
    generateBigDrop(const NumEffectState& state,Action& action)
    {
      using namespace addeffect8;
      Square target=state.kingSquare<alt(P)>();
      if (state.template hasPieceOnStand<BISHOP>(P)){
	generateLongDrop<P,BISHOP,Action>(state,target,action);
      }
      if (state.template hasPieceOnStand<ROOK>(P)){
	generateLongDrop<P,ROOK,Action>(state,target,action);
      }
    }

    template<Player P>
    template<class Action>
    void AddEffect8<P>::
    generateNotBigDrop(const NumEffectState& state,Action& action)
    {
      using namespace addeffect8;
      Square target=state.kingSquare<alt(P)>();
      generateShort<P,PAWN,Action>(state,target,action);
      generateLong<P,LANCE,Action>(state,target,action);
      if (state.template hasPieceOnStand<LANCE>(P)){
	generateLongDrop<P,LANCE,Action>(state,target,action);
      }
      generateShort<P,KNIGHT,Action>(state,target,action);
      generateShort<P,SILVER,Action>(state,target,action);
      generateGold<P,Action>(state,target,action);
      // no king move
      generateLong<P,BISHOP,Action>(state,target,action);
      generateLong<P,ROOK,Action>(state,target,action);
    }

    template<Player P>
    template<class Action>
    void AddEffect8<P>::
    generate(const NumEffectState& state,Action& action)
    {
      generateNotBigDrop(state,action);
      generateBigDrop(state,action);
    }
  }
}
#endif /* _MOVE_GENERATOR_ADD_EFFECT8_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
