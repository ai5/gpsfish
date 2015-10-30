/* pin.h
 */
#ifndef _PIN_H
#define _PIN_H

#include "osl/numEffectState.h"
#include "osl/bits/pieceMask.h"
namespace osl
{
  namespace effect_util
  {

    class PinOrOpen
    {
    private:
      /**
       * 駒から8近傍をサーチしていって，その方向の利きがあるか?
       */
      template <Player Defense,Direction DIR>
      static void findDirectionStep(const NumEffectState& state, Square target,
				    PieceMask& pins, PieceMask const& onBoard)
      {
	const Offset offset = DirectionTraits<DIR>::blackOffset();
	Square pos=target-offset;
	int num;
	while(Piece::isEmptyNum(num=state.pieceAt(pos).number()))
	  pos-=offset;
	if(Piece::isEdgeNum(num)) return;
	int num1=state.longEffectNumTable()[num][DIR];
	if(Piece::isPieceNum(num1) && onBoard.test(num1)){
	  pins.set(num);
	}
      }
    public:
      template<Player Defense>
      static PieceMask makeStep(const NumEffectState& state, Square target)
      {
	PieceMask pins;
	PieceMask mask=state.piecesOnBoard(alt(Defense));
	findDirectionStep<Defense,UL>(state,target,pins,mask);
	findDirectionStep<Defense,U>(state,target,pins,mask);
	findDirectionStep<Defense,UR>(state,target,pins,mask);
	findDirectionStep<Defense,L>(state,target,pins,mask);
	findDirectionStep<Defense,R>(state,target,pins,mask);
	findDirectionStep<Defense,DL>(state,target,pins,mask);
	findDirectionStep<Defense,D>(state,target,pins,mask);
	findDirectionStep<Defense,DR>(state,target,pins,mask);
	return pins;
      }

      static PieceMask makeStep(const NumEffectState& state, Square target, 
				       Player defense)
      {
	if(defense==BLACK)
	  return makeStep<BLACK>(state,target);
	else
	  return makeStep<WHITE>(state,target);
      }
      static PieceMask make(const NumEffectState& state,Player defense)
      {
	return makeStep(state,state.kingSquare<BLACK>(),defense);
      }
    };
    /**
     * 素抜きがあるため動けない駒を求める.
     * TODO: 差分計算で高速に更新する
     */
    class Pin
    {
    private:
      template <Direction DIR>
      static void findDirection(const SimpleState& state, Square target,
				Player defense, PieceMask& pins)
      {
	const Offset diff = Board_Table.getOffset(defense, DIR);
	const Piece pin = state.nextPiece(target, diff);
	if(!pin.isOnBoardByOwner(defense)) return;
	const Piece attack_piece = state.nextPiece(pin.square(), diff);
	if(!attack_piece.isOnBoardByOwner(alt(defense))) return;
	if (Ptype_Table.getMoveMask(attack_piece.ptype())
	    & DirectionTraits<DirectionTraits<DIR>::longDir>::mask)
	  pins.set(pin.number());
      }
      /**
       * targetにはdefenseのKINGがあるという前提
       * Pはdefense
       * targetにdefenseのlanceがあると働かない．
       */
      template<Player P>
      static void findLance(const NumEffectState& state, Square target,
			    PieceMask& pins)
      {
	assert(target==state.kingSquare<P>());
	const Offset diff = DirectionPlayerTraits<U,P>::offset();
	Square pos = target+diff;
	Piece pin;
	while ((pin=state.pieceAt(pos)) == Piece::EMPTY())
	  pos += diff;
	if (! pin.isOnBoardByOwner<P>() )
	  return;
	NumBitmapEffect effect=state.effectSetAt(pos);
	mask_t mask=(effect.getMask(1)&mask_t::makeDirect(PtypeFuns<LANCE>::indexMask<<8));
	if(mask.any()){
	  pins.set(pin.number());
	}
      }
    public:
      /**
       * 8方向計算する方法
       * @see make, makeByPiece
       */
      static PieceMask makeNaive(const SimpleState& state, Square target, 
				 Player defense);
    private:
      static bool hasEffectWithOffset(const SimpleState& state, 
				      Piece attack_piece, Piece pin, Offset diff)
      {
	const Piece attack_piece2 = state.nextPiece(pin.square(), diff);
	return attack_piece == attack_piece2;
      }
      static bool hasEffectWithOffset(const NumEffectState& state, 
				      Piece attack_piece, Piece pin, Offset)
      {
	return state.hasEffectByPiece(attack_piece, pin.square());
      }
      static void findOffset(const NumEffectState& state, 
			     Piece attack_piece, Square target, 
			     Player defense, Offset diff, PieceMask& pins)
      {
	const Piece pin = state.nextPiece(target, diff);
	assert(pin.isPiece());
	if (pin.owner() != defense)
	  return;
	if (! hasEffectWithOffset(state, attack_piece, pin, diff))
	  return;
	pins.set(pin.number());
      }
      template <Ptype PTYPE>
      static void findPtype(const NumEffectState& state, Square target, 
			    Player attack, Player defense, PieceMask& result)
      {
	static_assert((PTYPE == ROOK) || (PTYPE == BISHOP), "ptype");
	const PtypeO attack_ptypeo = newPtypeO(attack, PTYPE);
	for (int i=PtypeTraits<PTYPE>::indexMin; 
	     i < PtypeTraits<PTYPE>::indexLimit; ++i)
	{
	  const Piece attack_piece = state.pieceOf(i);
	  if (attack_piece.isOnBoardByOwner(attack))
	  {
	    const Square attack_position = attack_piece.square();
	    const Offset32 diff(attack_position, target);
	    const EffectContent effect 
	      = Ptype_Table.getEffect(attack_ptypeo, diff);
	    if (!effect.hasBlockableEffect()) // 利きはあるか
	      continue;
	    const Offset offset = effect.offset();
#if 0
	    if (offset.zero())	// 隣にいる場合: pin はない
	      continue;
#endif
	    findOffset(state, attack_piece, target, defense, 
			    offset, result);
	  }
	}
      }
    public:
      /**
       * 飛車角は駒の位置から判断する計算方法.
       * @see make, makeNaive
       */
      static PieceMask makeByPiece(const NumEffectState& state, Square target, 
				   Player defense);

      /**
       * 飛車角は駒の位置から判断, KINGに特化
       * @see make, makeNaive
       */
      static PieceMask makeByPieceKing(const NumEffectState& state, Square target, 
				       Player defense);

      /**
       * 駒から8近傍をサーチしていって，その方向の利きがあるか?
       */
      template <Player Defense,Direction DIR>
      static void findDirectionStep(const NumEffectState& state, Square target,
				    PieceMask& pins)
      {
	const Offset offset = DirectionTraits<DIR>::blackOffset();
	Square pos=target-offset;
	int num;
	while(Piece::isEmptyNum(num=state.pieceAt(pos).number()))
	  pos-=offset;
	if(Piece::isEdgeNum(num)) return;
	if(Defense==BLACK){
	  if(!state.pieceAt(pos).pieceIsBlack()) return;
	}
	else{
	  if(state.pieceAt(pos).pieceIsBlack()) return;
	}
	int num1=state.longEffectNumTable()[num][DIR];
	if(!Piece::isPieceNum(num1)) return;
	if(Defense==BLACK){
	  if(!state.pieceOf(num1).pieceIsBlack())
	    pins.set(num);
	}
	else{
	  if(state.pieceOf(num1).pieceIsBlack())
	    pins.set(num);
	}
      }
      template<Player Defense>
      static PieceMask makeStep(const NumEffectState& state, Square target)
      {
	PieceMask pins;
	findDirectionStep<Defense,UL>(state,target,pins);
	findDirectionStep<Defense,U>(state,target,pins);
	findDirectionStep<Defense,UR>(state,target,pins);
	findDirectionStep<Defense,L>(state,target,pins);
	findDirectionStep<Defense,R>(state,target,pins);
	findDirectionStep<Defense,DL>(state,target,pins);
	findDirectionStep<Defense,D>(state,target,pins);
	findDirectionStep<Defense,DR>(state,target,pins);
	return pins;
      }

      static PieceMask makeStep(const NumEffectState& state, Square target, 
				       Player defense)
      {
	if(defense==BLACK)
	  return makeStep<BLACK>(state,target);
	else
	  return makeStep<WHITE>(state,target);
      }
      template<Player Defense>
      static PieceMask makeStep1(const NumEffectState& state, Square target)
      {
	PieceMask pins=PinOrOpen::makeStep<Defense>(state,target);;
	pins &= state.piecesOnBoard(Defense);
	return pins;
      }

      static PieceMask makeStep1(const NumEffectState& state, Square target, 
				       Player defense)
      {
	if(defense==BLACK)
	  return makeStep1<BLACK>(state,target);
	else
	  return makeStep1<WHITE>(state,target);
      }
      /**
       * pin されている駒を計算する
       * @param target 守るマス
       * @param defense pin されている駒の所有者
       * @see makeByPiece, makeNaive
       */
      static PieceMask make(const NumEffectState& state, Square target, 
			    Player defense)
      {
	return makeByPiece(state, target, defense);
      }
      /**
       * defense の王を守るために pin されている駒を計算する
       */
      static PieceMask make(const NumEffectState& state, Player defense)
      {
	return makeByPiece(state, defense);
      }
      static PieceMask makeNaive(const SimpleState& state, Player defense)
      {
	return makeNaive(state, state.kingSquare(defense), defense);
      }
      static PieceMask makeByPiece(const NumEffectState& state, Player defense)
      {
	return makeByPieceKing(state, state.kingSquare(defense), defense);
      }
      /**
       * defense の王について pin されている駒を計算する
       */
      static int count(const NumEffectState& state, Player defense)
      {
	const PieceMask pins = make(state, defense);
	return pins.countBit();
      }
      static int count(const NumEffectState& state, Square target, 
		       Player defense)
      {
	const PieceMask pins = make(state, target, defense);
	return pins.countBit();
      }
    };
    
    
  } // namespace effect_util
} // namespace osl

#endif /* _PIN_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
