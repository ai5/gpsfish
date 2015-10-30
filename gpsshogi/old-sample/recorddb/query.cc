/* query.cc
 */
#include "gpsshogi/recorddb/recordDb.h"
#include "gpsshogi/recorddb/facade.h"
#include "osl/state/numEffectState.h"
#include "osl/apply_move/applyMove.h"
#include "osl/record/kakinoki.h"
#include "osl/record/ki2.h"
#include "osl/record/ki2IOError.h"
#include "osl/record/csaRecord.h"
#include "osl/record/csaIOError.h"
#include "osl/record/kisen.h"
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;

std::unique_ptr<RecordDB> db;

void run(const std::string& filename, int move_number);
void run_kisen(const std::string& filename, int id, int move_number);
void query(const NumEffectState& state);

int verbose;
int main(int argc, char **argv)
{
  std::string database;
  std::vector<std::string> filenames;
  std::string kisen_filename;
  int kisen_id, move_number;
  po::options_description options;
  options.add_options()
    ("kisen,k", po::value<std::string>(&kisen_filename)->default_value(""),
     "specify filename of kisen")
    ("kisen-id,i", po::value<int>(&kisen_id)->default_value(0),
     "id of kisen file")
    ("move-number,m", po::value<int>(&move_number)->default_value(0),
     "move number, -1 for all")
    ("database,d", po::value<std::string>(&database)->default_value(""),
     "filename for database")
    ("verbose,v", po::value<int>(&verbose)->default_value(0),
     "set verbose level")
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
  if (vm.count("help"))
  {
    std::cerr << "Usage: " << argv[0] << " [options] files" << std::endl;
    std::cout << options << std::endl;
    return 1;
  }

  if (database != "")
  {
    db.reset(new RecordDB(database.c_str(), true));
  }
  if (filenames.empty() && kisen_filename == "") 
  {
    NumEffectState state;
    query(state);
  }
  else
  {
    for (size_t i=0; i<filenames.size(); ++i)
      run(filenames[i], move_number);
    if (kisen_filename != "")
      run_kisen(kisen_filename, kisen_id, move_number);
  }
}

void query(const NumEffectState& state)
{
  if (! db)
  {
    int win, loss, gps_win, gps_loss, bonanza_win, bonanza_loss;
    recorddb::query(state, win, loss, gps_win, gps_loss, bonanza_win, bonanza_loss);
    std::cout << win << " " << loss
	      << "  " << gps_win << " " << gps_loss
	      << "  " << bonanza_win << " " << bonanza_loss
	      << "\n";
    return;
  }
  
  std::string key = db->makeKey(state);
  SquareInfo info;
  db->get(key, info);
  if (verbose) 
    std::cerr << info.DebugString() << "\n";
  int win = info.win(), loss = info.loss(), gps_win = info.win_by_gps(), gps_loss = info.loss_by_gps();
  if (state.turn() != BLACK)
  {
    std::swap(win, loss);
    if (gps_win || gps_loss)
    {
      const int a = win - gps_loss, b = loss - gps_win;
      gps_win = a; gps_loss = b;
    }
  }
  std::cout << win << " " << loss // viewpoint of black player
	    << "  " << gps_win << " " << gps_loss << "\n"; 
}

void run(const std::string& filename, int move_number) 
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
  vector<Move> moves = record.getMoves();
  NumEffectState state = record.getInitialState();
  for (size_t i=0; i<moves.size(); ++i)  
  {  
    if ((int)i == move_number || move_number < 0)
      query(state);
    ApplyMoveOfTurn::doMove(state, moves[i]);
  }
  if ((int)moves.size() == move_number || move_number < 0)
    query(state);
}
void run_kisen(const std::string& filename, int kisen_id, int move_number)
{
  KisenFile kisen(filename);
  NumEffectState state = kisen.getInitialState();
  vector<Move> moves = kisen.getMoves(kisen_id);
  for (int i=0; i<move_number; ++i) 
    ApplyMoveOfTurn::doMove(state, moves[i]);
  query(state);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
