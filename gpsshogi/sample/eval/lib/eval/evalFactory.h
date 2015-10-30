/* evalFactory.h
 */
#ifndef _GPSSSHOGI_SAMPLE_EVAL_LIB_EVAL_FACTORY_H
#define _GPSSSHOGI_SAMPLE_EVAL_LIB_EVAL_FACTORY_H

#include <string>

namespace gpsshogi
{
  class Eval;
  class EvalFactory
  {
  public:
    static gpsshogi::Eval *newEval(const std::string &name);
  };
}

#endif /* _GPSSSHOGI_SAMPLE_EVAL_LIB_EVAL_FACTORY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
