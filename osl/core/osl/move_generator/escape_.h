#ifndef OSL_MOVE_GENERATOR_ESCAPE_H
#define OSL_MOVE_GENERATOR_ESCAPE_H
#include "osl/move_generator/pieceOnBoard.h"
#include "osl/move_generator/capture_.h"
#include "osl/move_generator/move_action.h"
#include "osl/basic_type.h"

namespace osl
{
  namespace move_generator
  {
    /**
     * 逃げる手を生成
     * 生成される手はunique
     */
    template<class Action>
    class Escape
    {
    public:
      /**
       * Square toにある玉以外の駒pにfromにある駒から王手がかかってい
       * る時に，長い利きの途中に入る手を
       * 生成する(合駒，駒移動)．
       * breakThreatmateから直接呼ばれる．
       */
      template<Player P,bool CheapOnly>
      static void generateBlocking(const NumEffectState& state,Piece p,Square to,Square from,Action &action);
      /**
       * 玉pにfromにある駒から王手がかかってい
       * る時に，長い利きの途中に入る手を
       * 生成する(合駒，駒移動)．
       * breakThreatmateから直接呼ばれる．
       */
      template<Player P,bool CheapOnly>
      static void generateBlockingKing(const NumEffectState& state,Piece p,Square from,Action &action);
      /**
       * 相手の駒を取ることによって利きを逃れる.
       * 逃げ出す駒で取る手は生成しない（2003/5/12）
       * @param target toru koma no pos
       */
      template<Player P>
      static void generateCaptureKing(const NumEffectState& state,Piece p,Square target,Action& action) {
	Capture<Action>::template escapeByCapture<P>(state,target,p,action);
      }
      template<Player P>
      static void generateCapture(const NumEffectState& state,Piece p,Square target,Action& action) {
	Capture<Action>::template escapeByCapture<P>(state,target,p,action);
      }
      template<Player P,Ptype Type>
      static void generateEscape(const NumEffectState& state,Piece p,Action& action,Int2Type<Type>);

      /**
       * @param p 逃げ出す駒
       */
      template<Player P,Ptype Type>
      static void generateEscape(const NumEffectState& state,Piece p,Action& action){
	/**
	 * @param 王の場合だけ特別扱いできる
	 */
	if(Type==KING){
	  assert(p.owner()==P && p.ptype()==KING);
	  PieceOnBoard<Action>::template 
	    generatePtype<P,Type>(state,p,action);
	}
	else
	{
	  typedef move_action::NoEffectFilter<P,Action> CheckAction;
	  CheckAction checkAction(state,action,p.square());
	  PieceOnBoard<CheckAction>::template 
	    generatePtype<P,Type>(state,p,
					     checkAction);
	}
      }

    public:
      template<Player P,bool cheapOnly>
      static void generateKingEscape(const NumEffectState& state,Action& action);

      /** 
       * @param p 例えば king
       */
      template<Player P,Ptype Type,bool CheapOnly>
      static void generateMovesBy(const NumEffectState& state,Piece p,Piece attacker,Action& action);
      template<Player P,Ptype Type,bool CheapOnly>
      static void generateMovesBy(const NumEffectState& state,Piece p,Move last_move,Action& action);
      template<Player P,Ptype Type,bool CheapOnly>
      static void generateMovesBy(const NumEffectState& state,Piece p,Action& action);

      /**
       * attacker からの利きを逃れる.  
       * CAVEAT: 両王手の場合はPIECE_EMPTYにしておく必要がある
       */
      template<Player P,bool CheapOnly>
      static void generateMoves(const NumEffectState& state,Piece piece,Piece attacker,Action& action);

      /**
       * attacker からの利きを逃れる.  
       */
      template<Player P,bool shouldPromote,bool CheapOnly>
      static void generate(const NumEffectState& state,Piece piece,Action& action);
    };

    template <Player P>
    struct GenerateEscape
    {
      template <class Action>
      static void generate(const NumEffectState& state, Piece piece, Action& a)
      {
	Escape<Action>::template generate<P,true,false>(state, piece, a);
      }
      static void generate(const NumEffectState& state, Piece piece, MoveVector& out)
      {
	move_action::Store store(out);
	Escape<move_action::Store>::template generate<P,true,false>(state, piece, store);
      }
      template <class Action>
      static void generateCheap(const NumEffectState& state, Piece piece, Action& a)
      {
	Escape<Action>::template generate<P,true,true>(state, piece, a);
      }

      static void generateCheap(const NumEffectState& state, Piece piece, MoveVector& out)
      {
	move_action::Store store(out);
	Escape<move_action::Store>::template generate<P,true,false>(state, piece, store);
      }

      /** 不成の受けは作成しないので必要な場合はユーザが作成 */
      template <size_t Capacity>
      static void generateKingEscape(const NumEffectState& state, FixedCapacityVector<Move,Capacity>& out)
      {
	move_action::Store store(out);
	Escape<move_action::Store>::generateKingEscape<P,false>(state, store);
      }
      template <size_t Capacity>
      static void generateCheapKingEscape(const NumEffectState& state, FixedCapacityVector<Move,Capacity>& out)
      {
	move_action::Store store(out);
	Escape<move_action::Store>::generateKingEscape<P,true>(state, store);
      }
    };
    struct GenerateEscapeOfTurn
    {
      template <class Action>
      static void generate(const NumEffectState& state, Piece piece, Action& a)
      {
	if (state.turn() == BLACK)
	{
	  Escape<Action>::template generate<BLACK,true,false>(state, piece, a);
	}
	else
	{
	  Escape<Action>::template generate<WHITE,true,false>(state, piece, a);
	}
      }
    };

  } // namespace move_generator
  struct GenerateEscapeKing
  {
    /** 不成の受けも作成 */
    static void generate(const NumEffectState& state, MoveVector& out);
    static void generateCheap(const NumEffectState& state, MoveVector& out);
  };
  using move_generator::GenerateEscape;
} // namespace osl
#endif // OSL_MOVE_GENERATOR_ESCAPE_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
