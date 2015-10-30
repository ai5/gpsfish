/* rating.cc
 */
// http://remi.coulom.free.fr/Amsterdam2007/

#include "osl/rating/bradleyTerry.h"
#include "osl/rating/ratingEnv.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/numEffectState.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/record/kisen.h"
#include "osl/record/csaRecord.h"
#include "osl/stat/average.h"
#include "osl/stat/histogram.h"

#include <boost/program_options.hpp>
#include <algorithm>
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <unistd.h>

// more bigram attack

using namespace osl;
using namespace osl::rating;

namespace po = boost::program_options;

void analyze();

size_t kisen_start, num_records;
std::string kisen_filename;
int verbose;
std::string output_directory, input_directory;
bool verify_mode;
std::string csa_filename;
size_t num_cpus;
std::string feature_type;
int fix_group;
size_t min_rating = 1500;

int main(int argc, char **argv)

{
  nice(20);
  po::options_description options("all_options");
  options.add_options()
    ("num-records,n",
     po::value<size_t>(&num_records)->default_value(100),
     "number of records to be analyzed (all if 0)")
    ("kisen-file,k",
     po::value<std::string>(&kisen_filename)->default_value("../../data/kisen/01.kif"),
     "filename for records to be analyzed")
    ("kisen-start",
     po::value<size_t>(&kisen_start)->default_value(0),
     "start id of kisen records")
    ("fix-group",
     po::value<int>(&fix_group)->default_value(-1),
     "do not update after this group")
    ("output,o",
     po::value<std::string>(&output_directory)->default_value("./new-data"),
     "directory for storng rating conputed")
    ("load,l",
     po::value<std::string>(&input_directory)->default_value("./data"),
     "directory for initial rating")
    ("num-cpus,N",
     po::value<size_t>(&num_cpus)->default_value(1),
     "number cpus to be used")
    ("verify",
     po::value<bool>(&verify_mode)->default_value(false),
     "verify instead of learn")
    ("csa-filename",
     po::value<std::string>(&csa_filename)->default_value(""),
     "position of csa filename for verify")
    ("feature-type",
     po::value<std::string>(&feature_type)->default_value("standard"),
     "standard, capture or tactical")
    ("verbose,v",
     po::value<int>(&verbose)->default_value(1),
     "verboseness")
    ("help", "produce help message")
    ;

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);
  }
  catch (std::exception& e) {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }
  if (vm.count("help")) {
    std::cerr << options << std::endl;
    return 0;
  }
  analyze();
}

/* ------------------------------------------------------------------------- */

void verify(const FeatureSet& f,
	    NumEffectState& state, const std::vector<Move>& moves,
	    stat::Average& order, stat::Histogram& order_hist)
{
  RatingEnv env;
  env.make(state);
  for (size_t j=0; j<moves.size(); j++) {
    const Player turn = state.turn();
    if (moves.size() > 10 && j<2)
      goto next;
    if (! state.inCheck(turn)
	&& ImmediateCheckmate::hasCheckmateMove(turn, state))
      break;
      
    {
      RatedMoveVector my_moves;
      f.generateRating(state, env, 2000, my_moves);
      const RatedMove *p = my_moves.find(moves[j]);
      if (! p)
	break;
      int cur_order = p - &*my_moves.begin();
      order.add(cur_order);
      order_hist.add(cur_order);
      if (verbose > 2 || (verbose > 1 && cur_order > 10 && j > 40) || cur_order > 1000) {
	std::cerr << j << " th move\n" << state << "selected " << moves[j] << " order " << cur_order << "\n";
	double total = 0.0;
	for (size_t i=0; i<my_moves.size(); ++i) 
	  total += exp(my_moves[i].rating()/400.0*log(10));
	for (size_t i=0; i<my_moves.size(); ++i) {
	  const double prob = exp(my_moves[i].rating()/400.0*log(10)) / total;
	  std::cerr << i << " " << csa::show(my_moves[i].move()) << " rate " << my_moves[i].rating()
		    << " prob " << prob*100.0 << " rp " << -100*log(prob)/log(2) << "\n";
	}
      }
    }
  next:
    state.makeMove(moves[j]);
    env.update(state, moves[j]);
  }
}

void verify(const FeatureSet& f)
{
  std::cerr << std::setprecision(3);
  stat::Average order;
  stat::Histogram order_hist(10,10);
  if (! csa_filename.empty()) {
    csa::CsaFileMinimal csa(csa_filename);
    NumEffectState state(csa.initialState());
    const auto moves = csa.moves();
    verify(f, state, moves, order, order_hist);
  }
  else {
    KisenFile kisen_file(kisen_filename.c_str());
    KisenIpxFile ipx(KisenFile::ipxFileName(kisen_filename));
    if (num_records==0)
      num_records=kisen_file.size()-kisen_start;
    int skip = 0;
    for (size_t i=kisen_start; i<num_records+kisen_start; i++) {
      if (ipx.rating(i, BLACK) < min_rating 
	  || ipx.rating(i, WHITE) < min_rating) {
	++skip;
	continue;
      }
      NumEffectState state(kisen_file.initialState());
      const auto moves=kisen_file.moves(i);
      verify(f, state, moves, order, order_hist);
    }
    std::cerr << "skip " << skip << " / " << num_records << "\n";
  }
  std::cerr << order.average() << "\n";
  order_hist.show(std::cerr);
}

void analyze()
{
  std::unique_ptr<FeatureSet> f;
  if (feature_type == "standard") 
    f.reset(new StandardFeatureSet(true));
  else if (feature_type == "capture")
    f.reset(new CaptureSet(true));
#if 0
  else if (feature_type == "tactical")
    f.reset(new TacticalSet(true));
#endif
  else {
    std::cerr << "unknown feature set " << feature_type << "\n";
    return;
  }
  f->tryLoad(input_directory);

  if (verbose > 1)
    for (size_t i=0; i<f->groupSize(); ++i)
      f->showGroup(std::cerr, i);
  
  if (verify_mode)
    verify(*f);
  else {
    BradleyTerry b(*f, kisen_filename, kisen_start);
    b.setNumCpus(num_cpus);
    b.setVerbose(verbose);
    b.setNumRecords(num_records);

    b.setOutputDirectory(output_directory);
    b.setFixGroup(fix_group);
    b.setMinRating(min_rating);
    b.iterate();
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
