/* searchTable.h
 */
#ifndef SEARCH_SEARCHTABLE_H
#define SEARCH_SEARCHTABLE_H

namespace osl
{
  namespace search
  {
    struct SearchTable
    {
      /** 詰を発見した場合に探索深さとして記録する数値 (上書きされないように大きくする) */
      static const int CheckmateSpecialDepth = 10000;
      /** 今までの指手へのループの場合の探索深さとして記録する数値 (上書きされないように大きくする) */
      static const int HistorySpecialDepth =  9998;
    };

  } // namespace search
} // namespace osl

#endif /* SEARCH_SEARCHTABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
