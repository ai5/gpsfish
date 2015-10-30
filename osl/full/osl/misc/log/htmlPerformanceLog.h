/* htmlPerformanceLog.h
 */
#ifndef OSL_HTMLPERFORMANCELOG_H
#define OSL_HTMLPERFORMANCELOG_H

#include "osl/misc/log/performanceLog.h"
#include <fstream>

/**
 * mtdfstat, alphabetastat 用の記録
 */
namespace osl
{
  namespace misc
  {
    namespace log
    {
  struct HtmlPerformanceLog : public PerformanceLog
  {
    std::ofstream os;
    HtmlPerformanceLog(const char *filename, const char *title);
    ~HtmlPerformanceLog();
    void record(const char *name, Move correctMove, Move result, 
		unsigned int nodes, unsigned int qnodes,
		double seconds,
		int depth);
  };
    }
  }
}


#endif /* OSL_HTMLPERFORMANCELOG_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
