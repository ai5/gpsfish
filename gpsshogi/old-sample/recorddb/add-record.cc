/* add-record.cc
 */
#include "gpsshogi/recorddb/recordDb.h"
#include "osl/state/numEffectState.h"
#include "osl/apply_move/applyMove.h"
#include "osl/record/kakinoki.h"
#include "osl/record/ki2.h"
#include "osl/record/ki2IOError.h"
#include "osl/record/csaRecord.h"
#include "osl/record/csaIOError.h"
#include "osl/record/kisen.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/progress.hpp>
#include <iostream>

#ifndef USE_TOKYO_CABINET
#  error "programs here are useless without USE_TOKYO_CABINET=1"
#endif

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;

void run(RecordDB&, const std::string& filename);
void run_kisen(RecordDB&, const std::string& filename);
void show_stat(RecordDB&);
std::string category;

int main(int argc, char **argv)
{
  po::options_description options;
  std::string database;
  std::vector<std::string> filenames;
  std::string kisen_filename;
  options.add_options()
    ("kisen,k", po::value<std::string>(&kisen_filename)->default_value(""),
     "specify filename of kisen")
    ("category,c", po::value<std::string>(&category)->default_value(""),
     "specify category of database")
    ("database,d", po::value<std::string>(&database)->default_value("recorddb.tch"),
     "filename for database")
    ("help,h", "Show help message");
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("files", po::value<std::vector<std::string> >());
  po::options_description all_options;
  all_options.add(options).add(hidden);
  po::positional_options_description p;
  p.add("files", -1);

  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).
	      options(all_options).positional(p).run(), vm);
    po::notify(vm);
    if (vm.count("files"))
      filenames = vm["files"].as<std::vector<std::string> >();
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }
  if (vm.count("help") || (filenames.empty() && kisen_filename == "") )
  {
    std::cerr << "Usage: " << argv[0] << " [options] files" << std::endl;
    std::cout << options << std::endl;
    return 1;
  }

  RecordDB db(database.c_str(), false);
  if (filenames.size() == 1 && filenames[0] == "-")
  {
    std::string line;
    size_t count = 0;
    while (std::getline(std::cin, line))
    {
      if (++count % 1024 == 0)
	db.optimize();
      run(db, line);
    }
  }
  else
  {
    for (size_t i=0; i<filenames.size(); ++i)
    {
      if (++i % 1024 == 0)
	db.optimize();
      run(db, filenames[i]);
    }
  }
  if (kisen_filename != "")
    run_kisen(db, kisen_filename);
  show_stat(db);
  db.optimize();
}

size_t new_position = 0, overwrite = 0, skip_same_records = 0, added_record = 0, skip_draw = 0;

void add(RecordDB& db, const std::string& filename, int move_number,
	 const NumEffectState& state, Player winner,
	 const std::string& black, const std::string& white,
	 int sub_index = -1)
{
  std::string key = db.makeKey(state);
  SquareInfo info;
  if (db.get(key, info))
    ++overwrite;
  else
  {
    info.set_win(0);
    info.set_loss(0);
    ++new_position;
  }
  std::string turn = black, opponent = white;
  if (state.turn() != BLACK)
    std::swap(turn, opponent);
  if (winner == state.turn())
  {
    info.set_win(info.win()+1);
    if (turn.find("gps") == 0)
      info.set_win_by_gps(info.win_by_gps()+1);
  }
  else
  {
    info.set_loss(info.loss()+1);
    if (turn.find("gps") == 0)
      info.set_loss_by_gps(info.loss_by_gps()+1);
  }
  info.set_count_for_average(info.count_for_average()+1);
  const double diff = move_number - info.average_moves();
  info.set_average_moves(info.average_moves() + diff/info.count_for_average());
  RecordInfo *rinfo = info.add_reference();
  rinfo->set_category(category);
  rinfo->set_path(boost::filesystem::path(filename).filename());
  if (sub_index >= 0)
    rinfo->set_sub_index(sub_index);
  
  bool success = db.put(key, info);
  if (! success)
    std::cerr << "db write failed\n" << state << winner << "\n";
}

bool have_same_record(RecordDB& db, const NumEffectState& initial, const vector<Move>& moves)
{
  if (moves.size() < 2)
    return true;		// no interest
  NumEffectState state(initial);
  for (size_t i=0; i+1<moves.size(); ++i)
    ApplyMoveOfTurn::doMove(state, moves[i]);
  SquareInfo info;
  if (db.get(state, info))
    return true;
  ApplyMoveOfTurn::doMove(state, moves[moves.size()-1]);
  return db.get(state, info);
}

void run(RecordDB& db, const std::string& filename) 
{
  Record record;
  try 
  {
    if (filename.find(".kif") == filename.size()-4) 
    {
      KakinokiFile file(filename);
      record = file.getRecord();
    } 
    else if (filename.find(".ki2") == filename.size()-4) 
    {
      Ki2File file(filename);
      record = file.getRecord();
    }
    else if (filename.find(".csa") == filename.size()-4) 
    {
      CsaFile file(filename);
      record = file.getRecord();
    }
  }
  catch (std::exception& e)
  {
    std::cerr << filename << " " << e.what() << "\n";    
    return;
  }
  if (record.getResult() == Record::UNKNOWN)
  {
    std::cerr << filename << " game result is unknown\n";
    return;
  }
  if (record.getResult() != Record::BLACK_WIN
      && record.getResult() != Record::WHITE_WIN)
  {
    ++skip_draw;
    return;
  }
  const Player winner = (record.getResult() == Record::BLACK_WIN)
    ? BLACK : WHITE;
  vector<Move> moves = record.getMoves();
  NumEffectState state = record.getInitialState();
  if (have_same_record(db, state, moves)) 
  {
    ++skip_same_records;
  }
  ++added_record;
  add(db, filename, 0, state, winner, record.getPlayer(BLACK), record.getPlayer(WHITE));
  for (size_t i=0; i<moves.size(); ++i) 
  {
    ApplyMoveOfTurn::doMove(state, moves[i]);
    add(db, filename, i+1, state, winner, record.getPlayer(BLACK), record.getPlayer(WHITE));
  }
}
void run_kisen(RecordDB& db, const std::string& filename)
{
  KisenFile kisen(filename);
  KisenIpxFile ipx(kisen.getIpxFileName());
  boost::progress_display progress(kisen.size());

  for (size_t kisen_id=0; kisen_id<kisen.size(); ++kisen_id, ++progress) 
  {
    if (kisen_id % 1024 == 0)
      db.optimize();
    const unsigned int result = ipx.getResult(kisen_id);
    Player winner = BLACK;
    switch (result)
    {
    case KisenIpxFile::BLACK_WIN: 
    case KisenIpxFile::BLACK_WIN_256:
      winner = BLACK;
      break;
    case KisenIpxFile::WHITE_WIN:
    case KisenIpxFile::WHITE_WIN_256:
      winner = WHITE;
      break;
    default:
      ++skip_draw;
      continue;
    }
    NumEffectState state = kisen.getInitialState();
    vector<Move> moves = kisen.getMoves(kisen_id);
    if (have_same_record(db, state, moves)) 
    {
      ++skip_same_records;
      continue;
    }
    ++added_record;
    add(db, filename, 0, state, winner,
	ipx.getPlayer(kisen_id, BLACK), ipx.getPlayer(kisen_id, WHITE), kisen_id);
    for (size_t i=0; i<moves.size(); ++i) 
    {
      ApplyMoveOfTurn::doMove(state, moves[i]);
      add(db, filename, i+1, state, winner,
	  ipx.getPlayer(kisen_id, BLACK), ipx.getPlayer(kisen_id, WHITE), kisen_id);
    }
  }
}

void show_stat(RecordDB& db)
{
  NumEffectState state;
  SquareInfo info;  
  if (db.get(state, info))
  {
    std::cerr << "initial position " << info.win() << ' ' << info.loss() << ' ' << info.draw() << "\n";
    // std::cerr << info.DebugString() << "\n";
  }
  std::cerr << "added " << added_record << " skip " << skip_same_records << " + " << skip_draw << "\n";
  std::cerr << "new position " << new_position << " cached " << overwrite << "\n";
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
