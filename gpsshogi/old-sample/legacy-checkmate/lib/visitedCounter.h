/* visitedCounter.h
 */
#ifndef OSL_CHECKMATE_VISITEDCOUNTER_H
#define OSL_CHECKMATE_VISITEDCOUNTER_H

namespace osl
{
  namespace checkmate
  {
    /**
     * 外部からvisitedを書き込んだ数を記録しておく．
     * @see CheckHistoryToTable
     */
    class VisitedCounter
    {
      int counter;
    public:
      VisitedCounter();
      virtual ~VisitedCounter();
	  
      void incrementVisited() { ++counter; }
      int countVisited() const { return counter; }
    };
  } // namespace checkmate
} // namespace osl

#endif /* OSL_CHECKMATE_VISITEDCOUNTER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
