/* bradleyTerry.cc
 */
#include "osl/rating/bradleyTerry.h"
#include "osl/rating/group.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/record/kisen.h"

#include <thread>
#include <iostream>
#include <iomanip>

#ifndef MINIMAL
osl::rating::
BradleyTerry::BradleyTerry(FeatureSet& f, const std::string& kisen_file, int kisen_start)
  : features(f), kisen_filename(kisen_file), kisen_start(kisen_start), num_cpus(1), num_records(200),
    verbose(1), fix_group(-1), min_rating(0)
{
}

osl::rating::BradleyTerry::~BradleyTerry()
{
}

bool osl::rating::
BradleyTerry::addSquare(size_t g, const NumEffectState& state, 
			  const RatingEnv& env, Move selected,
			  valarray_t& wins, std::valarray<long double>& denominator) const
{
  MoveVector moves;
  state.generateLegal(moves);
  if (! moves.isMember(selected))
    return false;			// checkmate or illegal move  
  const range_t range = features.range(g);
#ifdef SPEEDUP_TEST
  const bool in_check = EffectUtil::isKingInCheck(state.turn(), state);
  if (! in_check || features.effectiveInCheck(g))
#endif
  {
    int found = features.group(g).findMatch(state, selected, env);
    if (found >= 0)
      ++wins[found+range.first];
  }
  valarray_t sum_c(0.0, range.second-range.first);
  long double sum_e = 0.0;
  for (size_t i=0; i<moves.size(); ++i) {
    Move m = moves[i];
    double product = 1.0;
    int count = 0;
    int match_id = -1;
    for (size_t j=0; j<features.groupSize(); ++j) {
#ifdef SPEEDUP_TEST
      if (in_check && ! features.effectiveInCheck(j))
	continue;
#endif
      int found = features.group(j).findMatch(state, m, env);
      if (found < 0)
	continue;
      found += features.range(j).first;
      product *= features.weight(found);
      ++count;
      if (j == g) {
	assert(range.first <= found && found < range.second);
	match_id = found;
      }
    }
    assert(count);
    sum_e += product;
    if (match_id >= 0)
      sum_c[match_id-range.first] += product / features.weight(match_id);
  }
  assert(sum_e > 0);
  for (int f=range.first; f<range.second; ++f)
    denominator[f] += sum_c[f-range.first]/sum_e;
  return true;
}

class osl::rating::
  BradleyTerry::Thread
{
public:
  const BradleyTerry *features;
  size_t target;
  size_t first, last;
  valarray_t *wins;
  std::valarray<long double> *denominator;
  size_t *skip;
  Thread(const BradleyTerry *a, size_t t, size_t f, size_t l, valarray_t *w, std::valarray<long double> *d,
	 size_t *s)
    : features(a), target(t), first(f), last(l), wins(w), denominator(d), skip(s)
  {
  }
  void operator()()
  {
    *skip = features->accumulate(target, first, last, *wins, *denominator);
  }
};

size_t osl::rating::
BradleyTerry::accumulate(size_t g, size_t first, size_t last, valarray_t& wins, std::valarray<long double>& denominator) const
{
  assert(wins.size() == features.featureSize());
  KisenFile kisen_file(kisen_filename.c_str());
  KisenIpxFile ipx(KisenFile::ipxFileName(kisen_filename));
  size_t skip = 0;
  for (size_t i=first; i<last; i++) {
    if ((i % 4000) == 0)
      std::cerr << ".";
    if (ipx.rating(i, BLACK) < min_rating 
	|| ipx.rating(i, WHITE) < min_rating) {
      ++skip;
      continue;
    }
    NumEffectState state(kisen_file.initialState());
    RatingEnv env;
    env.make(state);
    const auto moves=kisen_file.moves(i+kisen_start);
    for (size_t j=0; j<moves.size(); j++) {
      if (j<2)
	goto next;
      {
	const Player turn = state.turn();
	if (! state.inCheck()
	    && ImmediateCheckmate::hasCheckmateMove(turn, state))
	  break;
      }
      if (! addSquare(g, state, env, moves[j], wins, denominator))
	break;
    next:
      state.makeMove(moves[j]);
      env.update(state, moves[j]);
    }
  }
  return skip;
}

void osl::rating::
BradleyTerry::update(size_t g)
{
  std::valarray<valarray_t> wins(valarray_t(0.0, features.featureSize()), num_cpus);
  std::valarray<std::valarray<long double> > denominator(std::valarray<long double>(0.0, features.featureSize()), num_cpus);
  assert(wins.size() == num_cpus);

  KisenFile kisen_file(kisen_filename.c_str());
  if (num_records==0)
    num_records=kisen_file.size();
  if (num_cpus == 1) {
    accumulate(g, 0, num_records, wins[0], denominator[0]);
  }
  else {
    size_t cur = 0;
    size_t last = num_records, step = (last - cur)/num_cpus;
    boost::ptr_vector<std::thread> threads;  
    std::valarray<size_t> skip((size_t)0, num_cpus);
    for (size_t i=0; i<num_cpus; ++i, cur += step) {
      size_t next = (i+1 == num_cpus) ? last : cur + step;
      threads.push_back(new std::thread(Thread(this, g, cur, next, &wins[i], &denominator[i], &skip[i])));
    }
    for (size_t i=0; i<num_cpus; ++i) 
      threads[i].join();
    if (g == 0)
      std::cerr << "skip " << skip.sum() << " / " << num_records << "\n";
  }
  const range_t range = features.range(g);
  for (int f=range.first; f<range.second; ++f) {
    const int NPRIOR = 10; // assume NPRIOR wins, NPRIOR losses
    double sum_win = NPRIOR;
    long double sum_denom = (1.0 / (features.weight(f) + 1.0)) * 2 * NPRIOR;
    for (size_t i=0; i<num_cpus; ++i) {
      sum_win += wins[i][f];
      sum_denom += denominator[i][f];
    }
#ifdef DEBUG
    std::cerr << "  " << std::setw(14) << features.feature(f).name() 
	      << " " << features.weight(f) << " => " << sum_win/sum_denom
	      << "  " << sum_win << " / " << sum_denom
	      << "  " << 400*log10(sum_win/sum_denom) << "\n";
#endif
    // update
    if (sum_denom)
      features.setWeight(f, sum_win/sum_denom);
    assert(! std::isinf(features.weight(f)));
    assert(! std::isnan(features.weight(f)));
  }
  
  features.showGroup(std::cerr, g);
}

void osl::rating::
BradleyTerry::iterate()
{
  for (int j=0; j<16; ++j) {
    std::cerr << "\nnew iteration " << j << "\n";
    for (size_t i=0; i<features.groupSize(); ++i) {
      update(i);
      features.save(output_directory, i); 
      if ((int)(i+1) == fix_group)
	break;
    }
  }
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
