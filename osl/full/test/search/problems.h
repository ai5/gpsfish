/* problems.h
 */
#ifndef _PROBLEMS_H
#define _PROBLEMS_H

#include "osl/basic_type.h"

namespace osl
{
  struct Problem
  {
    enum { MaxExpected = 8 };
    const char *state;
    const char *expected[MaxExpected];
    bool acceptable(Move) const;
  };

  // 1手で解けると思われる問題
  extern const Problem problems1[];
  extern const int numProblems1;

  // 3手で解けると思われる問題 ...
  extern const Problem problems3[];
  extern const int numProblems3;
}

#endif /* _PROBLEMS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
