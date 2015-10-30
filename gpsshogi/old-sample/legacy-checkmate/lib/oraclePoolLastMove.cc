/* oraclePoolLastMove.cc
 */
#include "oraclePoolLastMove.h"
#include "checkHashRecord.h"
#include "osl/state/simpleState.h"
#include "osl/stl/hash_map.h"
#ifdef OSL_SMP
#  include "osl/misc/lightMutex.h"
#endif
#include <algorithm>

struct osl::checkmate::OraclePoolLastMove::Table 
{
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >=3)
  typedef osl::hash_map<int,const osl::checkmate::CheckHashRecord*,
			std::tr1::hash<int> > hash_map_t;
#else
  typedef osl::hash_map<int,const osl::checkmate::CheckHashRecord*,
			__gnu_cxx::hash<int> > hash_map_t;
#endif

#ifdef OSL_SMP
  typedef osl::misc::LightMutex Mutex;
  mutable Mutex mutex;
#endif

  hash_map_t table;
  const Player defender;

  explicit Table(Player attacker) : defender(alt(attacker))
  {
  }
  /**
   * from と old ptype だけ見る
   */
  int makeKey(Square king_position, Move last_move) const
  {
    assert(! last_move.isDrop());
    assert(last_move.player() == defender);
    int result = king_position.index() * Square::indexMax();
    result += last_move.to().index();
    result *= Square::indexMax();
    result += last_move.from().index();
    result *= PTYPE_SIZE;
    result += last_move.oldPtype();
    return result;
  }
  const CheckHashRecord *& get(Square king_position, Move last_move)
  {
    const int key = makeKey(king_position, last_move);
#ifdef OSL_SMP
    SCOPED_LOCK(lk,mutex);
#endif
    return table[key];
  }
  const CheckHashRecord * find(Square king_position, Move last_move) const
  {
    const int key = makeKey(king_position, last_move);
#ifdef OSL_SMP
    SCOPED_LOCK(lk,mutex);
#endif
    hash_map_t::const_iterator pos = table.find(key);
    if (pos != table.end())
      return pos->second;
    return 0;
  }
  size_t size() const
  {
    return table.size();
  }
};

osl::checkmate::
OraclePoolLastMove::OraclePoolLastMove(Player attacker)
  : oracles(new Table(attacker))
{
}
  
osl::checkmate::
OraclePoolLastMove::~OraclePoolLastMove()
{
}
  
void osl::checkmate::
OraclePoolLastMove::addOracle(const SimpleState& state, Move last_move, 
			      const CheckHashRecord* oracle)
{
  assert(state.turn() == alt(oracles->defender));
  if (last_move.isPass())
    return;
  const Square king_position = state.kingSquare(oracles->defender);
  assert(oracle->hasProofPieces());
  oracles->get(king_position,last_move) = oracle;
}
  
const osl::checkmate::CheckHashRecord *osl::checkmate::
OraclePoolLastMove::findOracle(const SimpleState& state,
			       Move last_move, PieceStand black_stand,
			       unsigned short& oracle_age) const
{
  assert(state.turn() == alt(oracles->defender));
  if (oracle_age)
    return 0;
  if (last_move.isPass())
    return 0;
  
  const Square king_position = state.kingSquare(oracles->defender);
  const CheckHashRecord *record = oracles->find(king_position, last_move);
  if (record)
  {
    ++oracle_age;
    const PieceStand attack_stand 
      = (oracles->defender != BLACK) ? black_stand : PieceStand(WHITE, state);
    if (attack_stand.hasMoreThan<BLACK>(record->proofPieces()))
      return record;
  }
  return 0;
}

size_t osl::checkmate::
OraclePoolLastMove::size() const
{
  return oracles->size();
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
