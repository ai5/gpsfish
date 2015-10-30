/* king8Info.cc
 */
#include "osl/bits/king8Info.h"
#include "osl/numEffectState.h"
#include "osl/additionalEffect.h"
#include <bitset>
#include <iostream>

#ifndef MINIMAL
std::ostream& osl::checkmate::operator<<(std::ostream& os, King8Info info)
{
  typedef std::bitset<8> bs_t;
  os << bs_t(info.moveCandidate2()) << " " 
     << bs_t(info.libertyCandidate()) << " " 
     << bs_t(info.liberty()) << " " 
     << bs_t(info.dropCandidate());
  return os;
}
#endif
namespace osl
{
  namespace
  {
/**
 * Pの玉やpinされている駒以外からの利きがある.
 * @param state - 盤面(alt(P)の手番とは限らない)
 * @param target - Pの玉の位置
 * @param pos - 盤面上の(Pの玉から長い利きの位置にあるとは限らない)
 * @param pinned - pinされているPの駒のmask
 * @param on_baord_defense - alt(P)の盤面上の駒のうちkingを除いたもの
 * @param dir - dir方向へのattack
 */
template<Player P> inline
bool
#ifdef __GNUC__
__attribute__ ((pure))
#endif
 hasEnoughEffect(NumEffectState const& state,Square target,Square pos, const PieceMask& pinned,
		     const PieceMask& on_board_defense,
  Direction dir)
{
  assert(state.kingSquare(P)==target);
  assert(pos.isOnBoard());
  PieceMask pieceMask = state.effectSetAt(pos)&on_board_defense;
  if(pieceMask.none()) return false;
  PieceMask pieceMask1=pieceMask&~pinned;
  if(pieceMask1.any()) return true;
  pieceMask&=pinned;
  assert(pieceMask.any());
  do {
    int num=pieceMask.takeOneBit();
    Piece p=state.pieceOf(num);
    assert(p.isOnBoardByOwner(P));
    Square pos1=p.square();
    assert(Board_Table.getShortOffset(Offset32(pos,target))
	   == pos-target);
    Direction dir1=Board_Table.getShort8<P>(target,pos1);
    if(dir1==dir) return true;
  } while(pieceMask.any());
  return false;
}
  }
}

template<osl::Player P,osl::Direction Dir>
uint64_t osl::checkmate::
King8Info::hasEffectMask(NumEffectState const& state,Square target, PieceMask pinned,
			 PieceMask on_board_defense)
{
  const Player altP=alt(P);
  Square pos=target-DirectionPlayerTraits<Dir,P>::offset();
  Piece p=state.pieceAt(pos);
  if(p.isEdge())
    return 0ull;
  if(!state.hasEffectAt(P,pos)){
    if(p.canMoveOn<altP>()){ // 自分の駒か空白
      if(p.isEmpty())
	return 0x1000000000000ull+(0x100010100ull<<static_cast<int>(Dir));
      else
	return 0x1000000000000ull+(0x10100ull<<static_cast<int>(Dir));
    }
    else // 相手の駒
      return 0ull;
  }
  const bool has_enough_effect = hasEnoughEffect<altP>(state,target,pos,pinned,on_board_defense,Dir);
  if(has_enough_effect){
    if(p.canMoveOn<altP>()){
      if(p.isEmpty())
	return 0x10100010000ull<<static_cast<int>(Dir);
      else
	return 0x10000ull<<static_cast<int>(Dir);
    }
    else
      return 0x10000000000ull<<static_cast<int>(Dir);
  }
  else{
    if(p.isEmpty())
      return 0x10101010001ull<<static_cast<int>(Dir);
    else if(p.isOnBoardByOwner<P>())
      return 0x10000ull<<static_cast<int>(Dir);
    else
      return 0x10001000000ull<<static_cast<int>(Dir);
  }
}

template<osl::Player P>
const osl::checkmate::King8Info 
#if (defined __GNUC__) && (! defined GPSONE) && (! defined GPSUSIONE)
__attribute__ ((noinline))
#endif
osl::checkmate::King8Info::make(NumEffectState const& state,Square target, PieceMask pinned)
{
  PieceMask on_board_defense=state.piecesOnBoard(alt(P));
  on_board_defense.reset(KingTraits<alt(P)>::index);
  uint64_t canMoveMask=
    hasEffectMask<P,UR>(state,target,pinned,on_board_defense)+
    hasEffectMask<P,R>(state,target,pinned,on_board_defense)+
    hasEffectMask<P,DR>(state,target,pinned,on_board_defense)+
    hasEffectMask<P,U>(state,target,pinned,on_board_defense)+
    hasEffectMask<P,D>(state,target,pinned,on_board_defense)+
    hasEffectMask<P,UL>(state,target,pinned,on_board_defense)+
    hasEffectMask<P,L>(state,target,pinned,on_board_defense)+
    hasEffectMask<P,DL>(state,target,pinned,on_board_defense);
  mask_t longMask=state.longEffectAt(target,P);
  while(longMask.any()){
    int num=longMask.takeOneBit()+PtypeFuns<LANCE>::indexNum*32;
    Piece attacker=state.pieceOf(num);
    Direction d=
      Board_Table.getShort8<P>(target,attacker.square());
    if((canMoveMask&(0x100<<d))!=0){
      canMoveMask-=((0x100<<d)+0x1000000000000ull);
    }
  }
  return King8Info(canMoveMask);
}

template <osl::Player P>
const osl::checkmate::King8Info 
#if (defined __GNUC__) && (! defined GPSONE) && (! defined GPSUSIONE)
__attribute__ ((noinline))
#endif
osl::checkmate::
King8Info::make(NumEffectState const& state, Square target)
{
  return make<P>(state,target,state.pin(alt(P)));
}

const osl::checkmate::King8Info osl::checkmate::
King8Info::make(Player attack, NumEffectState const& state)
{
  const Square king=state.kingSquare(alt(attack));
  if (attack == BLACK)
    return make<BLACK>(state, king);
  else
    return make<WHITE>(state, king);
}

const osl::checkmate::King8Info osl::checkmate::
King8Info::makeWithPin(Player attack, NumEffectState const& state,
		       const PieceMask& pins)
{
  const Square king=state.kingSquare(alt(attack));
  if (attack == BLACK)
    return make<BLACK>(state, king, pins);
  else
    return make<WHITE>(state, king, pins);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

