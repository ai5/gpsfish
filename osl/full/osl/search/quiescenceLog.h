/* quiescenceLog.h
 */
#ifndef SEARCH_QUIESCENCELOG
#define SEARCH_QUIESCENCELOG
#include "osl/numEffectState.h"
#include <iosfwd>

namespace osl
{
  namespace search
  {
    class QuiescenceRecord;
    /**
     * 取り合い探索の記録をとる.
     * init しない限り記録は残らない
     */
    struct QuiescenceLog
    {
      static void enter(const SimpleState&);
      static void pushMove(int depth, Move m, const QuiescenceRecord *r);
      static void staticValue(int depth, int value);
      static void node(int depth, int alpha, int beta, int result);
      static void init(const char *filename);
      static void close();
      static std::ostream *os();
    };
  } // namespace search
} // namespace osl


#endif /* SEARCH_QUIESCENCELOG */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
