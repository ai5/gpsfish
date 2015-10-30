#ifndef _OSL_RECORD_CHECK_DUPLICATE_H
#define _OSL_RECORD_CHECK_DUPLICATE_H

#include "osl/basic_type.h"
#include "osl/hashKey.h"
#include "osl/pathEncoding.h"

#include <unordered_map>
#include <deque>
#include <vector>

namespace osl
{
  namespace record
  {
    /**
     * Container of moves to check duplicated games.
     */
    class CheckDuplicate
    {
      typedef std::unordered_map<HashKey, std::vector<PathEncoding>, std::hash<HashKey>> keymap_t;
      /** container of moves */
      keymap_t keys;
      /** couter for registing (trials) */
      size_t regist_counter;
      /** counter for hash matches with different moves */
      size_t duplicated_hash_counter;
      /** counter for exact matches of moves */
      size_t duplicated_moves_counter;

    public:
      static std::pair<HashKey,PathEncoding> getLastState(const std::vector<Move>& moves);

      /**
       * Result type of checking duplicates.
       */
      enum DUPLICATE_RESULT
      {
        NO_DUPLICATE    = 0,
        HASH_DUPLICATE  = 1,
        MOVES_DUPLICATE = 2
      };

      /**
       * Constructor
       */
      CheckDuplicate()
        : regist_counter(0),
          duplicated_hash_counter(0), 
          duplicated_moves_counter(0)
      {}

      /**
       * Insert a key if the key is new. The key is the last state of the
       * moves.
       *
       * @param moves 
       * @return false if (i) the key is new or (ii) duplicated with 
       * different moves; true if the moves are exactly found in this
       * container. 
       */
      DUPLICATE_RESULT regist(const std::vector<Move>& moves);
      
      /**
       * Output the result
       */
      void print(std::ostream& out) const;

      /**
       * Return a couter of registings (trials)
       */
      size_t getRegists() const
      { return regist_counter; }

      /**
       * Return a counter of duplicated ending states. The moves may or may
       * not match.
       */
      size_t getDuplicatedHash() const
      { return duplicated_hash_counter; }

      /**
       * Return a counter of duplicated moves. The moves are exactly same.
       */
      size_t getDuplicatedMoves() const
      { return duplicated_moves_counter; }

    private:
      /**
       * Insert a key if the key is new.
       *
       * @param key a hash key of the last state of the moves
       * @param moves 
       * @return false if (i) the key is new or (ii) duplicated with 
       * different moves; true if the moves are exactly found in this
       * container. 
       */
      DUPLICATE_RESULT regist(const HashKey& key, 
                              const PathEncoding& moves);
    };

  } // namespace record
} // namespace osl


#endif /* _OSL_RECORD_CHECK_DUPLICATE_H */

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
