/* moveCache.h
 */
#ifndef _MOVECACHE_H
#define _MOVECACHE_H

#include "osl/record/kisen.h"
#include "osl/state/numEffectState.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/progress/ptypeProgress.h"
#include "osl/apply_move/applyMove.h"
#include <deque>
#include <iostream>

namespace osl
{
  class MoveCache
  {
    /** moves of all mathes */
    std::deque<osl::vector<Move> > moves;
    std::deque<osl::vector<Move> > preMoves;
  public:
    void registerMatch(const osl::vector<Move>& moves,
		       int progressMin, int progressMax)
    {
      preMoves.push_back(osl::vector<Move>());
      osl::vector<Move>& pre = preMoves[preMoves.size()-1];
      this->moves.push_back(moves);
      osl::vector<Move>& cur = this->moves[this->moves.size()-1];
      cur.clear();		// 空をいれて reserve したのと同じ

      NumEffectState state((PawnMaskState(HIRATE)));
      PtypeProgress progress(state);
      for (size_t j=0; j<moves.size(); j++)
      {
	const Player turn = state.turn();
	// 自分の手番で相手の王が利きがある => 直前の手が非合法手
	if (EffectUtil::isKingInCheck(alt(turn), state)
	    || (! state.isValidMove(moves[j]))) 
	{
	  std::cerr << "e"; // eState;
	  break;
	}
	if ((progress.progress() < progressMin)
	    && cur.empty())
	{
	  pre.push_back(moves[j]);
	  goto next;
	}
	if (progress.progress() >= progressMax)
	  break;
	cur.push_back(moves[j]);
      next:
	ApplyMoveOfTurn::doMove(state, moves[j]);
	progress.update(state, moves[j]);
      }
    }
    const osl::vector<Move>& getMoves(NumEffectState& state, size_t matchID) const
    {
      assert(matchID < moves.size());
      for (size_t i=0; i<preMoves[matchID].size(); ++i)
      {
	ApplyMoveOfTurn::doMove(state, preMoves[matchID][i]);
      }
      return moves[matchID];
    }
    /** getMoves が返す第一手目が実際の棋譜の何手目かを調べる */
    size_t getSquareID(size_t matchID) const
    {
      assert(matchID < moves.size());
      return preMoves[matchID].size();
    }
    void getAllMoves(KisenFile& kisenFile, size_t maxGames,
		     int progressMin, int progressMax)
    {
      for (size_t i=0;i<maxGames;i++)
      {
	if (i % 10000 == 0)
	  std::cerr << "\ncaching " << i << "-" << i+10000 << " th record\n";
	if ((i % 1000) == 0) 
	  std::cerr << '.';
	const osl::vector<Move>& moves=kisenFile.getMoves(i);
	registerMatch(moves, progressMin, progressMax);
      }
      size_t numInstances = 0;
      for (size_t i=0; i<moves.size(); ++i)
	numInstances += moves[i].size();
      std::cerr << numInstances << " moves registered\n";
    }
  };

} // namespace osl


#endif /* _MOVECACHE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
