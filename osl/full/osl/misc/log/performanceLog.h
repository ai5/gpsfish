/* performanceLog.h
 */
#ifndef OSL_PERFORMANCELOG_H
#define OSL_PERFORMANCELOG_H

#include "osl/basic_type.h"

/**
 * mtdfstat, alphabetastat 用の記録の親クラス
 * 
 */
namespace osl
{
  namespace misc
  {
    namespace log
    {
  struct PerformanceLog
  {
    virtual ~PerformanceLog() {}
    virtual void record(const char *name, Move correctMove,
			Move result, unsigned int nodes,
			unsigned int qnodes,
			double seconds,
			int depth) = 0;
  };
    }
  }
} // namespace osl


#endif /* OSL_PERFORMANCELOG_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
