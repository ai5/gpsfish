/* ntesukiList.cc
 */
#include "osl/ntesuki/ntesukiMoveList.h"
#include <iostream>

osl::ntesuki::NtesukiMoveList::
NtesukiMoveList()
{
}

osl::ntesuki::NtesukiMoveList::
NtesukiMoveList(const NumEffectState& state,
		const osl::MoveVector& mv)
{
  ntesuki_assert(empty());
  if (!state.kingSquare(alt(state.turn())).isOnBoard())
  {
    for (size_t i = 0; i < mv.size(); ++i)
    {
      NtesukiMove move(mv[i]);
      push_front(move);
    }
  }
  else
  {
    for (size_t i = 0; i < mv.size(); ++i)
    {
      NtesukiMove move(mv[i]);
      if (move_classifier::PlayerMoveAdaptor<move_classifier::Check>::isMember(state, mv[i]))
      {
	move.setCheck();
      }
      push_front(move);
    }
  }
}

/*
 * 重複をしないように手を追加するメソッド.
 * @c move があるかどうか捜し，あった場合には
 * 既にある move への参照を返す.
 * なかった場合には @c move と同じ osl::Move を持つような
 * NtesukiMove を追加する.
 * 勝敗に関係する flags や record 等の情報は保持しないので注意.
 * 通常の追加の場合には push_front を用いること.
 */
const osl::ntesuki::NtesukiMove& osl::ntesuki::NtesukiMoveList::
find(const NtesukiMove& move) const
{
  const_iterator it;
  for (it = begin(); it != end(); it++)
  {
    if(it->getMove() == move.getMove())
    {
      return *it;
    }
  }

  return *it;
}

osl::ntesuki::NtesukiMove* osl::ntesuki::NtesukiMoveList::
add(const NtesukiMove& move)
{
  for (iterator it = begin(); it != end(); it++)
  {
    if(it->getMove() == move.getMove())
    {
      return &(*it);
    }
  }

  push_front(NtesukiMove(move.getMove()));
  if (move.isCheck())
  {
    front().setCheck();
  }
  return &(front());
}

std::ostream& osl::ntesuki::
operator<<(std::ostream& os, const NtesukiMoveList& l)
{
  for (NtesukiMoveList::const_iterator p=l.begin(); p!=l.end(); ++p)
  {
    os << (*p) << " ";
  }
  return os << "\n";
}

