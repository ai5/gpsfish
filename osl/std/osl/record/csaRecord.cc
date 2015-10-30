#include "osl/record/csaRecord.h"
#include "osl/simpleState.h"
#include "osl/oslConfig.h"
#include "osl/usi.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <sstream>

/* ------------------------------------------------------------------------- */

osl::record::SearchInfo 
osl::record::CsaFile::makeInfo(const SimpleState& initial,
			       const std::string& line,
			       Move last_move)
{
  std::istringstream is(line);
  SearchInfo info;
  is >> info.value;

  NumEffectState state(initial);
  std::string s;
  while (is >> s)
  {
    if (s == csa::show(last_move) ||
        s == usi::show(last_move)) // effective only if s is the first move in a comment
      continue;
    last_move = Move::INVALID();
    try
    {
      Move move;
      if (s == "%PASS" || /* gekisashi */ s == "<PASS>" || s == "PASS") {
        move = Move::PASS(state.turn());
      } else if (s[0] == '+' || s[0] == '-') {
        move = csa::strToMove(s, state);
      } else {
        move = usi::strToMove(s, state);
      }
      if (move.isPass()
	  || (move.isNormal() && state.isValidMove(move,false)))
      {
	state.makeMove(move);
	if (! state.inCheck(alt(state.turn()))) {
	  info.moves.push_back(move);
	  continue;
	}
	// fall through
      }
    }
    catch(CsaIOError& e)
    {
      // fall through
    }
    catch(usi::ParseError &e)
    {
      // fall through
    }
    std::cerr << "drop illegal move in comment " << s << std::endl;
    break;
  }
  return info;
}

void osl::record::
CsaFile::parseLine(SimpleState& state, Record& record, std::string s,
		   bool parse_move_comment)
{
  switch(s.at(0)){
  case '\'': /* コメント行 */
    if (s.substr(1,2) == "* ")
    {
      record.setMoveComment(s.substr(3));
    }
    else if (s.substr(1,2) == "**" && parse_move_comment) 
    {
      record.setMoveInfo(makeInfo(state, s.substr(3), record.lastMove()));
    }
    return;
  case '$': /* コメント行 */
    if (s.find("$START_TIME:") == 0) {
      const std::string YYMMDD = s.substr(12,10);
#if 0
      std::vector<std::string> e;
      boost::algorithm::split(e, YYMMDD, boost::algorithm::is_any_of("/"));
      if (e.size() < 3) 
	throw CsaIOError("csa date fail "+YYMMDD);
      start_date = boost::gregorian::date(stoi(e[0]), stoi(e[1]), stoi(e[2]));
      assert(!start_date.is_special());
#endif
      record.setDate(YYMMDD);
      return;
    }
    Record::addWithNewLine(record.initial_comment, s.substr(1));
    return;
  case 'V': /* バージョン番号 */
    record.version = s.substr(1);
    return;
  case 'N': /* 対局者名 */
    switch(s.at(1)){
    case '+':
    case '-':
      record.player[csa::charToPlayer(s.at(1))] = s.substr(2);
      break;
    default:
      std::cerr << "Illegal csa line " << s << std::endl;
      throw CsaIOError("illegal csa line "+s);
    }
    break;
  case 'T':
  {
    record.setMoveTime(atoi(s.c_str()+1));
    return;
  }
  case '%':
    if (s.find("%TORYO") == 0 || s.find("%ILLEGAL_MOVE") == 0)
      record.result = ((state.turn() == BLACK) 
		       ? Record::WhiteWin : Record::BlackWin);
    else if (s.find("%SENNICHITE") == 0)
      record.result = Record::Sennnichite;
    else if (s.find("%KACHI") == 0)
      record.result = ((state.turn() == BLACK) 
		       ? Record::BlackWin : Record::WhiteWin);
    else if (s.find("%JISHOGI") == 0 || s.find("%HIKIWAKE") == 0)
      record.result = Record::JiShogi;
    else if (s.find("%+ILLEGAL_ACTION") == 0)
      record.result = Record::WhiteWin;
    else if (s.find("%-ILLEGAL_ACTION") == 0)
      record.result = Record::BlackWin;
    return;
  default:
    throw CsaIOError("unknown character in csaParseLine "+s);
  }
}

osl::record::
CsaFile::CsaFile(const std::string& filename)
{
  std::ifstream ifs(filename);
  if (! ifs) {
    const std::string msg = "CsaFile::CsaFile file cannot read ";
    std::cerr << msg << filename << "\n";
    throw CsaIOError(msg + filename);
  }
  read(ifs);
}

osl::record::
CsaFile::CsaFile(std::istream& is)
{
  read(is);
}

osl::record::
CsaFile::~CsaFile()
{
}

void osl::record::CsaFile::
read(std::istream& is)
{
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

    std::vector<std::string> elements;
    boost::algorithm::split(elements, line, boost::algorithm::is_any_of(","));
    for (auto& e: elements) {
      boost::algorithm::trim(e);
      boost::algorithm::trim_left(e);
      if (! CsaFileMinimal::parseLine(work, record.record, e, board_parsed))
	parseLine(work, record, e, !OslConfig::inUnitTest());
    }
  }
  if (*std::min_element(board_parsed.begin(), board_parsed.end()) == false)
    throw CsaIOError("incomplete position description in csaParseLine");
  assert(record.record.initial_state.isConsistent());
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
