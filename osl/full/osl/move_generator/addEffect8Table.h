#ifndef _ADD_EFFECT8_TABLE_H
#define _ADD_EFFECT8_TABLE_H

#include "osl/numEffectState.h"
#include <utility>

namespace osl
{
  namespace move_generator
  {
    namespace addeffect8
    {
    /**
     * 利きをつける手を生成するためのテーブル．
     * 
     */
    typedef std::pair<Offset,Offset> OffsetPair;
    typedef std::pair<Square,Offset> PO;
    typedef std::pair<Square,OffsetPair> POO;
    class AddEffect8Table
    {
      static const int maxDropSquare=32;
      CArray3d<Square,PTYPE_SIZE,Square::SIZE,maxDropSquare> dropSquare;
      static const int maxLongDropDirect=8;
      CArray3d<Offset,PTYPE_SIZE,Square::SIZE,maxLongDropDirect> longDropDirect;

      static const int maxLongDropSquare=32;
      CArray3d<PO,PTYPE_SIZE,Square::SIZE,maxLongDropSquare> longDropSquare;
      static const int maxLongDrop2Square=8;
      CArray3d<POO,PTYPE_SIZE,Square::SIZE,maxLongDrop2Square> longDrop2Square;

      static const int maxShortMoveOffset=32;
      CArray3d<Offset,PTYPE_SIZE,Offset32::SIZE,maxShortMoveOffset> shortMoveOffset;
      static const int maxShortPromoteMoveOffset=32;
      CArray3d<Offset,PTYPE_SIZE,Offset32::SIZE,maxShortPromoteMoveOffset> shortPromoteMoveOffset;
      static const int maxLongMoveOffset=32;
      CArray3d<OffsetPair,PTYPE_SIZE,Offset32::SIZE,maxLongMoveOffset> longMoveOffset;
      CArray2d<OffsetPair,PTYPE_SIZE,Offset32::SIZE> betweenOffset;
    public:
      void init();

      /**
       * ptypeの駒を打って敵の玉の8近傍に短い利きがつく地点の相対位置.
       * 長い利きを8近傍内につけるのも可
       * 黒から見てkingSquare+offsetに駒を打つと良い
       * 手番から見た相手の玉のSquare
       */
      Square getDropSquare(Ptype ptype,Square kingSquare,int i) const
      {
	//	std::cerr << "getDropOffset(" << ptype << "," << i << ")" << std::endl;
	return dropSquare[ptype][kingSquare.index()][i];
      }

      /**
       * ptypeの駒を打って敵の玉の8近傍に長い利きをつける.
       * 間に駒がなければ，王手になるタイプ
       */
      Offset getLongDropDirect(Ptype ptype,Square kingSquare,int i) const
      {
	return longDropDirect[ptype][kingSquare.index()][i];
      }

      /**
       * ptypeの駒を打って敵の玉の8近傍に長い利きをつける.
       * 1方向
       * 黒から見てkingSquare+firstに打つ手から始めて，+secondしていっても
       * emptyならOK
       */
      PO getLongDropSquare(Ptype ptype,Square kingSquare,int i) const
      {
	return longDropSquare[ptype][kingSquare.index()][i];
      }

      /**
       * ptypeの駒を打って敵の玉の8近傍に長い利きをつける.
       * 1方向
       * 黒から見てkingSquare+firstに打つ手から始めて，+secondしていっても
       * emptyならOK
       */
      POO getLongDrop2Square(Ptype ptype,Square kingSquare,int i) const
      {
	return longDrop2Square[ptype][kingSquare.index()][i];
      }

      /**
       * ptypeの駒を動かして(長い動きも可
       * )敵の玉の8近傍に短い利きがつく地点の相対位置.
       * 黒から見てkingSquare+offsetに駒を移動すると良い
       * @param isPromote - promoteするかしないか
       * @param ptype - 移動前の駒の種類
       * @param o32 - targetからみたfromの相対位置 = Offset32(from,target)
       * @param i - 何番目か
       */
      Offset getShortMoveOffset(bool isPromote,Ptype ptype,Offset32 o32,int i) const
      {
	if(!isPromote){
	  assert(i<maxShortMoveOffset);
	  return shortMoveOffset[ptype][o32.index()][i];
	}
	else{
	  assert(i<maxShortPromoteMoveOffset);
	  return shortPromoteMoveOffset[ptype][o32.index()][i];
	}
      }
      /**
       * ptypeの駒を動かして敵の玉の8近傍に長い利きがつく地点の相対位置.
       * 黒から見てkingSquare+firstに駒を移動できて，
       * kingSquare+secondとの間がずっとemptyなら良い
       * @param ptype - 移動前の駒の種類
       * @param o32 - targetからみたfromの相対位置 = Offset32(from,target)
       * @param i - 何番目か
       */
      OffsetPair getLongMoveOffset(Ptype ptype,Offset32 o32,int i) const
      {
	assert(i<maxLongMoveOffset);
	return longMoveOffset[ptype][o32.index()][i];
      }
      /**
       * ptypeの駒が敵の駒がなければ8近傍に長い利きがある場合.
       * 黒からsecondの方向にたどってkingSquare+firstまでに邪魔が1つなら良い
       * @param ptype - 移動前の駒の種類
       * @param o32 - targetからみたfromの相対位置 = Offset32(from,target)
       */
      OffsetPair getBetweenOffset(Ptype ptype,Offset32 o32) const
      {
	return betweenOffset[ptype][o32.index()];
      }
    private:
      void initDropSquare();
      void initLongDropSquare();
      void initMoveOffset();
    };
    }
    extern addeffect8::AddEffect8Table Add_Effect8_Table;

  } // namespace move_generator
} // namespace osl
#endif /* _ADD_EFFECT8_TABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
