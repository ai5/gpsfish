/* disproofTreeTraverser.h
 */
#ifndef _CHECK_DISPROOFTREETRAVERSER_H
#define _CHECK_DISPROOFTREETRAVERSER_H

#include "analyzer/treeTraverser.h"
namespace osl
{
  namespace checkmate
  {
    namespace analyzer
    {
      /**
       * disproof tree を検証
       */
      class DisproofTreeTraverser : public TreeTraverser
      {
	/** root から呼んでいない場合，isVisitedの判定ができないことがある*/
	bool isPartialStack;
      public:
	DisproofTreeTraverser(TreeWriter&, const TwinTable&,
			      bool isPartialStack=false);
	~DisproofTreeTraverser();
      protected:      
	void orNode(Move m, const CheckHashRecord *record, const HashKey& key, 
		    const PathEncoding& path);
	void andNode(Move, const CheckHashRecord *record, const HashKey& key, 
		     const PathEncoding& path);
      };
    } // namespace analyzer
  } // namespace checkmate
} // namespace osl


#endif /* _CHECK_TREETRAVERSER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
