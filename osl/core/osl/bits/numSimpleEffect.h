#ifndef OSL_NUM_SIMPLE_EFFECT_H
#define OSL_NUM_SIMPLE_EFFECT_H

#include "osl/simpleState.h"
#include "osl/bits/numBitmapEffect.h"
#include "osl/bits/boardMask.h"
#include "osl/bits/bitXmask.h"
#include "osl/bits/effectedNumTable.h"
#include "osl/mobility/mobilityTable.h"

namespace osl 
{
  namespace checkmate
  {
    class King8Info;
  }
  namespace effect
  {
    class NumSimpleEffectTable;
    bool operator==(const NumSimpleEffectTable&,const NumSimpleEffectTable&);
    std::ostream& operator<<(std::ostream&, const NumSimpleEffectTable&);

    /**
     * 局面全体の利きデータ.
     */
    class NumSimpleEffectTable
    {
    protected:
      CArray<NumBitmapEffect, Square::SIZE> effects
#ifdef __GNUC__
      __attribute__((aligned(16)))
#endif
	;
      CArray<BoardMask,2> changed_effects; // each player
      /** set of pieces whose effect changed by previous move */
      NumBitmapEffect changed_effect_pieces; 
    public:
      CArray<PieceMask,2> effected_mask;
      CArray<PieceMask,2> effected_changed_mask;
      /** mobility */
      mobility::MobilityTable mobilityTable;
      /** effected num */
      EffectedNumTable effectedNumTable;
      /**
       * ある位置からある方向に短い利きがある時に，その方向の利きを更新する.
       * @param P(template) - ある位置にある駒の所有者
       * @param T(template) - ある位置にある駒の種類
       * @param D(template) - 駒の所有者の立場から見た方向
       * @param OP(template) - 利きを足すか，減らすか
       * @param pos - 駒の位置
       * @param num - 駒番号
       */
      template<Player P,Ptype T,Direction Dir,NumBitmapEffect::Op OP,bool UC>
      void doEffectShort(const SimpleState& state,Square pos,int num)
      {
	if ((PtypeTraits<T>::moveMask & DirectionTraits<Dir>::mask)!=0)
	  {
	    const Square target = pos+DirectionPlayerTraits<Dir,P>::offset();
	    effects[target.index()].template opEqual<OP>(NumBitmapEffect::makeEffect<P>(num));
	    if(UC){
	      int posIndex=BoardMask::index(pos);
	      changed_effects[P].set(posIndex+BoardMask::getIndexOffset<Dir,P>());
	      int num1;
	      if(Piece::isPieceNum(num1=state.pieceAt(target).number())){
		if(OP==NumBitmapEffect::Add){
		  effected_mask[P].set(num1);
		}
		else{ // OP==Sub
		  if((effects[target.index()].getMask(1)&NumBitmapEffect::playerEffectMask(P)).none()){
		    effected_mask[P].reset(num1);
		  }
		}
		effected_changed_mask[P].set(num1);
	      }
	    }
	  }
      }
      /**
       * ある位置からある方向に長い利きがある時に，その方向の利きを更新する.
       * @param P(template) - ある位置にある駒の所有者
       * @param T(template) - ある位置にある駒の種類
       * @param Dir(template) - 黒の立場から見た方向
       * @param OP(template) - 利きを足すか，減らすか
       * @param state - 盤面(動かした後)
       * @param pos - 駒の位置
       * @param num - 駒番号
       */
      template<Player P,Ptype T,Direction Dir,NumBitmapEffect::Op OP,bool UC>
      void doEffectLong(const SimpleState& state,Square pos,int num)
      {
	if ((PtypeTraits<T>::moveMask & DirectionTraits<DirectionPlayerTraits<Dir,P>::directionByBlack>::mask)!=0)
	  {
	    int posIndex;
	    if(UC){
	      posIndex=BoardMask::index(pos);
	    }
	    const Offset offset=DirectionPlayerTraits<Dir,BLACK>::offset();
	    assert(!offset.zero());
	    NumBitmapEffect effect=NumBitmapEffect::makeLongEffect<P>(num);

	    const Direction SD=longToShort(Dir);
	    if(OP==NumBitmapEffect::Sub){
	      Square ePos=mobilityTable.get(longToShort(Dir),num);
	      int count=((SD==D || SD==DL || SD==DR) ? ePos.y()-pos.y() :
			 ( (SD==U || SD==UL || SD==UR) ? pos.y()-ePos.y() :
			   ( SD==L ? ePos.x()-pos.x() : pos.x()-ePos.x())));
	      assert(0<=count && count<=9);
	      if(UC){
		for(int i=1;i<count;i++){
		  pos+=offset;
		  posIndex+=BoardMask::getIndexOffset<Dir,BLACK>();
		  effects[pos.index()].template opEqual<OP>(effect);
		  changed_effects[P].set(posIndex);
		}
		Piece p;
		mobilityTable.set(longToShort(Dir),num,Square::STAND());
		int num1=state.pieceAt(ePos).number();
		if (!Piece::isEdgeNum(num1)){
		  effectedNumTable[num1][SD]=EMPTY_NUM;
		  effects[ePos.index()].template opEqual<OP>(effect);
		  effected_changed_mask[P].set(num1);
		  posIndex+=BoardMask::getIndexOffset<Dir,BLACK>();
		  changed_effects[P].set(posIndex);
		  if((effects[ePos.index()].getMask(1)&NumBitmapEffect::playerEffectMask(P)).none()){
		    effected_mask[P].reset(num1);
		  }
		}
	      }
	      else{
		for(int i=0;i<count;i++){
		  pos+=offset;
		  effects[pos.index()].template opEqual<OP>(effect);
		}
		int num1=state.pieceAt(ePos).number();
		if (!Piece::isEdgeNum(num1))
		  effectedNumTable[num1][SD]=EMPTY_NUM;
	      }
	    }
	    else{ // OP==Add
	      for (;;)
		{
		  pos=pos+offset;
		  if(UC){
		    posIndex+=BoardMask::getIndexOffset<Dir,BLACK>();
		    changed_effects[P].set(posIndex);
		  }
		  effects[pos.index()].template opEqual<OP>(effect);
		  // effect内にemptyを含むようにしたら短くなる
		  int num1=state.pieceAt(pos).number();
		  if (!Piece::isEmptyNum(num1)){
		    if(UC){
		      mobilityTable.set(longToShort(Dir),num,pos);
		      if(!Piece::isEdgeNum(num1)){
			effectedNumTable[num1][SD]=num;
			changed_effects[P].set(posIndex);
 			effected_mask[P].set(num1);
			effected_changed_mask[P].set(num1);
 		      }
 		    }
		    else if(!Piece::isEdgeNum(num1)){
		      effectedNumTable[num1][SD]=num;
		    }
		    break;
 		  }
 		}
	    }
 	  }
      }
      /**
       * ある種類の駒が持つ利きを更新する.
       * @param P(template) - ある位置にある駒の所有者
       * @param T(template) - ある位置にある駒の種類
       * @param OP(template) - 利きを足すか，減らすか
       * @param state - 盤面(動かした後)
       * @param pos - 駒の位置
       * @param num - 駒番号
       */
      template<Player P,Ptype T,NumBitmapEffect::Op OP,bool UC>
      void doEffectBy(const SimpleState& state,Square pos,int num);
      /**
       * ある種類の駒が持つ利きを更新する.
       * @param OP(template) - 利きを足すか，減らすか
       * @param state - 盤面(動かした後)
       * @param ptypeo - 駒の種類
       * @param pos - 駒の位置
       * @param num - 駒番号
       */
      template<NumBitmapEffect::Op OP,bool UC>
      void doEffect(const SimpleState& state,PtypeO ptypeo,Square pos,int num);

      /**
       * ある駒が持つ利きを更新する.
       * @param OP(template) - 利きを足すか，減らすか
       * @param state - 盤面(動かした後)
       * @param p - 駒
       */
      template<NumBitmapEffect::Op OP,bool UC>
      void doEffect(const SimpleState& state,Piece p)
      {
	doEffect<OP,UC>(state,p.ptypeO(),p.square(),p.number());
      }
      /**
       * 盤面のデータを元に初期化する.
       * @param state - 盤面
       */
      void init(const SimpleState& state);
      /**
       * コンストラクタ.
       */
      NumSimpleEffectTable(const SimpleState& state)
      {
	assert(reinterpret_cast<size_t>(this) % 16 == 0);
	init(state);
      }
      /**
       * ある位置の利きデータを取り出す.
       * @param pos - 位置
       */
      const NumBitmapEffect effectSetAt(Square pos) const
      {
	return effects[pos.index()];
      }
      /**
       * posに駒を設置/削除して長い利きをブロック/延長する際の利きデータの更新.
       * @param OP(template) - 利きを足すか，減らすか
       * @param state - 局面の状態 posに駒を置く前でも後でもよい
       * @param pos - 変化する位置
       */
      template<NumBitmapEffect::Op OP,bool UC>
      void doBlockAt(const SimpleState& state,Square pos,int piece_num);
      friend bool operator==(const NumSimpleEffectTable& et1,const NumSimpleEffectTable& et2);
      /*
       *
       */
      const BoardMask changedEffects(Player pl) const{
	return changed_effects[pl];
      }
      const NumBitmapEffect changedPieces() const {
	return changed_effect_pieces;
      }
      const PieceMask effectedMask(Player pl) const {
	return effected_mask[playerToIndex(pl)];
      }
      const PieceMask effectedChanged(Player pl) const {
	return effected_changed_mask[playerToIndex(pl)];
      }
      void setChangedPieces(NumBitmapEffect const& effect) {
	changed_effect_pieces |= effect;
      }
      void clearChangedEffects(){
	changed_effects[0].clear();
	changed_effects[1].clear();
	changed_effect_pieces.resetAll();
      }
      void invalidateChangedEffects(){
	changed_effects[0].invalidate();
	changed_effects[1].invalidate();
	changed_effect_pieces.setAll();
      }
      void clearEffectedChanged(){
	effected_changed_mask[0].resetAll();
	effected_changed_mask[1].resetAll();
      }
      /** 主要部分を高速にコピーする. 盤の外や直前の利きの変化などの情報はコピーされない*/
      void copyFrom(const NumSimpleEffectTable& src);
    };

    inline bool operator!=(const NumSimpleEffectTable& et1,const NumSimpleEffectTable& et2)
    {
      return !(et1==et2);
    }

  } // namespace effect
  using effect::NumBitmapEffect;

} // namespace osl

/**
 * posに駒を設置/削除して長い利きをブロック/延長する際の利きデータの更新.
 * xorなのでposに元々駒があって，取り除く時にも呼び出せる．
 * @param state - 局面の状態 posに駒を置く前でも後でもよい
 * @param pos - 変化する位置
 */
template<osl::effect::NumBitmapEffect::Op OP,bool UC>
void osl::effect::
NumSimpleEffectTable::doBlockAt(const SimpleState& state,Square pos,int piece_num)
{
  if(UC){
    setChangedPieces(effects[pos.index()]);
  }
  mask_t mask1 =((effects[pos.index()].getMask(1))
		 & NumBitmapEffect::longEffectMask());
  while (mask1.any()){
    int num=mask1.takeOneBit()+NumBitmapEffect::longToNumOffset;
    assert(32<=num && num<=39);
    Piece p1=state.pieceOf(num);
    Player pl1=p1.owner();
    assert(p1.ptype()!=PPAWN);
    Square pos1=p1.square();
    Offset offset0;
    Direction d=Board_Table.getShort8<BLACK>(pos1,pos,offset0);
    if(OP==NumBitmapEffect::Sub){
      Square endSquare=mobilityTable.get(d,num);
      NumBitmapEffect effect=NumBitmapEffect::makeLongEffect(pl1,num);
      Piece p;
      Square pos2=pos+offset0;
      int pos2Index, offset81;
      if(UC){
	int posIndex=BoardMask::index(pos);
	pos2Index=BoardMask::index(pos2);
	offset81=pos2Index-posIndex;
      }
      for(;pos2!=endSquare;pos2+=offset0){
 	if(UC){
	  changed_effects[pl1].set(pos2Index);
	  pos2Index+=offset81;
	}
	effects[pos2.index()].template opEqual<OP>(effect);
      }
      effects[pos2.index()].template opEqual<OP>(effect);
      int num1=state.pieceAt(endSquare).number();
      if (!Piece::isEdgeNum(num1)){
	effectedNumTable[num1][d]=EMPTY_NUM;
	if(UC){
	  changed_effects[pl1].set(pos2Index);
	  if((effects[endSquare.index()].getMask(1)&NumBitmapEffect::playerEffectMask(pl1)).none()){
	    effected_mask[pl1].reset(num1);
	  }
	  effected_changed_mask[pl1].set(num1);
	  mobilityTable.set(d,num,pos);
	}
      }
      else 
	mobilityTable.set(d,num,pos);
      effectedNumTable[piece_num][d]=num;
    }
    else{
      NumBitmapEffect effect=NumBitmapEffect::makeLongEffect(pl1,num);
      Square pos2=pos+offset0;
      int pos2Index, offset81;
      if(UC){
	int posIndex=BoardMask::index(pos);
	pos2Index=BoardMask::index(pos2);
	offset81=pos2Index-posIndex;
      }
      for(;;){
	int num1=state.pieceAt(pos2).number();
        if(!Piece::isEmptyNum(num1)){
	  if(UC){
	    mobilityTable.set(d,num,pos2);
	    if(!Piece::isEdgeNum(num1)){
	      effectedNumTable[num1][d]=num;
	      effects[pos2.index()].template opEqual<OP>(effect);
	      changed_effects[pl1].set(pos2Index);
	      effected_mask[pl1].set(num1);
	      effected_changed_mask[pl1].set(num1);
	    }
	  }
	  else if(!Piece::isEdgeNum(num1)){
	    effectedNumTable[num1][d]=num;
	    effects[pos2.index()].template opEqual<OP>(effect);
	  }
	  break;
 	}
	if(UC){
	  changed_effects[pl1].set(pos2Index);
	  pos2Index+=offset81;
	}
	effects[pos2.index()].template opEqual<OP>(effect);
	pos2+=offset0;
      }
    }
  }
}

#endif // OSL_NUM_SIMPLE_EFFECT_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
