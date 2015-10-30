#ifndef BOARD_H
#define BOARD_H

#include "osl/numEffectState.h"
#include "osl/record/kanjiPrint.h"
#include "osl/record/searchInfo.h"

  namespace gpsshell
  {
     class Board
     {
       osl::NumEffectState initial;
       std::vector<osl::Move> moves;
       std::vector<int> time;
       std::vector<osl::record::SearchInfo> search_info;
       std::vector<std::string> comments;
       /** N-th move, not index of the moves array */
       size_t current_move;
     public:
       Board();
       ~Board();
       bool next(size_t n=1);
       bool prev(size_t n=1);
       void first();
       void last();
       void move(const osl::Move move);
       const osl::Move getCurrentMove() const
       {
         if (moves.empty() || isInitialState())
         {       
           osl::Move move;
           return move; // invalid value
         }
         else
           return moves.at(current_move-1);
       }
       std::vector<osl::Move> getMovesToCurrent() const
       {
         std::vector<osl::Move>::const_iterator it = moves.begin();
         it += std::min(moves.size(), current_move);

         std::vector<osl::Move>::const_iterator begin = moves.begin();
         std::vector<osl::Move> ret(begin, it); // copy
         return ret;
       } 
       void showState() const;
       void showHistory() const;
       void showUsiHistory() const;
       void showEval(const std::string& name) const;
       void setMoves(const osl::SimpleState& state,
		     const std::vector<osl::Move>& _moves, const size_t _current_move = 0)
       {
	 initial = osl::NumEffectState(state);
         moves = _moves;
         current_move = _current_move;
	 time.clear();
	 search_info.clear();
       }
       void setTime(const std::vector<int>& t)
       {
	 time = t;
       }
       void setComments(const std::vector<std::string>& c)
       {
	 comments = c;
       }
       void setSearchInfo(const std::vector<osl::record::SearchInfo>& info) 
       {
	 search_info = info;
       }
       void shrink();
       bool isEndOfMoves() const
       {
         return moves.size() == current_move;
       }
       bool isInitialState() const
       {
         return current_move == 0;
       }
       void setBlackColor(const osl::record::Color& color)
       {
         black_color = color;
       }
       void setWhiteColor(const osl::record::Color& color)
       {
         white_color = color;
       }
       void setLastMoveColor(const osl::record::Color& color)
       {
         last_color = color;
       }
       osl::NumEffectState getState() const;
       osl::NumEffectState getInitialState() const { return initial; }
     private:
       osl::record::Color black_color;
       osl::record::Color white_color;
       osl::record::Color last_color; 
     };

  } // namespace gpsshell

#endif /* BOARD_H */

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
