#include <tcutil.h>
#include <tchdb.h>

#include "moves.pb.h"
#include "osl/record/compactBoard.h"
#include "osl/apply_move/applyMove.h"
#include "osl/state/numEffectState.h"
#include "osl/search/alphaBeta2.h"
#include "osl/search/moveWithComment.h"
#include "osl/search/simpleHashTable.h"
#include "osl/stl/vector.h"
#include "osl/record/csaRecord.h"
#include "gpsshogi/dbm/tokyoCabinet.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <sstream>

namespace po = boost::program_options;
using namespace osl;

class NextState
{
public:
  virtual std::string nextState() = 0;
};

class FileNameNextState : public NextState
{
public:
  FileNameNextState(const std::vector<std::string> &f,
	   boost::mutex &m) : files(f), mutex(m), index(0) { }
  std::string nextState()
  {
    boost::mutex::scoped_lock lock(mutex);
    if (index < files.size())
    {
      return files[index++];
    }
    return "";
  }
private:
  const std::vector<std::string> &files;
  boost::mutex &mutex;
  size_t index;
};

class BoostDirNextState : public NextState
{
public:
  BoostDirNextState(const std::string &dir,
		    boost::mutex &m) : dir_it(dir), mutex(m) { }
  std::string nextState()
  {
    boost::mutex::scoped_lock lock(mutex);
    if (dir_it != boost::filesystem::directory_iterator())
    {
      std::string path = dir_it->string();
      ++dir_it;
      return path;
    }
    return "";
  }
private:
  boost::filesystem::directory_iterator dir_it;
  boost::mutex &mutex;
};

struct ComputePV
{
  ComputePV(gpsshogi::dbm::TokyoCabinetWrapper *t, int l, NextState *next)
    : db(t), limit(l), next_state(next) { }

  void operator()()
  {
    std::string filename;
    while (!(filename = next_state->nextState()).empty())
    {
      CsaFile file(filename);
      NumEffectState state = file.getInitialState();
      Record r = file.getRecord();
      const vector<Move> moves = r.getMoves();
      MoveStack history;

      for (size_t i = 0; i < moves.size(); ++i)
      {
	history.push(moves[i]);
	ApplyMoveOfTurn::doMove(state, moves[i]);
      }
      record::CompactBoard board(state);
      std::ostringstream oss(std::ostringstream::out);
      oss << board;
      const std::string &key = oss.str();

      search::SearchState2::checkmate_t checker;
      osl::CountRecorder recorder;

      SimpleHashTable table(std::numeric_limits<size_t>::max(), 200);
      table.setVerbose(2); // debug
      AlphaBeta2OpenMidEndingEval searcher(state, checker, &table, recorder);
      searcher.setHistory(history);

      MoveWithComment pv;
      const Move best_move = searcher.computeBestMoveIteratively(
	limit, 200, 400,
	std::numeric_limits<size_t>::max(),
	search::TimeAssigned(milliseconds(120*120)),
	&pv);
      Moves pv_moves;
      pv_moves.add_moves(best_move.intValue());
      for (size_t j = 0; j < pv.moves.size(); ++j)
      {
	pv_moves.add_moves(pv.moves[j].intValue());
      }
      std::string value;
      pv_moves.SerializeToString(&value);
      if (!db->put(key, value))
      {
	std::cerr << "failed to write " << state << best_move << std::endl;
      }
    }
  }

private:
  gpsshogi::dbm::TokyoCabinetWrapper *db;
  int limit;
  NextState *next_state;
};

int main(int argc, char **argv)
{
  int limit;
  int num_threads;
  std::string input_dir;

  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("search-limit,l",
     po::value<int>(&limit)->default_value(1200),
     "maximum limit of search")
    ("input-file", po::value<std::vector<std::string> >(),
     "input CSA files")
    ("input-dir", po::value<std::string>(&input_dir),
     "input directory with CSA files")
    ("threads,N", po::value<int>(&num_threads)->default_value(1),
     "number of threads to use")
    ;

  po::positional_options_description p;
  p.add("input-file", -1);

  std::vector<std::string> files;
  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).
	      options(command_line_options).positional(p).run(),
	      vm);
    po::notify(vm);
    if (vm.count("input-file"))
    {
      files = vm["input-file"].as<std::vector<std::string> >();
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  if (!eval::ml::OpenMidEndingEval::setUp())
  {
    std::cerr << "Failed to load eval data" << std::endl;
    return 1;
  }
  if (!progress::ml::NewProgress::setUp())
  {
    std::cerr << "Failed to load progress data" << std::endl;
    return 1;
  }

  if (!input_dir.empty() && !files.empty())
  {
    std::cerr << "input-file and input-dir cannot be specified at the "
	      << "same time" << std::endl;
    return 1;
  }
  else if (input_dir.empty() && files.empty())
  {
    std::cerr << "input-file or input-dir needs to be specified"
	      << std::endl;
    return 1;
  }

  gpsshogi::dbm::TokyoCabinetWrapper wrapper("pvs.tch",
					     HDBOWRITER | HDBOCREAT);

  std::unique_ptr<NextState> next_state;
  boost::mutex mutex;
  if (!files.empty())
  {
    next_state.reset(new FileNameNextState(files, mutex));
  }
  else
  {
    next_state.reset(new BoostDirNextState(input_dir, mutex));
  }
  boost::ptr_vector<boost::thread> threads;
  for (int i = 0; i < num_threads; ++i)
  {
    threads.push_back(new boost::thread(ComputePV(&wrapper, limit,
						  next_state.get())));
  }
  for (int i = 0; i < num_threads; ++i)
  {
    threads[i].join();
  }

  return 0;
}
