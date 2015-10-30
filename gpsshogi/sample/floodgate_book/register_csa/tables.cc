#include "tables.h"
#include "exception.h"
#include "logging.h"
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>

namespace bf = boost::filesystem;

typedef std::unique_ptr<sql::Statement> StatementPtr;
typedef std::unique_ptr<sql::PreparedStatement> PreparedStatementPtr;
typedef std::unique_ptr<sql::ResultSet> ResultSetPtr;

extern std::unique_ptr<sql::Connection> con;

std::string readFile(const std::string& path)
{
  std::ifstream ifs(path, std::ifstream::binary);
  std::filebuf* pbuf = ifs.rdbuf();
  const std::size_t size = pbuf->pubseekoff (0,ifs.end,ifs.in);
  if (size == 0) {
    return "";
  }
  pbuf->pubseekpos (0, ifs.in);
  std::vector<char> buffer(size);
  pbuf->sgetn(&buffer[0], buffer.size());
  ifs.close();

  return std::string(&buffer[0], buffer.size()); 
}

uint64_t selectLastAutoIncrementId()
{
  const static std::string method = "selectLastAutoIncrementId";

  StatementPtr stmt(con->createStatement());
  ResultSetPtr rs(stmt->executeQuery("select LAST_INSERT_ID()"));
  if (rs->next()) {
    return rs->getUInt64(1);
  } else {
    const std::string msg = "Failed to select LAST_INSERT_ID";
    logError(method, msg);
    throw GpsException(msg);
  }
}

boost::optional<Game> selectGameByName(const std::string& game_name)
{
  PreparedStatementPtr stmt(con->prepareStatement(
    "SELECT game_name, game_id, timestamp FROM GAMES where game_name = ?"));
  stmt->setString(1, game_name);
  ResultSetPtr rs(stmt->executeQuery());
  if (rs->next()) {
    const Game game(rs->getString(1), rs->getUInt(2), rs->getString(3));
    return boost::make_optional(game);
  } else {
    return boost::optional<Game>();
  }
}

boost::optional<Game> selectGameById(const uint32_t game_id)
{
  PreparedStatementPtr stmt(con->prepareStatement(
    "SELECT game_name, timestamp, kifu_file FROM GAMES where game_id = ?"));
  stmt->setUInt(1, game_id);
  ResultSetPtr rs(stmt->executeQuery());
  if (rs->next()) {
    const Game game(rs->getString(1), game_id, rs->getString(2), rs->getString(3));
    return boost::make_optional(game);
  } else {
    return boost::optional<Game>();
  }
}

Game addGame(const std::string& csa_file)
{
  const static std::string method = "addGame";

  const bf::path file(csa_file);
  const std::string game_name = file.filename().replace_extension("").string();
  
  try {
    const boost::optional<Game> game = selectGameByName(game_name);
    if (game) return *game;

    const std::string content = readFile(csa_file);

    // Parse start time
    std::string timestamp;
    boost::regex r("^\\$START_TIME:(\\d{4}/\\d{2}/\\d{2} \\d{2}:\\d{2}:\\d{2})");
    boost::smatch what;
    if (boost::regex_search(content.begin(), content.end(), what, r)) {
      timestamp.assign(what[1].first, what[1].second);
    } else {
      const boost::format msg = boost::format("START_TIME not found: %s") % csa_file;
      logError(method, msg);
      throw GpsException(msg);
    }

    PreparedStatementPtr stmt(con->prepareStatement(
      "INSERT INTO GAMES(game_name, kifu_file, timestamp) VALUES (?,?,?) "
      "ON DUPLICATE KEY UPDATE kifu_file = VALUES(kifu_file), timestamp = VALUES(timestamp)"));
    stmt->setString(1, game_name);
    stmt->setString(2, content);
    stmt->setString(3, timestamp);
    stmt->execute();

    // Workaround a weird issue; a kifu string in the database sometimes
    // gets truncated. It might be a bug of MySQL Connector. If a truncated
    // string is found, just retry making a game record. 
    /*
    bool success = false;
    std::string result_content;
    for (int retry=0; retry<10; ++retry) {
      PreparedStatementPtr stmt(con->prepareStatement(
        "INSERT INTO GAMES(game_name, kifu_file, timestamp) VALUES (?,?,?) "
        "ON DUPLICATE KEY UPDATE kifu_file = VALUES(kifu_file), timestamp = VALUES(timestamp)"));
      stmt->setString(1, game_name);
      stmt->setString(2, content);
      stmt->setString(3, timestamp);
      stmt->execute();

      PreparedStatementPtr stmt2(con->prepareStatement(
        "SELECT kifu_file FROM GAMES where game_name = ?"));
      stmt2->setString(1, game_name);
      ResultSetPtr rs(stmt2->executeQuery());
      if (rs->next()) {
        result_content = rs->getString(1);
        if (result_content.size() >= content.size()) {
          success = true;
          break;
        }
      }
      logWarn(method, boost::format("Retry addGame: %d") % retry);
    } // for retry

    if (!success) {
      logError(method, boost::format("Adding a game %s cuases a kifu string truncated: %d/%d") % game_name % result_content.size() % content.size());
    }
    */

    const uint32_t game_id = selectLastAutoIncrementId();
    return Game(game_name, game_id, timestamp);
  } catch (sql::SQLException& e) {
    logError(method, e);
    throw GpsException(e);
  }
}

boost::optional<uint64_t> selectPositionId(const std::string& sfen)
{
  PreparedStatementPtr stmt(con->prepareStatement(
    "SELECT position_id FROM POSITIONS WHERE sfen = ?"));
  stmt->setString(1, sfen);
  ResultSetPtr rs(stmt->executeQuery());
  if (rs->next())
    return boost::make_optional(rs->getUInt64(1));
  else
    return boost::optional<uint64_t>();
}

uint64_t addPosition(const std::string& sfen)
{
  const static std::string method = "addPosition";

  try {
    boost::optional<uint64_t> position_id = selectPositionId(sfen);
    if (position_id) return *position_id;

    PreparedStatementPtr stmt(con->prepareStatement(
      "INSERT INTO POSITIONS(sfen) VALUES (?)"));
    stmt->setString(1, sfen);
    stmt->execute();

    return selectLastAutoIncrementId();
  } catch (sql::SQLException& e) {
    logError(method, e);
    throw GpsException(e);
  }
}

boost::optional<Moves> selectMoves(const uint32_t game_id, const uint32_t nth)
{
  PreparedStatementPtr stmt(con->prepareStatement(
    "SELECT move, position_id FROM MOVES WHERE game_id = ? AND nth = ?"));
  stmt->setUInt(1, game_id);
  stmt->setUInt(2, nth);
  ResultSetPtr rs(stmt->executeQuery());
  if (rs->next()) {
    const Moves moves(game_id,
                      nth,
                      rs->getString(1),
                      rs->getUInt64(2));
    return boost::make_optional(moves);
  } else {
    return boost::optional<Moves>();
  }
}

bool selectMovesByPositionId(const uint64_t position_id,
                             const std::string& timestamp,
                             std::vector<Moves>& ret)
{
  const static std::string method = "selectMovesByPositionId";

  try {
    PreparedStatementPtr stmt(con->prepareStatement(
      "SELECT m.game_id, m.nth, m.move FROM MOVES as m JOIN GAMES as g ON (m.game_id = g.game_id) WHERE m.position_id = ? AND g.timestamp < ?"));
    stmt->setUInt64(1, position_id);
    stmt->setString(2, timestamp);
    ResultSetPtr rs(stmt->executeQuery());
    while (rs->next()) {
      const Moves moves(rs->getUInt(1),
                        rs->getUInt(2),
                        rs->getString(3),
                        position_id);
      ret.push_back(moves);
    }
  } catch (sql::SQLException& e) {
    logError(method, e);
    throw GpsException(e);
  }

  return true;
}

bool addMoves(const Moves& moves)
{
  const static std::string method = "addMoves";

  try {
    PreparedStatementPtr stmt(con->prepareStatement(
      "INSERT INTO MOVES(game_id, nth, move, position_id) VALUES (?,?,?,?)"));
    stmt->setUInt(1, moves.game_id);
    stmt->setUInt(2, moves.nth);
    stmt->setString(3, moves.move);
    stmt->setUInt64(4, moves.position_id);
    stmt->execute();
  } catch (sql::SQLException& e) {
    if (e.getErrorCode() == ER_DUP_ENTRY) {
      const std::string msg = "Duplicate entry in MOVES";
      logInfo(method, msg);
      return false;
    }
    logError(method, e);
    throw GpsException(e);
  }

  return true;
}

uint32_t countGames(const Game& game, uint64_t position_id)
{
  const static std::string method = "countGames";

  try {
    PreparedStatementPtr stmt(con->prepareStatement(
      "SELECT COUNT(DISTINCT m.game_id) "
      "  FROM MOVES as m JOIN GAMES as g ON (m.game_id = g.game_id) "
      " WHERE m.game_id != ? AND m.position_id = ? AND g.timestamp < ? "));
    stmt->setUInt(1, game.game_id);
    stmt->setUInt64(2, position_id);
    stmt->setDateTime(3, game.timestamp);
    ResultSetPtr rs(stmt->executeQuery());
    rs->next();
    return rs->getUInt(1);    
  } catch (sql::SQLException& e) {
    logError(method, e);
    throw GpsException(e);
  }
}

bool pickUpMoveEntry(const std::string& table, MoveEntry& new_moves)
{
  const static std::string method = "pickUpMoveEntry/" + table;

  try {
    StatementPtr stmt(con->createStatement());
    ResultSetPtr rs(stmt->executeQuery(
      "SELECT game_id, nth, timestamp from " + table +
      " WHERE flag_post = 0 ORDER BY timestamp LIMIT 1;"));
    if (!rs->next()) {
      return false;
    }
    new_moves.game_id   = rs->getUInt(1);
    new_moves.nth       = rs->getUInt(2);
    new_moves.timestamp = rs->getString(3);

    return true;
  } catch (sql::SQLException& e) {
    logError(method, e);
    throw GpsException(e);
  }
}

bool setFlagMoveEntry(const std::string& table, const MoveEntry& new_moves)
{
  const static std::string method = "setFlagMoveEntry/" + table;

  try {
    PreparedStatementPtr stmt(con->prepareStatement(
      "UPDATE " + table + " SET flag_post = 1 WHERE game_id = ? AND nth = ?"));
    stmt->setUInt(1, new_moves.game_id);
    stmt->setUInt(2, new_moves.nth);
    stmt->execute();

    return true;
  } catch (sql::SQLException& e) {
    logError(method, e);
    throw GpsException(e);
  }
}

bool addMoveEntry(const std::string& table, uint32_t game_id, uint32_t nth)
{
  const static std::string method = "addMove/" + table;

  try {
    PreparedStatementPtr stmt(con->prepareStatement(
      "INSERT INTO " + table +
      " (game_id, nth, flag_post, timestamp) VALUES (?,?,0,NOW())"));
    stmt->setUInt(1, game_id);
    stmt->setUInt(2, nth);
    stmt->execute();
    return true;
  } catch (sql::SQLException& e) {
    if (e.getErrorCode() == ER_DUP_ENTRY) {
      logWarn(method, "Duplicate entry in NEW_MOVES");
      return false;
    }
    logError(method, e);
    throw GpsException(e);
  }
}
