/* king8Info.h
 */
#ifndef OSL_CHECKMATE_KING8INFO_H
#define OSL_CHECKMATE_KING8INFO_H

#include "osl/numEffectState.h"
#include "osl/additionalEffect.h"
#include <iosfwd>
namespace osl
{
  namespace checkmate
  {
    /**
     * 敵玉の8近傍の状態を表す. 王手がかかっている場合も含むことにする．
     * Dirは相手の玉に対してDir方向で王手をかける位置
     * 0-7 : 敵玉以外の利きがなく，自分の利きがある空白
     *       (駒を打つ候補となりうる点)
     * 8-15 : 敵玉がDirに移動可能(王手がかかっている場合は長い利きも延ばす)
     * 16-23 : 空白か味方の駒(利き次第では移動可能になる)
     * 24-31 : 敵玉以外の利きがなく，自分の利きがある空白，敵駒
     * (OLD 
     * 24-31 : 敵玉以外の利きがなく，自分の利きが2つ以上ある空白，敵駒
     *       (駒を動かす候補となりうる点) )
     * 32-39 : 空白(駒打ち王手の候補)
     * 40-47 : 味方の利き(kingの利きも含んでいる)がある空白，敵駒
     * 48-51 : 敵玉がDirに移動可能(王手がかかっている場合は長い利きも延ばす)な数
     */
    class King8Info
    {
      uint64_t value;
    public:
      explicit King8Info(uint64_t v) : value(v)
      {
      }

      template<Player P>
      static const King8Info make(NumEffectState const& state,Square king, PieceMask pinned);
      template<Player P>
      static const King8Info make(NumEffectState const& state,Square king);

      /** alt(attack) のking について計算 */
      static const King8Info make(Player attack, NumEffectState const& state);
      /** alt(attack) のking について計算. pinが既に求まっている */
      static const King8Info makeWithPin(Player attack, NumEffectState const& state,
					 const PieceMask& pinned);
      uint64_t uint64Value() const { return value; }
      
      /** 0-7 bit 目を返す */
      unsigned int dropCandidate() const
      {
	return (unsigned int)(value&0xffull);
      }
      /** 8-15 bit 目を 0-7bitにshiftして返す */
      unsigned int liberty() const
      {
	return (unsigned int)((value>>8)&0xffull);
      }
      /** 0-15bit */
      unsigned int libertyDropMask() const
      {
	return (unsigned int)(value&0xffffull);
      }
      /** 16-23 bit 目を 0-7bitにshiftして返す */
      unsigned int libertyCandidate() const
      {
	return (unsigned int)((value>>16)&0xffull);
      }
      /** 24-31 bit 目を 0-7bitにshiftして返す */
      unsigned int moveCandidate2() const
      {
	return (unsigned int)((value>>24)&0xffull);
      }
      unsigned int spaces() const
      {
	return (unsigned int)((value>>32)&0xffull);
      }
      unsigned int moves() const
      {
	return (unsigned int)((value>>40)&0xffull);
      }
      /** libertyの数 */
      unsigned int libertyCount() const
      {
	return (unsigned int)((value>>48)&0xfull);
      }
      template<Player P,Direction Dir>
      unsigned int moveCandidateDir(NumEffectState const& state,Square target) const{
	if((value & (1ull<<(24+Dir)))==0) return 0;
	Square pos=target-DirectionPlayerTraits<Dir,P>::offset();
	if(state.countEffect(P,pos)<2 &&
	   !effect_util::AdditionalEffect::hasEffect(state,pos,P)) return 0;
	return 1;
      }
      template<Player P>
      unsigned int countMoveCandidate(NumEffectState const& state) const
      {
	const Player altP=alt(P);
	Square king=state.kingSquare<altP>();
	return moveCandidateDir<P,UL>(state,king)+
	  moveCandidateDir<P,U>(state,king)+
	  moveCandidateDir<P,UR>(state,king)+
	  moveCandidateDir<P,L>(state,king)+
	  moveCandidateDir<P,R>(state,king)+
	  moveCandidateDir<P,DL>(state,king)+
	  moveCandidateDir<P,D>(state,king)+
	  moveCandidateDir<P,DR>(state,king);
      }
      unsigned int countMoveCandidate(Player player, NumEffectState const& state) const
      {
	if(player==BLACK) return countMoveCandidate<BLACK>(state);
	else return countMoveCandidate<WHITE>(state);
      }
      template<Player P>
      unsigned int moveCandidateMask(NumEffectState const& state) const
      {
	const Player altP=alt(P);
	Square king=state.kingSquare<altP>();
	return (moveCandidateDir<P,UL>(state,king)<<UL)+
	  (moveCandidateDir<P,U>(state,king)<<U)+
	  (moveCandidateDir<P,UR>(state,king)<<UR)+
	  (moveCandidateDir<P,L>(state,king)<<L)+
	  (moveCandidateDir<P,R>(state,king)<<R)+
	  (moveCandidateDir<P,DL>(state,king)<<DL)+
	  (moveCandidateDir<P,D>(state,king)<<D)+
	  (moveCandidateDir<P,DR>(state,king)<<DR);
      }
      template<Player P>
      bool hasMoveCandidate(NumEffectState const& state) const
      {
	const Player altP=alt(P);
	Square king=state.kingSquare<altP>();
	if(moveCandidateDir<P,U>(state,king)!=0) return true;
	if(moveCandidateDir<P,UL>(state,king)!=0) return true;
	if(moveCandidateDir<P,UR>(state,king)!=0) return true;
	if(moveCandidateDir<P,L>(state,king)!=0) return true;
	if(moveCandidateDir<P,R>(state,king)!=0) return true;
	if(moveCandidateDir<P,D>(state,king)!=0) return true;
	if(moveCandidateDir<P,DL>(state,king)!=0) return true;
	if(moveCandidateDir<P,DR>(state,king)!=0) return true;
	return false;
      }
    private:
      /**
       * alt(P)の玉にDirの方向で迫るcanMoveMaskを計算する.
       * @param P(template) - 攻撃側のplayer
       * @param Dir(template) - 敵玉に迫る方向(shortの8方向)
       * @param state - 初期状態
       * @param target - alt(P)の玉があるpotision
       */
      template<Player P,Direction Dir>
      static uint64_t
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      hasEffectMask(NumEffectState const& state,Square target, PieceMask pinned,
		    PieceMask on_board_defense);
    };

    std::ostream& operator<<(std::ostream&, King8Info);
  } // namespace checkmate
  using checkmate::King8Info;
} // namespace osl

#endif /* OSL_CHECKMATE_KING8INFO_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

