/* proofTreeTraverser.h
 */
#ifndef _CHECK_PROOFTREETRAVERSER_H
#define _CHECK_PROOFTREETRAVERSER_H

#include "analyzer/treeTraverser.h"
namespace osl
{
  namespace checkmate
  {
    namespace analyzer
    {
      /**
       * proof tree を検証
       */
      class ProofTreeTraverser : public TreeTraverser
      {
      public:
	ProofTreeTraverser(TreeWriter&, const TwinTable& table);
	~ProofTreeTraverser();
      protected:      
	void orNode(Move m, const CheckHashRecord *record, const HashKey& key, 
		    const PathEncoding& path);
	void andNode(Move, const CheckHashRecord *record, const HashKey& key, 
		     const PathEncoding& path);
      };
    } // namespace analyzer
  } // namespace checkmate
} // namespace osl


#endif /* _CHECK_PROOFTREETRAVERSER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
