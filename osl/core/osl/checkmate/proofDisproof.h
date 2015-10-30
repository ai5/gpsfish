#ifndef _PROOF_DISPROOF_H
#define _PROOF_DISPROOF_H

#include "osl/basic_type.h"
#include <cassert>
#include <iosfwd>
namespace osl
{
  namespace checkmate
  {
    /**
     * 証明数(proof number)と反証数(disproof number).
     * 詰み局面と確定した時は proofNumber=0
     * 不詰み局面と確定した時は disproofNumber=0
     */
    class ProofDisproof
    {
      unsigned long long pdp;
    public:
      enum {
	PROOF_SHIFT = 32,
	DISPROOF_MASK = 0xffffffffu,
	PROOF_MAX = (0xffffffffu / 16),
	DISPROOF_MAX = (0xffffffffu / 16),
        /** 反証数の定数: 詰んだ時には，詰の種類の区別に利用 */
	NO_ESCAPE_DISPROOF  = (DISPROOF_MAX - 1),
	CHECK_MATE_DISPROOF = (DISPROOF_MAX - 2),
        /** 証明数の定数: 反証された時には，不詰の種類の区別に利用 */
	NO_CHECK_MATE_PROOF   = (PROOF_MAX - 1),
	PAWN_CHECK_MATE_PROOF = (PROOF_MAX - 2),
	LOOP_DETECTION_PROOF  = (PROOF_MAX - 3),
	ATTACK_BACK_PROOF  = (PROOF_MAX - 4),
      };
    private:
      static void testConsistency();
    public:
      enum {
        /** 通常の反証数の上限 */
	DISPROOF_LIMIT = (DISPROOF_MAX - 3),
        /** 通常の証明数の上限 */
	PROOF_LIMIT           = (PROOF_MAX - 5),
      };
    private:
      static unsigned long long
      compose(unsigned long long proof, unsigned long long disproof) 
      {
	return (proof << PROOF_SHIFT) + disproof;
      }
      /** range check をしない private バージョン */
      ProofDisproof(unsigned long long value) : pdp(value)
      {
      }
      static const ProofDisproof
      make(unsigned int proof, unsigned int disproof)
      {
	return ProofDisproof(compose(proof, disproof));
      }
    public:
      ProofDisproof() : pdp(compose(1, 1))
      {
      }
      ProofDisproof(unsigned int proof, unsigned int disproof)
	: pdp(compose(proof,disproof))
      {
	assert(proof < PROOF_MAX);
	assert(disproof < DISPROOF_MAX);
	assert(proof || disproof);
	assert((proof == 0) ^ (disproof < DISPROOF_LIMIT));
	assert((disproof == 0) ^ (proof < PROOF_LIMIT));
      }
      static const ProofDisproof makeDirect(unsigned long long value) { return ProofDisproof(value); }

      // constants
      static const ProofDisproof NoEscape()  { return ProofDisproof(0, NO_ESCAPE_DISPROOF); }
      static const ProofDisproof Checkmate() { return ProofDisproof(0, CHECK_MATE_DISPROOF); }
      static const ProofDisproof NoCheckmate() { return ProofDisproof(NO_CHECK_MATE_PROOF,   0); }
      static const ProofDisproof PawnCheckmate() { return ProofDisproof(PAWN_CHECK_MATE_PROOF, 0); }
      static const ProofDisproof LoopDetection() { return ProofDisproof(LOOP_DETECTION_PROOF,  0); }
      static const ProofDisproof AttackBack() { return ProofDisproof(ATTACK_BACK_PROOF,  0); }
      static const ProofDisproof Unknown () { return ProofDisproof(1, 1); }
      /** 攻方にも受方にも不都合な仮想的な数 */
      static const ProofDisproof Bottom    () { return make(PROOF_MAX, DISPROOF_MAX); }

      unsigned int proof()    const { return pdp >> PROOF_SHIFT; }
      unsigned int disproof() const { return pdp & DISPROOF_MASK; }
      bool isCheckmateSuccess() const { return proof()==0; }
      bool isCheckmateFail() const { return disproof()==0; }
      bool isFinal() const { return isCheckmateSuccess() || isCheckmateFail(); }
      bool isUnknown() const { return !isFinal(); }

      /** 打歩詰めなら真 */
      bool isPawnDropFoul(Move move) const
      {
	return (pdp == NoEscape().pdp) && move.isNormal() && move.isDrop() 
	  && (move.ptype()==PAWN);
      }
      bool isLoopDetection() const { return pdp == LoopDetection().pdp; }
      
      unsigned long long ulonglongValue() const { return pdp; }

      static const unsigned int BigProofNumber=PROOF_MAX;

      /**
       * this が r より攻方に都合が良い時に真
       */
      bool isBetterForAttack(const ProofDisproof& r) const
      {
	const unsigned int lp = proof();
	const unsigned int rp = r.proof();
	if (lp != rp)
	  return lp < rp;
	return disproof() > r.disproof();
      }
      /**
       * this が r より受方に都合が良い時に真
       */
      bool isBetterForDefense(const ProofDisproof& r) const
      {
	const unsigned int ld = disproof();
	const unsigned int rd = r.disproof();
	if (ld != rd)
	  return ld < rd;
	return proof() > r.proof();
      }
      /**
       * 攻方に都合が良い方を返す
       */
      const ProofDisproof& betterForAttack(const ProofDisproof& r) const
      {
	return (isBetterForAttack(r) ? *this : r);
      }
      /**
       * 受方に都合が良い方を返す
       */
      const ProofDisproof& betterForDefense(const ProofDisproof& r) const
      {
	return (isBetterForDefense(r) ? *this : r);
      }
    };
    inline bool operator==(const ProofDisproof& l, const ProofDisproof& r)
    {
      return l.ulonglongValue() == r.ulonglongValue();
    }
    inline bool operator!=(const ProofDisproof& l, const ProofDisproof& r)
    {
      return ! (l == r);
    }
    inline bool operator<(const ProofDisproof& l, const ProofDisproof& r)
    {
      return l.ulonglongValue() < r.ulonglongValue();
    }
  
    std::ostream& operator<<(std::ostream& os,
			     const ProofDisproof& proofDisproof);
  } // namespace checkmate

  using checkmate::ProofDisproof;
} // namespace osl
#endif // _PROOF_DISPROOF_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
