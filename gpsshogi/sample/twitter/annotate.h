/* annotate.h
 */
#ifndef GPSSHOGI_ANNOTATE_H
#define GPSSHOGI_ANNOTATE_H

// This file is intentionally written in EUC-JP.

#include "osl/annotate/facade.h"
#include "osl/record/ki2.h"
#include "osl/record/kanjiCode.h"
#include "osl/usi.h"
#include <sstream>

void explain(const osl::NumEffectState& state, const osl::annotate::AnalysesResult& shared,
	     std::string& japanese_euc, std::string& english)
{
  using namespace osl;
  using namespace osl::record;
  using namespace osl::annotate;
  // repetition
  if (! shared.repetition.empty()) 
  {
    japanese_euc += "同一局面 (";
    english += "Repetition of position (";
    for (size_t i=0; i<shared.repetition.size(); ++i) {
      std::ostringstream ss;
      ss << (i?",":"") << shared.repetition[i];
      japanese_euc += ss.str();
      english += ss.str();
    }
    japanese_euc += "). ";
    english += "). ";
  }
  // checkmate
  if (shared.checkmate == True) 
  {
    japanese_euc += "詰みで";
    japanese_euc += (state.turn() == BLACK) ? K_WHITE : K_BLACK;
    japanese_euc += "の勝ち． ";
    english += (state.turn() == BLACK) ? "Gote" : "Sente";
    english += " can win by checkmate.  ";
  }
  // checkmate win
  if (shared.checkmate_win == True)
  {
    japanese_euc += (state.turn() == BLACK) ? K_WHITE : K_BLACK;
    japanese_euc += K_KING;
    japanese_euc += "は"+ki2::show(shared.checkmate_move, state)+"(以下)詰み． ";
    english += (state.turn() == BLACK) ? "Sente" : "Gote";
    english += " wins by checkmate (" + psn::showXP(shared.checkmate_move) + ").  ";
  }
  // threatmate
  if (shared.threatmate != True
      && shared.threatmate_probability >= 0.6
      && shared.threatmate_node_count >= 4000)
  {
    using namespace osl::record;
    japanese_euc += (state.turn() == BLACK) ? K_BLACK : K_WHITE;
    japanese_euc += K_KING "は危険になってきたが";
    english += (state.turn() == BLACK) ? "Sente" : "Gote";
    english += "'s king seems to be in danger, ";
    if (shared.threatmate == False)
    {
      japanese_euc += "詰めろではない． ";
      english += "but is not in threatmate.  ";
    }
    else
    {
      assert(shared.threatmate == Unknown);
      japanese_euc += "詰めろではなさそう． ";
      english += "but it is unclear whether he is in threatmate.  ";
    }
  }
  if (shared.threatmate == True)
  {
    if (shared.escape_from_check == True
	&& shared.checkmate_win != True)
    {
      japanese_euc += "王手を回避して，";
    }
    japanese_euc += (state.turn() == BLACK) ? K_BLACK : K_WHITE;
    japanese_euc += K_KING "は";
    japanese_euc += ki2::show(shared.threatmate_move, state) + "(以下)の詰めろ． ";
    english += (state.turn() == BLACK) ? "Sente" : "Gote";
    english += " is in threatmate (" + psn::showXP(shared.threatmate_move) + ").  ";
  }
  // checkmate for capture
  if (shared.checkmate_for_capture.checkmate_count)
  {
    if (shared.checkmate_for_capture.safe_count == 0) 
    {
      japanese_euc += "取ると詰む． ";
      english += ((state.turn() == BLACK) ? "Sente" : "Gote");
      english += " cannot capture this piece.  ";
    }
    else if (shared.checkmate_for_capture.see_plus_checkmate_count)
    {
      japanese_euc = "取り方によっては詰がある． ";
      english = ((state.turn() == BLACK) ? "Sente" : "Gote");
      english += " is in danger.  ";
    }
  }
  // checkmate for escape
  if (shared.checkmate_for_escape.checkmate_count)
  {
    if (shared.checkmate_for_escape.safe_count == 0) 
    {
      japanese_euc += "詰み． ";	// CheckmateAnalyzer を使っていればここにはこないはず
      english += "checkmate.  ";
    }
    else
    {
      japanese_euc += "応手によっては詰がある． ";
      english += ((state.turn() == BLACK) ? "Sente" : "Gote");
      english += " is in danger.";
    }
  }
  // threatmate_if_more_pieces
  if (! shared.threatmate_if_more_pieces.hand_ptype.empty())
  {
    japanese_euc += "持駒から";
    for (size_t i=0; i<shared.threatmate_if_more_pieces.hand_ptype.size(); ++i)
      japanese_euc += ki2::show(shared.threatmate_if_more_pieces.hand_ptype[i]);
    if (shared.threatmate_if_more_pieces.hand_ptype.size() > 1)
      japanese_euc += "のいずれか";
    japanese_euc += "を渡すと";
    japanese_euc += (state.turn() == BLACK) ? K_BLACK : K_WHITE;
    japanese_euc += K_KING "に詰が発生． ";
    english += (state.turn() == BLACK) ? "Sente" : "Gote";
    english += " would be in threatmate if ";
    english += (state.turn() == BLACK) ? "Gote" : "Sente";
    english += " had an extra piece of ";
    if (shared.threatmate_if_more_pieces.hand_ptype.size() > 1)
      english += "any of ";
    for (size_t i=0; i<shared.threatmate_if_more_pieces.hand_ptype.size(); ++i)
    {
      if (i && i+1 == shared.threatmate_if_more_pieces.hand_ptype.size())
	english += " and ";
      else if (i)
	english += ", ";
      english += Ptype_Table.getName(shared.threatmate_if_more_pieces.hand_ptype[i]);
    }
    english += ".  ";
  }
  if (! shared.threatmate_if_more_pieces.board_ptype.empty())
  {
    if (! shared.threatmate_if_more_pieces.hand_ptype.empty())
      japanese_euc += "あるいは";
    else
      japanese_euc += "仮に";
    japanese_euc += "盤上の";
    for (size_t i=0; i<shared.threatmate_if_more_pieces.board_ptype.size(); ++i) {
      const Piece p = shared.threatmate_if_more_pieces.board_ptype[i];
      japanese_euc += ki2::show(unpromote(p.ptype()));
      japanese_euc += "(" + ki2::show(p.square()) + ")";
    }
    if (shared.threatmate_if_more_pieces.board_ptype.size() > 1)
      japanese_euc += "のいずれか";
    japanese_euc += "が";
    japanese_euc += (state.turn() == BLACK) ? K_WHITE : K_BLACK;
    japanese_euc += "の持駒であれば";
    japanese_euc += (state.turn() == BLACK) ? K_BLACK : K_WHITE;
    japanese_euc += K_KING "に詰がある． ";
    const std::string turn = (state.turn() == BLACK) ? "Sente" : "Gote";
    english += turn;
    english += " will be in threatmate if the opponent captures ";
    if (shared.threatmate_if_more_pieces.board_ptype.size() > 1)
      english += "any of ";
    for (size_t i=0; i<shared.threatmate_if_more_pieces.board_ptype.size(); ++i)
    {
      if (i && i+1 == shared.threatmate_if_more_pieces.board_ptype.size())
	english += " and ";
      else if (i)
	english += ", ";
      const Piece p = shared.threatmate_if_more_pieces.board_ptype[i];
      english += Ptype_Table.getName(unpromote(p.ptype()));
      english += "(" + psn::show(p.square()) + ")";
    }
    english += ".  ";
  }
  if (! shared.vision.pv.empty())
  {
    const auto& pv = shared.vision.pv;
    japanese_euc += "次の狙いはたぶん";
    english += "Threat to ";
    english += ((state.turn() == BLACK) ? "Sente: " : "Gote: ");
    NumEffectState s = state;
    s.changeTurn();
    for (size_t i=0; i<std::min(pv.size(), (size_t)3); ++i) {
      if (i == 0)
	japanese_euc += ki2::show(pv[i], s);
      else
	japanese_euc += ki2::show(pv[i], s, pv[i-1]);
      english += psn::showXP(pv[i])+' ';
      s.makeMove(pv[i]);
    }
    japanese_euc += "("+std::to_string(shared.vision.cur_eval);
    japanese_euc += " → ";
    japanese_euc += std::to_string(shared.vision.eval)+"). "; 
    english += "("+std::to_string(shared.vision.cur_eval); 
    english += " -> ";
    english += std::to_string(shared.vision.eval)+").  "; 
  }
}

const std::string forced_move_in_japanese = "初手は絶対手";

#endif /* GPSSHOGI_ANNOTATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
