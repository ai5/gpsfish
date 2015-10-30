#ifndef _PROOFTREEDEPTH_H
#define _PROOFTREEDEPTH_H
#include "osl/hash/hashKey.h"
#include <boost/scoped_ptr.hpp>
namespace osl
{
  namespace state
  {
    class NumEffectState;
  }
  namespace checkmate
  {
    class CheckHashRecord;
    class DfpnTable;
    namespace analyzer
    {
      /**
       * 詰までの手数を数える.
       * 詰将棋ルーチン次第で，無駄合なども含まれるため
       * 人間の感覚と一致するとは限らない．
       */
      class ProofTreeDepth
      {
	class Table;
	std::unique_ptr<Table> table;
      public:
	ProofTreeDepth();
	~ProofTreeDepth();
	int depth(const CheckHashRecord *record, bool is_or_node) const;
      private:
	int orNode(const CheckHashRecord *record) const;
	int andNode(const CheckHashRecord *record) const;
      };

    } // namespace analyzer
  } // namespace checkmate
}

#endif /* _PROOFTREEDEPTH_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
