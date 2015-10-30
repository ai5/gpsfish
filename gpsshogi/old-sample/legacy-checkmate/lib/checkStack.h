/* checkStack.h
 */
#ifndef _CHECKSTACK_H
#define _CHECKSTACK_H

#include "checkMove.h"
#include "osl/hash/hashKey.h"
#include "osl/misc/fixedCapacityVector.h"
#include "osl/pathEncoding.h"
#include <iosfwd>
namespace osl
{
  namespace checkmate
  {
    class CheckHashRecord;
    class CheckMove;
    struct CheckStackEntry
    {
      CheckMove move;
      const char *name;
      const CheckHashRecord *record;
      HashKey hash;
      PathEncoding path;
      unsigned int proofLimit, disproofLimit;
      CheckStackEntry(const CheckMove *nextMove,
		      const char *name, 
		      const CheckHashRecord *record, 
		      const HashKey&, const PathEncoding&,
		      unsigned int proofLimit=0, 
		      unsigned int disproofLimit=0);
      CheckStackEntry() : move(Move::INVALID()), name(""), record(0),
			  proofLimit(0), disproofLimit(0)
      {
      }
    };
    class CheckStack : public FixedCapacityVector<CheckStackEntry,512> 
    {
    public:
      ~CheckStack();
      const_iterator findLoop(const CheckHashRecord *record) const;
      /** 同一盤面で attacker の持駒が多いものが存在するか */
      const_iterator findCover(Player attacker, 
			       const HashKey& hash,
			       const CheckHashRecord *record) const;
      /**
       * 最後のエントリ以外に存在するか
       */
      bool findNotLast(const CheckHashRecord *record) const
      {
	const_iterator pos = findLoop(record);
	return (pos - begin()) < (int)size()-1;
      }
    };

    std::ostream& operator<<(std::ostream& os, const CheckStack& s);
    std::ostream& operator<<(std::ostream& os, const CheckStackEntry& e);
  }
}

#endif /* _CHECKSTACK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
