/* ntesukiMoveList.h
 */
#ifndef _NTESUKI_MOVELIST_H
#define _NTESUKI_MOVELIST_H

#include "osl/ntesuki/ntesukiMove.h"
#include "osl/ntesuki/ntesukiExceptions.h"
#include "osl/state/numEffectState.h"
#include "osl/container/moveVector.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/check_.h"
#include "osl/stl/slist.h"
#include <cassert>
#include <algorithm>
#include <iterator>
#include <iosfwd>

namespace osl
{
  namespace ntesuki
  {
    // TODO: 自作すべき
    typedef slist<NtesukiMove> NtesukiMoveListBase;
    /** 
     * ntesuki 探索で使う指手のリスト
     */
    class NtesukiMoveList : public NtesukiMoveListBase
    {
    public:
      NtesukiMoveList();
      NtesukiMoveList(const NumEffectState& state,
		      const osl::MoveVector& mv);

      /**
       * 重複をしないように手を追加するメソッド.
       * @c move があるかどうか捜し，あった場合には
       * 既にある move への参照を返す.
       * なかった場合には @c move と同じ osl::Move を持つような
       * NtesukiMove を追加する.
       * 勝敗に関係する flags や record 等の情報は保持しないので注意.
       * 通常の追加の場合には push_front を用いること.
       */
      NtesukiMove* add(const NtesukiMove& move);
      const NtesukiMove& find(const NtesukiMove& move) const;
    };
    std::ostream& operator<<(std::ostream&, const NtesukiMoveList&);
  } // namespace ntesukimate
} // namespace osl

#endif /* _NTESUKI_MOVELIST_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
