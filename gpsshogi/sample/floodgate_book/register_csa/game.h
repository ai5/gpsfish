#ifndef _REGISTER_CSA_GAME_H
#define _REGISTER_CSA_GAME_H

#include "osl/basic_type.h"
#include "osl/record/csaRecord.h"
#include <boost/optional.hpp>
#include <memory> // shared_ptr
#include <string>
#include <vector>

struct Game
{
  std::string game_name;
  uint32_t game_id;
  std::string timestamp;
  std::string kifu;
  std::shared_ptr<osl::record::CsaFile> csa_file;

  Game(const std::string& _game_name,
       uint32_t _game_id,
       const std::string& _timestamp,
       const std::string& _kifu = "")
    : game_name(_game_name), game_id(_game_id), timestamp(_timestamp), kifu(_kifu)
  {}

  std::vector<osl::Move> moves();
  boost::optional<osl::Player> winner();
  boost::optional<std::string> winnerName();
  boost::optional<std::string> loserName();

  std::string blackPlayer();
  std::string whitePlayer();

private:
  void loadFile();

};


#endif /* _REGISTER_CSA_GAME_H */
