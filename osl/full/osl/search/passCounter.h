/* passCounter.h
 */
#ifndef SEARCH_PASSCOUNTER_H
#define SEARCH_PASSCOUNTER_H

namespace osl
{
  namespace search
  {
    class PassCounter
    {
      CArray<int,2> counter;
    public:
      PassCounter()
      {
	counter.fill(0);
      }
      void inc(Player moving)
      {
	assert(playerToIndex(moving) >= 0);
	++counter[moving];
      }
      void dec(Player moving)
      {
	--counter[moving];
	assert(playerToIndex(moving) >= 0);
      }
      bool loopByBothPass() const
      {
	return counter[0] && counter[1];
      }
    };
  } // namespace search
} // namespace osl

#endif /* __H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
