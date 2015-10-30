/* proofOracle.h
 */
#ifndef _PROOFORACLE_H
#define _PROOFORACLE_H

#include "checkHashRecord.h"
#include "checkAssert.h"
#include <cstddef>
namespace osl
{
  namespace checkmate
  {
    template <Player Attacker>
    struct ProofOracleDefense;
  
    /**
     * 詰む類似局面 (攻方).
     * OracleProver が使用
     */
    template <Player Attacker>
    struct ProofOracleAttack
    {
      const CheckHashRecord *guide;
      explicit ProofOracleAttack(const CheckHashRecord *g=0) : guide(g)
      {
	check_assert((!guide) 
		     || ((guide->proofDisproof().isCheckmateSuccess()
			  && (guide->hasBestMove() 
			      && guide->getBestMove()->move.player() == Attacker))
			 || (guide->dump(), 0)));
	// (guide->proofDisproof() == NoEscape)
      }
      Move oracle() 
      {
	check_assert(guide && guide->hasBestMove());
	return guide->getBestMove()->move;
      }
      inline ProofOracleDefense<Attacker> expandOracle();
      bool isValid() const { return guide; }
    };

    /**
     * 詰む類似局面 (受方).
     * OracleProver や CheckmateSearcher が使用
     */
    template <Player Attacker>
    struct ProofOracleDefense
    {
      const CheckHashRecord *guide;
      explicit ProofOracleDefense(const CheckHashRecord *g=0) : guide(g)
      {
	check_assert((!guide) 
		     || (guide->proofDisproof().isCheckmateSuccess()
			 || (g->dump(), 0)));
      }
      
      /** bestMove の後に受方が defense をしたときのoracle */
      ProofOracleAttack<Attacker> expandOracle(Move defense)
      {
	check_assert(guide);
	check_assert(alt(defense.player()) == Attacker);
	const CheckMoveList& moves 
	  = (guide->finalByDominance() 
	     ? guide->finalByDominance()->moves
	     : guide->moves);
	for (CheckMoveList::const_iterator p=moves.begin();
	     p!=moves.end(); ++p)
	{
	  if (p->move == defense)
	  {
	    check_assert(p->record || (guide->dump(), 0));
	    check_assert(p->record->proofDisproof().isCheckmateSuccess()
			 || (guide->dump(), 0));
	    return ProofOracleAttack<Attacker>(p->record);
	  }
	}
	if (defense.isDrop())
	{
	  const Square to = defense.to();
	  for (CheckMoveList::const_iterator p=moves.begin();
	       p!=moves.end(); ++p)
	  {
	    // TODO: 似た駒から..
	    if (p->move.isDrop() && p->move.to() == to)
	    {
	      check_assert(p->record || (guide->dump(), 0));
	      check_assert(p->record->proofDisproof().isCheckmateSuccess()
			   || (guide->dump(), 0));
	      return ProofOracleAttack<Attacker>(p->record);
	    }
	  }
	}
	return ProofOracleAttack<Attacker>(0);
      }
      bool isValid() const { return guide; }
    };

    template <Player Attacker> inline
    ProofOracleDefense<Attacker> ProofOracleAttack<Attacker>::expandOracle()
    {
      check_assert(guide);
      const CheckHashRecord *nextGuide =0;
      if (guide->hasBestMove() && guide->getBestMove()->record
	  && guide->getBestMove()->record->proofDisproof().isCheckmateSuccess())
	nextGuide = guide->getBestMove()->record;
      return ProofOracleDefense<Attacker>(nextGuide);
    }
  } // namespace checkmate
} // namespace osl

#endif /* _PROOFORACLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
