/* treeTraverser.cc
 */
#include "analyzer/treeTraverser.h"
#include "checkHashRecord.h"
#include <iostream>

osl::checkmate::analyzer::
TreeTraverser::TreeTraverser(TreeWriter& w, const TwinTable& t) 
  : writer(w), table(t)
{
}

osl::checkmate::analyzer::
TreeTraverser::~TreeTraverser()
{
  if (analyzerStack.size())
  {
    std::cerr << "unexpected abort in TreeTraverser\n";
    std::cerr << "current stack was \n" << analyzerStack;
  }
}

bool osl::checkmate::analyzer::
TreeTraverser::findLoopToStackByTwins(const TwinList& l) const
{
  for (TwinList::const_iterator p=l.begin(); p!=l.end(); ++p)
  {
    if (p->loopTo 
	&& (p->loopTo->isVisited 
	    || (analyzerStack.findLoop(p->loopTo) != analyzerStack.end())))
      return true;
  }
  return false;
}

bool osl::checkmate::analyzer::
TreeTraverser::findLoopToStackByStack(const TwinList& l) const
{
  for (CheckStack::const_iterator p=analyzerStack.begin(); 
       p != analyzerStack.end(); ++p)
  {
    if (l.findLoopTo(p->record))
    {
      return true;
    }
  }
  return false;
}

bool osl::checkmate::analyzer::
TreeTraverser::findLoopToStack(const TwinList& l) const
{
  const bool byTwins = findLoopToStackByTwins(l);
  //const bool byStack = findLoopToStackByStack(l);
  return byTwins;
}

void osl::checkmate::analyzer::
TreeTraverser::traverseOrNode(Move lastMove, const CheckHashRecord *record,
			      const HashKey& key, const PathEncoding& path)
{
  analyzerStack.push_back(CheckStackEntry(0, "T-OR ", record, 
					      key, path));
  orNode(lastMove, record, key, path);
  analyzerStack.pop_back();
}

void osl::checkmate::analyzer::
TreeTraverser::traverseAndNode(Move lastMove, const CheckHashRecord *record,
			       const HashKey& key, const PathEncoding& path)
{
  analyzerStack.push_back(CheckStackEntry(0, "T-AND", record, 
					      key, path));
  andNode(lastMove, record, key, path);
  analyzerStack.pop_back();
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
