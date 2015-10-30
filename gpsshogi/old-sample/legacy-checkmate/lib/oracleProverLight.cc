/* oracleProverLight.cc
 */
#include "oracleProverLight.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/checkmate/oracleAdjust.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/container/moveVector.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_action/store.h"
#include "osl/apply_move/applyMove.h"

namespace osl
{
  namespace checkmate
  {
    /**
     * OracleProverLight::defense を呼ぶhelper
     */
    template <Player Attacker>
    struct OracleProverLightDefense
    {
      OracleProverLight<Attacker> *prover;
      ProofOracleDefense<Attacker> oracle;
      ProofDisproof result;
      void operator()(Square)
      {
	result = prover->defense(oracle);
      }
    };

    /**
     * OracleProverLight::attack を呼ぶhelper
     */
    template <Player Attacker>
    struct OracleProverLightAttack
    {
      OracleProverLight<Attacker> *prover;
      ProofOracleAttack<Attacker> oracle;
      ProofDisproof result;
      void operator()(Square)
      {
	result = prover->attack(oracle);
      }
    };

  } // namespace checkmate
} // namespace osl

template <osl::Player Attacker>
bool osl::checkmate::OracleProverLight<Attacker>::
proofWin(state_t& state, ProofOracleAttack<Attacker> oracle, Move& best_move)
{
  check_assert(oracle.isValid());
  check_assert(state.turn() == Attacker);
  this->state = &state;
  fixed_searcher.setState(state);
  if ((! state.inCheck())
      && ImmediateCheckmate::hasCheckmateMove<Attacker>(state, best_move)) {
    assert(state.isValidMove(best_move));
    return true;
  }
  if (oracle.guide->getBestMove() 
      && oracle.guide->getBestMove()->flags.isSet(MoveFlags::ImmediateCheckmate))
    return fixed_searcher.hasCheckmateMove<Attacker>(2, best_move).isCheckmateSuccess();
  const ProofDisproof result = attack(oracle);
  if (! result.isCheckmateSuccess())
    return false;
  best_move = this->best_move;
  assert(state.isValidMove(best_move));
  return true;
}

template <osl::Player Attacker>
bool osl::checkmate::OracleProverLight<Attacker>::
proofLose(state_t& state, ProofOracleDefense<Attacker> oracle, Move last_move)
{
  check_assert(oracle.isValid());
  check_assert(state.turn() == alt(Attacker));
  this->state = &state;
  fixed_searcher.setState(state);

  const ProofDisproof result = defense(oracle);
  if (result.isPawnDropFoul(last_move))
    return false;
  return result.isCheckmateSuccess();
}

template <osl::Player Attacker>
const osl::checkmate::ProofDisproof
osl::checkmate::OracleProverLight<Attacker>::
attack(ProofOracleAttack<Attacker> oracle)
{
  ++node_count;
  check_assert(oracle.isValid());
  check_assert(state->turn() == Attacker);
#if 0
  if ((! EffectUtil::isKingInCheck(Attacker, *state))
      && ImmediateCheckmate::hasCheckmateMove<Attacker>(*state, best_move))
    return ProofDisproof::Checkmate();
  if (oracle.guide->getBestMove() 
      && oracle.guide->getBestMove()->flags.isSet(MoveFlags::ImmediateCheckmate))
    return fixed_searcher.hasCheckmateMove<Attacker>(2, best_move);
#endif
  if (fixed_searcher.hasCheckmateMove<Attacker>(2, best_move).isCheckmateSuccess()) {
    state->isValidMove(best_move);
    return ProofDisproof::Checkmate();
  }

  ProofOracleDefense<Attacker> new_oracle = oracle.expandOracle();
  if (! new_oracle.isValid())
    return ProofDisproof::Unknown();
  check_assert(oracle.oracle().player() == Attacker);
  Move check_move = OracleAdjust::attack(*state, oracle.oracle());
  if (! check_move.isNormal())
    return ProofDisproof::Unknown();

  OracleProverLightDefense<Attacker> oracle_prover = {this, new_oracle,
						      ProofDisproof::Unknown()};
  ApplyMove<Attacker>::doUndoMove(*state, check_move, oracle_prover);
  if (oracle_prover.result.isPawnDropFoul(check_move))
    return ProofDisproof::Unknown();

  best_move = check_move;
  return oracle_prover.result;
}

template <osl::Player Attacker>
const osl::checkmate::ProofDisproof
osl::checkmate::OracleProverLight<Attacker>::
defense(ProofOracleDefense<Attacker> oracle)
{
  ++node_count;
  check_assert(oracle.isValid());
  const Player Defense = PlayerTraits<Attacker>::opponent;

  const bool illegal = state->inCheck(Attacker);
  if (illegal)
    return ProofDisproof::NoCheckmate();
  const bool check = state->inCheck();
  if (! check)
    return ProofDisproof::NoCheckmate();

  MoveVector moves;
  move_generator::GenerateEscape<Defense>::
    generateKingEscape(*state,moves);

  if (moves.empty())
    return ProofDisproof::NoEscape();
  for (MoveVector::const_iterator p=moves.begin(); p!=moves.end(); ++p)
  {
    ProofOracleAttack<Attacker> new_oracle = oracle.expandOracle(*p);
    if (! new_oracle.isValid()) {
      const ProofDisproof pdp = fixed_searcher.hasEscapeMove<Attacker>(*p, 2);
      if (pdp.isCheckmateSuccess())
	return pdp;
      return ProofDisproof::Unknown();
    }
    OracleProverLightAttack<Attacker> oracle_prover = {this, new_oracle,
						       ProofDisproof::Unknown()};
    ApplyMove<Defense>::doUndoMove(*state, *p, oracle_prover);
    
    if (! oracle_prover.result.isCheckmateSuccess())
      return oracle_prover.result;
  }
  return ProofDisproof::Checkmate();
}
namespace osl
{
  namespace checkmate
  {
    template class OracleProverLight<BLACK>;
    template class OracleProverLight<WHITE>;
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
