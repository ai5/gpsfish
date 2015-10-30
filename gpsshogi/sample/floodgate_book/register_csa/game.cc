#include "game.h"

void
Game::loadFile()
{
  std::istringstream file(kifu);
  if (!csa_file) {
    csa_file.reset(new osl::record::CsaFile(file));
  }
}

std::vector<osl::Move>
Game::moves()
{
  loadFile();
  return csa_file->moves();
}

std::string
Game::blackPlayer()
{
  loadFile();
  const osl::record::Record record = csa_file->load();
  return record.player[osl::BLACK];
}

std::string
Game::whitePlayer()
{
  loadFile();
  const osl::record::Record record = csa_file->load();
  return record.player[osl::WHITE];
}

boost::optional<osl::Player>
Game::winner()
{
  loadFile();
  const osl::record::Record record = csa_file->load();
  const osl::record::Record::ResultType result = record.result;
  switch (result) {
  case osl::record::Record::ResultType::BlackWin:
    return boost::make_optional(osl::BLACK);
  case osl::record::Record::ResultType::WhiteWin:
    return boost::make_optional(osl::WHITE);
  default:
    return boost::optional<osl::Player>();
  }
}

boost::optional<std::string>
Game::winnerName()
{
  loadFile();
  const osl::record::Record record = csa_file->load();
  const osl::record::Record::ResultType result = record.result;
  switch (result) {
  case osl::record::Record::ResultType::BlackWin:
    return boost::make_optional(blackPlayer());
  case osl::record::Record::ResultType::WhiteWin:
    return boost::make_optional(whitePlayer());
  default:
    return boost::optional<std::string>();
  }
}

boost::optional<std::string>
Game::loserName()
{
  loadFile();
  const osl::record::Record record = csa_file->load();
  const osl::record::Record::ResultType result = record.result;
  switch (result) {
  case osl::record::Record::ResultType::BlackWin:
    return boost::make_optional(whitePlayer());
  case osl::record::Record::ResultType::WhiteWin:
    return boost::make_optional(blackPlayer());
  default:
    return boost::optional<std::string>();
  }
}

