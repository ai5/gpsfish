/* quiescenceSearch2.cc
 */
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceSearch2.tcc"
#include "osl/checkmate/fixedDepthSearcher.tcc"
#include "osl/eval/progressEval.h"
#include "osl/eval/openMidEndingEval.h"

namespace osl
{
#ifndef MINIMAL
  template class search::QuiescenceSearch2<eval::ProgressEval>;

  template int search::QuiescenceSearch2<eval::ProgressEval>::searchProbCut<BLACK>(int, int, eval::ProgressEval&, Move);
  template int search::QuiescenceSearch2<eval::ProgressEval>::searchProbCut<WHITE>(int, int, eval::ProgressEval&, Move);
#endif
  template class search::QuiescenceSearch2<eval::ml::OpenMidEndingEval>;

  template int search::QuiescenceSearch2<eval::ml::OpenMidEndingEval>::searchProbCut<BLACK>(int, int, eval::ml::OpenMidEndingEval&, Move);
  template int search::QuiescenceSearch2<eval::ml::OpenMidEndingEval>::searchProbCut<WHITE>(int, int, eval::ml::OpenMidEndingEval&, Move);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:


/* ------------------------------------------------------------------------- */
