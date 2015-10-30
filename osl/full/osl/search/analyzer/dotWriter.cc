/* dotWriter.cc
 */
#include "osl/search/analyzer/dotWriter.h"
#include "osl/search/analyzer/recordSet_.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/csa.h"
#include <boost/format.hpp>
#include <sstream>
#include <iostream>
#include <cassert>

// #define BOOST_FORMAT_BUG

osl::search::analyzer::DotWriter::
DotWriter(std::ostream& o)
    : written(new RecordSet()), os(o)
{
  os << "digraph OSL_DotWriter {\n";
}

osl::search::analyzer::DotWriter::
~DotWriter()
{
  os << "}\n" << std::flush;
}

void osl::search::analyzer::DotWriter::
showComment(const char *line) const
{
  os << "// " << line << "\n";
}

void osl::search::analyzer::DotWriter::
showNode(Player turn, const SimpleHashRecord *record,
	 int limit, NodeType type) const
{
  const bool black_turn = turn == BLACK;
  if (written->find(record) != written->end())
    return;
  written->insert(record);
  assert(record);
  std::stringstream range;
  int lower_limit = record->lowerLimit();
  int lower_bound = record->lowerBound();
  int upper_limit = record->upperLimit();
  int upper_bound = record->upperBound();
  if (! black_turn)
  {
    std::swap(lower_limit, upper_limit);
    std::swap(lower_bound, upper_bound);
  }
  int bound = 0;
  if (lower_limit >= 0) 
  {
    ++bound;
    range << (boost::format("%d(%d)") % lower_bound % lower_limit);
  }
  range << '<'; 
  if (upper_limit >= 0) 
  {
    ++bound;
    range << (boost::format("%d(%d)") % upper_bound % upper_limit);
  }
  const char *color = 0;
  switch (type)
  {
  case IMPORTANT:
    color = "blue";
    break;
  case ABNORMAL:
    color = "magenta";
    break;
  default:
    color = (bound == 2) ? "red" : "black";
  }
  std::stringstream bestmove;
  bestmove << csa::show(record->bestMove().move());
  os << (boost::format("N%x [label=\"l=%d\\n%s\\n%s\",color=%s,shape=box]\n")
	 % record % limit % range.str() % bestmove.str()
	 % color);
}

// TODO: 選手権後に showNode と共通部分をまとめる
void osl::search::analyzer::DotWriter::
showNodeQuiescence(Player turn, const SimpleHashRecord *record,
		   int limit, NodeType type) const
{
  bool black_turn = (turn == BLACK);
  if (written->find(record) != written->end())
    return;
  written->insert(record);
  assert(record);
  const QuiescenceRecord *qrecord = &record->qrecord;
  std::stringstream range;
  int lower_limit = qrecord->lowerDepth();
  int lower_bound = qrecord->lowerBound();
  int upper_limit = qrecord->upperDepth();
  int upper_bound = qrecord->upperBound();
  if (! black_turn)
  {
    std::swap(lower_limit, upper_limit);
    std::swap(lower_bound, upper_bound);
  }
  int bound = 0;
  if (lower_limit >= 0) 
  {
    ++bound;
    range << (boost::format("%d(%d)") % lower_bound % lower_limit);
  }
  range << '<'; 
  if (upper_limit >= 0) 
  {
    ++bound;
    range << (boost::format("%d(%d)") % upper_bound % upper_limit);
  }
  const char *color = 0;
  switch (type)
  {
  case IMPORTANT:
    color = "blue";
    break;
  case ABNORMAL:
    color = "magenta";
    break;
  default:
    color = (bound == 2) ? "burlywood" : "cyan";
  }
  os << (boost::format("N%x [label=\"l=%d\\n%s\",color=%s,shape=box]\n")
	 % record % limit % range.str() 
	 % color);
}

void osl::search::analyzer::DotWriter::
showArc(const SimpleHashRecord *from, const SimpleHashRecord *to,
	const MoveLogProb& move, bool important) const
{
  if ((written->find(from) != written->end())
      && (written->find(to) != written->end()))
    return;
  assert(from);
  assert(to);
  std::stringstream move_string;
  move_string << csa::show(move.move());
  const char *color = 0;
  if (important)
    color = "blue";
  else
    color = (move.logProb() <= 100) ? "red" : "black";
  os << (boost::format("N%x -> N%x [label=\"%s (%d)\", color=%s, style=bold]\n")
	 % from % to % move_string.str() % move.logProb() % color);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
