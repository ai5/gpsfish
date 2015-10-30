#include "osl/record/kisen.h"
#include "osl/misc/filePath.h"
#include "osl/misc/iconvConvert.h"
#include "osl/misc/sjis2euc.h"
#include <boost/filesystem/convenience.hpp>
#include <iostream>

osl::Square osl::record::KisenUtils::convertSquare( int pos ){
  assert(1<=pos && pos<=0x51);
  int y=((pos-1)/9)+1, x=((pos-1)%9)+1;
  return Square(x,y);
}
int osl::record::KisenUtils::convertSquare(Square pos)
{
  return ((pos.y() - 1) * 9 + 1) + pos.x() - 1;
}

osl::Move osl::record::KisenUtils::convertMove(SimpleState const& state,int c0,int c1){
  Move move;

  if(1<=c1 && c1<=0x51){
    Square from=convertSquare(c1),to;
    Piece fromPiece=state.pieceOnBoard(from);
    if (! fromPiece.isPiece()) 
      throw CsaIOError("Square error");
    assert(fromPiece.isPiece());
    assert(fromPiece.owner()==state.turn() ||
	   (std::cerr << c1 << "," << from << "," << fromPiece << std::endl,0)
      );
    bool isPromote=false;
    if(1<=c0 && c0<=0x51){
      to=convertSquare(c0);
    }
    else if(0x65<=c0 && c0<=0xb5){
      to=convertSquare(c0-0x64);
      isPromote=true;
    }
    else{
      throw CsaIOError("c0 range error");
    }
    Piece toPiece=state.pieceAt(to);
    if (! toPiece.isEmpty() && toPiece.owner()!=alt(state.turn()))
      throw CsaIOError("inconsintent move (to)");
    Ptype ptype=fromPiece.ptype();
    if(isPromote)ptype=promote(ptype);
    const Ptype captured = toPiece.ptype();
    if (captured == KING)
      return Move::INVALID();
    move=Move(from,to,ptype,captured,isPromote,state.turn());
  }
  else{
    assert(0x65<=c1);
    if (!(1<=c0&&c0<=0x51)) {
      throw CsaIOError("unknown kisen move "+std::to_string((int)c0));
    }
    assert(1<=c0&&c0<=0x51);
    Square to=convertSquare(c0);
    Ptype ptype=PTYPE_EMPTY;
    int piece_on_stand = c1;
    const Ptype ptypes[]={ROOK,BISHOP,GOLD,SILVER,KNIGHT,LANCE,PAWN};
    for(size_t i=0;i<sizeof(ptypes)/sizeof(Ptype);i++){
      int count=state.countPiecesOnStand(state.turn(),ptypes[i]);
      if(count>0){
	if(piece_on_stand>0x64){
	  piece_on_stand-=count;
	  if(piece_on_stand<=0x64) ptype=ptypes[i];
	}
      }
    }
    assert(ptype!=PTYPE_EMPTY ||
	   (std::cerr << state << to << " " << c1
	    << " " << piece_on_stand << std::endl, false));
    move=Move(to,ptype,state.turn());
  }
  if (! state.isValidMove(move,true)) {
    std::cerr << "warning: bad move in kisen\n" << state << move << "\n";
    return Move();
  }
  assert(state.isValidMove(move,true) ||
	 (std::cerr << state << move << std::endl, false));
  return move;
}

osl::record::KisenFile::KisenFile(const std::string& f) 
  :ifs(f), filename(f) 
{
  if (! ifs)
    throw CsaIOError("KisenFile not found "+f);
  ifs.seekg(0,std::ios::end);
  assert((ifs.tellg() % 512)==0);
  number_of_games=ifs.tellg()/512;
}

std::vector<osl::Move> osl::record::KisenFile::moves(size_t index)
{
  assert(index<size());
  std::vector<Move> moves;
  //    std::cerr << "Game[" << index << "]" << std::endl;
  ifs.seekg(index*512,std::ios::beg);
  CArray<unsigned char, 512> cbuf;
  ifs.read(reinterpret_cast<char *>(&cbuf[0]),512);
  NumEffectState state;
  //
  Player turn=BLACK;
  for(size_t turn_count=0; 
      (turn_count*2 < cbuf.size())
	&& cbuf[turn_count*2]!=0 && cbuf[turn_count*2+1]!=0;
      turn_count++, turn=alt(turn)){
    if(turn_count==KisenFile::MaxMoves || cbuf[  turn_count *2 ] == 0 || cbuf[ turn_count * 2 + 1 ] == 0 ){ break; }
    int c0=cbuf[turn_count*2], c1=cbuf[turn_count*2+1];
    if (moves.empty() && c0 == 0xff && c1 == 0xff) // komaochi
      break;
    const Move move=KisenUtils::convertMove(state,c0,c1);
    if (move.isInvalid())
      break;
    moves.push_back(move);
    state.makeMove(move);
    assert(state.isConsistent( true ) );
  }
  return moves;
}
#ifndef MINIMAL
std::string osl::record::KisenFile::ipxFileName(const std::string& filename) 
{
  namespace bf = boost::filesystem;
  const bf::path ipxfilename = bf::change_extension(bf::path(filename), ".ipx");
  return misc::file_string(ipxfilename);
}

osl::record::KisenIpxFile::KisenIpxFile(const std::string& filename) 
  :ifs(filename), file_name(filename) 
{
  if (! ifs)
    throw CsaIOError("KisenIpxFile not found "+filename);
  ifs.seekg(0,std::ios::end);
  assert((ifs.tellg() % 256)==0);
  number_of_games=ifs.tellg()/256;
}
std::string osl::record::KisenIpxFile::player(size_t index,Player pl)
{
  assert(index<size());
  ifs.seekg(index*256,std::ios::beg);
  CArray<unsigned char, 256> cbuf;
  ifs.read(reinterpret_cast<char *>(&cbuf[0]),256);
  int startIndex=0;
  if(pl==WHITE)startIndex=14;
  CArray<char,15> buf;
  buf[14]='\0';
  strncpy(&buf[0],reinterpret_cast<char *>(&cbuf[startIndex]),14);
  return misc::sjis2euc(std::string(&buf[0]));
}
unsigned int osl::record::KisenIpxFile::rating(size_t index,Player pl)
{
  assert(index<size());
  ifs.seekg(index*256,std::ios::beg);
  CArray<unsigned char, 256> cbuf;
  ifs.read(reinterpret_cast<char *>(&cbuf[0]),256);
  int startIndex=0324;
  if(pl==WHITE)startIndex=0326;
  return cbuf[startIndex]+256*cbuf[startIndex+1];
}
unsigned int osl::record::KisenIpxFile::result(size_t index)
{
  assert(index<size());
  ifs.seekg(index*256,std::ios::beg);
  CArray<unsigned char, 256> cbuf;
  ifs.read(reinterpret_cast<char *>(&cbuf[0]),256);
  return cbuf[64+48+6];
}
std::string osl::record::KisenIpxFile::title(size_t index,Player pl)
{
  assert(index<size());
  ifs.seekg(index*256,std::ios::beg);
  CArray<unsigned char, 256> cbuf;
  ifs.read(reinterpret_cast<char *>(&cbuf[0]),256);
  int startIndex=28;
  if(pl==WHITE)startIndex+=8;
  CArray<char,9> buf;
  buf[8]='\0';
  strncpy(&buf[0],reinterpret_cast<const char*>(&cbuf[startIndex]),8);
  return misc::sjis2euc(std::string(&buf[0]));
}
boost::gregorian::date osl::record::KisenIpxFile::startDate(size_t index)
{
  assert(index<size());
  ifs.seekg(index*256,std::ios::beg);
  CArray<unsigned char, 256> cbuf;
  ifs.read(reinterpret_cast<char *>(&cbuf[0]),256);
  const int startIndex=84;
  const unsigned int year  = cbuf[startIndex] + 256*cbuf[startIndex+1];
  const unsigned int month = cbuf[startIndex+2];
  const unsigned int day   = cbuf[startIndex+3];
  try {
    const boost::gregorian::date d = boost::gregorian::date(year, month, day);
    return d;
  } catch (std::out_of_range& e) {
    std::cerr << e.what() << ": ["
	      << index << "] " << year << "-" << month << "-" << day << "\n"; 
    return boost::gregorian::date(boost::gregorian::not_a_date_time); 
  }
}

osl::record::KisenPlusFile::KisenPlusFile(const std::string& filename) 
  :ifs(filename)
{
  if (! ifs)
    throw CsaIOError("KisenPlusFile not found");
  ifs.seekg(0,std::ios::end);
  assert((ifs.tellg() % 2048)==0);
  number_of_games=ifs.tellg()/2048;
}

std::vector<osl::Move> osl::record::KisenPlusFile::moves(size_t index)
{
  std::vector<Move> moves;
  std::vector<int> times;
  load(index, moves, times);
  return moves;
}

void osl::record::KisenPlusFile::load(size_t index,
				      std::vector<Move>& moves, std::vector<int>& times)
{
  assert(index<size());
  //    std::cerr << "Game[" << index << "]" << std::endl;
  ifs.seekg(index*2048,std::ios::beg);
  CArray<unsigned char, 2048> cbuf;
  ifs.read(reinterpret_cast<char *>(&cbuf[0]),2048);
  NumEffectState state;
  for (size_t i = 0; 
       i < 2048 && cbuf[i]!=0 && cbuf[i+1]!=0;
       i += 8)
  {
    int c0 = cbuf[i];
    int c1 = cbuf[i + 1];
    bool is_promote = false;
    Move move;

    if (c0 > 100)
    {
      is_promote = true;
      c0 = 256 - c0;
    }

    Square to(c0 % 10, c0 / 10);

    if (c1 < 10)
    {
      // drop
      move = Move(to,
		  PieceStand::order[c1 - 1],
		  state.turn());
    }
    else
    {
      Square from(c1 % 10, c1 / 10);
      Ptype type = state.pieceAt(from).ptype();
      if (is_promote)
	type = promote(type);
      move = Move(from, to,
		  type, state.pieceAt(to).ptype(),
		  is_promote, state.turn());
    }
    moves.push_back(move);
    times.push_back(cbuf[i + 7] * 60 + cbuf[i + 6]);
    state.makeMove(move);
    assert(state.isConsistent( true ) );
  }
}
#endif

osl::record::
KisenFile::~KisenFile()
{
}
#ifndef MINIMAL
osl::record::
KisenIpxFile::~KisenIpxFile()
{
}

void osl::record::
KisenWriter::save(const RecordMinimal& record)
{
  if (!(record.initial_state == NumEffectState()))
  {
    std::cerr << "Can not save non-HIRATE record" << std::endl;
    return;
  }
  NumEffectState state;
  const int max_length = std::min(256, static_cast<int>(record.moves.size()));
  for (int i = 0; i < max_length; ++i)
  {
    const Move move = record.moves[i];
    if (!move.isDrop())
    {
      int from = KisenUtils::convertSquare(move.from());
      int to = KisenUtils::convertSquare(move.to());
      if (move.isPromotion())
      {
	to += 100;
      }
      os << static_cast<char>(to) << static_cast<char>(from);
    }
    else
    {
      int to = KisenUtils::convertSquare(move.to());
      int count = 1;
      for (Ptype ptype: PieceStand::order) {
	if (ptype == move.ptype())
	{
	  break;
	}
	count += state.countPiecesOnStand(move.player(), ptype);
      }
      count += 100;
      os << static_cast<char>(to) << static_cast<char>(count);
    }
    state.makeMove(record.moves[i]);
  }
  for (int i = max_length; i < 256; ++i)
  {
    os << '\0' << '\0';
  }
}

void osl::record::
KisenIpxWriter::writeString(const std::string &name, size_t length)
{
  for (size_t i = 0; i < length; ++i)
  {
    if (i < name.length())
    {
      os << name[i];
    }
    else
    {
      os << '\0';
    }
  }
}

void osl::record::
KisenIpxWriter::writeRating(int rating)
{
  int high = rating / 256;
  int low = rating % 256;
  os << static_cast<char>(low) << static_cast<char>(high);
}

void osl::record::
KisenIpxWriter::writeStartDate(int year, int month, int day, int hour, int min)
{
  const int high_year = year / 256;
  const int low_year  = year % 256;
  os << static_cast<char>(low_year)
     << static_cast<char>(high_year)
     << static_cast<char>(month)
     << static_cast<char>(day)
     << static_cast<char>(hour)
     << static_cast<char>(min);
}

void osl::record::
KisenIpxWriter::save(const Record &record,
		     int black_rating, int white_rating,
		     const std::string &black_title,
		     const std::string &white_title)
{
  // total 256 bytes
  // Player name: 14 bytes each
#ifndef _WIN32
  writeString(IconvConvert::convert("EUC-JP", "SJIS", record.player[BLACK]), 14);
  writeString(IconvConvert::convert("EUC-JP", "SJIS", record.player[WHITE]), 14);
  writeString(IconvConvert::convert("EUC-JP", "SJIS", black_title), 8);
  writeString(IconvConvert::convert("EUC-JP", "SJIS", white_title), 8);
#else
  writeString("", 14);
  writeString("", 14);
  writeString("", 8);
  writeString("", 8);
#endif
  for (int i = 44; i < 84; ++i)
  {
    os << '\0';
  }
  const boost::gregorian::date start_date = record.start_date;
  if (!start_date.is_special()) {
    // time is fixed with 9am
    writeStartDate(start_date.year(), start_date.month(), start_date.day(), 9, 0);
  } else {
    for (int i = 84; i < 90; ++i)
    {
      os << '\0';
    }
  }
  for (int i = 90; i < 118; ++i)
  {
    os << '\0';
  }
  std::vector<Move> moves = record.moves();
  std::vector<int> time = record.times;
  // TODO: sennichite, jishogi
  if (moves.size() <= 256)
  {
    if (moves.size() % 2 == 0)
      os << static_cast<char>(KisenIpxFile::WHITE_WIN);
    else
      os << static_cast<char>(KisenIpxFile::BLACK_WIN);
  }
  else
  {
    if (moves.size() % 2 == 0)
      os << static_cast<char>(KisenIpxFile::WHITE_WIN_256);
    else
      os << static_cast<char>(KisenIpxFile::BLACK_WIN_256);
  }
  for (int i = 119; i < 212; ++i)
  {
    os << '\0';
  }
  writeRating(black_rating);
  writeRating(white_rating);
  for (int i = 216; i < 256; ++i)
  {
    os << '\0';
  }
}
#endif
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
