#ifndef OSL_EFFECT_ACTION
#define OSL_EFFECT_ACTION
#include "osl/numEffectState.h"

namespace osl
{
  namespace effect_action
  {
    /**
     * 全ての指手を生成
     */
    template<class Action>
    class AlwaysMove
    {
    private:
      const NumEffectState& state;
      Action& ac;
    public:
      AlwaysMove(const NumEffectState& s, Action & a) : state(s), ac(a) {}
      /**
       * Ptypeをtemplate引数にできる場合
       */
      template<Player P,Ptype Type>
      void doActionPtype(Piece p1,Square to){
	assert(Type == unpromote(p1.ptype()));
	Square from=p1.square(); 
	if (canPromote(Type) &&
	   !p1.isPromotedNotKingGold() &&
	   (to.canPromote<P>() || from.canPromote<P>())){
	  ac.unknownMove(from,to,state.pieceAt(to),promote(Type),true,P);
	}
	if (!canPromote(Type) ||
	   PtypePlayerTraits<Type,P>::canDropTo(to) ||
	   p1.isPromotedNotKingGold()){
	  ac.unknownMove(from,to,state.pieceAt(to),p1.ptype(),false,P);
	}
      }
      /**
       * Ptypeをtemplate引数にできない場合
       */
      template<Player P>
      void doAction(Piece p1,Square to)
      {
	Square from=p1.square(); 
	Ptype ptype=p1.ptype();
	if(canPromote(ptype))
	{
	  if (to.canPromote<P>())
	  {
	    ac.unknownMove(from,to,state.pieceAt(to),promote(ptype),true,P);
	    if(Ptype_Table.canDropTo(P, ptype,to))
	    {
	      ac.unknownMove(from,to,state.pieceAt(to),ptype,false,P);
	    }
	  }
	  else if (from.canPromote<P>())
	  {
	    ac.unknownMove(from,to,state.pieceAt(to),promote(ptype),true,P);
	    ac.unknownMove(from,to,state.pieceAt(to),ptype,false,P);
	  }
	  else
	  {
	    ac.unknownMove(from,to,state.pieceAt(to),ptype,false,P);
	  }
	}
	else
	{
	  ac.unknownMove(from,to,state.pieceAt(to),ptype,false,P);
	}
      }
      bool done() const{ return false; }
    };

    /**
     * 成った方が良い駒は成る手のみ生成
     */
    template<class Action>
    class BetterToPromote
    {
    private:
      const NumEffectState& state;
      Action & ac;
    public:
      BetterToPromote(const NumEffectState& s, Action& a) 
	: state(s), ac(a)
      {
      }
      template<Player P,Ptype Type>
      void doActionPtype(Piece p1,Square to){
	assert(Type == unpromote(p1.ptype()));
	Square from=p1.square(); 
	Piece target = state.pieceAt(to);
	if (canPromote(Type) &&
	   !p1.isPromotedNotKingGold() &&
	   (to.canPromote<P>() || from.canPromote<P>())){
	  ac.unknownMove(from,to,target,promote(Type),true,P);
	}
	if (!canPromote(Type) ||
	   PtypePlayerTraits<Type,P>::canDropTo(to) ||
	   p1.isPromotedNotKingGold()){
	  if (! (to.canPromote<P>() || from.canPromote<P>())
	      || (! PtypeTraits<Type>::betterToPromote
		  && (p1.ptype() != LANCE
		      || PtypePlayerTraits<LANCE,P>::canDropTo(to + DirectionPlayerTraits<U,P>::offset()))))
	    ac.unknownMove(from,to,target,p1.ptype(),false,P);
	}
      }
      /**
       * Ptypeをtemplate引数にできない場合
       */
      template<Player P>
      void doAction(Piece p1,Square to)
      {
	Square from=p1.square(); 
	Ptype ptype=p1.ptype();
	Piece target = state.pieceAt(to);
	if(canPromote(ptype))
	{
	  if (to.canPromote<P>())
	  {
	    ac.unknownMove(from,to,target,promote(ptype),true,P);
	    if(Ptype_Table.canDropTo(P, ptype,to)
	       && ! Ptype_Table.isBetterToPromote(ptype)
	       && (ptype != LANCE
		   || PtypePlayerTraits<LANCE,P>::canDropTo(to + DirectionPlayerTraits<U,P>::offset())))

	    {
	      ac.unknownMove(from,to,target,ptype,false,P);
	    }
	    return;
	  }
	  if (from.canPromote<P>())
	  {
	    ac.unknownMove(from,to,target,promote(ptype),true,P);
	    if(! Ptype_Table.isBetterToPromote(ptype)
	       && (ptype != LANCE
		   || PtypePlayerTraits<LANCE,P>::canDropTo(to + DirectionPlayerTraits<U,P>::offset())))
	      ac.unknownMove(from,to,target,ptype,false,P);
	    return;
	  }
	  // fall through
	}
	ac.unknownMove(from,to,target,ptype,false,P);
      }
    };

    /**
     * PieceVector に格納
     */
    struct StorePiece
    {
      PieceVector *store;
      explicit StorePiece(PieceVector *s) : store(s)
      {
      }
      template<Player P,Ptype Type>
      void doActionPtype(Piece p, Square pos)
      {
	doAction<P>(p, pos);
      }
      template<Player P>
      void doAction(Piece p, Square)
      {
	store->push_back(p);
      }
    };

    /**
     * PtypeOSquareVector に格納
     */
    struct StorePtypeOSquare
    {
      PtypeOSquareVector *out;
      Square target;
      StorePtypeOSquare(PtypeOSquareVector *s, Square t) 
	: out(s), target(t)
      {
      }
      template<Player P,Ptype Type>
      void doActionPtype(Piece p)
      {
	store(p);
      }
      template<Player P>
      void doAction(Piece p, Square)
      {
	store(p);
      }
    
      void store(Piece p)
      {
	const PtypeO ptypeO = p.ptypeO();
	out->push_back(std::make_pair(ptypeO, p.square()));
      }
    };
  } // namespace effect_action
} // namespace osl
#endif // OSL_EFFECT_ACTION
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
