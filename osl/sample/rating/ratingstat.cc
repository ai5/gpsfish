#include "osl/rating/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "osl/rating/bradleyTerry.h"
#include "osl/eval/progressEval.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/record/csaRecord.h"
#include "osl/record/kisen.h"
#include "osl/misc/perfmon.h"
#include "osl/stat/histogram.h"
#include "osl/stat/variance.h"

#include <boost/format.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
using namespace osl;
using namespace osl::rating;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-v] [-f skip] csafiles or -k kisen-filename -n num\n"
       << endl;
  exit(1);
}

size_t first_skip = 3;
int verbose = 0;
const char *kisen_filename=0;
size_t num_kisen = 4000;
size_t kisen_start = 200000;
size_t min_rating = 1500;

struct KeepMin
{
  int min;
  explicit KeepMin(int initial = 10000) : min(initial)
  {
  }
  void add(int value) { min = std::min(value, min); }
  int value() const { return min; }
};

struct KeepMax
{
  int max;
  explicit KeepMax(int initial = -10000) : max(initial)
  {
  }
  void add(int value) { max = std::max(value, max); }
  int value() const { return max; }
};

struct Histogram8
{
  CArray<stat::Histogram*,8> histograms;
  const stat::Histogram& operator[](size_t i) const 
  {
    return *histograms[i];
  }
  Histogram8(int width, int length, int start=0)
  {
    for (int i=0; i<8; ++i)
      histograms[i] = new stat::Histogram(width, length, start);
  }
  void add(Progress16 progress, int data, double weight = 1.0)
  {
    const int min = histograms[0]->start();
    const int max = histograms[0]->start() + histograms[0]->width()*histograms[0]->length();
    if (data < min || data >= max) {
      return;
    }
    histograms[progress.value()/2]->add(data, weight);
  }
};

stat::Average moves, probs, order, top_score, selected_score;
const int width = 4, length = 20;
Histogram8 moves_histogram(width, length), selected_histogram(width, length);
Histogram8 all_moves_histogram(width, length);
const int sc_width = 100, sc_length = 16, sc_start = -400;
stat::Histogram takeback_histogram(sc_width, sc_length, sc_start), selected_takeback(sc_width, sc_length, sc_start);
stat::Histogram takeback_order(1, 10), takeback_order_all(1, 10), takeback_order_selected(1, 10);
stat::Histogram seeplus_histogram(sc_width, sc_length, sc_start), selected_seeplus(sc_width, sc_length, sc_start);
stat::Histogram seeplus_order(1, 10), seeplus_order_all(1, 10), seeplus_order_selected(1, 10);
stat::Histogram king_escape_histogram(sc_width, sc_length, sc_start), selected_king_escape(sc_width, sc_length, sc_start);
stat::Histogram kingescape_order(1, 10), kingescape_order_all(1, 10), kingescape_order_selected(1, 10);
Histogram8 score_histogram(sc_width, sc_length+4, sc_start), selected_score_histogram(sc_width, sc_length+4, sc_start);
Histogram8 all_score_histogram(sc_width, sc_length+4, sc_start);
Histogram8 rscore_histogram(sc_width, sc_length), rselected_score_histogram(sc_width, sc_length);
Histogram8 rall_score_histogram(sc_width, sc_length);
KeepMin min_selected, min_top;
KeepMax max_notakeback, max_nocapture;
const int sc_length_2d = sc_length+2;
const int sc_start_2d = -100;

namespace osl
{
  void showLogProb(const stat::Histogram& numerator, const stat::Histogram& denominator)
  {
    assert(numerator.width() == denominator.width());
    assert(numerator.length() == denominator.length());
    assert(numerator.start() == denominator.start());
    stat::Histogram logprob(numerator.width(), numerator.length(), numerator.start());
    for (size_t i=0; i<numerator.length(); ++i) {
      const double n = numerator.frequency(i);
      const double d = denominator.frequency(i);
      const double prob = n / d;
      logprob.frequency(i) = d >= 15 ? static_cast<int>(-100.0*log(prob)/log(2.0)) : 0;
    }  
    logprob.show(std::cout);
  }
  void showLogProb(const stat::Histogram& numerator, const stat::Histogram& denom1, const stat::Histogram& denom2)
  {
    assert(numerator.width() == denom1.width());
    assert(numerator.length() == denom1.length());
    assert(numerator.start() == denom1.start());
    assert(denom1.width() == denom2.width() && denom1.length() == denom2.length() && denom1.start() == denom2.start());
    stat::Histogram l1(numerator.width(), numerator.length(), numerator.start()), 
      l2(numerator.width(), numerator.length(), numerator.start());
    for (size_t i=0; i<numerator.length(); ++i) {
      const double n = numerator.frequency(i);
      const double d1 = denom1.frequency(i);
      const double d2 = denom2.frequency(i);
      const double p1 = n / d1;
      const double p2 = n / d2;
      l1.frequency(i) = d1 > 20 ? static_cast<int>(-100.0*log(p1)/log(2.0)) : 0;
      l2.frequency(i) = d2 > 20 ? static_cast<int>(-100.0*log(p2)/log(2.0)) : 0;
    }  
    int value=l1.start();
    for (size_t i=0; i<l1.length(); ++i, value+=l1.width()) {
      std::cout << std::setw(5) << value << " - " 
		<< std::setw(5) << value+(int)l1.width();
      std::cout << " " << std::setw(8) << l1.frequency(i)
		<< " " << std::setw(8) << l2.frequency(i) << "\n";
    }
  }
  void showLogProb(const Histogram8& numerator, const Histogram8& denom1, const Histogram8& denom2)
  {
    assert(numerator[0].width()  == denom1[0].width());
    assert(numerator[0].length() == denom1[0].length());
    assert(numerator[0].start()  == denom1[0].start());
    assert(denom1[0].width() == denom2[0].width() && denom1[0].length() == denom2[0].length() && denom1[0].start() == denom2[0].start());
    // for human
    int value=numerator[0].start();
    for (size_t i=0; i<numerator[0].length(); ++i, value+=numerator[0].width()) {
      std::cout << std::setw(4) << value << " - " 
		<< std::setw(4) << value+(int)numerator[0].width();

      for (int p=0; p<8; ++p) {
	const double n = numerator[p].frequency(i);
	const double d1 = denom1[p].frequency(i);
	const double d2 = denom2[p].frequency(i);
	const double p1 = n / d1;
	const double p2 = n / d2;
	const double f1 = n > 20 ? static_cast<int>(-100.0*log(p1)/log(2.0)) : 0;
	const double f2 = n > 20 ? static_cast<int>(-100.0*log(p2)/log(2.0)) : 0;
	std::cout << " " << std::setw(5) << f1
		  << " " << std::setw(4) << f2;
      }
      std::cout << "\n";
    }
    // depth
    std::cout << "static const osl::CArray2d<int, 8, " 
	      << numerator[0].length() << "> xxx_to_depth = {{\n";
    for (int p=0; p<8; ++p) {
      std::cout << " { ";
      for (size_t i=0; i<numerator[0].length(); ++i, value+=numerator[0].width()) {
	const double n = numerator[p].frequency(i);
	const double d = denom1[p].frequency(i);
	const double p = n / d;
	const double f = n > 20 ? static_cast<int>(-100.0*log(p)/log(2.0)) : 0;
	std::cout << std::setw(4) << f << ",";
	if (i % 5 == 4)
	  std::cout << " ";
      }
      std::cout << "},\n";
    }
    std::cout << "}};\n";
    // width
    std::cout << "static const osl::CArray2d<int, 8, " 
	      << numerator[0].length() << "> xxx_to_width = {{\n";
    for (int p=0; p<8; ++p) {
      std::cout << " { ";
      for (size_t i=0; i<numerator[0].length(); ++i, value+=numerator[0].width()) {
	const double n = numerator[p].frequency(i);
	const double d = denom2[p].frequency(i);
	const double p = n / d;
	const double f = n > 20 ? static_cast<int>(-100.0*log(p)/log(2.0)) : 0;
	std::cout << std::setw(4) << f << ",";
	if (i % 5 == 4)
	  std::cout << " ";
      }
      std::cout << "},\n";
    }
    std::cout << "}};\n";
  }
  
  enum Property { 
    All, 
    /** 一手目の取り返し */
    TakeBack, 
    /** 2手目の取り返し */
    TakeBack2, 
    /** 取り返しでない先頭 */
    NoTakeBack, 
    /** 取り返し以外の駒得の先頭 */
    SeePlus, 
    SeePlus2, 
    /** 3手目の駒得または駒得以外の先頭 */
    SeePlusX, 
    /** 取り返しでも駒得でもない先頭 */
    NoSeePlus 
  };
  size_t find(Property property, const NumEffectState& state, const RatingEnv& e,
	      const RatedMoveVector& moves, Move selected)
  {
    if (moves.empty())
      return 1;
    size_t i = 0;
    if (property == TakeBack || property == TakeBack2) {
      if (! e.history.lastMove().isNormal())
	return moves.size();
      for (; i<moves.size(); ++i)
	if (moves[i].move().to() == e.history.lastMove().to())
	  break;
    } 
    if (property == TakeBack2) {
      ++i;
      for (; i<moves.size(); ++i)
	if (moves[i].move().to() == e.history.lastMove().to())
	  break;
    }
    if (property == NoTakeBack) {
      if (e.history.lastMove().isNormal()) {
	for (; i<moves.size(); ++i)
	  if (moves[i].move().to() != e.history.lastMove().to())
	    break;
      }
    }
    if (property == SeePlus || property == SeePlus2) {
      for (; i<moves.size(); ++i) {
	if (e.history.lastMove().isNormal() && moves[i].move().to() == e.history.lastMove().to())
	  continue;
	if (PieceEval::computeDiffAfterMoveForRP(state, moves[i].move()) > 0)
	  break;
      }
    }
    if (property == SeePlus2) {
      ++i;
      for (; i<moves.size(); ++i) {
	if (PieceEval::computeDiffAfterMoveForRP(state, moves[i].move()) > 0)
	  break;
      }
    }
    if (property == SeePlusX) {
      int num_seeplus=0;
      for (; i<moves.size(); ++i) {
	if (e.history.lastMove().isNormal() && moves[i].move().to() == e.history.lastMove().to())
	  continue;
	if (PieceEval::computeDiffAfterMoveForRP(state, moves[i].move()) > 0) {
	  if (++num_seeplus <= 1) // use 2 for 3rd see+
	    continue;
	}
	break;
      }
    }
    if (property == NoSeePlus) {
      for (; i<moves.size(); ++i) {
	if (e.history.lastMove().isNormal() && moves[i].move().to() == e.history.lastMove().to())
	  continue;
	if (PieceEval::computeDiffAfterMoveForRP(state, moves[i].move()) > 0)
	  continue;
	break;
      }
    }
    return i;
  }
  /** カテゴリ内でトップの手が指された確率 */
  struct TopProb
  {
    stat::Histogram selected;
    stat::Histogram generated, generated_all;
    Property property;
    TopProb(Property p) 
      : selected(sc_width,sc_length+1,200), generated(sc_width,sc_length+1,200), 
	generated_all(sc_width,sc_length+1,200), 
	property(p)
    {
    }
    void add(const NumEffectState& state, const RatingEnv& e,
	     const RatedMoveVector& moves, Move selected) 
    {
      const size_t i = find(property, state, e, moves, selected);
      if (i >= moves.size())
	return;
      generated_all.add(moves[i].rating());
      const RatedMove *found = moves.find(selected);
      if (found && (found - &*moves.begin()) <= (int)i) {
	if (moves[i].move() == selected)
	  this->selected.add(moves[i].rating());
	generated.add(moves[i].rating());
      }
    }
    void show()
    {
      showLogProb(selected, generated, generated_all);
    }
  };
  /** rating とその局面のratingの最大値との差に基づく 2次元の実現確率 */
  struct RatingDiffRange
  {
    stat::Histogram selected;
    stat::Histogram generated;
    stat::Histogram all_generated;
    CArray<stat::Variance, sc_length_2d*sc_length_2d> variance;
    size_t first, last;
    
    RatingDiffRange(size_t f, size_t l) 
      : selected(1,sc_length_2d*sc_length_2d), generated(1,sc_length_2d*sc_length_2d), all_generated(1,sc_length_2d*sc_length_2d),
	first(f), last(l)
    {
    }

    static int index(int score, int diff) 
    {
      const int score_index = std::max(0, std::min((score - sc_start_2d) / sc_width, sc_length_2d-1));
      const int diff_index = std::min(diff / sc_width, sc_length_2d-1);
      assert(diff_index >= 0);
      return score_index*sc_length_2d + diff_index;
    }
    void add(const NumEffectState& state, const RatedMoveVector& moves, Move selected)
    {
      if (moves.empty() || state.inCheck())
	return;
      const int highest = moves[0].rating();
      const RatedMove *found = moves.find(selected);
      if (! found) 
	return;			// records may contain illegal move
      const size_t selected_order = found - &*moves.begin();
      if (first <= selected_order && selected_order < last) {
	const int selected_index = index(found->rating(), highest - found->rating());
	this->selected.add(selected_index);
      }
      for (size_t i=first; i<std::min(last,moves.size()); ++i) {
	const int index = this->index(moves[i].rating(), highest - moves[i].rating());
	all_generated.add(index);
	if (i <= selected_order)
	  generated.add(index);
	variance[index].add(i);
      }
    }
  
    void show(std::ostream& os)
    {
      os << "depth\n";
      for (int i=0; i<sc_length_2d; ++i) {
	for (int j=0; j<sc_length_2d; ++j) {
	  double s = selected.frequency(i*sc_length_2d+j);
	  double g = generated.frequency(i*sc_length_2d+j);
	  // os << std::setw(5) << (g > 20 ? s/g : 0.0);
	  os << std::setw(5) << (std::min(s,g) > 20 ? static_cast<int>(-100.0*log(s/g)/log(2.0)) : 0);
	}
	os << "\n";
      }
      os << "width\n";
      for (int i=0; i<sc_length_2d; ++i) {
	for (int j=0; j<sc_length_2d; ++j) {
	  double s = selected.frequency(i*sc_length_2d+j);
	  double a = all_generated.frequency(i*sc_length_2d+j);
	  // os << std::setw(5) << (a > 20 ? s/a : 0.0);
	  os << std::setw(5) << (std::min(s,a) > 20 ? static_cast<int>(-100.0*log(s/a)/log(2.0)) : 0);
	}
	os << "\n";
      }
      os << "order\n";
      for (int i=0; i<sc_length_2d; ++i) {
	for (int j=0; j<sc_length_2d; ++j) {
	  // os << std::setw(5) << std::setprecision(3) << variance[i*sc_length_2d+j].getAverage();
	  os << std::setw(5) << static_cast<int>(variance[i*sc_length_2d+j].average());
	}
	os << "\n";
      }
    }
  };
  struct RatingDiff
  {
    RatingDiffRange r0, r1, r_all;
    RatingDiff() :
      r0(1,25), r1(25,800), r_all(1,800)
    {
    }
    void add(const NumEffectState& state, const RatedMoveVector& moves, Move selected)
    {
#ifdef SHOW_SPLIT_RATING
      r0.add(state, moves, selected);
      r1.add(state, moves, selected);
#endif
      r_all.add(state, moves, selected);
    }
    void show(std::ostream& os)
    {
#if SHOW_SPLIT_RATING
      r0.show(os);
      r1.show(os);
#endif
      r_all.show(os);
    }
  };
}

RatingDiff rating_diff;
TopProb top_prob(All), takeback_topprob(TakeBack), takeback2_topprob(TakeBack2), 
  no_takeback_topprob(NoTakeBack), seeplus_topprob(SeePlus), seeplus2_topprob(SeePlus2), seeplusx_topprob(SeePlusX);
CArray<stat::Variance, 8> top_rating_progress;

void test_file(const FeatureSet&, const char *filename);
void test_record(const FeatureSet& f, 
		 const SimpleState& initial,
		 const std::vector<osl::Move>& moves);

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
    
  char c;
  while ((c = getopt(argc, argv, "f:k:n:vh")) != EOF)
  {
    switch(c)
    {
    case 'f':	first_skip = atoi(optarg);
      break;
    case 'k':	kisen_filename = optarg;
      break;
    case 'n':	num_kisen = atoi(optarg);
      break;
    case 'v':	++verbose;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag || (!kisen_filename && argc < 1))
    usage(program_name);

  eval::ProgressEval::setUp();
  StandardFeatureSet f;

  if (kisen_filename) {
    KisenFile kisen_file(kisen_filename);
    KisenIpxFile ipx(kisen_file.ipxFileName());
    size_t skip = 0;
    for (size_t i=0; i<num_kisen; i++) {
      if (ipx.rating(i, BLACK) < min_rating 
	  || ipx.rating(i, WHITE) < min_rating) {
	++skip;
	continue;
      }
      if (i % 128 == 0)
	std::cerr << '.';
      test_record(f, kisen_file.initialState(), kisen_file.moves(i+kisen_start));
    }
  }
  
  for (int i=0; i<argc; ++i)
  {
    if (i % 128 == 0)
      std::cerr << '.';
    test_file(f, argv[i]);
  }

  std::cout << "\n"
	    << "average moves/position " << moves.average() << "\n"
	    << "average order " << order.average() << "\n"
	    << "average selected score " << selected_score.average() << "\n"
	    << "min selected score " << min_selected.value() << "\n"
	    << "average top score " << top_score.average() << "\n"
	    << "min top score " << min_top.value() << "\n"
	    << "max top score (notakeback) " << max_notakeback.value() << "\n"
	    << "max top score (nocapture) " << max_nocapture.value() << "\n";
  std::cout << "order to logprob (depth, width)\n";
  showLogProb(selected_histogram, moves_histogram, all_moves_histogram);
  std::cout << "score to logprob (all)\n";
  showLogProb(selected_score_histogram, score_histogram, all_score_histogram);
  std::cout << "relative score to logprob (all)\n";
  showLogProb(rselected_score_histogram, rscore_histogram, rall_score_histogram);
  std::cout << "score to logprob (takeback)\n";
  showLogProb(selected_takeback, takeback_histogram);
  std::cout << "score to logprob (see+)\n";
  showLogProb(selected_seeplus, seeplus_histogram);
  std::cout << "score to logprob (king_escape)\n";
  showLogProb(selected_king_escape, king_escape_histogram);
  if (verbose) {
    std::cout << "order to logprob (takeback)\n";
    showLogProb(takeback_order_selected, takeback_order, takeback_order_all);
    std::cout << "order to logprob (seeplus)\n";
    showLogProb(seeplus_order_selected, seeplus_order, seeplus_order_all);
    std::cout << "order to logprob (kingescape)\n";
    showLogProb(kingescape_order_selected, kingescape_order, kingescape_order_all);
    rating_diff.show(std::cout);
    std::cout << "top move\n";
    top_prob.show();
    std::cout << "top move (takeback)\n";
    takeback_topprob.show();
    std::cout << "top move (2nd takeback)\n";
    takeback2_topprob.show();
    if (verbose > 1) {
      std::cout << "top move (no takeback)\n";
      no_takeback_topprob.show();
    }
    std::cout << "top move (see+)\n";
    seeplus_topprob.show();
    std::cout << "top move (2nd see+)\n";
    seeplus2_topprob.show();
    std::cout << "top move (2nd see+ or no see+)\n";
    seeplusx_topprob.show();
  }
  std::cout << "top rating for each progress8\n";
  for (size_t i=0; i<top_rating_progress.size(); ++i)
    std::cout << "progress8 " << i << "\tave. " << std::setprecision(3) << top_rating_progress[i].average()
	      << "\tsigma " << sqrt(top_rating_progress[i].variance()) << "\n";
}

/* ------------------------------------------------------------------------- */

size_t num_positions = 0;
void test_position(const FeatureSet& f, Move next_move, Move last_move, const RatingEnv& env,
		   const NumEffectState& state, const eval::ProgressEval& eval)
{
  const bool in_check = state.inCheck();
  RatedMoveVector moves;
  f.generateRating(state, env, 2000, moves);

  if (moves.empty()) 
    return;
  const RatedMove *p = moves.find(next_move);
  if (! p) 
    return;

  rating_diff.add(state, moves, next_move);
  top_prob.add(state, env, moves, next_move);
  takeback_topprob.add(state, env, moves, next_move);
  takeback2_topprob.add(state, env, moves, next_move);
  no_takeback_topprob.add(state, env, moves, next_move);
  seeplus_topprob.add(state, env, moves, next_move);
  seeplus2_topprob.add(state, env, moves, next_move);
  seeplusx_topprob.add(state, env, moves, next_move);
  
  bool notakeback_added = in_check, nocapture_added = in_check;
  const int highest = moves[0].rating();
  min_top.add(highest);
  top_score.add(highest);
  if (! in_check) {
    size_t index = find(NoSeePlus, state, env, moves, next_move);
    if (index < moves.size())
      top_rating_progress[eval.progress16().value()/2].add(moves[index].rating());
  }
  if (! notakeback_added
      && moves[0].move().to() != last_move.to()) {
    nocapture_added = true;
    max_notakeback.add(highest);
  }
  if (! nocapture_added
      && moves[0].move().capturePtype() == PTYPE_EMPTY
      && ! moves[0].move().isPromotion()) {
    nocapture_added = true;
    max_nocapture.add(highest);
  }

  const int count = moves.size();
  const int order = p ? p - &*moves.begin() +1 : count;
  ::order.add(order);
  const double selected_weight = 1.0-1.0/(moves.size()-order+1);
  const double other_weight    = 1.0; // -1.0/moves.size();

  if (in_check) {
    for (int i=0; i<count; ++i) {
      if (i < order) {
	king_escape_histogram.add(moves[i].rating(), other_weight);
	kingescape_order.add(i);
	if (moves[i].move() == next_move) {
	  selected_king_escape.add(moves[i].rating(), selected_weight);
	  kingescape_order_selected.add(i);
	}
      }
      kingescape_order_all.add(i);
    }
    return;
  }
  selected_histogram.add(env.progress, order, selected_weight);
  selected_score.add(p->rating());
  min_selected.add(p->rating());
  if (p->rating() < -2000) {
    std::cerr << state << "selected " << *p << "\n" << moves;
  }
  for (int i=0; i<order; ++i)
    moves_histogram.add(env.progress, i, other_weight);
  for (size_t i=0; i<moves.size(); ++i)
    all_moves_histogram.add(env.progress, i, other_weight);
  ::moves.add(count);
  ++num_positions;

  int j=0;
  for (int i=0; i<count; ++i) {
    if (moves[i].move().to() != last_move.to())
      continue;
    if (i < order) {
      takeback_histogram.add(moves[i].rating(), other_weight);
      takeback_order.add(j);
      if (moves[i].move() == next_move) {
	selected_takeback.add(moves[i].rating(), selected_weight);
	takeback_order_selected.add(j);
      }
    }
    takeback_order_all.add(j);
    ++j;
  }
  j=0;
  for (int i=0; i<count; ++i) {
    if (moves[i].move().to() == last_move.to())
      continue;
    if (! (moves[i].move().capturePtype() != PTYPE_EMPTY
	   || moves[i].move().isPromotion())
	|| PieceEval::computeDiffAfterMoveForRP(state, moves[i].move()) <= 0)
      continue;
    if (i<order) {
      seeplus_histogram.add(moves[i].rating(), other_weight);
      seeplus_order.add(j);
      if (moves[i].move() == next_move) {
	selected_seeplus.add(moves[i].rating(), selected_weight);
	seeplus_order_selected.add(j);
      }
    }
    seeplus_order_all.add(j);
    ++j;
  }

  for (int i=0; i<order; ++i) {
    score_histogram.add(env.progress, moves[i].rating(), other_weight);
    if (moves[i].move() == next_move)
      selected_score_histogram.add(env.progress, moves[i].rating(), selected_weight);
  }
  for (size_t i=0; i<moves.size(); ++i) {
    all_score_histogram.add(env.progress, moves[i].rating(), other_weight);
    if (! notakeback_added && moves[i].move().to() != last_move.to()) {
      notakeback_added = true;
      max_notakeback.add(moves[i].rating());
    }
    if (! nocapture_added && moves[i].move().capturePtype() == PTYPE_EMPTY
	&& ! moves[i].move().isPromotion()) {
      nocapture_added = true;
      max_nocapture.add(moves[i].rating());
    }
  }
  if (moves[0].move() != next_move) {
    const int top_score = moves[0].rating();
    for (int i=1; i<order; ++i) {
      rscore_histogram.add(env.progress, top_score - moves[i].rating(), other_weight);
      if (moves[i].move() == next_move)
	rselected_score_histogram.add(env.progress, top_score - moves[i].rating(), selected_weight);
    }
    for (size_t i=1; i<moves.size(); ++i) {
      rall_score_histogram.add(env.progress, top_score - moves[i].rating(), other_weight);
    }
  }
}

void test_record(const FeatureSet& f, 
		 const SimpleState& initial,
		 const std::vector<osl::Move>& moves)
{
  NumEffectState state(initial);

  RatingEnv env;
  env.make(state);
  eval::ProgressEval eval(state);
  for (size_t i=0; i<moves.size(); ++i) {
    if (state.inCheck(alt(state.turn())))
      break;			// illegal

    const Move move = moves[i];
    assert(state.isValidMove(move));
    if (i >= first_skip) {
      test_position(f, moves[i], (i>0 ? moves[i-1] : Move::PASS(alt(moves[i].player()))),
		    env, state, eval);
    }
    state.makeMove(move);
    eval.update(state, move);
    env.update(state, move);
  }
}

void test_file(const FeatureSet& f, const char *filename)
{
  RecordMinimal record;
  try {
    record = CsaFileMinimal(filename).load();
  }
  catch (CsaIOError& e) {
    std::cerr << "skip " << filename <<"\n";
    std::cerr << e.what() << "\n";
    return;
  }
  catch (...) {
    throw;
  }
  test_record(f, record.initialState(), record.moves);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
