/* dfpnRecord.h
 */
#ifndef OSL_DFPNRECORD_H
#define OSL_DFPNRECORD_H

#include "osl/checkmate/proofDisproof.h"
#include "osl/basic_type.h"

#define NAGAI_DAG_TEST

namespace osl
{
  namespace checkmate
  {
    struct DfpnRecordBase
    {
      ProofDisproof proof_disproof;
      /** 手番に否定的に結果が判明したリスト loop は除く */
      uint64_t solved;		
#ifdef NAGAI_DAG_TEST
      /** 合流を引き起こす指手一覧 */
      uint64_t dag_moves;
#endif
      Move best_move;
      PieceStand proof_pieces;
      mutable unsigned int node_count;
      unsigned int tried_oracle;
      /** 合流検知+simulation中の簡易 無限ループ回避 */
      Move last_move;		
      /** solved のmax */
      PieceStand proof_pieces_candidate;
      unsigned int min_pdp;	// solved のmin
      uint32_t working_threads;
      Square last_to;
      enum ProofPiecesType { UNSET=0, PROOF, DISPROOF };
      int8_t proof_pieces_set;
      char need_full_width, false_branch;
#ifdef NAGAI_DAG_TEST
      bool dag_terminal;
#endif

      DfpnRecordBase() 
	: solved(0),
#ifdef NAGAI_DAG_TEST
	  dag_moves(0), 
#endif
	  node_count(0), tried_oracle(0), min_pdp(ProofDisproof::PROOF_MAX),
	  working_threads(0),
	  proof_pieces_set(UNSET), need_full_width(false), false_branch(false)
#ifdef NAGAI_DAG_TEST
	, dag_terminal(0)
#endif
      {
      }
    };

    class DfpnRecord : public DfpnRecordBase
    {
    public:
      CArray<PieceStand,2> stands;

      DfpnRecord() {}
      DfpnRecord(PieceStand black, PieceStand white) { stands[BLACK] = black; stands[WHITE] = white; }

      void setFrom(const DfpnRecordBase& src) 
      {
	static_cast<DfpnRecordBase*>(this)->operator=(src);
	node_count = 1;
	solved = 0;
	last_to = Square();
	last_move = Move();
	need_full_width = false_branch = false;
#ifdef NAGAI_DAG_TEST
	dag_moves = 0;
	dag_terminal = false;
#endif
      }
      unsigned int proof() const { return proof_disproof.proof(); }
      unsigned int disproof() const { return proof_disproof.disproof(); }
      void setProofPieces(PieceStand a) 
      {
	assert(proof_pieces_set == UNSET);
	assert((stands[BLACK] == PieceStand() && stands[WHITE] == PieceStand())
	       || stands[BLACK].isSuperiorOrEqualTo(a)
	       || stands[WHITE].isSuperiorOrEqualTo(a));
	proof_pieces_set = PROOF;
	proof_pieces = a;
      }
      void setDisproofPieces(PieceStand a) 
      {
	assert(proof_pieces_set == UNSET);
	assert((stands[BLACK] == PieceStand() && stands[WHITE] == PieceStand())
	       || stands[BLACK].isSuperiorOrEqualTo(a)
	       || stands[WHITE].isSuperiorOrEqualTo(a));
	proof_pieces_set = DISPROOF;
	proof_pieces = a;
      }
      const PieceStand proofPieces() const 
      {
	assert(proof_pieces_set == PROOF);
	return proof_pieces;
      }
      const PieceStand disproofPieces() const 
      {
	assert(proof_pieces_set == DISPROOF);
	return proof_pieces;
      }
    };
  }
}



#endif /* OSL_DFPNRECORD_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
