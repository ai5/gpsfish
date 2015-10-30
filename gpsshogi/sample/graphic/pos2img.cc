#include "pos2img.h"
#include "osl/apply_move/applyMove.h"
#include "osl/c/facade.h"
#include "osl/misc/iconvConvert.h"
#include "osl/record/kanjiCode.h"
#include "osl/record/usi.h"
#include "osl/pieceStand.h"
#include <boost/algorithm/string.hpp>
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
  piece->font("VL-Gothic-regular"); // VLGothic or DejimaMincho
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
BoardImage::draw()
{
  const int LINE_WIDTH = 1;
  image.reset(new Magick::Image(Magick::Geometry(image_width, image_height), 
                                Magick::Color("white")));

  image->strokeAntiAlias(true);
  image->strokeColor("black");
  image->fillColor("white");

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

    static const std::string threatmate_color = "violet";
    static const std::string non_threatmate_color = "yellow";
    std::string bg_color = "white";
    if (last_move.isNormal() &&
        pos == last_move.to()) {
      if (is_threatmate)
        bg_color = threatmate_color;
      else
        bg_color = non_threatmate_color;
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
BoardImage::write(const std::string& file_name) const
{
  image->write(file_name);
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

  static const boost::regex e("^[PLNSGBRKplnsgbrk1-9bw\\._@\\-]+$", boost::regex::perl);
  if (!boost::regex_match(sfen, e))
    return false;
  return true;
}

bool
ProcessBoard::isPossibleSfenMoveCharacters(const std::string& str) const
{
  if (str.empty())
    return false;

  static const boost::regex e1("^[1-9][a-i][1-9][a-i]$", boost::regex::perl);
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
                             std::string& file_name) const
{
  std::shared_ptr<BoardImage> ret; // NULL

  std::string board_id, move_str;
  if (!splitMove(line, board_id, move_str))
    return ret;
  assert(!board_id.empty());

  try {
    file_name.assign(line + ".png");

    std::string unescaped(line);
    osl::record::usi::unescape(unescaped);
    osl::SimpleState state;
    osl::vector<osl::Move> moves;
    osl::record::usi::parse(unescaped, state, moves);

    assert(moves.size() <= 1);

    if (moves.empty()) {
      ret.reset(new BoardImage(state));
    } else {
      const osl::Move move = moves.front();
      if (!move.isNormal() || !state.isValidMove(move))
        return ret; // NULL
      osl::ApplyMoveOfTurn::doMove(state, move);
      assert(state.isConsistent());
      bool is_threatmate = false;
      if (checkmate_limit) {
        is_threatmate = checkmateEscape(state);
      }
      ret.reset(new BoardImage(state, move, is_threatmate));
    }
    assert(ret);
    ret->draw();
    return ret;
  } catch (osl::record::usi::ParseError& pe) {
    std::cerr << "ERROR: Failed to parse " << line << std::endl;
    std::cerr << pe.what() << std::endl;
  } catch (std::exception& e) {
    std::cerr << "ERROR: Failed to process " << line << std::endl;
    std::cerr << e.what() << std::endl;
  }
  return ret;
}

std::string 
ProcessBoard::generate(const std::string& line,
                       const boost::filesystem::path& dir) const
{
  std::string file_name;
  std::shared_ptr<BoardImage> image
    = generate_image(line, file_name);
  if (image) {
    assert(!file_name.empty());
    const bf::path file_path = dir / file_name;
    image->write(file_path.string());
  } else
    return "";
  return file_name;
}

std::string 
ProcessBoard::generate(const std::string& line,
                       Magick::Blob& blob) const
{
  std::string file_name;
  std::shared_ptr<BoardImage> image
    = generate_image(line, file_name);
  if (image)
    image->write(blob, "PNG");
  else
    return "";
  return file_name;
}

bool
ProcessBoard::splitMove(const std::string& source,
                        std::string& sfen_string,
                        std::string& move1) const
{
  std::vector<std::string> strs;
  boost::split(strs, source, boost::is_any_of("."));

  std::string board;

  switch (strs.size()) {
  case 7:
    if (strs[5] == "moves" &&
        isPossibleSfenMoveCharacters(strs[6])) 
      move1.assign(strs[6]);
    else
      goto error;
    // pass through
  case 4:
    if (strs[0] != "sfen")
      goto error;
    board = strs[1] + "." + strs[2] + "." + strs[3];
    if (isPossibleSfenCharacters(board)) {
      sfen_string.assign("sfen." + board);
    } else
      goto error;
    break;
  default:
    goto error;
  }

  return true;

error:
  sfen_string.clear();
  move1.clear();
  return false;
}

bool
ProcessBoard::checkmateEscape(const osl::SimpleState& state) const
{
  std::ostringstream os;
  os << state;
  const int ret = ::checkmate_escape(os.str().c_str(), checkmate_limit);
  if (ret)
    return true;
  else
    return false;
}

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
