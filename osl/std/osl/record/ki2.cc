#include "osl/record/ki2.h"
#include "osl/record/kanjiCode.h"
#include "osl/record/kanjiPrint.h"
#include "osl/record/ki2IOError.h"
#include "osl/misc/sjis2euc.h"
#include "osl/bits/ptypeTable.h"
#include <boost/algorithm/string/trim.hpp>
#include <fstream>
#include <iostream>

osl::ki2::Ki2File::ParseResult
osl::ki2::Ki2File::parseLine(NumEffectState& state, Record& record, KanjiMove& kmove,
			     std::string line)
{
  boost::algorithm::trim(line);

  if (line.empty() || line.at(0) == '*')
    return OK;
  else if (line.size() > 10 && line.substr(0,10) == (K_KAISHI K_NICHIJI K_COLON))
  {
    std::string date_str(line.substr(10)); // this may or may not include HH:MM
    /** eliminates HH:MM part */
    const static std::string spaces[] = {" ", K_SPACE};
    for (const auto& space: spaces) {
      const std::string::size_type pos_space = date_str.find(space);
      if (pos_space != std::string::npos)
	date_str = date_str.substr(0, pos_space);
    }
    record.setDate(date_str);
    // an invalid date_str results in an invalid boost::gregorian::date value
    // you can check it date.is_special()
    return OK;
  }
  else if (line.size() > 6 && line.substr(0,6) == (K_BLACK K_COLON))
  {
    const std::string player_name(line.substr(6));
    record.player[BLACK] = player_name;
    return OK;
  }
  else if (line.size() > 6 && line.substr(0,6) == (K_WHITE K_COLON))
  {
    const std::string player_name(line.substr(6));
    record.player[WHITE] = player_name;
    return OK;
  }
  else if (line.size() > 6 && line.substr(0,6) == (K_KISEN K_COLON))
  {
    record.tournament_name = line.substr(6);
    return OK;
  }
  else if (line.size() > 8 && line.substr(0,8) == (K_TEAIWARI K_COLON))
    return Komaochi;
  else if (line.substr(0,2) != K_BLACK_SIGN && line.substr(0,2) != K_WHITE_SIGN)
    return OK;
    
  std::string move_str;
  for (size_t i = 0; ; ) {
    if (i < line.size() && 
	(line.at(i) == ' ' || line.at(i) == '\t'))
    {
      ++i;
      continue;
    }

    if ( (line.substr(i,2) == K_BLACK_SIGN || 
	  line.substr(i,2) == K_WHITE_SIGN ||
	  i+1 >= line.size())
	 && !move_str.empty())
    {
      // apply move_str
      Move last_move;
      if (record.moves().size() > 0)
	last_move = record.moves().back();
      const Move move = kmove.strToMove(move_str, state, last_move);
      if (!move.isValid()) {
	if (move_str.find(K_RESIGN) != move_str.npos)
	  return OK;
	return Illegal;
      }
      record.record.moves.push_back(move);
      state.makeMove(move);
      move_str.clear();
    }
    if (i+1 >= line.size())
      return OK;
    move_str.append(line.substr(i,2));
    i += 2;
  } // for
}

osl::ki2::
Ki2File::Ki2File(const std::string& filename, bool v)
  : verbose(v)
{
  std::ifstream is(filename);
  if (! is)
  {
    const std::string msg = "Ki2File::Ki2File file cannot read ";
    std::cerr << msg << filename << "\n";
    throw Ki2IOError(msg + filename);
  }
  KanjiMove kmove;
  kmove.setVerbose(verbose);

  NumEffectState work;
  record.record.initial_state = work;
  std::string line;
  while (std::getline(is, line)) 
  {
    line = misc::sjis2euc(line);
    const ParseResult result = parseLine(work, record, kmove, line);
    switch (result)
    {
      case OK:
        continue;
      case Komaochi:
      {
        const std::string msg = "ERROR: Komaochi (handicapped game) records are not available: ";
        std::cerr << msg << "\n";
        throw Ki2IOError(msg);
      }
      case Illegal:
      {
        const std::string msg = "ERROR: An illegal move found in a record.";
        throw Ki2IOError(msg);
      }
      default:
        assert(false);
    }
  }
}

const std::string osl::ki2::show(Square position)
{
  using namespace record;
  if (position.isPieceStand())
    return "";
  const int x = position.x(), y = position.y();
  return StandardCharacters::suji[x] + StandardCharacters::dan[y];
}

const std::string osl::ki2::show(Ptype ptype)
{
  using namespace record;
  switch (ptype) 
  {
  case PSILVER: case PKNIGHT: case PLANCE:
    return K_NARU + StandardCharacters().kanji(unpromote(ptype));
  default:
    ;
  }
  return StandardCharacters().kanji(ptype);
}

const std::string osl::ki2::showPromote(bool promote)
{
  return promote ? K_NARU : K_FUNARI;
}

const std::string osl::ki2::show(Square cur, Square prev)
{
  if (cur == prev)
    return K_ONAZI;
  return show(cur);
}

const std::string osl::ki2::show(Move m, const NumEffectState& state,
					 Move prev)
{
  std::string ret = (m.player() == BLACK) ? K_BLACK_SIGN : K_WHITE_SIGN;
  if (m.isPass()) {
    ret += K_PASS;
    return ret;
  }
  const Square from = m.from(), to = m.to();
  const Ptype ptype = m.oldPtype();
  const Player player = m.player();
  mask_t pieces = state.allEffectAt(player, ptype, to);
  const mask_t promoted = state.promotedPieces().getMask(Ptype_Table.getIndex(ptype));
  if (isPromoted(ptype))
    pieces &= promoted;
  else
    pieces &= ~promoted;
  if (from.isPieceStand()) {
    ret += show(to) + show(ptype);
    int has_effect = 0;
    while (pieces.any()) {
      const Piece p = state.pieceOf(pieces.takeOneBit());
      if (p.ptype() == ptype)
	++has_effect;
    }
    if (has_effect)
      ret += K_UTSU;
    return ret;
  }
  ret += prev.isNormal() && (to == prev.to())
    ? K_ONAZI : show(to);
  ret += show(m.oldPtype());
  const int count = pieces.countBit();
  if (count >= 2) {
    CArray<int,3> x_count = {{ 0 }}, y_count = {{ 0 }};
    int my_x = 0, my_y = 0;
    while (pieces.any()) {
      const int n = pieces.takeOneBit() + Ptype_Table.getIndex(ptype)*32;
      const Piece p = state.pieceOf(n);
      if (p.ptype() != ptype)
	continue;
      int index_x = 1, index_y = 1;
      if (p.square().x() != to.x())
	index_x = ((p.square().x() - to.x()) * sign(player) > 0)
	  ? 2 : 0;
      if (p.square().y() != to.y())
	index_y = ((p.square().y() - to.y()) * sign(player) > 0)
	  ? 2 : 0;
      if (p.square() == from)
	my_x = index_x, my_y = index_y;
      x_count[index_x]++;
      y_count[index_y]++;
    }
    if (y_count[my_y] == 1) {
      if (from.y() == to.y()) 
	ret += K_YORU;
      else if ((to.y() - from.y())*sign(player) > 0)
	ret += K_HIKU;
      else
	ret += K_UE;
    }
    else if (x_count[my_x] == 1) {
      if (from.x() == to.x()) {
	if (isPromoted(ptype) && isMajor(ptype)) {
	  const Piece l = state.pieceAt
	    (Square(from.x() - sign(player), from.y()));
	  if (l.isOnBoardByOwner(player) && l.ptype() == ptype)
	    ret += K_HIDARI;
	  else
	    ret += K_MIGI;
	}
	else 
	  ret += K_SUGU;
      }
      else if ((to.x() - from.x())*sign(player) > 0)
	ret += K_MIGI;
      else
	ret += K_HIDARI;
    }
    else if (from.x() == to.x()) {
      if ((to.y() - from.y())*sign(player) > 0)
	ret += K_HIKU;
      else
	ret += K_SUGU;
    }
    else {
      if ((to.x() - from.x())*sign(player) > 0)
	ret += K_MIGI;
      else
	ret += K_HIDARI;
      if ((to.y() - from.y())*sign(player) > 0)
	ret += K_HIKU;
      else
	ret += K_UE;
    }
  }
  if (canPromote(m.oldPtype()))
    if (m.isPromotion()
	|| to.canPromote(player) || from.canPromote(player)) {
      ret += showPromote(m.isPromotion());
  }
  return ret;
}

const std::string osl::
ki2::show(const Move *first, const Move *last,
	  const char *threatmate_first, const char *threatmate_last,
	  const NumEffectState& initial, Move prev)
{
  if (first == last || first->isInvalid())
    return "";
  NumEffectState state(initial);
  std::string ret = show(*first, state, prev);
  if (threatmate_first != threatmate_last
      && *threatmate_first++)
    ret += "(" K_TSUMERO ")";
  for (; first+1 != last; ++first) {
    if (first->isInvalid())
      break;
    state.makeMove(*first);
    ret += show(*(first+1), state, *first);
    if (threatmate_first != threatmate_last
	&& *threatmate_first++)
      ret += "(" K_TSUMERO ")";
  }
  return ret;
}

const std::string osl::
ki2::show(const Move *first, const Move *last, const NumEffectState& initial, Move prev)
{
  std::vector<char> threatmate(last-first, false);
  return show(first, last, &*threatmate.begin(), &*threatmate.end(), initial, prev);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
