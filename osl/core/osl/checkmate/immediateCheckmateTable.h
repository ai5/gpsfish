/* immediateCheckmateTable.h
 */
#ifndef OSL_CHECKMATE_IMMEDIATE_CHECKMATE_TABLE_H
#define OSL_CHECKMATE_IMMEDIATE_CHECKMATE_TABLE_H
#include "osl/basic_type.h"
#include "osl/bits/king8Info.h"

namespace osl
{
  namespace checkmate
  {
    class ImmediateCheckmateTable
    {
    private:
      CArray<unsigned char,0x10000u> dropPtypeMasks;
      CArray2d<unsigned char,0x100u,PTYPE_SIZE> ptypeDropMasks;
      CArray2d<unsigned char,PTYPE_SIZE,8> blockingMasks;
      CArray2d<unsigned short,PTYPE_SIZE,8> noEffectMasks;
    public:
      ImmediateCheckmateTable();
      unsigned char dropPtypeMaskOf(unsigned int liberty_drop_mask) const
      {
	return dropPtypeMasks[liberty_drop_mask];
      }
      unsigned char dropPtypeMask(King8Info canMoveMask) const
      {
	return dropPtypeMaskOf(canMoveMask.libertyDropMask());
      }
      unsigned int ptypeDropMask(Ptype ptype,King8Info canMoveMask) const
      {
	return ptypeDropMasks[canMoveMask.liberty()][ptype];
      }
      unsigned int blockingMask(Ptype ptype,Direction dir) const
      {
	assert(static_cast<int>(dir)<8);
	return blockingMasks[ptype][dir];
      }
      unsigned int noEffectMask(Ptype ptype,Direction dir) const
      {
	assert(static_cast<int>(dir)<8);
	return noEffectMasks[ptype][dir];
      }
    };
    extern const ImmediateCheckmateTable Immediate_Checkmate_Table;
  }
}

#endif /* OSL_CHECKMATE_IMMEDIATE_CHECKMATE_TABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

