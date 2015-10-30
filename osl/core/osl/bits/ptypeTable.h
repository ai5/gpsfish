/* ptypeTable.h
 */
#ifndef OSL_PTYPETABLE_H
#define OSL_PTYPETABLE_H

#include "osl/basic_type.h"
#include "osl/bits/ptypeTraits.h"
#include "osl/bits/effectContent.h"
#include "osl/container.h"
#include "osl/bits/offset32.h"
#include "osl/bits/mask.h"

namespace osl
{
  class PtypeTable
  {
  private:
    CArray<mask_t, PTYPE_SIZE> numMaskLows;
    CArray<int, PTYPE_SIZE> numIndices;
    CArray<const char *, PTYPE_SIZE> names;
    CArray<const char *, PTYPE_SIZE> csaNames;
    CArray<bool, PTYPE_SIZE> betterToPromote;
    CArray<int, PTYPE_SIZE> moveMasks;
    CArray<int, PTYPE_SIZE> indexMins;
    CArray<int, PTYPE_SIZE> indexLimits;

    CArray2d<int, 2, PTYPE_SIZE> canDropLimit;
    // これらの2次元配列は2^nにそろえておいた方が速い．
    CArray2d<EffectContent,PTYPEO_SIZE,Offset32::SIZE> effectTable;
    CArray2d<EffectContent,PTYPEO_SIZE,Offset32::SIZE> effectTableNotLongU;
    CArray2d<unsigned int, 2, SHORT_DIRECTION_SIZE> shortMoveMask;
    
    template<Ptype T> void initPtypeSub(Int2Type<false> isBasic);
    template<Ptype T> void initPtypeSub(Int2Type<true> isBasic);
    template<Ptype T> void initPtype();
  public:
    PtypeTable();
  private:
    void init();
  public:
    unsigned int getShortMoveMask(Player p,PtypeO ptypeo,Direction dir) const
    {
      return shortMoveMask[playerToIndex(p)][static_cast<int>(dir)] &
	(1<<(ptypeo-PTYPEO_MIN));
    }
    mask_t getMaskLow(Ptype ptype) const
    {
      return numMaskLows[ptype];
    }
    int getIndex(Ptype) const { return 0; }
    /**
     * 遅くて良い?
     */
    bool hasLongMove(Ptype ptype) const
    {
      return getIndexMin(unpromote(ptype))>=32;
    }
    bool isBetterToPromote(Ptype ptype) const
    {
      return betterToPromote[ptype];
    }
    int getCanDropLimit(Player player,Ptype ptype) const
    {
      assert(isValid(ptype) && !isPromoted(ptype));
      return canDropLimit[playerToIndex(player)][ptype];
    }

    bool canDropTo(Player pl, Ptype ptype, Square pos) const
    {
      if (pl == BLACK)
	return pos.y() >= getCanDropLimit(BLACK,ptype);
      else
	return pos.y() <= getCanDropLimit(WHITE,ptype);
    }

    const char *getName(Ptype ptype) const
    {
      return names[ptype];
    }
    const char *getCsaName(Ptype ptype) const
    {
      return csaNames[ptype];
    }
    int getMoveMask(Ptype ptype) const
    {
      return moveMasks[ptype];
    }
    int getIndexMin(Ptype ptype) const
    {
      assert(isBasic(ptype));
      return indexMins[ptype];
    }
    int getIndexLimit(Ptype ptype) const
    {
      assert(isBasic(ptype));
      return indexLimits[ptype];
    }
    static int getKingIndex(Player p) 
    {
      assert(isValid(p));
      if (p==BLACK)
	return KingTraits<BLACK>::index;
      else
	return KingTraits<WHITE>::index;
    }
    /** 
     * fromにいるptypeoがtoに利きを持つか?
     * @param ptypeo - 駒の種類
     * @param from - 駒の位置
     * @param to - 利きをチェックするマスの位置
     */
    const EffectContent getEffect(PtypeO ptypeo,Square from, Square to) const
    {
      assert(from.isOnBoard() && to.isOnBoard());
      return getEffect(ptypeo,Offset32(to,from));
    }
    const EffectContent& getEffect(PtypeO ptypeo,Offset32 offset32) const
    {
      assert(isValidPtypeO(ptypeo));
      return effectTable[ptypeo-PTYPEO_MIN][offset32.index()];
    }
  private:
    EffectContent& effect(PtypeO ptypeo,Offset32 offset32)
    {
      assert(isValidPtypeO(ptypeo));
      const int i1 = ptypeo-PTYPEO_MIN;
      const int i2 = offset32.index();
      return effectTable[i1][i2];
    }
  public:
    /** ptypeo が，自分から offset のところに効きを持つか? U除く */
    const EffectContent
    getEffectNotLongU(PtypeO ptypeo, Square from, Square to) const
    {
      assert(isValidPtypeO(ptypeo));
      assert(from.isOnBoard() && to.isOnBoard());
      Offset32 offset32=Offset32(to,from);
      return effectTableNotLongU[ptypeo-PTYPEO_MIN][offset32.index()];
    }
    bool hasUnblockableEffect(PtypeO attacker, Square from, Square to) const
    {
      const EffectContent effect = getEffect(attacker, from, to);
      return effect.hasUnblockableEffect();
    }
  };
  
  extern const PtypeTable Ptype_Table;

} // namespace osl


#endif /* OSL_PTYPETABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
