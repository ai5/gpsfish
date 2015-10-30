/* hashRejections.h
 */
#ifndef _HASHREJECTIONS_H
#define _HASHREJECTIONS_H

#include "osl/numEffectState.h"
#include "osl/hashKey.h"

namespace osl
{
  namespace search
  {
    class HashRejections
    {
      struct RootTable;
      struct Table;
      std::shared_ptr<RootTable> root_table;
      std::unique_ptr<Table> table;
    public:
      HashRejections();
      HashRejections(const HashRejections&);
      ~HashRejections();
      HashRejections& operator=(const HashRejections&);
      
      void addRejectionRoot(const NumEffectState& parent, const HashKey& key, Move move);
      void clearRejectionRoot(const NumEffectState& parent, const HashKey& key, Move move);
      void addRejection(const NumEffectState& parent, const HashKey& key, Move move);
      void clearRejection(const NumEffectState& parent, const HashKey& key, Move move);

      bool rejectionProbe(const HashKey& cur, const HashKey& parent) const;
    };
  }
}

#endif /* _HASHREJECTIONS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
