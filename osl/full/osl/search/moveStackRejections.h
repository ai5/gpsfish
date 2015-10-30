/* moveStackRejections.h
 */
#ifndef _MOVE_STACK_REJECTIONS_H
#define _MOVE_STACK_REJECTIONS_H
#include "osl/numEffectState.h"
#include "osl/container/moveStack.h"
#include <iosfwd>

namespace osl
{
  namespace search
  {
    /**
     * Moveを分解した差分要素のうちの盤面上の駒に関して保持するデータ
     * pos, ptypeOをshortに入れる．
     */
    struct OnBoardElement {
      short posPtypeO;
      OnBoardElement() {}
      OnBoardElement(Square pos_,PtypeO ptypeO_){
	posPtypeO=makePosPtypeO(pos_,ptypeO_);
      }
      static short makePosPtypeO(Square pos,PtypeO ptypeO)
      {
	return static_cast<short>(pos.uintValue()+(ptypeO<<8));
      }
      Square pos() const{
	return Square::makeDirect(posPtypeO&0xff);
      }
      PtypeO ptypeO() const{
	return static_cast<PtypeO>(posPtypeO>>8);
      }
    };
    /**
     * Moveを分解した差分要素のうち持駒の増減のみを管理する．
     * 相手の持ち駒が増えたか，損得なしかを高速に判定できる．
     */
    struct StandElements {
      union {
	CArray<char,8> c8;
	unsigned long long l8;
      } v;
      StandElements() { v.l8=0x8080808080808080ull; }
      /**
       * altPにとって増える
       */
      void add(Ptype ptype){
	assert(ptype>=PTYPE_BASIC_MIN);
	v.c8[ptype-PTYPE_BASIC_MIN]++;
      }
      /**
       * altPにとって減る
       */
      void sub(Ptype ptype){
	assert(ptype>=PTYPE_BASIC_MIN);
	v.c8[ptype-PTYPE_BASIC_MIN]--;
      }
      bool isZero() const{
	return v.l8==0x8080808080808080ull;
      }
      bool gtZero() const{
	return !isZero() && geZero();
      }
      bool geZero() const{
	return (v.l8&0x8080808080808080ull)==0x8080808080808080ull;
      }
    };
    /**
     * 複数のmoveによる差分を分解したもの
     * 一回のmoveでOnBoarPlusは手番の駒のみ
     *             OnBoardMinusは手番と相手の駒がありうる
     * OnBoardPlusは 最大深さ/2
     * OnBoardMinusは 最大深さ分用意する．
     */
    struct StateElements {
      FixedCapacityVector<OnBoardElement,32> myOnboardPlus;
      FixedCapacityVector<OnBoardElement,32> opOnboardPlus;
      FixedCapacityVector<OnBoardElement,64> myOnboardMinus;
      FixedCapacityVector<OnBoardElement,64> opOnboardMinus;
      StandElements stand;
    public:
      StateElements() {
      }
      void clear() {
      }
      /**
       * 相手が駒を取りptypeの持駒が増えた．
       * 自分がptypeの持駒を使った
       */
      void addStand(Ptype ptype);
      /**
       * 相手がtypeの持駒を使った
       * 自分が駒を取りptypeの持駒が増えた．
       */
      void subStand(Ptype ptype);
      /**
       * 
       */
      void addMyBoard(Square pos,PtypeO ptypeO);
      void subMyBoard(Square pos,PtypeO ptypeO);
      void addOpBoard(Square pos,PtypeO ptypeO);
      void subOpBoard(Square pos,PtypeO ptypeO);
      /**
       * 自分のmoveに従って更新　
       */
      void addMyMove(Move move);
      /**
       * 相手のmoveに従って更新
       */
      void addOpMove(Move move);
      /**
       * 盤面が増減なし
       */
      bool isLoop() const{
	return myOnboardPlus.size()==0 && opOnboardPlus.size()==0 &&
	  myOnboardMinus.size()==0 && opOnboardMinus.size()==0;
      }
      /**
       * PによるSimpleMoveが可能
       */
      template<Player P>
      bool validSimpleMove(NumEffectState const& state,OnBoardElement const& fromElement,OnBoardElement const& toElement) const;
      /**
       * stateにlastMoveを施した後の盤面で　
       * PによるSimpleMoveが可能
       */
      template<Player P>
      bool validSimpleMove(NumEffectState const& state,OnBoardElement const& fromElement,OnBoardElement const& toElement,Move lastMove) const;
      /**
       * PによるcaptureMoveが可能
       */
      template<Player P>
      bool validCaptureMove(NumEffectState const& state, OnBoardElement const& fromElement,OnBoardElement const& toElement,OnBoardElement const& captureElement) const;
      /**
       * stateにlastMoveを施した後の盤面で　
       * PによるCaptureMoveが可能
       */
      template<Player P>
      bool validCaptureMove(NumEffectState const& state, OnBoardElement const& fromElement,OnBoardElement const& toElement,OnBoardElement const& captureElement,Move lastMove) const;
      /**
       * Pがあるmoveをする前の2n手前からmove後への差分からrejectするかどうか決める．
       * playerに取って有利な持ち駒渡しがある -> false
       * playerに取って不利な持ち駒渡しがある
       *   差分がない場合 -> 一手パス+駒損 -> true
       *   差分が自分一手分の時 -> その手が2n手前に可能ならtrue
       *   差分が相手一手分の時 -> move後に相手が可能ならtrue
       * 持ち駒渡しがない
       *   差分がない場合 -> 一手パス
       *    isRootMoveの時は false(rootではPASSできない)
       *    root以外では true
       *   差分が自分一手分の時 -> その手が2n手前に可能ならtrue
       *   差分が相手一手分の時 -> move後に相手が可能で
       *    mayRejectSennichiteの時は true (有利だったら相手に千日手のチャンスを耐えない)
       *    そうでないときは false (千日手の方がましな可能性がある)
       */
      template<Player P>
      bool canReject(NumEffectState const& state,bool mayRejectSennichite,bool isRootMove,Move lastMove,Move actualMove) const;
    };
    std::ostream& operator<<(std::ostream&,OnBoardElement const&);
    std::ostream& operator<<(std::ostream&,StandElements const&);
    std::ostream& operator<<(std::ostream&,StateElements const&);
    class MoveStackRejections
    {
    public:
      /**
       * P - 手番(mのplayer)の立場で判別
       * state - m を実行する前の状態
       * history - mを含まない過去の記録．
       * ply - 探索開始からの深さ
       * m - チェックする手
       * alpha - windowのPに取っての下限
       * checkCountOfAltP - 相手が連続王手の時にいくつ続いたか
       */
      template<Player P>
      static bool probe(NumEffectState const& state,MoveStack const& history,int ply,Move const& m,int alpha, int checkCountOfAltP);
    };
  }
}
#endif /* _MOVE_STACKREJECTIONS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
