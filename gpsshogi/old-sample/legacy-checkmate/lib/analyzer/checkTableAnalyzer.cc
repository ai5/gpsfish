/* checkTableAnalyzer.cc
 */
#include "analyzer/checkTableAnalyzer.h"
#include "analyzer/treeWriter.h"
#include "analyzer/proofTreeTraverser.h"
#include "analyzer/disproofTreeTraverser.h"
#include "analyzer/recordSet.h"
#include "analyzer/showAllTree.h"
#include "checkHashRecord.h"
#include "checkMoveList.h"
#include "osl/checkmate/proofDisproof.h"
#include "corruptCheckTable.h"
#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <fstream>

namespace osl
{
  namespace checkmate
  {
    using namespace analyzer;
    static void traverse(const CheckHashRecord *record, RecordSet& out)
    {
      if (! record)
	return;
      if (! out.insert(record).second)
	return;
    
      for (CheckMoveList::const_iterator p=record->moves.begin();
	   p!=record->moves.end(); ++p)
      {
	traverse(p->record, out);
      }
    }


    static void examineProofTree(const CheckHashRecord *record,
				 const TwinTable& table,
				 const HashKey& key,
				 const PathEncoding& path,
				 bool orNode, TreeWriter& writer,
				 size_t& tree_size, size_t& leaf_size)
    {
      tree_size = 1;
      leaf_size = 1;
      assert(record);
      assert(record->proofDisproof().isCheckmateSuccess());
      ProofTreeTraverser traverser(writer, table);
      if (orNode)			// 手番側が詰めた
	traverser.traverseOrNode(Move::INVALID(), record, key, path);
      else			// 手番側が詰まされた
	traverser.traverseAndNode(Move::INVALID(), record, key, path);
      tree_size = traverser.getVisited().size();
      leaf_size = traverser.getLeaves();
    }

    static void examineDisproofTree(const CheckHashRecord *record,
				    const TwinTable& table,
				    const HashKey& key,
				    const PathEncoding& path,
				    bool orNode, bool isPartialStack,
				    TreeWriter& writer,
				    size_t& tree_size, size_t& leaf_size)
    {
      tree_size = 1;
      leaf_size = 1;
      assert(record);
      if (! record->proofDisproof().isCheckmateFail())
      {
	if (! record->findLoop(path, table))
	{
	  assert(isPartialStack);
	  assert(! record->twins.empty());
	  return;
	}
      }
      DisproofTreeTraverser traverser(writer, table, isPartialStack);
      if (orNode)			// 手番側が詰を逃れた
	traverser.traverseOrNode(Move::INVALID(), record, key, path);
      else			// 手番側が逃げられた
	traverser.traverseAndNode(Move::INVALID(), record, key, path);
      tree_size = traverser.getVisited().size();
      leaf_size = traverser.getLeaves();
    }
    
    static void examineTreeGuess(const CheckHashRecord *record,
				 const TwinTable& table,
				 const HashKey& key,
				 const PathEncoding& path,
				 bool orNode, bool isPartialStack,
				 TreeWriter& writer,
				 size_t& tree_size, size_t& leaf_size)
    {
      if (! record)
      {
	tree_size = 0;
	leaf_size = 0;
	return;
      }
      if ((! record->proofDisproof().isFinal())
	  &&  (! record->findLoop(path, table)))
      {
	tree_size = 1;
	leaf_size = 1;
	return;
      }
      if (record->proofDisproof().isCheckmateFail()
	  || record->findLoop(path, table))
      {
	examineDisproofTree(record, table, key, path, orNode, isPartialStack, 
			    writer, tree_size, leaf_size);
      }
      else
      {
	examineProofTree(record, table, key, path, orNode, 
			 writer, tree_size, leaf_size);
      }
    }
  } // namespace checkmate
} // namespace osl

osl::checkmate::analyzer::
CheckTableAnalyzer::CheckTableAnalyzer(const TwinTable& t, bool outline)
  : table(t), useOutlineFormat(outline)
{
}
  
osl::checkmate::analyzer::
CheckTableAnalyzer::~CheckTableAnalyzer()
{
}

void osl::checkmate::analyzer::CheckTableAnalyzer::
showTree(const CheckHashRecord *record, std::ostream& os,
	 int maxDepth, bool expandFinalState, bool showTerminalMoves,
	 size_t threshold) const
{
  if (! record)
    return;
  ShowAllTree traverser(os, maxDepth, expandFinalState, showTerminalMoves);
  if (useOutlineFormat)
    traverser.showOutline(record);
  else
    traverser.showDot(record, threshold);
}

size_t osl::checkmate::analyzer::
CheckTableAnalyzer::treeSize(const CheckHashRecord *record) const
{
  RecordSet nodes;
  traverse(record, nodes);
  return nodes.size();
}

size_t osl::checkmate::analyzer::CheckTableAnalyzer::
proofTreeSize(const CheckHashRecord *record, const HashKey& key, 
	      const PathEncoding& path, bool orNode) const
{
  size_t leaf_size;
  return proofTreeSize(record, key, path, orNode, leaf_size);
}
size_t osl::checkmate::analyzer::CheckTableAnalyzer::
proofTreeSize(const CheckHashRecord *record, 
	      const HashKey& key, const PathEncoding& path, 
	      bool orNode, size_t& leaf_size) const
{
  try
  {
    TreeWriter writer;
    size_t tree_size;
    examineProofTree(record, table, key, path, orNode, 
		     writer, tree_size, leaf_size);
    return tree_size;
  }
  catch (CorruptCheckTable& t)
  {
    std::cerr << t.what() << ", corrupt tree was " << t.record << "\n";
    t.record->dump();
    throw;
  }
}

size_t osl::checkmate::analyzer::CheckTableAnalyzer::
disproofTreeSize(const CheckHashRecord *record, 
		 const HashKey& key, const PathEncoding& path, 
		 bool orNode, bool isPartialStack) const
{
  size_t leaf_size;
  return disproofTreeSize(record, key, path, orNode, leaf_size, 
			  isPartialStack);
}

size_t osl::checkmate::analyzer::CheckTableAnalyzer::
disproofTreeSize(const CheckHashRecord *record, 
		 const HashKey& key, const PathEncoding& path, 
		 bool orNode, size_t& leaf_size, bool isPartialStack) const
{
  try
  {
    TreeWriter writer;
    size_t tree_size;
    examineDisproofTree(record, table, key, path, orNode, isPartialStack, 
			writer, tree_size, leaf_size);
    return tree_size;
  }
  catch (CorruptCheckTable& t)
  {
    std::cerr << t.what() << ", corrupt tree was " << t.record << "\n";
    t.record->dump();
    throw;
  }
}

size_t osl::checkmate::analyzer::CheckTableAnalyzer::
proofOrDisproofTreeSize(const CheckHashRecord *record, 
			const HashKey& key, 
			const PathEncoding& path, 
			bool orNode, bool isPartialStack) const
{
  size_t leaf_size;
  return proofOrDisproofTreeSize(record, key, path, orNode, 
				 leaf_size, isPartialStack);
  
}

size_t osl::checkmate::analyzer::CheckTableAnalyzer::
proofOrDisproofTreeSize(const CheckHashRecord *record, 
			const HashKey& key, const PathEncoding& path, 
			bool orNode, size_t& leaf_size, 
			bool isPartialStack) const
{
  try
  {
    TreeWriter writer;
    size_t tree_size;
    examineTreeGuess(record, table, key, path, orNode, isPartialStack, writer, 
		     tree_size, leaf_size);
    return tree_size;
  }
  catch (CorruptCheckTable& t)
  {
    std::cerr << t.what() << ", corrupt tree was " << t.record << "\n";
    t.record->dump();
    const char *filename = "checkmate-oops.log";
    std::ofstream os(filename);
    std::cerr << "entire tree is being written in " << filename << "\n";
    CheckTableAnalyzer a(table);
    a.showTree(record, os, 100, true, true);
    throw;
  }
}

size_t osl::checkmate::analyzer::CheckTableAnalyzer::
showProofTree(const CheckHashRecord *record, 
	      const HashKey& key, const PathEncoding& path,
	      bool orNode, std::ostream& os, bool isPartialStack) const
{
  std::unique_ptr<TreeWriter> writer;
  if (useOutlineFormat)
    writer.reset(new TreeStreamWriter(&os, true));
  else 
    writer.reset(new DotWriter(os));
  size_t tree_size, leaf_size;
  examineTreeGuess(record, table, key, path, orNode, isPartialStack, *writer,
		   tree_size, leaf_size);
  return tree_size;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
