/* treeWriter.cc
 */
#include "analyzer/treeWriter.h"
#include "analyzer/recordSet.h"
#include "checkHashRecord.h"
#include "checkMove.h"
#include "sameBoardList.h"
#include "osl/container/moveVector.h"
#include "osl/record/csa.h"

// TreeWriter

osl::checkmate::analyzer::
TreeWriter::TreeWriter()
  : depth(1)
{
}
osl::checkmate::analyzer::
TreeWriter::~TreeWriter()
{
}

// DotWriter
osl::checkmate::analyzer::
DotWriter::DotWriter(std::ostream& o, size_t m, const char *name)
  : os(o), visited(new RecordSet()), minimumPdp(m)
{
  os << "// minimumPdp " << minimumPdp << "\n";
  os << "digraph " << (name ? name : "OSL_DotWriter") << " {\n";
}

osl::checkmate::analyzer::
DotWriter::~DotWriter()
{
  os << "}\n";
}

const std::string osl::checkmate::analyzer::
DotWriter::header() const
{
  return "  "; 
}

namespace osl
{
  namespace 
  {
    bool focus(const CheckHashRecord& record, size_t threshold)
    {
      const ProofDisproof& pdp = record.proofDisproof();
      return (! record.twins.empty())
	|| pdp.isFinal()
	|| ((pdp.proof() > threshold) || (pdp.disproof() > threshold));
    }
    bool primaryFocus(const CheckHashRecord& record)
    {
      const size_t threshold = 300;
      return focus(record, threshold);
    }

    void writeEdge(std::ostream& os, const std::string& header,
		   const CheckHashRecord *from,
		   const CheckHashRecord *to,
		   const char *color, const char *style)
    {
      os << header << 'N' << from << " -> " << 'N' << to
	 << " [weight=0, color=" << color << ", style=" << style << "]\n";
    }
  }
}

void osl::checkmate::analyzer::
DotWriter::showRecord(const CheckHashRecord *record)
{
  assert(record);
  if (! visited->insert(record).second)
    return;
  
  os << header() << 'N' << record << ' ';
  os << "[label=\"" << record->proofDisproof() 
     << "\\nd=" << record->distance
     << (record->useMaxInsteadOfSum ? "(max)" : "")
     << "\"";
  if (record->proofDisproof().isFinal())
    os << ",fontcolor=blue";
  else if (primaryFocus(*record))
    os << ",fontcolor=red";
  if (getDepth() % 2)
    os << ",shape=box";
  os << "]\n";
  // TODO: bestMove, twins

  // dominance
  if (record->finalByDominance())
  {
    writeEdge(os, header(), record->finalByDominance(), record, 
	      "green", "dashed");
  }
  if (record->sameBoards)
  {
    for (SameBoardList::const_iterator p=record->sameBoards->begin();
	 p!=record->sameBoards->end(); ++p)
    {
      if (p->stand(BLACK) == record->stand(BLACK))
	continue;
      if (p->stand(BLACK).hasMoreThan<BLACK>(record->stand(BLACK)))
      {
	writeEdge(os, header(), record, &*p, "blue", "dotted");
	showRecord(&*p);
      }
#if 0
      // 念の為 ShowAllTree::isTerminal との連携がないと，ちゅうぶらりんの
      // ノードができる．
      if (record->blackStand.hasMoreThan<BLACK>(p->blackStand))
      {
	writeEdge(os, header(), &*p, record, "blue", "dotted");
	showRecord(&*p);
      }
#endif
    }
  }
}

void osl::checkmate::analyzer::
DotWriter::showMove(const CheckHashRecord *from, const CheckMove& move)
{
  if (! focus(*from, minimumPdp))
  {
    // os << "skip " << from << "\n";
    return;
  }
  const CheckHashRecord *record = move.record;
  if (! record)
  {
    assert(move.flags.isSet(MoveFlags::ImmediateCheckmate));
    return;
  }
  showRecord(record);
  int weight = 1;
  if (move.flags.isSet(MoveFlags::Upward))
    weight = 0;
  if (from->bestMove && (from->bestMove->move == move.move))
    weight = 2;
  if (primaryFocus(*record))
    weight += 1;
  const Move m = move.move;
  os << header() << 'N' << from << " -> " << 'N' << record
     << " [label=\"";
  csaShow(os, m);
  os << "\", weight=" << weight;
  if (move.flags.isSet(MoveFlags::Upward))
    os << ", color=magenta";
  else if (primaryFocus(*record))
    os << ", color=red";
  if (from->bestMove 
      && (from->bestMove->move == move.move))
    os << ", style=bold";
  os << "]\n";
}

void osl::checkmate::analyzer::
DotWriter::showMoves(const MoveVector& moves)
{
  if (moves.size() == 0)
    return;
  os << "// T";	// terminal
  for (MoveVector::const_iterator p=moves.begin(); p!=moves.end(); ++p)
  {
    os << ' ';
    csaShow(os, *p);
  }
  os << "\n";
}

void osl::checkmate::analyzer::
DotWriter::writeln(const char *msg)
{
  os << "// " << msg << "\n";
}

// OutlineWriter

osl::checkmate::analyzer::
OutlineWriter::OutlineWriter()
{
}

osl::checkmate::analyzer::
OutlineWriter::~OutlineWriter()
{
}

const std::string osl::checkmate::analyzer::
OutlineWriter::header() const
{
  return std::string(getDepth(), '*'); 
}

// TreeStreamWriter

osl::checkmate::analyzer::
TreeStreamWriter::TreeStreamWriter(std::ostream *o, bool s)
  : os(o), simpleMove(s)
{
}
osl::checkmate::analyzer::
TreeStreamWriter::~TreeStreamWriter()
{
}

void osl::checkmate::analyzer::
TreeStreamWriter::showRecord(const CheckHashRecord *record)
{
  assert(record);
  (*os) << header() << ' ' << record->proofDisproof() << ' ' << record->filter;
  if (record->hasBestMove())
  {
    (*os) << " best move ";
    csaShow(*os, record->bestMove->move);
    (*os) << "(" << record->bestMove->record << ")";
  }
  (*os) << "\n";
#if 0
  (*os) << "bestResultInSolved " << record->bestResultInSolved << "\n";
#endif
  if (! record->twins.empty())
  {
    for (TwinList::const_iterator p=record->twins.begin();
	 p!=record->twins.end(); ++p)
    {
      (*os) << " [" << p->path << " ";
      csaShow(*os, p->move.move);
      (*os) << " " << p->move.record << "]";
    }
    (*os) << "\n";
  }
}

void osl::checkmate::analyzer::
TreeStreamWriter::showMove(const CheckMove& m)
{
  (*os) << header() << ' ';
  csaShow(*os, m.move);
  (*os) << " " << m.flags;
  if (m.record)
  {
    (*os) << m.record->proofDisproof();
    if (! m.record->twins.empty())
    {
      (*os) << " twins " << m.record->twins.size();
      for (TwinList::const_iterator p=m.record->twins.begin(); 
	   p!=m.record->twins.end(); ++p)
      {
	(*os) << " " << p->path << " ";
	csaShow(*os, p->move.move);
      }
    }
  }
  (*os) << "\n";
}

void osl::checkmate::analyzer::
TreeStreamWriter::showMove(const CheckHashRecord */*from*/,
			   const CheckMove& move)
{
  if (simpleMove)
  {
    showMove(move);
    return;
  }
  const CheckHashRecord *record = move.record;

  const Move m = move.move;
  (*os) << header() << ' ';
  csaShow(*os, m);
  (*os) << ' ' << record->distance << ' ' << move.flags << ' '
	<< record->proofDisproof() << ' ' << record->filter;
  if (record->isVisited)
    (*os) << " (v)";
  if (record->isConfluence)
    (*os) << " (c)";
  if (record->useMaxInsteadOfSum)
    (*os) << " (sum/max)";
  (*os) << ' ' << record << " up " << record->parent << " ";
  if (! record->twins.empty())
  {
    (*os) << " twins " << record->twins.size();
    for (TwinList::const_iterator p=record->twins.begin();
	 p!=record->twins.end(); ++p)
    {
      (*os) << " " << p->path << " ";
      csaShow(*os, p->move.move);
    }
  }
  if (record->hasBestMove() && (! record->bestMove->move.isPass()))
  {
    (*os) << " best move ";
    csaShow(*os, record->bestMove->move);
    (*os) << "(" << record->bestMove->record << ")";
  }
  (*os) << "\n";
#if 0
  (*os) << "bestResultInSolved " << record->bestResultInSolved << "\n";
#endif
  if (! record->twins.empty())
  {
    for (TwinList::const_iterator p=record->twins.begin();
	 p!=record->twins.end(); ++p)
    {
      (*os) << " [" << p->path << " ";
      csaShow(*os, p->move.move);
      (*os) << " " << p->move.record << "]";
    }
    (*os) << "\n";
  }
}

void osl::checkmate::analyzer::
TreeStreamWriter::showMoves(const MoveVector& moves)
{
  if (moves.size() == 0)
    return;
  (*os) << 'T';	// terminal
  for (MoveVector::const_iterator p=moves.begin(); p!=moves.end(); ++p)
  {
    (*os) << ' ';
    csaShow(*os, *p);
  }
  (*os) << "\n";
}

void osl::checkmate::analyzer::
TreeStreamWriter::writeln(const char *msg)
{
  (*os) << msg << "\n";
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
