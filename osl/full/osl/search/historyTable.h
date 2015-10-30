/* historyTable.h
 */
#ifndef OSL_HISTORYTABLE_H
#define OSL_HISTORYTABLE_H

#include "osl/basic_type.h"
#include "osl/container.h"
#ifdef OSL_SMP
#  include "osl/misc/lightMutex.h"
#endif
#include <vector>
#include <iosfwd>
namespace osl
{
  namespace search
  {
    class HistoryTable
    {
    public:
      struct Entry
      {
	uint64_t value;
#ifdef OSL_SMP
	mutable LightMutex mutex;
#endif
	Entry() : value(0)
	{
	}
      };
    private:
      CArray<CArray2d<Entry,Square::SIZE, Square::SIZE>,2> table;
    public:
      uint64_t value(Move move) const 
      {
	if (! move.isNormal())
	  return 0;
	const int from_index = move.isDrop() ? (int)move.ptype() : (int)move.from().uintValue();
	const Entry& e = table[move.player()][from_index][move.to().uintValue()];
	return e.value;
      }
      void add(Move move, int inc)
      {
	if (! move.isNormal())
	  return;
	const int from_index = move.isDrop() ? (int)move.ptype() : (int)move.from().uintValue();
	Entry& e = table[move.player()][from_index][move.to().uintValue()];
#ifdef OSL_SMP
	SCOPED_LOCK(lk, e.mutex);
#endif
	e.value += inc;
      }
      void clear(Move move)
      {
	if (! move.isNormal())
	  return;
	const int from_index = move.isDrop() ? (int)move.ptype() : (int)move.from().uintValue();
	Entry& e = table[move.player()][from_index][move.to().uintValue()];
	e.value = 0;
      }
      struct OutputEntry
      {
	int from_or_ptype;
	Square to;
	uint64_t value;
	explicit OutputEntry(int i=0, int j=0, uint64_t v=0) 
	  : from_or_ptype(i), to(Square::makeDirect(j)), value(v)
	{
	}
	bool operator>(const OutputEntry& r) const
	{
	  if (value != r.value)
	    return value > r.value;
	  if (from_or_ptype != r.from_or_ptype)
	    return from_or_ptype > r.from_or_ptype;
	  return to > r.to;
	}
      };
      void extractTopN(Player p, std::vector<OutputEntry>& out, size_t limit) const;
    };    
    std::ostream& operator<<(std::ostream&, const HistoryTable::OutputEntry&);
  }
};

#endif /* OSL_HISTORYTABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
