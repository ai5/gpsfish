#ifndef _GPS_POS2IMG_H
#define _GPS_POS2IMG_H

#include "osl/basic_type.h"
#include "osl/record/kanjiPrint.h"
#include "osl/simpleState.h"
#include <Magick++.h> 
#include <string>

typedef std::shared_ptr<Magick::Image> ImagePtr;

/**
 * PieceImage represents an image of a piece.
 */
class PieceImage
{
private:
  const double dpi; /// dot per inch
  const double cell_width;
  const double cell_height;
  const double scale; /// ratio: cell_width to cell_hight
  const osl::record::StandardCharacters standardCharacters;
  const std::string bg_color; /// background color of this piece
public:
  explicit PieceImage(const double _cell_width,
                      const double _scale,
                      const std::string& _bg_color="white")
    : dpi(72.0), 
      cell_width(_cell_width),
      cell_height(_cell_width*_scale),
      scale(_scale),
      standardCharacters(),
      bg_color(_bg_color)
  {}
  /**
   * Return an image of a ptypeO.
   */
  ImagePtr image(const osl::PtypeO& ptypeO) const;
  /**
   * Return an image of a representing string.
   */
  ImagePtr image(const std::string& s) const;
};

struct MetaData
{
  std::string title;
  std::string black_name;
  std::string white_name;
  std::string last_move;
};

enum Threat {
  NONE,
  THREATMATE,
  CHECKMATE
};

/**
 * BoardImage reprensets an image of a board.
 */
class BoardImage
{
private:
  const osl::SimpleState state;
  const osl::record::StandardCharacters standardCharacters;
  const osl::Move last_move;
  const std::string title;
  const std::string black_name;
  const std::string white_name;
  const Threat threat;
  double scale; /// ratio of width to height
  double meta_line_height;
  double image_width;
  double image_height;
  double board_origin_x;
  double board_origin_y;
  double board_width;
  double board_hegiht;
  double cell_width;
  double cell_height;
  std::unique_ptr<Magick::Image> image;

  /**
   * Return a piece string in Black's stand
   */
  std::string blackStand() const;
  /**
   * Return a piece string in White's stand
   */
  std::string whiteStand() const;
  /**
   *
   */
  void writeLine(const double start_x, const double start_y,
                 const std::string& line);
public:
  /**
   * Constructor.
   * @param _state a state to be drawn (after the last move)
   * @param _last_move the last move.
   */
  explicit BoardImage(const osl::SimpleState& _state,
                      const osl::Move _last_move,
                      const std::string& _title,
                      const std::string& _black_name,
                      const std::string& _white_name,
                      Threat _threat = Threat::NONE)
    : state(_state),
      standardCharacters(),
      last_move(_last_move),
      title(_title),
      black_name(_black_name),
      white_name(_white_name),
      threat(_threat)
  {
    scale          = 1.1; // width-height ratio of the board
    image_width    = 297.0;
    board_origin_x = image_width*0.2;
    board_width    = image_width - board_origin_x*2.0;
    board_hegiht   = board_width*scale;
    cell_width     = board_width/9.0;
    cell_height    = cell_width*scale;
    meta_line_height = cell_height;
    board_origin_y = meta_line_height;
    image_height   = image_width;
    if (!title.empty())
      image_height += meta_line_height;
    if (!black_name.empty())
      image_height += meta_line_height;
    if (!white_name.empty())
      image_height += meta_line_height;
    if (last_move.isNormal())
      image_height += meta_line_height;
  }
  /**
   * Draw an image in a memory area
   */
  void draw();
  /**
   * Write an image to Magick::Blob in memory.
   * Draw() must be called beforehand.
   */
  void write(Magick::Blob& blob, const std::string& file_type="PNG") const;
};

class ProcessBoard
{
  const int checkmate_limit;
public:
  explicit ProcessBoard(int _checkmate_limit)
    : checkmate_limit(_checkmate_limit)
  {}
  /**
   * Generate an image of a board_id in a memory area.
   */
  bool generate(const std::string& line,
                const MetaData& md,
                Magick::Blob& blob) const;

private:
  /**
   * Valid syntax is one of 
   *   1. board only: sfen <sfen string without move-th>
   *   2. board and last move: sfen <sfen string with move-th> moves <move1>
   *   3. start position and moves: position startpos moves <move1> ...
   *
   * For the (1) format, move1 gets empty.
   *
   * ex. sfen 1+Pnlr1p1l/1+R4gk1/2S3np1/+B1g1GB2p/2nP2PP1/+l1K1P4/1+n3P2P/5S3/5G2L w 2S8P 1
   * ex. sfen 1+Pnlr1p1l/1+R4gk1/2S3np1/+B1g1GB2p/2nP2PP1/+l1K1P4/1+n3P2P/5S3/5G2L w 2S8P 1 moves 7g7f
   * ex. position startpos moves 7g7f
   *
   * @return true for a valid syntax; false otherwise.
   */
  bool splitMove(const std::string& source,
                 std::string& sfen_string,
                 std::string& move1) const;
  /**
   * If a string sfen begins with "sfen ", eliminate the characters; return
   * a string sfen as it is, otherwise.
   */
  std::string eliminateSfen(const std::string& sfen) const;
  /**
   * See if an sfen consists of the characters allowed for the escaped format.
   *
   * Return true if valid; otherwise, false
   */
  bool isPossibleSfenCharacters(const std::string& sfen) const;
  /**
   * See if a move string str consists of the characters allowed for the escaped format.
   *
   * Return true if valid; otherwise, false
   */
  bool isPossibleSfenMoveCharacters(const std::string& str) const;
  /**
   * Generate a PNG image file of a board specifyed by a line.
   * @param line an SFEN string.
   *        ex. sfen 1+Pnlr1p1l/1+R4gk1/2S3np1/+B1g1GB2p/2nP2PP1/+l1K1P4/1+n3P2P/5S3/5G2L w 2S8P 1
   *        ex. sfen 1+Pnlr1p1l/1+R4gk1/2S3np1/+B1g1GB2p/2nP2PP1/+l1K1P4/1+n3P2P/5S3/5G2L w 2S8P 1 moves 7g7f
   *        ex. position startpos moves 7g7f
   * @return an instance of BoardImage. If a line is invalid, return a null pointer.
   */
  std::shared_ptr<BoardImage> generate_image(const std::string& line,
                                               const MetaData& md) const;
  bool isCheckmate(const osl::SimpleState& state) const;
  bool isThreatmate(const osl::SimpleState &state) const;
};

#endif /* _GPS_POS2IMG_H */

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
