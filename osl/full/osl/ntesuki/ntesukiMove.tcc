/* ntesukiMove.tcc
 */
#include "osl/ntesuki/ntesukiMove.h"
#include "osl/ntesuki/ntesukiRecord.h"

template<osl::Player P>
void osl::ntesuki::NtesukiMove::
setImmediateCheckmate()
{
  if (P == osl::BLACK)
    flags |= (IS_SUCCESS_BLACK_MASK | IMMEDIATE_CHECKMATE);
  else
    flags |= (IS_SUCCESS_WHITE_MASK | IMMEDIATE_CHECKMATE);
};

/* private functions to check FLAGS for success/fail
 */
template <osl::Player P>
int osl::ntesuki::NtesukiMove::
is_success_flag(int pass_left) const
{
  ntesuki_assert(pass_left >= 0 && 
		 pass_left <= 3);
  if (P == osl::BLACK)
    return 1 << (IS_SUCCESS_BLACK_SHIFT + pass_left);
  if (P == osl::WHITE)
    return 1 << (IS_SUCCESS_WHITE_SHIFT + pass_left);
}

template <osl::Player P>
int osl::ntesuki::NtesukiMove::
is_fail_flag(int pass_left) const
{
  ntesuki_assert(pass_left >= 0 && 
		 pass_left <= 3);
  if (P == osl::BLACK)
    return 1 << (IS_FAIL_BLACK_SHIFT + pass_left);
  else
    return 1 << (IS_FAIL_WHITE_SHIFT + pass_left);
}

template<osl::Player P>
void
osl::ntesuki::NtesukiMove::
setCheckmateSuccess(int pass_left)
{
  ntesuki_assert(!(flags & is_success_flag<P>(pass_left)));
  ntesuki_assert(!(flags & is_fail_flag<P>(pass_left)));
  flags |= is_success_flag<P>(pass_left);
};

template<osl::Player P>
bool osl::ntesuki::NtesukiMove::
isCheckmateSuccess(int pass_left) const
{
  return (flags & is_success_flag<P>(pass_left)) == is_success_flag<P>(pass_left);
}

template<osl::Player P>
void osl::ntesuki::NtesukiMove::
setCheckmateFail(int pass_left)
{
  ntesuki_assert(!(flags & is_fail_flag<P>(pass_left)));
  ntesuki_assert(!(flags & is_success_flag<P>(pass_left)));
  flags |= is_fail_flag<P>(pass_left);
};


template<osl::Player P>
bool osl::ntesuki::NtesukiMove::
isCheckmateFail(int pass_left) const
{
  return (flags & is_fail_flag<P>(pass_left)) == is_fail_flag<P>(pass_left);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
