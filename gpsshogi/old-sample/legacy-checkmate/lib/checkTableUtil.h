/* checkTableUtil.h
 */
#ifndef _CHECKTABLEUTIL_H
#define _CHECKTABLEUTIL_H

#include "checkHashRecord.h"
#include "osl/hash/hashKey.h"
#include <cassert>
namespace osl
{
  namespace hash
  {
    class HashKey;
  }
  namespace checkmate
  {
    struct CheckTableUtil
    {
      template <class Table>
      static
      void allocate(CheckHashRecord*& record, Table& table, 
		    const HashKey& key, PieceStand white_stand,
		    const PathEncoding& path,
		    CheckHashRecord *parent)
      {
	check_assert(record == 0);
	record = table.allocate(key, white_stand, path);
	if (! record->parent)
	{
	  record->parent = parent;
	  record->distance = parent->distance+1;
	}
      }

      /**
       * white_stand を作ってからrecordを確保する
       */
      template <class Table>
      static
      void allocate(Move last_move, CheckHashRecord*& record, Table& table, 
		    const HashKey& key, const PathEncoding& path,
		    CheckHashRecord *parent)
      {
	assert(parent);
	const PieceStand white_stand
	  = parent->stand(WHITE).nextStand(WHITE, last_move);
	allocate(record, table, key, white_stand, path, parent);
      }

      /**
       * keyBefore と pathBefore に nextMove を指した後のものにしてから
       * record を確保する
       */
      template <class Table>
      static
      void allocateNext(Move next_move, 
			CheckHashRecord*& record, Table& table, 
			const HashKey& key_before, 
			const PathEncoding& path_before,
			CheckHashRecord *parent)
      {
	assert(parent);
	const PieceStand white_stand
	  = parent->stand(WHITE).nextStand(WHITE, next_move);
	return allocate(record, table, key_before.newHashWithMove(next_move), 
			white_stand,
			PathEncoding(path_before, next_move), parent);
      }
      /** defense node で move を指すと check_move でfixed_searher で詰みと記録 */
      template <Player P, class Table>
      static void registerImmediateCheckmateInDefense
      (const HashKey& key, const PathEncoding& path,
       CheckHashRecord *record, CheckMove& move, 
       ProofDisproof pdp, Move check_move, PieceStand proof_pieces,
       Table& table);
    };
  } // namespace checkmate
} // namespace osl

template <osl::Player P, class Table>
void osl::checkmate::CheckTableUtil::
registerImmediateCheckmateInDefense(const HashKey& key, const PathEncoding& path,
				    CheckHashRecord *record, CheckMove& move,
				    ProofDisproof pdp,
				    Move check_move, PieceStand proof_pieces,
				    Table& table)
{
  assert(pdp.isCheckmateSuccess());
  const HashKey new_key = key.newHashWithMove(move.move);
  const PathEncoding new_path(path, move.move);
  if (! move.record)
    CheckTableUtil::allocate
      (move.move, move.record, table, new_key, new_path, record);
  if (! move.record->proofDisproof().isCheckmateSuccess())
  {
    if (move.record->moves.empty())
    {
      CheckMove best_move(check_move);
      best_move.flags.set(MoveFlags::ImmediateCheckmate);
      move.record->moves.setOne(best_move, table.listProvider());
      move.record->bestMove = &*(move.record->moves.begin());
    }
    else
    {
      CheckMove *attack = move.record->moves.find(check_move);
      assert(attack);
      move.record->bestMove = attack;
      move.record->bestMove->flags.set(MoveFlags::ImmediateCheckmate);
      move.record->bestMove->record = 0;
    }
    move.record->setProofPieces(proof_pieces);
    move.record->propagateCheckmate<P>(ProofDisproof::Checkmate());
  }
  record->addToSolvedInDefense(move, pdp);
}

#endif /* _CHECKTABLEUTIL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
