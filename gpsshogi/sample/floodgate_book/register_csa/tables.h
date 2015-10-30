#ifndef _REGIST_CSA_TABLES_H
#define _REGIST_CSA_TABLES_H

#include "game.h"
#include <boost/optional.hpp>
#include <iostream>
#include <string> 

class Game;

struct Moves
{
  uint32_t game_id;
  uint32_t nth;         // stats with 1
  std::string move;
  uint64_t position_id;
  uint32_t think_time;
  std::string comment;

  Moves(uint32_t _game_id,
        uint32_t _nth,
        const std::string& _move,
        uint64_t _position_id,
        uint32_t _think_time = 0,
        const std::string& _comment = "")
    : game_id(_game_id), nth(_nth), move(_move), position_id(_position_id),
      think_time(_think_time), comment(_comment)
  {}

  friend std::ostream& operator<<(std::ostream& out, const Moves& m)
  {
    out << "game_id: " << m.game_id << ", " <<
           "nth: " << m.nth << ", " <<
           "move: " << m.move << ", " <<
           "position_id: " << m.position_id;
    return out;
  }
};


struct MoveEntry
{
  uint32_t game_id;
  uint32_t nth;
  std::string timestamp;
};

uint64_t selectLastAutoIncrementId();
boost::optional<uint32_t> selectGameId(const std::string& game_name);
boost::optional<Game> selectGameById(const uint32_t game_id);
Game addGame(const std::string& csa_file);
boost::optional<uint64_t> selectPositionId(const std::string& sfen);
uint64_t addPosition(const std::string& sfen);
boost::optional<Moves> selectMoves(const uint32_t game_id, const uint32_t nth);

/**
 * Select a result set from MOVES which have the same position and took place before
 * a timestamp.
 */
bool selectMovesByPositionId(const uint64_t position_id,
                             const std::string& timestamp,
                             std::vector<Moves>& ret);
/**
 * Returns true if an insertion takes place; otherwise, false.
 */
bool addMoves(const Moves& moves);

uint32_t countGames(const Game& game, uint64_t position_id);

/**
 * Add a new entry in a table.
 *
 * nth: n-th move, starting with 1.
 */
bool addMoveEntry(const std::string& table, uint32_t game_id, uint32_t nth);

/**
 * Select a single new game in order of timestamp.
 */
bool pickUpMoveEntry(const std::string& table, MoveEntry& new_moves);

/**
 * Set a New Move flag since a new_moves is finished and no longer a new
 * move. 
 */
bool setFlagMoveEntry(const std::string& table, const MoveEntry& new_moves);

#endif /* _REGIST_CSA_TABLES_H */
