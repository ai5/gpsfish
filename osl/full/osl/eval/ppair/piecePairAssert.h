/* piecePairAssert.h
 */
#ifndef EVAL_PPAIR_PIECEPAIRASSERT_H
#define EVAL_PPAIR_PIECEPAIRASSERT_H

#include <cassert>

#ifdef PIECE_PAIR_EXTRA_DEBUG
#  define piece_pair_assert(x) assert(x)
#else
#  define piece_pair_assert(x)
#endif

#endif /* EVAL_PPAIR_PIECEPAIRASSERT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
