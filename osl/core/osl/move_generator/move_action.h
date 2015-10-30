/* move_action.h
 */
#ifndef OSL_MOVEACTION_H
#define OSL_MOVEACTION_H
#include "osl/numEffectState.h"
#include "osl/move_classifier/kingOpenMove.h"

namespace osl
{
  namespace move_action
  {
    /**
     * 指手を MoveVector に保管
     */
    struct Store
    {
      FixedCapacityVectorPushBack<Move> moves;
      template <size_t Capacity>
      explicit Store(FixedCapacityVector<Move, Capacity>& v) 
	: moves(v.pushBackHelper())
      {
      }
      /** コマをとらないMove */
      void simpleMove(Square /*from*/,Square /*to*/,Ptype /*ptype*/, bool /*isPromote*/,Player /*p*/,Move move){
	assert(move.isValid());
	moves.push_back(move);
      }
      /** 
       *	コマを取るかもしれないMove 
       * @param from - 駒の移動元
       * @param to - 駒の移動先
       * @param p1 - 移動先のマスの駒
       * @param ptype - 移動後の駒のptype
       * @param isPromote - 成りか?
       * @param p - プレイヤー
       */
      void unknownMove(Square /*from*/,Square /*to*/,Piece /*p1*/,Ptype /*ptype*/,bool /*isPromote*/,Player /*p*/,Move move)
      {
	assert(move.isValid());
	moves.push_back(move);
      }
      /** コマを打つMove */
      void dropMove(Square /*to*/,Ptype /*ptype*/,Player /*p*/,Move move)
      {
	assert(move.isValid());
	moves.push_back(move);
      }
      // old interfaces
      void simpleMove(Square from,Square to,Ptype ptype, 
		      bool isPromote,Player p)
      {
	simpleMove(from,to,ptype,isPromote,p,
		   Move(from,to,ptype,PTYPE_EMPTY,isPromote,p));
      }
      void unknownMove(Square from,Square to,Piece captured,
		       Ptype ptype,bool isPromote,Player p)
      {
	unknownMove(from,to,captured,ptype,isPromote,p,
		    Move(from,to,ptype,captured.ptype(),isPromote,p));
      }
      void dropMove(Square to,Ptype ptype,Player p)
      {
	dropMove(to,ptype,p,
		 Move(to,ptype,p));
      }
    };

    /**
     * 利きのないところへ動くためのフィルタ
     */
    template<Player P,class OrigAction>
    class NoEffectFilter
    {
      const NumEffectState& state;
      OrigAction & action;
      Square removed;
    public:
      NoEffectFilter(const NumEffectState& s, OrigAction & action,Square pos) : state(s), action(action),removed(pos) {}
      void simpleMove(Square from,Square to,Ptype ptype, bool isPromote,Player /* p */,Move m){
	if(!state.template hasEffectByWithRemove<alt(P)>(to,removed))
	  action.simpleMove(from,to,ptype,isPromote,P,m);
      }
      void unknownMove(Square from,Square to,Piece p1,Ptype ptype,bool isPromote,Player /* p */,Move m){
	if(!state.template hasEffectByWithRemove<alt(P)>(to,removed)){
	  action.unknownMove(from,to,p1,ptype,isPromote,P,m);
	}
      }
      void dropMove(Square to,Ptype ptype,Player /* p */,Move m){
	/** ここは呼ばれないはず */
	if(!state.template hasEffectByWithRemove<alt(P)>(to,removed))
	  action.dropMove(to,ptype,P,m);
      }
      // old interfaces
      void simpleMove(Square from,Square to,Ptype ptype, 
		      bool isPromote,Player p)
      {
	simpleMove(from,to,ptype,isPromote,p,
		   Move(from,to,ptype,PTYPE_EMPTY,isPromote,p));
      }
      void unknownMove(Square from,Square to,Piece captured,
		       Ptype ptype,bool isPromote,Player p)
      {
	unknownMove(from,to,captured,ptype,isPromote,p,
		    Move(from,to,ptype,captured.ptype(),isPromote,p));
      }
      void dropMove(Square to,Ptype ptype,Player p)
      {
	dropMove(to,ptype,p,
		 Move(to,ptype,p));
      }
    };

    /**
     * 指定したSquareに利きをつける手をフィルタ
     */
    template<class OrigAction>
    class NoAddEffectFilter
    {
      const NumEffectState& state;
      OrigAction & action;
      Square target;
    public:
      NoAddEffectFilter(const NumEffectState& s, OrigAction & action,Square target) : state(s), action(action),target(target) {}
      void simpleMove(Square from,Square to,Ptype ptype, bool isPromote,Player p,Move m){
	if(!state.hasEffectIf(newPtypeO(p,ptype),to,target))
	  action.simpleMove(from,to,ptype,isPromote,p,m);
      }
      void unknownMove(Square from,Square to,Piece p1,Ptype ptype,bool isPromote,Player p,Move m){
	if(!state.hasEffectIf(newPtypeO(p,ptype),to,target))
	  action.unknownMove(from,to,p1,ptype,isPromote,p,m);
      }
      void dropMove(Square to,Ptype ptype,Player p,Move m){
	if(!state.hasEffectIf(newPtypeO(p,ptype),to,target))
	  action.dropMove(to,ptype,p,m);
      }
      // old interfaces
      void simpleMove(Square from,Square to,Ptype ptype, 
		      bool isPromote,Player p)
      {
	simpleMove(from,to,ptype,isPromote,p,
		   Move(from,to,ptype,PTYPE_EMPTY,isPromote,p));
      }
      void unknownMove(Square from,Square to,Piece captured,
		       Ptype ptype,bool isPromote,Player p)
      {
	unknownMove(from,to,captured,ptype,isPromote,p,
		    Move(from,to,ptype,captured.ptype(),isPromote,p));
      }
      void dropMove(Square to,Ptype ptype,Player p)
      {
	dropMove(to,ptype,p,
		 Move(to,ptype,p));
      }
    };

    /**
     * 相手の間接利きを止めている駒を動かさない
     */
    template<Player P,class OrigAction>
    struct NotKingOpenFilter
    {
      const NumEffectState& state;
      OrigAction & action;
    public:
      NotKingOpenFilter(const NumEffectState& s, OrigAction & action) 
	: state(s), action(action) {
      }
      bool isNotKingOpenMove(Ptype ptype,Square from,Square to)
      {
	return !move_classifier::KingOpenMove<P>::isMember(state, ptype, from, to);
      }
      void simpleMove(Square from,Square to,Ptype ptype, bool isPromote,Player 
#ifndef NDEBUG
		      p
#endif
		      ,Move m
	){
	assert(p == P);
	if(isNotKingOpenMove(ptype,from,to))
	  action.simpleMove(from,to,ptype,isPromote,P,m);
      
      }
      void unknownMove(Square from,Square to,Piece p1,Ptype ptype,bool isPromote,Player 
#ifndef NDEBUG
		       p
#endif
		       ,Move m
	){
	assert(p == P);
	if(isNotKingOpenMove(ptype,from,to))
	  action.unknownMove(from,to,p1,ptype,isPromote,P,m);
      }
      /**
       * dropMoveが自殺手になることはない
       */
      void dropMove(Square to,Ptype ptype,Player 
#ifndef NDEBUG
		    p
#endif
		    ,Move m
	){
	assert(p == P);
	action.dropMove(to,ptype,P,m);
      }
      // old interfaces
      void simpleMove(Square from,Square to,Ptype ptype, 
		      bool isPromote,Player p)
      {
	simpleMove(from,to,ptype,isPromote,p,
		   Move(from,to,ptype,PTYPE_EMPTY,isPromote,p));
      }
      void unknownMove(Square from,Square to,Piece captured,
		       Ptype ptype,bool isPromote,Player p)
      {
	unknownMove(from,to,captured,ptype,isPromote,p,
		    Move(from,to,ptype,captured.ptype(),isPromote,p));
      }
      void dropMove(Square to,Ptype ptype,Player p)
      {
	dropMove(to,ptype,p,
		 Move(to,ptype,p));
      }
    };
  } // namespace move_action
} // namespace osl

#endif /* OSL_MOVEACTION_STORE */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
