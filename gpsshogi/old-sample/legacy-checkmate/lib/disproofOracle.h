/* disproofOracle.h
 */
#ifndef _DISPROOFORACLE_H
#define _DISPROOFORACLE_H

#include "checkHashRecord.h"
#include "checkAssert.h"
#include "osl/pathEncoding.h"

#include <cstddef>
namespace osl
{
  namespace checkmate
  {
    template <Player Attacker>
    struct DisproofOracleAttack;
  
    /**
     * 詰まない類似局面 (受方).
     * OracleDisrover が使用
     */
    template <Player Attacker>
    struct DisproofOracleDefense
    {
      const CheckHashRecord *guide;
      const CheckHashRecord *next_guide;
      PathEncoding path;
      Move best_move;
      DisproofOracleDefense(CheckHashRecord *g, const PathEncoding& p)
	: guide(g), next_guide(0), path(p), best_move(Move::INVALID())
      {
	if (guide)
	{
	  if (guide->hasBestMove() 
	      && guide->getBestMove()->record 
	      && guide->getBestMove()->record->proofDisproof().isCheckmateFail())
	  {
	    next_guide = guide->getBestMove()->record;
	    best_move = guide->getBestMove()->move;
	    if (! guide->proofDisproof().isCheckmateFail())
	    {
	      g->setDisproofPiecesDefense(alt(Attacker));
	      g->propagateNoCheckmate<Attacker>(g->getBestMove()->record->proofDisproof());
	    }
	  }
	  else
	  {
	    if (const TwinEntry *loop = guide->findLoopInList(path)) // LoopDetection
	    {
	      next_guide = loop->move.record;
	      best_move = loop->move.move;
	    }
	    else
	    {
	      setValidTwins();
	    }
	  }
	  check_assert((guide->proofDisproof().isCheckmateFail()
			&& guide->hasBestMove() 
			&& (guide->getBestMove()->move.player()==alt(Attacker)))
		       || (guide->findLoopInList(path))
		       || (guide->proofDisproof() == ProofDisproof::NoEscape())
		       || (guide->dump(), 0));
	}
      }
      explicit DisproofOracleDefense(CheckHashRecord *g)
	: guide(g), next_guide(0), path(alt(Attacker)), best_move(Move::INVALID())
      {
	if (guide)
	{
	  if (guide->hasBestMove() && guide->getBestMove()->record)
	  {
	    if (guide->getBestMove()->record->proofDisproof().isCheckmateFail())
	    {
	      next_guide = guide->getBestMove()->record;
	      best_move = guide->getBestMove()->move;
	      if (! guide->proofDisproof().isCheckmateFail())
	      {
		g->setDisproofPiecesDefense(alt(Attacker));
		g->propagateNoCheckmate<Attacker>(g->getBestMove()->record->proofDisproof());
	      }
	    }
	    else if (! guide->bestMove->record->twins.empty())
	    {
	      next_guide = guide->getBestMove()->record;
	      best_move = guide->getBestMove()->move;
	      path = guide->bestMove->record->twins.begin()->path;
	      path.popMove(guide->bestMove->move);
	    }
	  }
	  if ((! next_guide) && (! guide->twins.empty()))
	  {
	    setValidTwins();
	  }
	  check_assert((! guide->hasBestMove())
		       || guide->getBestMove()->move.player()==alt(Attacker));
	  check_assert((guide->hasBestMove() 
			&& (guide->getBestMove()->record->proofDisproof().isCheckmateFail()
			    || (guide->getBestMove()->findLoopInList(path))))
		       || (guide->findLoopInList(path))
		       || (guide->proofDisproof() == ProofDisproof::NoEscape())
		       || (guide->dump(), 0));
	}
      }
    private:
      void setValidTwins()
      {
	for (TwinList::const_iterator p=guide->twins.begin();
	     p!=guide->twins.end(); ++p)
	{
	  if (best_move.isInvalid())
	  {
	    assert(! best_move.isPass());
	    path = p->path;
	    best_move = p->move.move;
	    next_guide = p->move.record;
	    if (next_guide)
	      break;
	  }
	}
      }
    public:
      Move oracle() 
      {
	check_assert(guide && next_guide);
	return best_move;
      }
      inline DisproofOracleAttack<Attacker> expandOracle() const;
      bool isValid() const { return guide; }
    };

    /**
     * 詰まない類似局面 (攻方).
     * OracleDisprover が使用
     */
    template <Player Attacker>
    struct DisproofOracleAttack
    {
      const CheckHashRecord *guide;
      PathEncoding path;
      DisproofOracleAttack(const CheckHashRecord *g, const PathEncoding& p)
	: guide(g), path(p)
      {
	if (guide && (! guide->proofDisproof().isCheckmateFail())
	    && (! guide->findLoopInList(path)))
	{
	  if (! guide->twins.empty())
	    path = guide->twins.begin()->path;
	  else
	    guide = 0;
	  // TODO: ここに来るのは LoopDetection に失敗した時だけなので
	  // 事前に効率的に判定したい
	}
	check_assert((!guide) 
		     || guide->proofDisproof().isCheckmateFail()
		     || guide->findLoopInList(path)
		     || (g->dump(), 0));
      }
      DisproofOracleAttack(const CheckHashRecord *g)
	: guide(g), path(Attacker)
      {
	if (guide && (! guide->proofDisproof().isCheckmateFail())
	    && (! guide->twins.empty()))
	{
	  path = guide->twins.begin()->path;
	}
	check_assert((!guide) 
		     || guide->proofDisproof().isCheckmateFail()
		     || guide->findLoopInList(path)
		     || (g->dump(), 0));
      }
      bool invalidNextOracle(const CheckMove& move) const
      {
	if (! move.record)
	  return true;
	const ProofDisproof& pdp = move.record->proofDisproof();
	return (! pdp.isCheckmateFail())
	  && (! (guide->bestResultInSolved == ProofDisproof::PawnCheckmate()
		 && (pdp == ProofDisproof::NoEscape())))
	  && move.record->twins.empty();
      }
      DisproofOracleDefense<Attacker> makeOracle(const CheckMove& move) const
      {
	check_assert(move.record);
#ifndef NDEBUG
	const ProofDisproof& pdp = move.record->proofDisproof();
#endif
	const PathEncoding new_path(path, move.move);
	check_assert((! move.record->twins.empty())
		     || (pdp.isCheckmateFail() && move.record->hasBestMove())
		     || (move.record->proofDisproof() == ProofDisproof::NoEscape())
		     || (guide->dump(), 0));
	return DisproofOracleDefense<Attacker>(move.record, new_path);
      }
      
      /** 攻方が attack をしたときのoracle */
      DisproofOracleDefense<Attacker> expandOracle(Move attack) const
      {
	check_assert(guide);
	check_assert(guide->proofDisproof().isCheckmateFail()
		     || guide->findLoopInList(path));
	check_assert(attack.player() == Attacker);
	const CheckMoveList& moves 
	  = (guide->finalByDominance()
	     ? guide->finalByDominance()->moves
	     : guide->moves);
	for (CheckMoveList::const_iterator p=moves.begin();
	     p!=moves.end(); ++p)
	{
	  if (p->move == attack)
	  {
	    if (invalidNextOracle(*p))
	      goto not_found;
	    return makeOracle(*p);
	  }
	}
	{
	  // 似た手を探す
	  const Square from = attack.from();
	  const Square to = attack.to();
	  const Ptype ptype = attack.ptype();
	  if (from.isPieceStand())
	  {
	    // 打った手
	    for (CheckMoveList::const_iterator p=moves.begin();
		 p!=moves.end(); ++p)
	    {
	      if (! p->move.isDrop())
		continue;
	      if (p->move.to() == to) // ptype が違う drop 
	      {
		if (! invalidNextOracle(*p))
		  return makeOracle(*p);
	      } 
	      else if (p->move.ptype() == ptype) // ptype が同じで，場所が違うdrop
	      {
		// 大駒を遠くから打ったような場合
		// TODO: 方向が同じことを確認，桂馬も含む?
		if (! invalidNextOracle(*p))
		  return makeOracle(*p);
	      }
	    }
	  }
	  else
	  {
	    // 動いた手
	    const CheckMove *ignorePtype = 0;
	    for (CheckMoveList::const_iterator p=moves.begin();
		 p!=moves.end(); ++p)
	    {
	      if ((p->move.to() != to) || invalidNextOracle(*p))
		continue;
	      if (unpromote(p->move.ptype()) == unpromote(ptype))
		return makeOracle(*p);
	      ignorePtype = &*p;	// 最後の手段
	    }
	    if (ignorePtype)
	      return makeOracle(*ignorePtype);
	  }
	}
      not_found:
	return DisproofOracleDefense<Attacker>(0,PathEncoding());
      }
      bool isValid() const { return guide; }
    };

    template <Player Attacker>
    inline DisproofOracleAttack<Attacker> 
    DisproofOracleDefense<Attacker>::expandOracle() const
    {
      PathEncoding new_path = path;
      new_path.pushMove(best_move);
      return DisproofOracleAttack<Attacker>(next_guide, new_path);
    }
  } // namespace checkmate
} // namespace osl

#endif /* _DISPROOFORACLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
