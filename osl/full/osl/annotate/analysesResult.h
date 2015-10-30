/* analysesResult.h
 */
#ifndef OSL_ANNOTATE_ANALYSESRESULT_H
#define OSL_ANNOTATE_ANALYSESRESULT_H

#include "osl/basic_type.h"
#include <iosfwd>
#include <vector>

namespace osl
{
  namespace annotate
  {
    enum Trivalent { Unknown=0, True=1, False=-1 };
    struct AnalysesResult
    {
      struct CheckmateForCapture
      {
	int safe_count, checkmate_count, see_plus_checkmate_count;
	CheckmateForCapture() 
	  : safe_count(0), checkmate_count(0), see_plus_checkmate_count(0)
	{
	}
	bool operator==(const CheckmateForCapture& r) const
	{
	  return safe_count == r.safe_count
	    && checkmate_count == r.checkmate_count
	    && see_plus_checkmate_count == r.see_plus_checkmate_count;
	}
      };
      struct CheckmateForEscape
      {
	int safe_count, checkmate_count;
	CheckmateForEscape() : safe_count(0), checkmate_count(0)
	{
	}
	bool operator==(const CheckmateForEscape& r) const
	{
	  return safe_count == r.safe_count
	    && checkmate_count == r.checkmate_count;
	}
      };
      struct ThreatmateIfMorePieces
      {
	std::vector<Ptype> hand_ptype;
	std::vector<Piece> board_ptype;
	bool operator==(const ThreatmateIfMorePieces& r) const
	{
	  return hand_ptype == r.hand_ptype
	    && board_ptype == r.board_ptype;
	}	
      };
      struct Vision
      {
	std::vector<Move> pv;
	int eval, cur_eval;
      };

      std::vector<int> repetition;
      Trivalent checkmate, checkmate_win, threatmate, escape_from_check;
      Move checkmate_move, threatmate_move;
      double threatmate_probability;
      size_t threatmate_node_count;
      CheckmateForCapture checkmate_for_capture;
      CheckmateForEscape checkmate_for_escape;
      ThreatmateIfMorePieces threatmate_if_more_pieces;
      Vision vision;

      AnalysesResult()
	: checkmate(Unknown), checkmate_win(Unknown), threatmate(Unknown), 
	  escape_from_check(Unknown),
	  threatmate_probability(0), threatmate_node_count(0)
      {
      }
    };
    bool operator==(const AnalysesResult& l, const AnalysesResult& r);
    std::ostream& operator<<(std::ostream&, Trivalent);
    std::ostream& operator<<(std::ostream&, const AnalysesResult&);
  }
}

#endif /* OSL_ANNOTATE_ANALYSESRESULT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
