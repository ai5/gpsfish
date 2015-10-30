/* sennichite.h
 */
#ifndef OSL_SENNICHITE_H
#define OSL_SENNICHITE_H

#include "osl/basic_type.h"
#include <iosfwd>

namespace osl
{
  class Sennichite 
  {
    friend bool operator==(const Sennichite&, const Sennichite&);
    struct Result
    {
      enum Values { NORMAL, DRAW, BLACK_LOSE, WHITE_LOSE };
    };
    Result::Values value;
    Sennichite(Result::Values v) : value(v) {}
  public:
    static Sennichite NORMAL() { return Result::NORMAL; }
    static Sennichite DRAW()   { return Result::DRAW; }
    static Sennichite BLACK_LOSE() { return Result::BLACK_LOSE; }
    static Sennichite WHITE_LOSE() { return Result::WHITE_LOSE; }

    bool isNormal() const { return value == Result::NORMAL; }
    bool isDraw() const { return value == Result::DRAW; }
    bool hasWinner() const 
    {
      return (value == Result::BLACK_LOSE) || (value == Result::WHITE_LOSE); 
    }
    Player winner() const;
  };

  inline bool operator==(const Sennichite& l, const Sennichite& r)
  {
    return l.value == r.value;
  }
  std::ostream& operator<<(std::ostream&, const Sennichite&);
} // namespace osl

#endif /* OSL_SENNICHITE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
