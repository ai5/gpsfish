/* rzone.h
 */
#ifndef _NTESUKI_RZONE_H
#define _NTESUKI_RZONE_H
#include "osl/state/numEffectState.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"

#include "osl/ntesuki/ntesukiExceptions.h"
#include "osl/container/moveVector.h"

#include <osl/stl/vector.h>
#include <bitset>
#include <iosfwd>

using namespace osl::state;

namespace osl
{
  namespace ntesuki
  {
    class
    Rzone
    {
      typedef std::bitset<0x100> mask_t;
      mask_t mask;
    public:
      /** state の玉の位置が mask された rzone. */
      Rzone (NumEffectState state,
	     Player p)
      {
	const Square pos = state.kingSquare(p);
	if (!pos.isOnBoard()) return;
	unsigned int index = pos.index();
	ntesuki_assert(index < Square::indexMax());
	mask.set(index);
      }

      Rzone() {}

      Rzone (const Square pos)
      {
	unsigned int index = pos.index();
	ntesuki_assert(index < Square::indexMax());
	mask.set(index);
      }

      /** rzone が一箇所でも set されているか調べる. */
      bool any() const
      {
	return mask.any();
      }

      /** pos に rzone が set されているか調べる. */
      bool test(Square pos) const
      {
	return mask.test(pos.index());
      }

      /** rzone の比較. */
      bool operator==(const Rzone rhs) const
      {
	return mask == rhs.mask;
      }

      /** rzone の和. */
      Rzone operator+(const Rzone rhs) const
      {
	mask_t m = mask | rhs.mask;
	return Rzone(m);
      }

      /** rzone の差. */
      Rzone operator-(const Rzone rhs) const
      {
	ntesuki_assert((rhs.mask & mask) == rhs.mask);
	mask_t m = (rhs.mask ^ mask) & mask;
	return Rzone(m);
      }

      /** rzone の更新, 差を返す. */
      Rzone update(const Rzone rhs)
      {
	mask_t mask_orig = mask;
	mask |= rhs.mask;

	mask_t mask_diff = (mask_orig ^ mask) & mask;

	return Rzone(mask_diff);
      }
      /** rzone の出力. */
      friend std::ostream& operator<<(std::ostream& os,
				      const Rzone& rzone)
      {
	return os << rzone.mask;
      }
    private:
      Rzone(mask_t _mask) : mask(_mask) {}
      
    };
  } //ntesuki
} //osl

#endif /* _NTESUKI_SEACHER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
