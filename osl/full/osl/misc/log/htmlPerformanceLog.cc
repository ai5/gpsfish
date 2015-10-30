/* htmlPerformanceLog.cc
 */
#include "osl/misc/log/htmlPerformanceLog.h"
#include "osl/csa.h"

osl::misc::log::HtmlPerformanceLog::
HtmlPerformanceLog(const char *filename, const char *title)
  : os(filename)
{
  os << "<html><head><title>" << title << "</title></head>\n<body>\n";
  os << "<table border=1>\n";
  os << "<tr><td></td><td>search result</td><td>correct move</td>"
     << "<td>#nodes</td><td>seconds</td><td>depth</td></tr>\n";
}

osl::misc::log::HtmlPerformanceLog::~HtmlPerformanceLog()
{
  os << "</table>\n";
  os << "</body>\n";
}

void osl::misc::log::HtmlPerformanceLog::
record(const char *name, Move correctMove, Move result, 
       unsigned int nodes, unsigned int qnodes, double seconds, int depth)
{
  if (correctMove == result)
    os << "<tr bgcolor=\"green\">";
  else
    os << "<tr>";
  os << "<td>" << name << "</td><td>";
  if (correctMove == result)
    os << "OK";
  else
  {
    os << csa::show(result);
  }
  os << "</td><td>";
  os << csa::show(correctMove);
  os << "</td>\n\t<td id=\"nodes\" align=right>" << nodes + qnodes << "</td>\n"
     << "\t<td id=\"seconds\" align=right>" << seconds << "</td>\n"
     << "\t<td id=\"depth\" align=right>" << depth << "</td></tr>\n"
     << std::flush;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
