#include "pos2img.h"
#include "osl/c/facade.h"
#include "osl/misc/iconvConvert.h"
#include "osl/record/kanjiCode.h"
#include "osl/record/kanjiPrint.h"
#include "osl/usi.h"
#include "osl/bits/pieceStand.h"
#include "osl/numEffectState.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <cassert>

namespace bf = boost::filesystem;

/* ===== Local functions ===== */

namespace {
  std::string toUtf8(const std::string& euc)
  {
    return osl::misc::IconvConvert::convert("EUC-JP", "UTF-8", euc);
  }

  /**
   * Converts a PSN move string to osl::Move.
   * Note that SimpleState s is the position after the move.
   */
  const osl::Move
  strToMove(const std::string& str, const osl::SimpleState& s)
  {
    using namespace osl;

    if (str.size() < 4)
      return Move();
  
    const Square to = psn::strToPos(str.substr(2,2));
    if (str[1] == '*') {
      const Ptype ptype = psn::charToPtype(str[0]);
      return Move(to, ptype, alt(s.turn()));
    }
    const Ptype ptype = s.pieceOnBoard(to).ptype();
    if (! isPiece(ptype))
      return Move();
  
    const Square from = psn::strToPos(str.substr(0,2));
    const Ptype captured = PTYPE_EMPTY; // we can not decide capture.
    bool promotion = false;
    if (str.size() > 4) {
      assert(str[4] == '+');
      promotion = true;
    }
    return Move(from, to, ptype, captured, promotion, alt(s.turn()));
  }

  /**
   * Returns a Japanese annotation coresponding to a move with UTF-8
   * encoding.
   * Note that an after_state is the position after the move is played.
   */
  std::string
  annotateMove(const osl::Move move, const osl::SimpleState& after_state)
  {
    std::string ret;
    const osl::record::StandardCharacters kanji;

    ret += (move.player() == osl::BLACK) ? K_BLACK_SIGN : K_WHITE_SIGN;

    const osl::Square to = move.to();
    ret += kanji.getSuji(to.x());
    ret += kanji.getDan(to.y());
    ret += kanji.kanji(move.oldPtype());
    if (move.isPromotion())
      ret += K_NARU;

    return toUtf8(ret);
  }
} // anonymous namespace


/* ===== PieceImage ===== */

ImagePtr
PieceImage::image(const osl::PtypeO& ptypeO) const
{
  const std::string kanji_piece = standardCharacters.kanji(ptypeO);
  ImagePtr piece = image(toUtf8(kanji_piece));
  if (getOwner(ptypeO) == osl::WHITE)
    piece->rotate(180);
  return piece;
}

ImagePtr
PieceImage::image(const std::string& s) const
{
  /* 大きな画像で描画し縮小する（文字つぶれ対策）*/
  const static double large_scale  = 3.0;
  const double large_image_width  = cell_width*large_scale;
  const double large_image_height = large_image_width*scale;

  ImagePtr piece(new Magick::Image(Magick::Geometry(large_image_width, large_image_height),
                                   Magick::Color(bg_color)));
  piece->strokeAntiAlias(true);
  piece->strokeColor("black");
  piece->strokeWidth(0.1);
  piece->font("/usr/share/fonts/truetype/vlgothic/VL-Gothic-Regular.ttf"); // VLGothic or DejimaMincho
  const static double margin_space_factor = 4.0;
  const int font_size = (cell_width * large_scale / dpi * 72.0) - 
                        margin_space_factor; /// 1 point = 1/72 inchi
  piece->fontPointsize(font_size);
  piece->density(Magick::Geometry(dpi, dpi));
  piece->annotate(s, Magick::CenterGravity);
  piece->resize(Magick::Geometry(cell_width, cell_height));
  return piece;
}


/* ===== BoardImage ===== */

std::string
BoardImage::blackStand() const
{
  using namespace osl::record;
  std::ostringstream os;
  os << K_BLACK_SIGN << K_BLACK << K_SPACE; // "▲先手　"
  for (unsigned int i=0; i<osl::PieceStand::order.size(); ++i) {
    const osl::Ptype ptype = osl::PieceStand::order[i];
    const int count = state.countPiecesOnStand(osl::BLACK, ptype);
    if (count > 0) {
      os << standardCharacters.kanji(newPtypeO(osl::BLACK, ptype));
      if (count > 1)
	os << osl::record::kanjiNumber(count);
    }
  }
  return toUtf8(os.str());
}

std::string
BoardImage::whiteStand() const
{
  using namespace osl::record;
  std::ostringstream os;
  os << K_WHITE_SIGN << K_WHITE << K_SPACE; // "△後手　"
  for (unsigned int i=0; i<osl::PieceStand::order.size(); ++i) {
    const osl::Ptype ptype = osl::PieceStand::order[i];
    const int count = state.countPiecesOnStand(osl::WHITE, ptype);
    if (count > 0) {
      os << standardCharacters.kanji(newPtypeO(osl::WHITE, ptype));
      if (count > 1)
	os << kanjiNumber(count);
    }
  }
  return toUtf8(os.str());
}


void
BoardImage::writeLine(const double start_x, const double start_y,
                      const std::string& line)
{
  using namespace Magick;
  std::list<Drawable> text_draw_list;
  //text_draw_list.push_back(DrawableStrokeColor(Color("black")));
  text_draw_list.push_back(DrawableStrokeColor(Color(0, 0, 0, MaxRGB)));
  //text_draw_list.push_back(DrawableFillColor(Color(0, 0, 0, MaxRGB)));
  text_draw_list.push_back(DrawableFillColor("black"));
  text_draw_list.push_back(DrawableStrokeWidth(2));
  //text_draw_list.push_back(DrawableStrokeAntialias(true)); // true: default
  //text_draw_list.push_back(DrawableFont("-misc-fixed-medium-o-semicondensed—13-*-*-*-c-60-iso8859-1"));
  text_draw_list.push_back(DrawableFont("/usr/share/fonts/truetype/vlgothic/VL-Gothic-Regular.ttf"));
  text_draw_list.push_back(DrawableText(start_x, start_y, line));
  image->fontPointsize(14);
  image->draw(text_draw_list);
}

void
BoardImage::draw()
{
  const int LINE_WIDTH = 1;
  image.reset(new Magick::Image(Magick::Geometry(image_width, image_height), 
                                Magick::Color("white")));

  image->strokeAntiAlias(true);
  image->strokeColor("black");
  image->fillColor("white");

  /* Draw annotations */
  const double line_x = 20;
  double line_y = meta_line_height;
  if (!title.empty()) {
    writeLine(line_x,line_y,title);
    line_y += meta_line_height;
  }
  if (!black_name.empty()) {
    writeLine(line_x,line_y, "先手: " + black_name);
    line_y += meta_line_height;
  }
  if (!white_name.empty()) {
    writeLine(line_x,line_y, "後手: " + white_name);
    line_y += meta_line_height;
  }
  if (last_move.isNormal()) {
    const std::string line = "図は" + annotateMove(last_move, state) + "まで";
    writeLine(line_x,line_y, line);
    line_y += meta_line_height;
  }
  board_origin_y += line_y;

  image->draw(Magick::DrawableRectangle(board_origin_x, board_origin_y,
                                        board_origin_x + board_width,
                                        board_origin_y + board_hegiht));
  image->strokeWidth(LINE_WIDTH);
  /* vertical lines */
  for (int i=1; i<9; ++i)
  {
    const int x0 = board_origin_x + cell_width*i;
    const int y0 = board_origin_y;
    const int x1 = x0;
    const int y1 = board_origin_y + board_hegiht;
    image->draw(Magick::DrawableLine(x0, y0, x1, y1));
  }
  /* horizontal lines */
  for (int i=1; i<9; ++i) {
    const int x0 = board_origin_x;
    const int y0 = board_origin_y + cell_height*i;
    const int x1 = x0 + board_width;
    const int y1 = y0;
    image->draw(Magick::DrawableLine(x0, y0, x1, y1));
  }
  /* pieces on the board */
  for (int i=0; i<40; ++i) {
    const osl::Piece p = state.pieceOf(i);
    const osl::Square pos = p.square();
    if (!pos.isOnBoard())
      continue;

    // Color table: http://www.imagemagick.org/script/color.php
    static const std::string non_threatmate_color = "yellow";
    static const std::string threatmate_color     = "LightBlue1";
    static const std::string checkmate_color      = "violet";
    std::string bg_color = "white";
    if (last_move.isNormal() &&
        pos == last_move.to()) {
      switch (threat) {
      case Threat::THREATMATE:
        bg_color = threatmate_color;
        break;
      case Threat::CHECKMATE:
        bg_color = checkmate_color;
        break;
      default:
        bg_color = non_threatmate_color;
      }
    }
    const double x0 = board_origin_x + cell_width*(9-pos.x());// + cell_width*0.5;
    const double y0 = board_origin_y + cell_height*(pos.y()-1);// + cell_height*0.5;
    PieceImage pieceImage(cell_width-2*LINE_WIDTH, scale, bg_color);
    ImagePtr piece = pieceImage.image(p.ptypeO());
    image->composite(*piece, x0+LINE_WIDTH*2, y0+LINE_WIDTH*2*scale, Magick::OverCompositeOp);
  }
  {
    /* pieces on Black's stand */
    PieceImage pieceImage(cell_width*0.8, scale);
    const std::string black_stands = blackStand();
    for (int i=0; i<black_stands.size()/3; ++i) {
      // UTF-8: each string is of 3-byte size
      const std::string s(black_stands.substr(i*3, 3));
      ImagePtr piece = pieceImage.image(s);
      const double x0 = board_origin_x + cell_width*10;
      const double y0 = board_origin_y + piece->size().height()*i;
      image->composite(*piece, x0, y0, Magick::OverCompositeOp);
    }
    /* pieces on White's stand */
    const std::string white_stands = whiteStand();
    for (int i=0; i<white_stands.size()/3; ++i) {
      // UTF-8: each string is of 3-byte size
      const std::string s(white_stands.substr(i*3, 3));
      ImagePtr piece = pieceImage.image(s);
      const double x0 = board_origin_x - cell_width;
      const double y0 = board_origin_y + board_hegiht - piece->size().height()*(i+1);
      piece->rotate(180);
      image->composite(*piece, x0, y0, Magick::OverCompositeOp);
    }
  }
}

void
BoardImage::write(Magick::Blob& blob, const std::string& file_type) const
{
  image->write(&blob, file_type);
}


/* ===== ProcessBoard ===== */

bool 
ProcessBoard::isPossibleSfenCharacters(const std::string& sfen) const
{
  if (sfen.empty())
    return false;

  static const boost::regex e("^[PLNSGBRKplnsgbrk1-9bw /\\-\\+\\*]+$", boost::regex::perl);
  if (!boost::regex_match(sfen, e))
    return false;
  return true;
}

bool
ProcessBoard::isPossibleSfenMoveCharacters(const std::string& str) const
{
  if (str.empty())
    return false;

  static const boost::regex e1("^[1-9][a-i][1-9][a-i]\\+?$", boost::regex::perl);
  static const boost::regex e2("^[PLNSGBR]\\*[1-9][a-i]$", boost::regex::perl); // drop move
  if (!boost::regex_match(str, e1) &&
      !boost::regex_match(str, e2))
    return false;
  return true;
}

std::string
ProcessBoard::eliminateSfen(const std::string& sfen) const
{
  const std::string::size_type loc = sfen.find("sfen ", 0);
  if (loc == 0)
    return sfen.substr(5, sfen.size()-5);
  return sfen;
}

std::shared_ptr<BoardImage>
ProcessBoard::generate_image(const std::string& line,
                             const MetaData& md) const
{
  std::shared_ptr<BoardImage> ret; // NULL

  std::string board_id, move_str;
  if (!splitMove(line, board_id, move_str))
    return ret;
  assert(!board_id.empty());

  try {
    osl::NumEffectState state;
    std::vector<osl::Move> moves;
    osl::usi::parse(line, state, moves);
    
    // Apply moves
    for (const osl::Move move : moves) {
      if (!move.isNormal() || !state.isValidMove(move))
        return ret; // NULL
      state.makeMove(move);
      assert(state.isConsistent());
    }

    // Parse last_move
    osl::Move last_move;
    if (!moves.empty()) {
      last_move = moves.back();
    } else if (!md.last_move.empty()) {
      last_move = strToMove(md.last_move, state);
    }

    // Threatmate
    Threat threat = Threat::NONE;
    if (checkmate_limit) {
      if (isCheckmate(state)) {
        threat = Threat::CHECKMATE;
      } else if (isThreatmate(state)) {
        threat = Threat::THREATMATE;
      }
    }

    // Set up a BoardImage.
    ret.reset(new BoardImage(state, last_move,
                             md.title, md.black_name, md.white_name,
                             threat));
    assert(ret);
    ret->draw();
  } catch (osl::usi::ParseError& pe) {
    std::cerr << "ERROR: Failed to parse " << line << std::endl;
    std::cerr << pe.what() << std::endl;
  } catch (std::exception& e) {
    std::cerr << "ERROR: Failed to process " << line << std::endl;
    std::cerr << e.what() << std::endl;
  }

  return ret;
}

bool
ProcessBoard::generate(const std::string& line,
                       const MetaData& md,
                       Magick::Blob& blob) const
{
  std::shared_ptr<BoardImage> image
    = generate_image(line, md);
  if (image)
    image->write(blob, "PNG");
  return blob.length() > 0;
}

bool
ProcessBoard::splitMove(const std::string& source,
                        std::string& sfen_string,
                        std::string& move1) const
{
  std::vector<std::string> strs;
  boost::split(strs, source, boost::is_any_of(" "));
  std::string board;

  if (strs.empty()) {
    goto error;
  }

  if (strs[0] == "position") {
    sfen_string.assign(source);
    move1.clear();
    return true;
  }

  switch (strs.size()) {
  case 7:
    if (strs[5] == "moves" &&
        isPossibleSfenMoveCharacters(strs[6])) 
      move1.assign(strs[6]);
    else
      goto error;
    // pass through
  case 4: // pass through
  case 5:
    if (strs[0] != "sfen")
      goto error;
    board = strs[1] + " " + strs[2] + " " + strs[3];
         // board,          b or w,         hand; ignore the tailing number
    if (isPossibleSfenCharacters(board)) {
      sfen_string.assign("sfen " + board);
    } else {
      std::cerr << "ERROR: invalid characters: " << source << std::endl;
      goto error;
    }
    break;
  default:
    std::cerr << "ERROR: invalid syntax: " << source << std::endl;
    goto error;
  }

  return true;

error:
  sfen_string.clear();
  move1.clear();
  return false;
}

bool
ProcessBoard::isCheckmate(const osl::SimpleState& state) const
{
  std::ostringstream os;
  os << state;
  const int ret = ::checkmate_escape(os.str().c_str(), checkmate_limit);
  if (ret) {
    return true;
  } else {
    return false;
  }
}

bool
ProcessBoard::isThreatmate(const osl::SimpleState& state) const
{
  osl::SimpleState s(state);
  s.changeTurn();
 
  std::ostringstream os;
  os << s;
  char dummy[32];
  int limit = checkmate_limit;
  const int ret = ::checkmate_attack(os.str().c_str(), limit, dummy);
  if (ret) {
    return true;
  } else {
    return false;
  }
}

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
