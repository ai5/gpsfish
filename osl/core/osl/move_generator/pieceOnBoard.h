#ifndef OSL_PIECE_ON_BOARD_H
#define OSL_PIECE_ON_BOARD_H
#include "osl/numEffectState.h"

namespace osl
{
  namespace move_generator
  {
    enum PromoteType{
      NoPromoteType=0,
      CanPromoteType=1,
      CheckPromoteType=2,
      MustPromoteType=3,
    };
    /**
     * 特定のpieceを動かす手を生成
     */
    template<class Action,bool noCapturePromote=false>
    struct PieceOnBoard
    {
      /**
       * 駒pがマスtargetに利きをもっているとして，手を生成する．
       */
      template<Player P>
      static void generatePieceUnsafe(const NumEffectState& state,Piece p, Square target, Piece p1,Action& action)
      {
	assert(state.hasEffectByPiece(p, target));
	Ptype ptype=p.ptype();
	Square from=p.square();
	if(canPromote(ptype)){
	  if(target.canPromote<P>()){
	    action.unknownMove(from,target,p1,promote(ptype),true,P);
	    int y=(P==BLACK ? target.y() : 10-target.y());
	    if(!Ptype_Table.isBetterToPromote(ptype) && 
	       (((ptype==LANCE || ptype==PAWN) ? y==3 : true )) &&
	       Ptype_Table.canDropTo(P,ptype,target))
	      action.unknownMove(from,target,p1,ptype,false,P);
	  }
	  else if(from.canPromote<P>()){
	    action.unknownMove(from,target,p1,promote(ptype),true,P);
	    if(!Ptype_Table.isBetterToPromote(ptype))
	      action.unknownMove(from,target,p1,ptype,false,P);
	  }
	  else
	    action.unknownMove(from,target,p1,ptype,false,P);
	}
	else{
	  action.unknownMove(from,target,p1,ptype,false,P);
	}
      }
      template<Player P>
      static void generatePiece(const NumEffectState& state,Piece p, Square target, Piece p1,Action& action)
      {
	if(p.ptype()==KING){
	  // 王手がかかっているときには自分の影になっている手も生成してしまう
	  const Player altP=alt(P);
//	  assert(!state.hasEffectAt<altP>(p.square()));
	  // 自殺手
	  if(state.hasEffectAt<altP>(target)) return;
	}
	if(state.pinOrOpen(P).test(p.number())){
	  Direction d=state.pinnedDir<P>(p);
	  Direction d1=Board_Table.getShort8Unsafe<P>(p.square(),target);
	  if(primDir(d)!=primDirUnsafe(d1)) return;
	}
	generatePieceUnsafe<P>(state,p,target,p1,action);
      }
      /**
       * PtypeがTの駒pがマスtargetに利きをもっているとして，手を生成する．
       * p1 - targetにある駒
       */
      template<Player P,Ptype T>
      static void generatePiecePtypeUnsafe(const NumEffectState& state,Piece p, Square target, Piece p1,Action& action)
      {
	assert(state.hasEffectByPiece(p, target));
	assert(p.ptype()==T);
//	Ptype ptype=p.ptype();
	Square from=p.square();
	if(canPromote(T) & (target.canPromote<P>() || from.canPromote<P>())){
	  action.unknownMove(from,target,p1,promote(T),true,P);
	  if(((T==PAWN || T==LANCE) && 
	      (P==BLACK ? target.y()==1 : target.y()==9))||
	     (T==KNIGHT && (P==BLACK ? target.y()<=2 : target.y()>=8)))
	    return;
	  if((T==ROOK || T==BISHOP || T==PAWN ||
	      (T==LANCE && (P==BLACK ? target.y()==2 : target.y()==8)))) 
	    return;
	}
	action.unknownMove(from,target,p1,T,false,P);
      }
      template<Player P,Ptype T>
      static void generatePiecePtype(const NumEffectState& state,Piece p, Square target, Piece p1,Action& action)
      {
	if(T==KING){
	  assert(!state.hasEffectAt(alt(P),p.square()));
	  if(state.hasEffectAt(alt(P),target)) return;
	}
	else if(state.pin(P).test(p.number())){
	  Direction d=state.pinnedDir<P>(p);
	  Direction d1=Board_Table.getShort8Unsafe<P>(p.square(),target);
	  if(primDir(d)!=primDirUnsafe(d1)) return;
	}
	generatePiecePtypeUnsafe<P,T>(state,p,target,p1,action);
      }
      /**
       * Generate moves without stating the Ptype as template param.
       * pinの場合はそれに応じた手を生成する
       * @param T - moveTypeがTの駒
       * @param state - 手を作成する局面，手番はPと一致
       * @param p - 盤面上に存在するPの駒
       * @param action - 手生成用のAction
       */
      template <Player P,Ptype T,bool useDirMask>
      static void generatePtype(const NumEffectState& state,Piece p, Action& action,int dirMask=0);

      template <Player P,Ptype T>
      static void generatePtype(const NumEffectState& state,Piece p, Action& action)
      {
	int dummy=0;
	generatePtype<P,T,false>(state,p,action,dummy);
      }
      /**
       * Generate moves without stating the Ptype as template param.
       * pinでないことが判明している時に呼び出す
       * @param T - moveTypeがTの駒
       * @param state - 手を作成する局面，手番はPと一致
       * @param p - 盤面上に存在するPの駒
       * @param action - 手生成用のAction
       */
      template <Player P,Ptype T,bool useDirMask>
      static void generatePtypeUnsafe(const NumEffectState& state,Piece p, Action& action,int dirMask);
      template <Player P,Ptype T>
      static void generatePtypeUnsafe(const NumEffectState& state,Piece p, Action& action)
      {
	int dummy=0;
	generatePtypeUnsafe<P,T,false>(state,p,action,dummy);
      }

      /**
       * Generate moves without stating the Ptype as template param.
       * 自玉に王手がかかっていない時に呼ぶ．
       * @param state - 手を作成する局面，手番はPと一致
       * @param p - 盤面上に存在するPの駒
       * @param action - 手生成用のAction
       */
      template <Player P,bool useDirMask>
      static void generate(const NumEffectState& state,Piece p, Action& action,int dirMask=0);
    };

    struct GeneratePieceOnBoard
    {
      static void generate(Player turn, const NumEffectState& state, Piece target,
			   MoveVector&);
    };
  } // namespace move_generator
  using move_generator::GeneratePieceOnBoard;
} // namespace osl

#endif /* OSL_PIECE_ON_BOARD_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
