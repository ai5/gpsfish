/* textPerformanceLog.cc
 */
#include "osl/misc/log/textPerformanceLog.h"
#include "osl/csa.h"
#include <iostream>

osl::misc::log::TextPerformanceLog::
TextPerformanceLog()
{
}

osl::misc::log::TextPerformanceLog::~TextPerformanceLog()
{
}

void osl::misc::log::TextPerformanceLog::
record(const char *name, Move correctMove, Move result, 
       unsigned int nodes, unsigned int qnodes, double seconds, int depth)
{
  std::cout << name << "\t";
  if (correctMove == result)
    std::cout << "OK";
  else
  {
    std::cout << csa::show(result);
  }
  std::cout << "\t";
  std::cout << csa::show(correctMove);
  std::cout << "\t" << nodes
	    << "\t" << qnodes
	    << "\t" << nodes + qnodes
	    << "\t" << seconds 
	    << "\t" << depth
	    << std::endl
	    << std::flush;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
