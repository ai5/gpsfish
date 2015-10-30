/* treeTraverser.h
 */
#ifndef _CHECK_TREETRAVERSER_H
#define _CHECK_TREETRAVERSER_H

#include "analyzer/treeWriter.h"
#include "analyzer/recordSet.h"
#include "checkStack.h"
namespace osl
{
  class PathEncoding;
  namespace hash
  {
    class HashKey;
  }
  namespace checkmate
  {
    class CheckHashRecord;
    class CheckMoveList;
    class TwinList;
    struct CorruptCheckTable;
    namespace analyzer
    {
      class TreeWriter;
      class TreeTraverser
      {
      protected:
	TreeWriter& writer;
	RecordSet visited;
	CheckStack analyzerStack;
	RecordSet leaves;
	const TwinTable& table;
	/**
	 * @see CheckTableAnalyzer::proofTreeSize
	 */
      public:
	TreeTraverser(TreeWriter&, const TwinTable&);

	const RecordSet& getVisited() const { return visited; }
	const RecordSet& getLeaveSet() const { return leaves; }
	unsigned int getLeaves() const { return leaves.size(); }

	void traverseOrNode(Move last_move, const CheckHashRecord *record,
			    const HashKey& key, const PathEncoding& path);
	void traverseAndNode(Move, const CheckHashRecord *record,
			     const HashKey& key, const PathEncoding& path);

	bool findLoopToStackByTwins(const TwinList& l) const;
	bool findLoopToStackByStack(const TwinList& l) const;
	bool findLoopToStack(const TwinList& l) const;
      protected:
	virtual ~TreeTraverser();
	virtual void orNode(Move m, const CheckHashRecord *record, const HashKey& key, 
			    const PathEncoding& path)=0;
	virtual void andNode(Move, const CheckHashRecord *record, const HashKey& key, 
			     const PathEncoding& path)=0;
      };
    } // namespace analyzer
  } // namespace checkmate
} // namespace osl


#endif /* _CHECK_TREETRAVERSER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
