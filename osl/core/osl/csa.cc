#include "osl/csa.h"
#include "osl/simpleState.h"
#include "osl/bits/pieceTable.h"
#include "osl/oslConfig.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <fstream>
#include <array>

/* ------------------------------------------------------------------------- */

osl::Player osl::csa::
charToPlayer(char c)
{
  if(c=='+') 
    return BLACK;
  if(c=='-') 
    return WHITE;
  throw CsaIOError("not a csa PlayerCharacter "+std::string(1,c));
}

const osl::Square osl::csa::
strToPos(const std::string& s)
{
  int x=s.at(0)-'0';
  int y=s.at(1)-'0';
  if(x==0 && y==0) 
    return Square::STAND();
  return Square(x,y);
}

osl::Ptype osl::csa::
strToPtype(const std::string& s)
{
  for(int i=0;i<16;i++){
    if(s == Ptype_Table.getCsaName(static_cast<Ptype>(i))) 
      return static_cast<Ptype>(i);
  }
  throw CsaIOError("unknown std::string in csa::strToPtype "+s);
}

const osl::Move osl::csa::
strToMove(const std::string& s,const SimpleState& state)
{
  if (s == "%KACHI")
    return Move::DeclareWin();
  if (s == "%TORYO")
    return Move::INVALID();
  if (s == "%PASS")		// FIXME: not in CSA protocol
    return Move::PASS(state.turn());

  Player pl=csa::charToPlayer(s.at(0));
  Square fromPos=csa::strToPos(s.substr(1,2));
  Square toPos=csa::strToPos(s.substr(3,2));
  Ptype ptype=csa::strToPtype(s.substr(5,2));
  if(fromPos==Square::STAND()){
    if (isPromoted(ptype))
      throw CsaIOError("drop with promote ?! in csa::strToMove "+s);
    return Move(toPos,ptype,pl);
  }
  else{
    Piece p0=state.pieceAt(fromPos);
    Piece p1=state.pieceAt(toPos);
    Ptype capturePtype=p1.ptype();
    bool isPromote=(p0.ptype()!=ptype);
    if (! ((p0.ptype()==ptype)||(p0.ptype()==unpromote(ptype))))
      throw CsaIOError("bad promotion in csa::strToMove "+s);
    return Move(fromPos,toPos,ptype,
		capturePtype,isPromote,pl);
  }
}

/* ------------------------------------------------------------------------- */
const std::string osl::csa::
show(Player player, std::string& buf, size_t offset)
{
  assert(buf.size() >= offset+1);
  buf[offset] = (player==BLACK) ? '+' : '-';
  return buf;
}

const std::string osl::csa::
show(Move move, std::string& buf)
{
  assert(buf.capacity() >= 7);
  buf.resize(7);
  if (move == Move::DeclareWin())
    return buf = "%KACHI";
  if (move.isInvalid())
    return buf = "%TORYO";
  if (move.isPass())
    return buf = "%PASS";		// FIXME: not in CSA protocol
  show(move.player(), buf);
  show(move.from(), buf, 1);
  show(move.to(), buf, 3);
  show(move.ptype(), buf, 5);
  return buf;
}

const std::string osl::csa::
show(Square pos, std::string& buf, size_t offset)
{
  assert(buf.size() >= offset+2);
  if (pos.isPieceStand()) 
  {
    buf[0+offset] = '0';
    buf[1+offset] = '0';
    return buf;
  }
  const int x = pos.x();
  const int y = pos.y();
  buf[offset+0] = x + '0';
  buf[offset+1] = y + '0';
  return buf;
}

const std::string osl::csa::
show(Ptype ptype, std::string& buf, size_t offset)
{
  assert(buf.size() >= offset+2);
  const char *name = Ptype_Table.getCsaName(ptype);
  buf[0+offset] = name[0];
  buf[1+offset] = name[1];
  return buf;
}

const std::string osl::csa::
show(Move move)
{
  // NOTE: copy コピーを返すので dangling pointer ではない
  std::string buf("+7776FU");
  return show(move, buf);
}

const std::string osl::csa::
fancyShow(Move move)
{
  std::string ret = show(move);
  if (move.isNormal()) {
    if (move.capturePtype() != PTYPE_EMPTY)
      ret += "x" + show(move.capturePtype());
    if (move.isPromotion())
      ret += '+';
  }
  return ret;
}

const std::string osl::csa::
show(Player player)
{
  std::string buf("+");
  return show(player, buf);
}

const std::string osl::csa::
show(Square position)
{
  std::string buf("00");
  return show(position, buf);
}

const std::string osl::csa::
show(Ptype ptype)
{
  std::string buf("OU");
  return show(ptype, buf);
}

const std::string osl::csa::
show(Piece piece)
{
  if (piece.isEdge())
    return "   ";
  if (piece.isEmpty())
    return " * ";

  assert(piece.isPiece() && isPiece(piece.ptype()));
  assert(unpromote(piece.ptype()) == Piece_Table.getPtypeOf(piece.number()));
  return show(piece.owner()) 
    + show(piece.ptype());
}

const std::string osl::csa::
show(const Move *first, const Move *last)
{
  std::ostringstream out;
  for (; first != last; ++first) {
    if (first->isInvalid())
      break;
    out << show(*first);
  }
  return out.str();
}

/* ------------------------------------------------------------------------- */
osl::csa::CsaFileMinimal::CsaFileMinimal(const std::string& filename)
{
  std::ifstream is(filename);
  if (! is) {
    const std::string msg = "CsaFileMinimal::CsaFileMinimal file open failed ";
    std::cerr << msg << filename << "\n";
    throw CsaIOError(msg + filename);
  }
  load(is);
}

osl::csa::CsaFileMinimal::CsaFileMinimal(std::istream& is)
{
  load(is);
}
osl::csa::CsaFileMinimal::~CsaFileMinimal()
{
}

void osl::csa::CsaFileMinimal::load(std::istream& is)
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
      parseLine(work, record, e, board_parsed);
    }
  }
  if (*std::min_element(board_parsed.begin(), board_parsed.end()) == false)
    throw CsaIOError("incomplete position description in csaParseLine");
  assert(record.initial_state.isConsistent());
}

bool osl::csa::
CsaFileMinimal::parseLine(SimpleState& state, RecordMinimal& record, std::string s,
			  CArray<bool,9>& board_parsed)
{
  while (! s.empty() && isspace(s[s.size()-1])) // ignore trailing garbage
    s.resize(s.size()-1);
  if (s.length()==0) 
    return true;
  switch(s.at(0)){
  case 'P': /* 開始盤面 */
    switch(s.at(1)){
    case 'I': /* 平手初期配置 */
      board_parsed.fill(true);
      state.init(HIRATE);
      break;
    case '+': /* 先手の駒 */
    case '-':{ /* 後手の駒 */
      Player pl=csa::charToPlayer(s.at(1));
      for(int i=2;i<=(int)s.length()-4;i+=4){
	Square pos=csa::strToPos(s.substr(i,2));
	if(s.substr(i+2,2) == "AL"){
	  state.setPieceAll(pl);
	}
	else{
	  Ptype ptype=csa::strToPtype(s.substr(i+2,2));
	  state.setPiece(pl,pos,ptype);
	}
      }
      break;
    }
    default:
      if(isdigit(s.at(1))){
	const int y=s.at(1)-'0';
	board_parsed[y-1] = true;
	for(unsigned int x=9,i=2;i<s.length();i+=3,x--){
	  if (s.at(i) != '+' && s.at(i) != '-' && s.find(" *",i)!=i) {
	    if (OslConfig::inUnitTest())
	      throw CsaIOError("parse board error " + s);
	    else
	      std::cerr << "possible typo for empty square " << s << "\n";
	  }   
	  if (s.at(i) != '+' && s.at(i) != '-') continue;
	  Player pl=csa::charToPlayer(s.at(i));
	  Square pos(x,y);
	  Ptype ptype=csa::strToPtype(s.substr(i+1,2));
	  state.setPiece(pl,pos,ptype);
	}
      }
    }
    break;
  case '+':
  case '-':{
    Player pl=csa::charToPlayer(s.at(0));
    if(s.length()==1){
      state.setTurn(pl);
      state.initPawnMask();
      record.initial_state = NumEffectState(state);
    }
    else{ // actual moves
      const Move m = csa::strToMove(s,state);
      if (! state.isValidMove(m))
      {
	std::cerr << "Illegal move " << m << std::endl;
	throw CsaIOError("illegal move "+s);
      }
      record.moves.push_back(m);
      NumEffectState copy(state);
      copy.makeMove(m);
      state = copy;
    }
    break;
  }
  default:
    return false;		// there are unhandled contents
  }
  return true;
}

/* ------------------------------------------------------------------------- */
osl::csa::CsaString::CsaString(const std::string& s) 
{
  std::istringstream is(s);
  load(is);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
