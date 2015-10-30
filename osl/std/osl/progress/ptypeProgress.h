/* ptpeProgress.h
 */
#ifndef PROGRESS_PTYPE_PROGRESS_H
#define PROGRESS_PTYPE_PROGRESS_H

#include "osl/numEffectState.h"
#include <iosfwd>
namespace osl
{
  namespace progress
  {
    /** ゲームの進行度を駒が自陣からどの程度前に進んでいるかの和で表現
     */
    class PtypeProgressTable
    {
    private:
      CArray<int,PTYPE_SIZE> ptype2Val;
      CArray2d<int,PTYPEO_SIZE,Square::SIZE> pos2Val;
      static const CArray<int,10> yVals;
    public:
      void init();
      ~PtypeProgressTable();
      int progress(PtypeO ptypeo,Square pos) const{
	return pos2Val[ptypeo-PTYPEO_MIN][pos.index()];
      }
    };
    extern PtypeProgressTable Ptype_Progress_Table;

    class PtypeProgress
    {
      int val;
    public:
      explicit PtypeProgress(SimpleState const& state);
      int progress() const{ return val; }
    private:
      void addVal(int d) { val+=d; }
    public:
      void changeTurn() {}
      static int getProgress(const SimpleState& state)
      {
	const PtypeProgress progress(state);
	return progress.progress();
      }
      void update(const SimpleState& , Move last_move) 
      {
	const PtypeO ptypeo = last_move.ptypeO();
	if (last_move.isDrop()) {
	  val += Ptype_Progress_Table.progress(ptypeo,last_move.to())
	    - Ptype_Progress_Table.progress(ptypeo,Square::STAND());
	  return;
	}
	val += Ptype_Progress_Table.progress(ptypeo,last_move.to())
	  - Ptype_Progress_Table.progress(ptypeo,last_move.from());
	Ptype ptype = last_move.capturePtype();
	if (ptype != PTYPE_EMPTY) 
	  val += Ptype_Progress_Table.progress(last_move.capturePtypeO(), Square::STAND())
	    - Ptype_Progress_Table.progress(last_move.capturePtypeO(), last_move.to());
      }

      bool operator==(const PtypeProgress rhs) const { return val == rhs.val; }
    };
    std::ostream& operator<<(std::ostream& os, PtypeProgress prog);
  } // namespace progress
  using progress::PtypeProgress;
} // namespace osl


#endif /* PROGRESS_PTYPE_PROGRESS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
