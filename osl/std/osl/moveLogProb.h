#ifndef OSL_MOVELOGPROB_H
#define OSL_MOVELOGPROB_H

#include "osl/basic_type.h"
#include <iosfwd>

namespace osl
{
  class MoveLogProb : private Move
  {
    int log_prob;
  public:
    MoveLogProb(Move m,int l) : Move(m),log_prob(l)
    {
      assert(m.isInvalid() || m.isValidOrPass());
    }
    MoveLogProb() : log_prob(0) {}

    void setLogProb(int l)  { log_prob=l; }
    /** 
     * logProb を数字上最低 l にする.
     * 確率としては，高すぎる確率の場合， l に補正する
     */
    void setLogProbAtLeast(int l)  
    { 
      if (logProb() < l)
	setLogProb(l);
    }
    /** 
     * logProb を数字上最高 l にする.
     * 確率としては，低すぎる確率の場合， l に補正する
     */
    void setLogProbAtMost(int l)  
    { 
      if (logProb() > l)
	setLogProb(l);
    }

    const Move move()const{ return *this; }
    int logProb()const{ return log_prob; }
    bool validMove() const { return log_prob > 0; }

    using Move::player;
    using Move::isNormal;
    using Move::isPass;
  };
  std::ostream& operator<<(std::ostream& os,MoveLogProb const& move);

  inline bool operator==(MoveLogProb const& lhs,MoveLogProb const& rhs)
  {
    return lhs.move()==rhs.move() && lhs.logProb()==rhs.logProb();
  }
  inline bool operator<(MoveLogProb const& lhs,MoveLogProb const& rhs)
  {
    if (lhs.move() != rhs.move())
      return lhs.move() < rhs.move();
    return lhs.logProb() < rhs.logProb();
  }

} // namespace ostream


#endif /* OSL_MOVELOGPROB_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
