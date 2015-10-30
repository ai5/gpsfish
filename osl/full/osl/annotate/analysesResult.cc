/* analysesResult.cc
 */
#include "osl/annotate/analysesResult.h"
#include "osl/container.h"
#include <iostream>

bool osl::annotate::operator==(const AnalysesResult& l, const AnalysesResult& r)
{
  return l.repetition == r.repetition
    && l.checkmate == r.checkmate
    && l.checkmate_win == r.checkmate_win
    && l.threatmate == r.threatmate
    && l.escape_from_check == r.escape_from_check
    && l.threatmate_probability == r.threatmate_probability
    && l.threatmate_node_count == r.threatmate_node_count
    && l.checkmate_for_capture == r.checkmate_for_capture
    && l.checkmate_for_escape == r.checkmate_for_escape
    && l.threatmate_if_more_pieces == r.threatmate_if_more_pieces;
}

std::ostream& osl::annotate::operator<<(std::ostream& os, Trivalent t)
{
  static const CArray<const char*,3> str = {{
      "False", "Unknown", "True",
    }};
  return os << str[t+1];
}
#define out(os, shared, x) os << #x << " " << shared.x << "  "
template <class T> void outt(std::ostream& os, const T& a, const char *str)
{
    if (a != T())
	os << str << " " << a << "  ";
}
#define outif(os, shared, x) outt(os, shared.x, #x)
std::ostream& osl::annotate::operator<<(std::ostream& os, const AnalysesResult& shared)
{
  if (! shared.repetition.empty()) {
    os << "repetition ";
    for (int p: shared.repetition)
      os << p << " ";
    os << " ";
  }
  if (shared.checkmate != False)
    out(os, shared, checkmate);
  if (shared.threatmate != False)
    out(os, shared, threatmate);
  if (shared.escape_from_check != False)
    out(os, shared, escape_from_check);
  outif(os, shared, checkmate_move);
  outif(os, shared, threatmate_move);
  outif(os, shared, threatmate_probability);
  outif(os, shared, threatmate_node_count);
  outif(os, shared, checkmate_for_capture.safe_count);
  outif(os, shared, checkmate_for_capture.checkmate_count);
  outif(os, shared, checkmate_for_capture.see_plus_checkmate_count);
  outif(os, shared, checkmate_for_escape.safe_count);
  outif(os, shared, checkmate_for_escape.checkmate_count);
  if (! shared.threatmate_if_more_pieces.hand_ptype.empty()) 
  {
    os << "hand ";
    for (Ptype ptype: shared.threatmate_if_more_pieces.hand_ptype)
      os << ptype << " ";
    os << " ";
  }
  if (! shared.threatmate_if_more_pieces.board_ptype.empty()) 
  {
    os << "board ";
    for (Piece piece: shared.threatmate_if_more_pieces.board_ptype)
      os << piece << " ";
    os << " ";
  }
  return os;
}
#undef out

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
