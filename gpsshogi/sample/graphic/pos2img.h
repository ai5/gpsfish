#ifndef _GPS_POS2IMG_H
#define _GPS_POS2IMG_H

#include "osl/ptype.h"
#include "osl/record/kanjiPrint.h"
#include "osl/state/simpleState.h"
#include <Magick++.h> 
#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
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


/**
 * BoardImage reprensets an image of a board.
 */
class BoardImage
{
private:
  const osl::SimpleState state;
  const osl::record::StandardCharacters standardCharacters;
  const osl::Move last_move;
  const bool is_threatmate;
  double scale; /// ratio of width to height
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
public:
  /**
   * Constructor.
   * @param _state a state to be drawn (after the last move)
   * @param _last_move the last move.
   */
  explicit BoardImage(const osl::SimpleState& _state,
                      const osl::Move _last_move=osl::Move(),
                      bool _is_threatmate=false)
    : state(_state),
      standardCharacters(),
      last_move(_last_move),
      is_threatmate(_is_threatmate)
  {
    scale          = 1.1; // width-height ratio of the board
    image_width    = 297.0;
    image_height   = image_width*scale;
    board_origin_x = image_width*0.2;
    board_origin_y = board_origin_x*scale;
    board_width    = image_width - board_origin_x*2.0;
    board_hegiht   = board_width*scale;
    cell_width     = board_width/9.0;
    cell_height    = cell_width*scale;
  }
  /**
   * Draw an image in a memory area
   */
  void draw();
  /** 
   * Write an in-memory image to a file.
   * Draw() must be called beforehand.
   */
  void write(const std::string& file_name) const;
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
   * Generate an image of a board_id as a file under a directory.
   */
  std::string generate(const std::string& line,
                       const boost::filesystem::path& dir) const;
  /**
   * Generate an image of a board_id in a memory area.
   */
  std::string generate(const std::string& line,
                       Magick::Blob& blob) const;

private:
  /**
   * Valid syntax (in the escaped format):
   * 1. board only: sfen <sfenstring without moveth>
   * 2. board and last move: sfen <sfenstring with move-th> moves <move1>
   *
   * For the (1) format, move1 is empty.
   *
   * ex. sfen.lnsgkgsnl_1r5b1_ppppppppp_9_9_9_PPPPPPPPP_1B5R1_LNSGKGSNL.b.-.png
   * ex. sfen.lnsgkgsnl_1r5b1_ppppppppp_9_9_9_PPPPPPPPP_1B5R1_LNSGKGSNL.b.-.1.moves.7g7f.png
   * ex. sfen.l4g2l_1r4sk1_p3p2pp_1p3bp2_5n1N1_3P2P2_PP2PP1PP_4K2GG_LNSG2@l1@r.w.N3Pb2sp.1.moves.B*7f.png
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
   * Generate a png image file of a board specifyed by a line.
   * @param line an Escaped SFEN string.
   *        ex. sfen.lnsgkgsnl_1r5b1_ppppppppp_9_9_9_PPPPPPPPP_1B5R1_LNSGKGSNL.b.-
   * @param file_name [out]
   *        ex. sfen.lnsgkgsnl_1r5b1_ppppppppp_9_9_9_PPPPPPPPP_1B5R1_LNSGKGSNL.b.-.png
   * @return an instance of BoardImage. If a line is invalid, return a null pointer.
   */
  std::shared_ptr<BoardImage> generate_image(const std::string& line,
                                               std::string& file_name) const;
  bool checkmateEscape(const osl::SimpleState& state) const;
};

#endif /* _GPS_POS2IMG_H */

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
