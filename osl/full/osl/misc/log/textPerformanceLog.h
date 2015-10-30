/* textPerformanceLog.h
 */
#ifndef OSL_TEXTPERFORMANCELOG_H
#define OSL_TEXTPERFORMANCELOG_H

#include "osl/misc/log/performanceLog.h"

/**
 * mtdfstat, alphabetastat 用の記録用 (text)
 * 
 */
namespace osl
{
  namespace misc
  {
    namespace log
    {
  struct TextPerformanceLog : public PerformanceLog
  {
    TextPerformanceLog();
    ~TextPerformanceLog();
    void record(const char *name, Move correctMove,
		Move result, unsigned int nodes, unsigned int qnodes,
		double seconds, int depth);
  };
    }
  }
} // namespace osl


#endif /* OSL_TEXTPERFORMANCELOG_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
