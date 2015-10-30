#include "osl/numEffectState.h"
#include "osl/hashKey.h"
#include "osl/csa.h"
#include "osl/record/kisen.h"
#include "osl/record/csaRecord.h"

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <unordered_set>
#include <iostream>
#include <fstream>

struct hash
{
  unsigned long operator() (const osl::SimpleState &state) const
  {
    return osl::hash::HashKey(state).signature();
  }
};

class StatePredicate
{
public:
  StatePredicate(const std::vector<std::string> &filenames) { }
  virtual ~StatePredicate() { }
  virtual bool match (const osl::NumEffectState &state) const
  {
    return false;
  }
  virtual bool isLoaded() const { return false; }
};

class CsaPredicate : public StatePredicate
{
public:
  CsaPredicate(const std::vector<std::string> &filenames)
    : StatePredicate(filenames)
  {
    for (size_t i = 0; i < filenames.size(); ++i)
    {
      osl::CsaFile file(filenames[i]);
      states.insert(file.initialState());
    }
  }
  ~CsaPredicate() { }
  bool match (const osl::NumEffectState &state) const
  {
    return states.find(state) != states.end();
  }
  bool isLoaded() const
  {
    return !states.empty();
  }
private:
  std::unordered_set<osl::SimpleState, hash> states;
};

class PieceStandPredicate : public StatePredicate
{
private:
  bool match(const osl::NumEffectState &state, osl::Player player) const
  {
    return state.countPiecesOnStand<osl::ROOK>(player) == 1 &&
      state.countPiecesOnStand<osl::BISHOP>(player) == 1 &&
      state.countPiecesOnStand<osl::GOLD>(player) == 0 &&
      state.countPiecesOnStand<osl::SILVER>(player) == 1 &&
      state.countPiecesOnStand<osl::KNIGHT>(player) == 3 &&
      state.countPiecesOnStand<osl::LANCE>(player) == 3;
  }
public:
  PieceStandPredicate(const std::vector<std::string> &filenames)
    : StatePredicate(filenames) { }
  bool match (const osl::NumEffectState &state) const
  {
    return match(state, osl::BLACK) || match(state, osl::WHITE);
  }
  bool isLoaded() const { return true; }
};

int main(int argc, char **argv)
{
  std::string kisen_filename, predicate_name;
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("kisen",
     boost::program_options::value<std::string>(&kisen_filename)->
     default_value(""),
     "Kisen filename to search")
    ("predicate",
     boost::program_options::value<std::string>(&predicate_name)->
     default_value("csa"),
     "Predicate to use.  Valid options are csa and stand")
    ("input-file", boost::program_options::value< std::vector<std::string> >(),
     "input files in kisen format")
    ("help", "Show help message");
  boost::program_options::variables_map vm;
  boost::program_options::positional_options_description p;
  p.add("input-file", -1);

  try
  {
    boost::program_options::store(
      boost::program_options::command_line_parser(
	argc, argv).options(command_line_options).positional(p).run(), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
      std::cerr << "Usage: " << argv[0] << " [options] CSA_FILES"
		<< std::endl;
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0] << " [options] CSA_FILES" << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  std::vector<std::string> files;

  if (vm.count("input-file"))
    files = vm["input-file"].as< std::vector<std::string> >();

  std::unique_ptr<StatePredicate> predicate;
  if (predicate_name == "csa")
  {
    predicate.reset(new CsaPredicate(files));
  }
  else if (predicate_name == "stand")
  {
    predicate.reset(new PieceStandPredicate(files));
  }
  else
  {
    std::cerr << "Unknown predicate "  << predicate_name;
    return 1;
  }

  if (!predicate->isLoaded())
  {
    std::cerr << "No target" << std::endl;
  }
  osl::record::KisenFile kisen(kisen_filename);
  for (size_t i = 0; i < kisen.size(); i++)
  {
    const auto moves = kisen.moves(i);
    auto state = kisen.initialState();
    for (size_t j = 0; j < moves.size(); j++)
    {
      const osl::Square opKingSquare 
	= state.kingSquare(alt(state.turn()));
      if (state.hasEffectAt(state.turn(), opKingSquare))
      {
	break;
      }
      state.makeMove(moves[j]);
      if (predicate->match(state))
      {
	std::cout << i << " " << j << std::endl << state;
      }
    }
  }

  return 0;
}
