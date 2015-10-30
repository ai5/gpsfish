/* gameState.h
 */
#ifndef OSL_GAMESTATE_H
#define OSL_GAMESTATE_H

#include "osl/numEffectState.h"
#include <vector>
namespace osl
{
  class Sennichite;
  class RepetitionCounter;
  namespace container
  {
    class MoveStack;
  }
  namespace hash
  {
    class HashKeyStack;
  }
  namespace game_playing
  {
    /**
     * State と千日手状態などを一元管理
     */
    class GameState
    {
    private:
      struct State;
      std::unique_ptr<State> stack;
      explicit GameState(const State& src);
    public:
      explicit GameState(const SimpleState& initial_state);
      ~GameState();

      enum MoveType { VALID, PAWN_DROP_FOUL, UNSAFE_KING, OTHER_INVALID };
      MoveType isIllegal(Move m) const;
      const Sennichite pushMove(Move m, int eval=0);
      const Move popMove();
      bool canPopMove() const;

      const NumEffectState& state() const;
      const RepetitionCounter& counter() const;
      const container::MoveStack& moveHistory() const;
      const hash::HashKeyStack& hashHistory() const;
      int moves() const;
      int chessMoves() const { return moves() / 2 + 1; }
      const SimpleState& initialState() const;

      /**
       * GameState のコピーを作る．
       * 現在の局面を初期局面として扱うため，
       * pushMoveしない限りpopMoveはできない
       */
      const std::shared_ptr<GameState> clone() const;

      const std::vector<int>& evalStack() const;
      void generateNotLosingMoves(MoveVector& normal_or_win_or_draw, 
				  MoveVector& loss) const;
      void generateMoves(MoveVector& normal_moves, 
			 MoveVector& win, 
			 MoveVector& draw, 
			 MoveVector& loss) const;
      bool rejectByStack(Move move) const;
    };
  } // namespace game_playing
} // namespace osl

#endif /* OSL_GAMESTATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
