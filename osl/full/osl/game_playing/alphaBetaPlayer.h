/* alphaBetaPlayer.h
 */
#ifndef GAMEPLAYING_ALPHABETAPLAYER_H
#define GAMEPLAYING_ALPHABETAPLAYER_H

#include "osl/game_playing/searchPlayer.h"

namespace osl
{
  namespace search
  {
    struct AlphaBeta2SharedRoot;
  }
  namespace game_playing
  {
    class AlphaBeta2ProgressEvalPlayer : public SearchPlayer
    {
    public:
      AlphaBeta2ProgressEvalPlayer();
      ~AlphaBeta2ProgressEvalPlayer();
      ComputerPlayer* clone() const;

      const MoveWithComment searchWithSecondsForThisMove(const GameState&, const search::TimeAssigned&);
      bool isReasonableMove(const GameState&, Move move, int pawn_sacrifice);
    };

    class AlphaBeta2OpenMidEndingEvalPlayer : public SearchPlayer
    {
    public:
      AlphaBeta2OpenMidEndingEvalPlayer();
      ~AlphaBeta2OpenMidEndingEvalPlayer();
      ComputerPlayer* clone() const;

      const MoveWithComment searchWithSecondsForThisMove(const GameState&, const search::TimeAssigned&);
      bool isReasonableMove(const GameState&, Move move, int pawn_sacrifice);

      const MoveWithComment analyzeWithSeconds(const GameState& gs, const search::TimeAssigned& org,
					       search::AlphaBeta2SharedRoot& out);
    };

    class AlphaBeta3OpenMidEndingEvalPlayer : public SearchPlayer
    {
    public:
      AlphaBeta3OpenMidEndingEvalPlayer();
      ~AlphaBeta3OpenMidEndingEvalPlayer();
      ComputerPlayer* clone() const;

      const MoveWithComment searchWithSecondsForThisMove(const GameState&, const search::TimeAssigned&);
      bool isReasonableMove(const GameState&, Move move, int pawn_sacrifice);
    };

    class AlphaBeta4Player : public SearchPlayer
    {
    public:
      AlphaBeta4Player();
      ~AlphaBeta4Player();
      ComputerPlayer* clone() const;

      const MoveWithComment searchWithSecondsForThisMove(const GameState&, const search::TimeAssigned&);
      bool isReasonableMove(const GameState&, Move move, int pawn_sacrifice);
    };


    class UsiProxyPlayer : public SearchPlayer
    {
    public:
      UsiProxyPlayer();
      ~UsiProxyPlayer();
      ComputerPlayer* clone() const;

      const MoveWithComment searchWithSecondsForThisMove(const GameState&, const search::TimeAssigned&);
      bool isReasonableMove(const GameState&, Move move, int pawn_sacrifice);
    };
  } // namespace game_playing
} // namespace osl

#endif /* GAMEPLAYING_ALPHABETAPLAYER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
