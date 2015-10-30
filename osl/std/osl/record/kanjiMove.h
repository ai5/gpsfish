/* kajiMove.h
 */
#ifndef OSL_RECORD_KANJIMOVE_H
#define OSL_RECORD_KANJIMOVE_H

#include "osl/numEffectState.h"
#include <unordered_map>
#include <string>
#include <list>

namespace osl
{
  namespace record
  {
    /**
     * Parse kanji records such as "７六歩", the style of which is 
     * generally used to write Shogi records in Japanese.
     */
    class KanjiMove
    {
    public:
      KanjiMove();
      ~KanjiMove();

      /**
       * Convert a Japanese string (one token) to a move object
       */
      const Move strToMove(const std::string&, 
                                const NumEffectState& state, 
                                const Move& last_move) const;
      void setVerbose(bool verbose) {this->verbose = verbose;}

      Square toSquare(const std::string&) const;
      Ptype toPtype(const std::string&) const;

      static const KanjiMove& instance();
    private:
      typedef std::list<Move> found_moves_t;
      void selectCandidates(found_moves_t& found, 
                            std::string& str, 
                            const Square& to_pos,
                            const Player& player) const;
      typedef std::unordered_map<std::string, Square>
      str2position_t;
      str2position_t str2position;
      typedef std::unordered_map<std::string, Ptype> str2piece_t;
      str2piece_t str2piece;
      bool verbose;
    };
  } // record
  using record::KanjiMove;
} // osl

#endif /* OSL_RECORD_KANJIMOVE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
