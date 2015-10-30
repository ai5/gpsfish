#ifndef OSL_MOVE_LOG_PROB_VECTOR_H
#define OSL_MOVE_LOG_PROB_VECTOR_H
#include "osl/moveLogProb.h"
#include "osl/container.h"
#include <iosfwd>

namespace osl
{
  namespace container
  {
  typedef FixedCapacityVector<MoveLogProb,Move::MaxUniqMoves> MoveLogProbVectorBase;

  class MoveLogProbVector : public MoveLogProbVectorBase
  {
    typedef MoveLogProbVectorBase base_t;
  public:
    MoveLogProbVector() {}
    explicit MoveLogProbVector(size_t size) : MoveLogProbVectorBase(size)
    {
    }
    MoveLogProbVector(const MoveLogProbVector& src) : MoveLogProbVectorBase(src)
    {
    }
    template <class RangeIterator>
    MoveLogProbVector(const RangeIterator& first, const RangeIterator& last)
      : MoveLogProbVectorBase(first, last)
    {
    }
    void push_back(Move move,int prob) {
      base_t::push_back(MoveLogProb(move,prob));
    }
    void push_back(const MoveLogProb& move) {
      base_t::push_back(move);
    }
    template <class RangeIterator>
    void push_back(const RangeIterator& first, const RangeIterator& last)
    {
      MoveLogProbVectorBase::push_back(first, last);
    }
    /** 確率が高い順にsort */
    void sortByProbability();
    /** 確率が低い順にsort */
    void sortByProbabilityReverse();
    const MoveLogProb* find(Move) const;
  };
  std::ostream& operator<<(std::ostream& os,MoveLogProbVector const& mv);
  bool operator==(const MoveLogProbVector& l, const MoveLogProbVector& r);

  } // namespace container
  using container::MoveLogProbVector;
} // namespace osl
#endif // OSL_MOVE_LOG_PROB_VECTOR_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
