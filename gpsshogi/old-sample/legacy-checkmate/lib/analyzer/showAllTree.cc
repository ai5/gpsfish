/* showAllTree.cc
 */
#include "analyzer/showAllTree.h"
#include "analyzer/treeWriter.h"
#include "analyzer/recordSet.h"
#include "checkHashRecord.h"
#include "sameBoardList.h"
#include "osl/container/moveVector.h"
#include <map>

namespace osl
{
  namespace checkmate
  {
    typedef std::multimap<int, const CheckMove*> CheckMoveMap;
  }
}

osl::checkmate::analyzer::
ShowAllTree::ShowAllTree(std::ostream& o, int m, bool e, bool s)
  : os(o), maxDepth(m), expandFinalState(e), showTerminalMoves(s)
{
  os << "// ShowAllTree::config " << m << " " << e << " " << s << "\n";
}

bool osl::checkmate::analyzer::
ShowAllTree::isTerminal(const CheckHashRecord *record)
{
  return (! record) 
    || (record->needMoveGeneration()
#if 0
	&& record->twins.empty()
#endif
	&& ((!record->sameBoards) || (record->sameBoards->size() == 1)))
    ;
}

void osl::checkmate::analyzer::
ShowAllTree::showOutline(const CheckHashRecord *record) const
{
  TreeStreamWriter writer(&os, false);
  show(writer, record);
}

void osl::checkmate::analyzer::
ShowAllTree::showDot(const CheckHashRecord *record, size_t threshold) const
{
  DotWriter writer(os, threshold);
  show(writer, record);
}

void osl::checkmate::analyzer::
ShowAllTree::show(TreeWriter& writer, const CheckHashRecord *record) const
{
  RecordSet visited;
  writer.incDepth();
  if (! record)
  {
    writer.writeln("null tree");
  }
  else
  {
    writer.showRecord(record);
    show(record, writer, visited);
  }
  writer.decDepth();
}

void osl::checkmate::analyzer::
ShowAllTree::show(const CheckHashRecord *record, 
		  TreeWriter& writer, RecordSet& visited) const
{
  if (! visited.insert(record).second)
  {
    writer.writeln("(confluence)");
    if (! record->isConfluence)
      writer.writeln(" not recognized !!");
    return;
  }
  if ((! expandFinalState) 
      && (record->proofDisproof().isFinal()
	  || (! record->twins.empty())))
  {
    // writer.writeln("final");
    return;
  }
  writer.incDepth();
  if (/*writer.getDepth()*/
    record->distance < maxDepth)
  {
    // 末端ノードをまとめて表示
    if (showTerminalMoves)
    {
      MoveVector moves;
      for (CheckMoveList::const_iterator p=record->moves.begin(); 
	   p!=record->moves.end(); ++p)
      {
	if (isTerminal(p->record))
	  moves.push_back(p->move);
      }
      writer.showMoves(moves);
    }
    // 展開したノードを proof number 順に表示 (本当は攻方受方を考慮)
    CheckMoveMap m;
    for (CheckMoveList::const_iterator p=record->moves.begin(); 
	 p!=record->moves.end(); ++p)
    {
      if (! isTerminal(p->record))
	m.insert(std::make_pair(p->record->proof(), &*p));
    }
    for (CheckMoveMap::const_iterator p=m.begin(); p!=m.end(); ++p)
    {
      writer.showMove(record, *p->second);
      show(p->second->record, writer, visited);
    }
  }
  writer.decDepth();
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
