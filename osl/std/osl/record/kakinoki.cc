/* kakinoki.cc
 */
#include "osl/record/kakinoki.h"
#include "osl/record/kanjiMove.h"
#include "osl/record/kanjiCode.h"
#include "osl/misc/sjis2euc.h"
#include "osl/misc/eucToLang.h"
#include "osl/simpleState.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <sstream>

void osl::kakinoki::
KakinokiFile::parseLine(SimpleState& state, Record& record, 
			std::string s, CArray<bool,9>& board_parsed)
{
  static const KanjiMove& Kanji_Move = KanjiMove::instance();
  static const CArray<std::string,11> n_str = {{
      "", K_K1, K_K2, K_K3, K_K4, K_K5, K_K6, K_K7, K_K8, K_K9, K_K10
    }};
  // header
  if (s[0] == '|') {
    if (s.size() < 1+3*9+1+2)
      throw KakinokiIOError("board too short in kakinokiParseLine "+s);
    const int y = std::find(n_str.begin(), n_str.end(), s.substr(s.size()-2))
      - n_str.begin();
    if (! (1 <= y && y <= 9))
      throw KakinokiIOError("unknown y in kakinokiParseLine "+s);
    board_parsed[y-1] = true;
    for (unsigned int x=9,i=1;i<s.length()&&x>0;i+=3,x--) {
      std::pair<Player,Ptype> pp=kakinoki::strToPiece(s.substr(i,3));
      if (! isPiece(pp.second))
	continue;
      state.setPiece(pp.first, Square(x,y), pp.second);
    }
  }
  if (s.find(K_TESUU "--") == 0) {
    // moves start
    if (std::find(board_parsed.begin(), board_parsed.end(), true)
	== board_parsed.end()) {
      state.init(HIRATE);
      board_parsed.fill(true);
    }
    if (*std::min_element(board_parsed.begin(), board_parsed.end()) == false)
      throw KakinokiIOError("incomplete position description in kakinokiParseLine");
    state.initPawnMask();
    record.record.initial_state = NumEffectState(state);
    return;
  }
  if (s.size() > 6)
  {
    if (s.find(K_BLACK K_COLON) == 0) {
      record.player[BLACK] = s.substr(6);
      return;
    }
    if (s.find(K_WHITE K_COLON) == 0) {
      record.player[WHITE] = s.substr(6);
      return;
    }
    if (s.find(K_KISEN K_COLON) == 0)
    {
      record.tournament_name = s.substr(6);
      return;
    }
    if (s.find(K_KAISHI K_NICHIJI K_COLON) == 0) {
      boost::gregorian::date date =
	boost::gregorian::from_string(s.substr(strlen(K_KAISHI K_NICHIJI K_COLON),10));
      record.start_date = date;
      return;
    }
    if (s.find(K_MOCHIGOMA K_COLON) != s.npos
	&& s.find(K_NASHI) == s.npos) {
      std::string piece_str = s.substr(s.find(K_COLON)+2);
      boost::algorithm::replace_all(piece_str, K_SPACE, " ");
      std::vector<std::string> pieces;
      boost::algorithm::split(pieces, piece_str,
			      boost::algorithm::is_any_of(" "));
      Player player;
      if (s.find(K_BLACK) == 0) player = BLACK;
      else if (s.find(K_WHITE) == 0) player = WHITE;
      else throw KakinokiIOError("error in stand "+ s);

      for (const auto& e: pieces) {
	if (e.empty()) continue;
	if (e.size() < 2) throw KakinokiIOError("error in stand "+ e);
	const Ptype ptype = Kanji_Move.toPtype(e.substr(0,2));
	int n = 1;
	if (e.size() >= 4)
	  n = std::find(n_str.begin(),n_str.end(),e.substr(2,2))
	    - n_str.begin();
	if (e.size() >= 6)
	  n = n * ((e.substr(2,2) == K_K10) ? 1 : 10)
	    + (std::find(n_str.begin(),n_str.end(),e.substr(4,2))
	       - n_str.begin());
	for (int i=0; i<n; ++i)
	  state.setPiece(player, Square::STAND(), ptype);
      }
    }
  }

  // moves
  if (s[0] == '*') 
  {
    if (record.moves().empty())
      Record::addWithNewLine(record.initial_comment, s.substr(1));
    else
      record.setMoveComment(s.substr(1));
    return;
  }
  if (s[0] != ' ') 
  {
    if (record.moves().empty())
      Record::addWithNewLine(record.initial_comment, s);
    return;			// otherwise ignore
  }
  if (s.find(K_TORYO) != s.npos)
  {
    record.result = ((state.turn() == BLACK)
		     ? Record::WhiteWin : Record::BlackWin);
    return;
  }

  {
    // replace '(' and ')' with white space if exists
    size_t p = s.find('(');
    if (p != s.npos)
      s.replace(p, 1, 1, ' ');
    p = s.find(')');
    if (p != s.npos)
      s.replace(p, 1, 1, ' ');
  }      
  Move last_move = record.lastMove();
  const Move m = kakinoki::strToMove(s, state, last_move);
  if (m.isNormal()) {
    if (! state.isValidMove(m)) {
      std::ostringstream ss;
      ss << state << misc::eucToLang(s) << "\n" << m;
      std::cerr << ss.str();
      throw KakinokiIOError(ss.str());
    }	
    record.record.moves.push_back(m);
    NumEffectState copy(state);
    copy.makeMove(m);
    state = copy;
  }
}

std::pair<osl::Player,osl::Ptype> osl::
kakinoki::strToPiece(const std::string& s)
{
  static const KanjiMove& Kanji_Move = KanjiMove::instance();
  if (s.size() != 3 || (s[0] != 'v' && s[0] != ' '))
    throw KakinokiIOError("error in strToPiece " + s);
  const Player pl = s[0] == ' ' ? BLACK : WHITE;
  const Ptype ptype = Kanji_Move.toPtype(s.substr(1,2));
  return std::make_pair(pl, ptype);
}

osl::Move osl::
kakinoki::strToMove(const std::string& s, const SimpleState& state, Move last_move)
{
  static const KanjiMove& Kanji_Move = KanjiMove::instance();
  std::istringstream is(s);
  int move_number, from_number;
  std::string move_string;
  is >> move_number >> move_string;

  Square to, from;
  if (move_string.substr(0,2) == K_ONAZI)
    to = last_move.to();
  else
    to = Kanji_Move.toSquare(move_string.substr(0,4));
  if (to == Square())		// resign?
    return Move();
  
  Ptype ptype;
  size_t cur = 4;
  if (move_string.substr(cur,2) == K_NARU) // PLANCE, PKIGHT, PSILVER
  {
    assert(move_string.size() >= cur+4);
    ptype = Kanji_Move.toPtype(move_string.substr(cur,4));
    cur += 4;
  }
  else
  {
    ptype = Kanji_Move.toPtype(move_string.substr(cur,2));
    cur += 2;
  }
  if (move_string.size() >= cur+2 && move_string.substr(cur,2)
      == K_UTSU)
    from = Square();
  else 
  {
    if (! (is >> from_number))
      throw KakinokiIOError("error in move from");
    from = Square(from_number / 10, from_number % 10);
  }
  
  bool is_promote = false;
  if (move_string.size() >= cur+2 && move_string.substr(cur,2) == K_NARU)
    is_promote = true;

  if (from.isPieceStand())
    return Move(to, ptype, state.turn());
  Ptype captured = state.pieceOnBoard(to).ptype();
  return Move(from, to, is_promote ? promote(ptype) : ptype,
	      captured, is_promote, state.turn());
}

osl::kakinoki::
KakinokiFile::KakinokiFile(const std::string& filename)
{
  std::ifstream is(filename);
  if (! is)
  {
    const std::string msg = "KakinokiFile::KakinokiFile file cannot read ";
    std::cerr << msg << filename << "\n";
    throw KakinokiIOError(msg + filename);
  }
  SimpleState work;
  work.init();
  std::string line;
  CArray<bool, 9> board_parsed = {{ false }};
  while (std::getline(is, line)) 
  {
    // quick hack for \r
    if ((! line.empty())
	&& (line[line.size()-1] == 13))
      line.erase(line.size()-1);
    if (line.length()==0) 
      continue;
    // to euc
    line = misc::sjis2euc(line);
    // skip variations
    if (line.find(K_HENKA) == 0)
      break;
    if (! line.empty() && line[0] == '#' 
	&& line.find("separator") != line.npos)
      break;			// tanase shogi
    parseLine(work, record, line, board_parsed);
  }
  assert(work.isConsistent());
}

osl::kakinoki::
KakinokiFile::~KakinokiFile()
{
}

bool osl::kakinoki::
KakinokiFile::isKakinokiFile(const std::string& filename)
{
  std::ifstream is(filename.c_str());
  std::string line;
  if (! is || ! getline(is, line)) 
    return false;
  line = misc::sjis2euc(line);
  return line.find("Kifu for Windows") != line.npos
    || line.find("KIFU") != line.npos
    || line.find(K_SENKEI) == 0
    || line.find(K_TEAIWARI) == 0
    || (line.find("#") == 0 && line.find(K_KIFU) != line.npos);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
