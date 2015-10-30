/* oraclePoolLastMove.h
 */
#ifndef _ORACLEPOOL_LASTMOVE_H
#define _ORACLEPOOL_LASTMOVE_H

#include "osl/move.h"
#include "osl/pieceStand.h"
#include <boost/scoped_ptr.hpp>
#include <cstddef>

namespace osl
{
  namespace state
  {
    class SimpleState;
  }
  namespace checkmate
  {
    using state::SimpleState;
    class CheckHashRecord;
    /**
     * 前回動いた駒に基づくOracleの管理
     * 動いてはいけない駒のチェック．
     * 駒を渡すと詰というチェックはできなくなる．
     */
    class OraclePoolLastMove
    {
      class Table;
      std::unique_ptr<Table> oracles;
    public:
      explicit OraclePoolLastMove(Player attacker);
      ~OraclePoolLastMove();

      void addOracle(const SimpleState&,Move last_move, const CheckHashRecord*);
      const CheckHashRecord *findOracle(const SimpleState&, Move last_move,
					PieceStand black_stand,
					unsigned short& oracle_age) const;
      size_t size() const;
    };

  } // namespace checkmate
} // namespace osl

#endif /* _ORACLEPOOL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
