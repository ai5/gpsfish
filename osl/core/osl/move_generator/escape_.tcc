#ifndef OSL_GENERATE_ESCAPE_MOVES_TCC
#define OSL_GENERATE_ESCAPE_MOVES_TCC

#include "osl/move_generator/escape_.h"
#include "osl/move_generator/capture_.h"
#include "osl/move_generator/move_action.h"

namespace osl
{
  namespace move_generator
  {
    namespace escape
    {
      /**
       * Tの駒をtoに打つ手を生成する．
       * 生成できたらtrueを返す．
       */
      template<Player P,class Action,Ptype Type>
      bool generateDrop(const NumEffectState& state,Square to,Action& action){
	if(state.template hasPieceOnStand<Type>(P)){
	  if((Type!=PAWN || !state.isPawnMaskSet(P,to.x())) &&
	     PtypePlayerTraits<Type,P>::canDropTo(to)){
	    action.dropMove(to,Type,P);
	    return true;
	  }
	}
	return false;
      }
      /*
       * 駒をtoに打つ手を生成する．
       * CheapOnlyの時は最も価値の低い駒を打つ手のみ生成する．
       */
      template<Player P,class Action,bool CheapOnly>
      void generateDropAll(const NumEffectState& state,Square to,Action& action)
      {
	bool gen = generateDrop<P,Action,PAWN>(state,to,action); if (CheapOnly && gen) return;
	gen = generateDrop<P,Action,LANCE>(state,to,action);     if (CheapOnly && gen) return;
	gen = generateDrop<P,Action,KNIGHT>(state,to,action);    if (CheapOnly && gen) return;
	gen = generateDrop<P,Action,SILVER>(state,to,action);    if (CheapOnly && gen) return;
	gen = generateDrop<P,Action,GOLD>(state,to,action);      if (CheapOnly && gen) return;
	gen = generateDrop<P,Action,BISHOP>(state,to,action);    if (CheapOnly && gen) return;
	generateDrop<P,Action,ROOK>(state,to,action);      
      }

      /**
       * 安い駒でposへ移動する手を生成する．
       * 自殺手も生成している．
       * TODO: あんまりなif文 PAWN,LANCE mask, それ以外maskでOK
       */
      template<Player P,class Action,bool CheapOnly>
      void
      blockByMoveOne(const NumEffectState& state, Square pos, Action &action)
      {
	const PieceMask pieces = state.effectSetAt(pos) & state.piecesOnBoard(P);
	int offset = 0;
	mask_t m = pieces.selectBit<PAWN>();
	if (m.none()) { 
	  m = pieces.selectBit<LANCE>(); 
	  offset = PtypeFuns<LANCE>::indexNum*32; 
	  if (m.none()) { 
	    m = pieces.selectBit<KNIGHT>(); 
	    offset = PtypeFuns<KNIGHT>::indexNum*32; 
	    if (m.none()) { 
	      m = pieces.selectBit<SILVER>(); 
	      offset = PtypeFuns<SILVER>::indexNum*32; 
	      if (m.none()) { 
		m = pieces.selectBit<GOLD>(); 
		offset = PtypeFuns<GOLD>::indexNum*32; 
		if (m.none()) { 
		  m = pieces.selectBit<BISHOP>(); 
		  offset = PtypeFuns<BISHOP>::indexNum*32; 
		  if (m.none()) { 
		    m = pieces.selectBit<ROOK>(); 
		    offset = PtypeFuns<ROOK>::indexNum*32; 
		    if (m.none()) 
		      return;
		  }
		}
	      }
	    }
	  }
	}
	const Piece p = state.pieceOf(m.takeOneBit() + offset);
	PieceOnBoard<Action>::template generatePiece<P>(state,p,pos,Piece::EMPTY(),action);
      }
    } // end of namespace escape
    using escape::generateDropAll;
    using escape::blockByMoveOne;

    /**
     * Square toにある玉以外の駒にfromにある駒から王手がかかっている時に，長い利きの途中に入る手を
     * 生成する(合駒，駒移動)．
     * pが動く手は生成しない
     * CheapOnlyの時は
     * TODO: 自殺手も生成してしまう
     *  短い利きの時にもこちらに入ってしまう
     */
    template<class Action>
    template<Player P,bool CheapOnly>
    void Escape<Action>::
    generateBlocking(const NumEffectState& state,Piece p,Square to,Square from,Action &action)
    {
      assert(from.isOnBoard());
      Offset offset=Board_Table.getShortOffset(Offset32(from,to));
      assert(!offset.zero());
      for(Square pos=to+offset;pos!=from;pos+=offset){
	assert(state.pieceAt(pos).isEmpty());
	if (! CheapOnly) {
	  Capture<Action>::template escapeByCapture<P>(state,pos,p,action);
	  // 駒を置いて
	  generateDropAll<P,Action,false>(state,pos,action);
	}
	else {
	  // 駒を動かして
	  const int e = state.countEffect(P, pos);
	  if (e >= 2) 
	    blockByMoveOne<P,Action,CheapOnly>(state, pos, action);
	  // 駒を置いて
	  if (e)
	    generateDropAll<P,Action,true>(state,pos,action);
	}
      }
    }
      /**
       * 玉pにfromにある駒から王手がかかっている時に，長い利きの途中に入る手を
       * 生成する(合駒，駒移動)．
       *  短い利きの時にもこちらに入ってしまう
       */
    template<class Action>
    template<Player P,bool CheapOnly>
    void Escape<Action>::
    generateBlockingKing(const NumEffectState& state,Piece p,Square from,Action &action)
    {
      Square to=p.square();
      Offset offset=Board_Table.getShortOffset(Offset32(from,to));
      assert(!offset.zero());
      for(Square pos=to+offset;pos!=from;pos+=offset){
	assert(state.pieceAt(pos).isEmpty()); 
	Capture<Action>::template escapeByCapture<P>(state,pos,p,action);
	// 駒を置いて
	generateDropAll<P,Action,CheapOnly>(state,pos,action);
      }
    }
    template<class Action>
    template<Player P,Ptype Type,bool CheapOnly>
    void Escape<Action>::
    generateMovesBy(const NumEffectState& state,Piece p,Piece const attacker,Action& action)
    {
      if(attacker==Piece::EMPTY()){
	/** escape only */
	generateEscape<P,Type>(state,p,action);
      }
      else if(Type == KING){
#ifndef NDEBUG
	{
	  Piece attack_by_position;
	  state.template findCheckPiece<P>(attack_by_position);
	  assert(attacker == attack_by_position);
	}
#endif
	Square attackFrom=attacker.square();

	generateCaptureKing<P>( state, p, attackFrom, action );
	/** escape */
	generateEscape<P,Type>( state,p,action);
	/** 合い駒 */
	generateBlockingKing<P,CheapOnly>(state,p,attackFrom,action);
      }
      else{
	Square attackFrom=attacker.square();
	generateCapture<P>( state, p, attackFrom, action );
	/** escape */
	generateEscape<P,Type>( state,p,action);
	/** 合い駒 */
	generateBlocking<P,CheapOnly>(state,p,p.square(),attackFrom,action);
      }
    }

    template<class Action>
    template<Player P,bool CheapOnly>
    void Escape<Action>::
    generateKingEscape(const NumEffectState& state,Action& action){
      Piece kingPiece=state.pieceOf(KingTraits<P>::index);
      Piece attacker;
#ifndef NDEBUG
      const bool is_attacked=
#endif
	state.template findCheckPiece<P>(attacker);
      assert(is_attacked); // 相手からの利きがないのに呼ぶな
      generateMovesBy<P,KING,CheapOnly>(state,kingPiece,attacker,action);
    }

    template<class Action>
    template<Player P,Ptype TYPE,bool CheapOnly>
    void Escape<Action>::
    generateMovesBy(const NumEffectState& state,Piece p,Action& action)
    {
      Square target=p.square();
      Piece attacker;
#ifndef NDEBUG
      const bool is_attacked=
#endif
	state.template hasEffectAt<alt(P)>(target,attacker);
      assert(is_attacked); // 相手からの利きがないのに呼ぶな
      generateMovesBy<P,TYPE,CheapOnly>(state,p,attacker,action);
    }

    template<class Action>
    template<Player P,bool CheapOnly>
    void Escape<Action>::
    generateMoves(const NumEffectState& state,Piece piece,Piece attacker,Action& action)
    {
      switch(piece.ptype()){
      case PAWN: generateMovesBy<P,PAWN,CheapOnly>(state,piece,attacker,action); break;
      case LANCE: generateMovesBy<P,LANCE,CheapOnly>(state,piece,attacker,action); break;
      case KNIGHT: generateMovesBy<P,KNIGHT,CheapOnly>(state,piece,attacker,action); break;
      case SILVER: generateMovesBy<P,SILVER,CheapOnly>(state,piece,attacker,action); break;
      case PPAWN: generateMovesBy<P,PPAWN,CheapOnly>(state,piece,attacker,action); break;
      case PLANCE: generateMovesBy<P,PLANCE,CheapOnly>(state,piece,attacker,action); break;
      case PKNIGHT: generateMovesBy<P,PKNIGHT,CheapOnly>(state,piece,attacker,action); break;
      case PSILVER: generateMovesBy<P,PSILVER,CheapOnly>(state,piece,attacker,action); break;
      case GOLD: generateMovesBy<P,GOLD,CheapOnly>(state,piece,attacker,action); break;
      case BISHOP: generateMovesBy<P,BISHOP,CheapOnly>(state,piece,attacker,action); break;
      case PBISHOP: generateMovesBy<P,PBISHOP,CheapOnly>(state,piece,attacker,action); break;
      case ROOK: generateMovesBy<P,ROOK,CheapOnly>(state,piece,attacker,action); break;
      case PROOK: generateMovesBy<P,PROOK,CheapOnly>(state,piece,attacker,action); break;
      case KING: generateMovesBy<P,KING,CheapOnly>(state,piece,attacker,action); break;
      default: assert(0);
	
      }
    }
    template<class Action>
    template<Player P,bool shouldPromote,bool CheapOnly>
    void Escape<Action>::
    generate(const NumEffectState& state,Piece piece,Action& action)
    {
      assert(piece.owner() == P);
      Square target=piece.square();
      Piece attacker;
      state.template hasEffectAt<alt(P)>(target,attacker);
      generateMoves<P,CheapOnly>(state,piece,attacker,action);
    }
  } // namespace move_generator

} // namespace osl

#endif // OSL_GENERATE_ESCAPE_MOVES_TCC
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

