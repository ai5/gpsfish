/* myshogi.cc
 */
#include "osl/record/myshogi.h"
#include "osl/record/ki2.h"
#include <sstream>

std::string osl::record::
myshogi::show(const NumEffectState& state,
	      Move last_move, const NumEffectState& prev, bool add_csa_move)
{
  std::ostringstream os;
  os << "\\begin{myshogi}[.7] \\banmen \n";
  os << "\\mochigoma{\\sente}";
  for (Ptype ptype: PieceStand::order)
    os << "{" << state.countPiecesOnStand(BLACK, ptype) << "}";
  os << "\n\\mochigoma{\\gote}";
  for (Ptype ptype: PieceStand::order)
    os << "{" << state.countPiecesOnStand(WHITE, ptype) << "}";
  os << "\n";
  if (last_move.isNormal()) {
    os << "\\lastmove[" << last_move.to().x() << last_move.to().y()
       << "]{" << ki2::show(last_move, prev);
    if (add_csa_move)
      os << '(' << csa::show(last_move) << ')';
    os << "}\n";
  }
  for (int i=0; i<Piece::SIZE; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard())
      os << show(p);
    if (i % 2)
      os << "\n";
  }
  os << "\\end{myshogi}\n";
  return os.str();
}

std::string osl::record::
myshogi::show(const NumEffectState& state) 
{
  static NumEffectState dummy;
  return show(state, Move(), dummy);
}

std::string osl::record::
myshogi::show(Ptype p)
{
  static CArray<std::string, PTYPE_SIZE> names = {{
      "", "", 
      "\\tokin", "\\narikyou", "\\narikei", "\\narigin", "\\uma", "\\ryu", 
      "\\ou", // todo: \\gyoku   
      "\\kin", "\\fu", "\\kyou", "\\kei", "\\gin", "\\kaku", "\\hi"
    }};
  return names[p];
}

std::string osl::record::
myshogi::show(Square p)
{
  std::string ret = "xx";
  ret[0] = '0'+p.x();
  ret[1] = '0'+p.y();
  return ret;
}

std::string osl::record::
myshogi::show(Piece p)
{
  if (! p.isOnBoard())
    return "";
  return std::string("\\koma{") + show(p.square()) + "}"
    + "{" + show(p.owner()) + "}{" + show(p.ptype()) + "}";
}

std::string osl::record::
myshogi::show(Player p)
{
  return p == BLACK ? "\\sente" : "\\gote";
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
