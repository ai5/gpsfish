/* alphaBeta4.cc
 */
#include "osl/search/alphaBeta4.h"
#include "osl/search/searchRecorder.h"

osl::search4::AlphaBeta4::
AlphaBeta4(const NumEffectState& /*state*/, checkmate_t& /*checkmate*/,
	   SimpleHashTable */*table*/, CountRecorder& /*recorder*/)
{
}

osl::search4::AlphaBeta4::
~AlphaBeta4()
{
}

osl::Move osl::search4::AlphaBeta4::
computeBestMoveIteratively(int /*limit*/, int /*step*/, int /*initial_limit*/, 
			   size_t /*node_limit*/, 
			   const TimeAssigned& assign,
			   MoveWithComment */*additional_info*/)
{
  this->setStartTime(clock::now());
  this->setTimeAssign(assign);
  return Move();
}

bool osl::search4::AlphaBeta4::
isReasonableMove(Move /*move*/, int /*pawn_sacrifice*/)
{
  return true;
}

void osl::search4::AlphaBeta4::
setRootIgnoreMoves(const MoveVector * /*rim*/, bool)
{
}
void osl::search4::AlphaBeta4::
setHistory(const MoveStack& /*h*/)
{
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
