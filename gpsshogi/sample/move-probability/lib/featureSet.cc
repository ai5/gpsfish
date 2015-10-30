/* featureSet.cc
 */
#include "featureSet.h"
#include "feature.h"
#include "osl/csa.h"
#include "osl/eval/see.h"
#include "osl/eval/pieceEval.h"
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cstdio>

gpsshogi::
FeatureSet::FeatureSet()
{
}

gpsshogi::
FeatureSet::~FeatureSet()
{
}

void gpsshogi::
FeatureSet::pushBack(Feature *f, bool light)
{
  features.push_back(f);
  if (light)
    light_features.push_back(features.size()-1);
}

void gpsshogi::
FeatureSet::addFinished()
{
  offsets.resize(features.size()+1);
  offsets[0] = 0;
  for (size_t i=0; i<features.size(); ++i)
    offsets[i+1] = offsets[i] + features[i].dimension();
}

gpsshogi::weight_t gpsshogi::FeatureSet::
match(const StateInfo& state, Move move, index_list_t& out,
      const valarray_t& weights) const
{
  MoveInfo info(state, move);
  assert(offsets.size() == features.size()+1);
  out.clear();
#ifndef NDEBUG
  size_t prev = 0;
#endif
  for (size_t i=0; i<features.size(); ++i) {
    features[i].match(state, info, offsets[i], out);
#ifndef NDEBUG
    for (size_t j=prev; j<out.size(); ++j) {
      assert(offsets[i] <= out[j].first);
      assert(out[j].first < offsets[i+1]);
    }
    prev = out.size();
#endif
  }
  return accumulate(out, weights);
}

gpsshogi::weight_t gpsshogi::FeatureSet::
matchExp(const StateInfo& state, Move move, index_list_t& out,
      const valarray_t& weights) const
{
  return exp(match(state, move, out, weights));
}

gpsshogi::weight_t gpsshogi::FeatureSet::
matchLight(const StateInfo& state, Move move, index_list_t& out,
	   const valarray_t& weights) const
{
  MoveInfo info(state, move);
  assert(offsets.size() == features.size()+1);
  out.clear();
  for (size_t i: light_features) {
    features[i].match(state, info, offsets[i], out);
  }
  return accumulate(out, weights);
}


void gpsshogi::FeatureSet::
analyze(const StateInfo& state, Move move, const valarray_t& weights) const
{
  MoveInfo info(state, move);
  index_list_t matched;
  std::cerr << csa::show(move) << "\n";
  std::vector<std::pair<double, std::string> > out;
  for (size_t i=0; i<features.size(); ++i) {
    matched.clear();
    features[i].match(state, info, offsets[i], matched);
    if (! matched.empty())
      out.push_back(make_pair(accumulateExp(matched, weights),
			      features[i].name()));
  }
  std::sort(out.begin(), out.end());
  std::reverse(out.begin(), out.end());
  for (size_t i=0; i<out.size(); ++i) {
    std::cerr << boost::format("%16s %6.2f ") % out[i].second % out[i].first;
    if (i % 3 == 2)
      std::cerr << "\n";
  }
  if (out.size() % 3 != 0)
    std::cerr << "\n";
}

gpsshogi::weight_t gpsshogi::FeatureSet::
accumulate(const index_list_t& features, const valarray_t& weights)
{
  weight_t sum = 0.0;
  for (const index_list_t::value_type& p: features) {
    assert(p.first < weights.size());
    sum += weights[p.first] * p.second;
  }
  assert(sum == 0.0 || std::isnormal(sum));
  return sum;
}

gpsshogi::weight_t gpsshogi::FeatureSet::
accumulateExp(const index_list_t& features, const valarray_t& weights)
{
  return exp(accumulate(features, weights));
}

gpsshogi::weight_t gpsshogi::FeatureSet::
generateRating(const StateInfo& state, WeightedMoveVector& out,
	       const valarray_t& weights) const
{
  MoveVector moves;
  state.state.generateLegal(moves);
  weight_t sum = 0.0;
  for (Move move: moves) {
    index_list_t dummy;
    weight_t score = match(state, move, dummy, weights);
    out.push_back(WeightedMove(score, move));
    sum += exp(score);
  }
  return sum;
}

void gpsshogi::FeatureSet::
ratingToLogProb(const WeightedMoveVector& rating,
		weight_t sum,
		MoveLogProbVector& out)
{
  static const double scale = 100.0 / log(0.5);
  for (WeightedMove move: rating) {
    weight_t p = exp(move.first)/sum;
    if (std::isnan(p) || p <= 1.0/(1<<12)) 
      p = 1.0/(1<<12);
    const int logp = std::max(50, static_cast<int>(log(p)*scale));
    out.push_back(MoveLogProb(move.second, logp));
  }
  out.sortByProbability();
}

void gpsshogi::FeatureSet::
generateLogProb(const StateInfo& state, MoveLogProbVector& out,
		const valarray_t& weights) const
{
  WeightedMoveVector moves;
  weight_t sum = generateRating(state, moves, weights);
  ratingToLogProb(moves, sum, out);
}

int gpsshogi::FeatureSet::
logLikelihood(const StateInfo& state,
	      Move move, const valarray_t& weights) const
{
  MoveLogProbVector moves;
  generateLogProb(state, moves, weights);
  const MoveLogProb *p = moves.find(move);
  if (!p)
    return 1200;
  return p->logProb();
}

void gpsshogi::FeatureSet::
save(const char *base_filename, const valarray_t& weights) const
{
  assert(weights.size() == dimension());
  std::string info_filename = std::string(base_filename) + "-info.txt";
  {
    std::ofstream os(info_filename.c_str());
    os << "#* all\n";
    for (const Feature& f: features)
      os << f.name() << ' ' << f.dimension() << "\n";
  }
  std::string filename = std::string(base_filename) + ".txt";
  {
    FILE *fp = fopen(filename.c_str(), "w");
    if (! fp)
      return;
    for (size_t i=0; i<weights.size(); ++i)
      fprintf(fp, "%.8f\n", weights[i]);
    fclose(fp);
  }
}

bool gpsshogi::FeatureSet::
load(const char *base_filename, valarray_t& weights) const
{
  std::string filename = std::string(base_filename) + ".txt";
  weights.resize(dimension());
  weights = 0.0;
  std::ifstream is(filename.c_str());
  for (size_t i=0; i<dimension(); ++i) {
    is >> weights[i];
    if (! is) {
      std::cerr << "load failed at " << i << " in " << dimension() << "\n";
      break;
    }
  }
  return static_cast<bool>(is);
}

void gpsshogi::FeatureSet::
showSummary(const valarray_t& weights) const
{
  assert(weights.size() == dimension());
  for (size_t i=0; i<features.size(); ++i) {
    const Feature& f = features[i];
    using namespace boost::accumulators;
    accumulator_set<weight_t, stats<tag::mean, tag::min, tag::max> > acc;
    int zero = 0;
    for (size_t j=offsets[i]; j<offsets[i+1]; ++j)
      if (weights[j])
	acc(weights[j]);
      else 
	++zero;
    std::cerr << std::setw(16) << f.name() 
	      << " dim " << std::setw(5) << f.dimension() - zero
	      << "/" << std::setw(5) << f.dimension()
	      << " min " << std::setw(6) << min(acc)
	      << " max " << std::setw(6) << max(acc)
	      << " mean " << std::setw(6) << mean(acc)
	      << "\n";
  }
}


gpsshogi::StandardFeatureSet::
StandardFeatureSet()
{
  pushBack(new TakeBackFeature, 1);
  pushBack(new CheckFeature, 1);
  pushBack(new SeeFeature, 1);
  pushBack(new ContinueCapture, 1);
  pushBack(new DropCaptured);
  pushBack(new SquareY, 1);
  pushBack(new SquareX, 1);
  pushBack(new KingRelativeY, 1);
  pushBack(new KingRelativeX, 1);
  pushBack(new FromEffect, 1);
  pushBack(new ToEffect, 1);
  pushBack(new FromEffectLong, 1);
  pushBack(new ToEffectLong, 1);
  pushBack(new Pattern(0,-1));	// U
  pushBack(new Pattern(1,-1));	// UL
  pushBack(new Pattern(1,0));	// L
  pushBack(new Pattern(0,1));	// D
  pushBack(new Pattern(1,1));	// DL
  pushBack(new Pattern(1,-2));	// UUL
  pushBack(new Pattern(0,-2));	// UU
  pushBack(new Pattern(0,2));	// DD
  pushBack(new Pattern(2,0));	// LL
  pushBack(new Pattern(1,2));	// DDL
  pushBack(new MoveFromOpposingSliders);
  pushBack(new AttackToOpposingSliders);
  pushBack(new PawnAttack);
  pushBack(new CapturePtype, 1);
  pushBack(new BlockLong);
  pushBack(new BlockLongFrom);
  pushBack(new LanceAttack);
  pushBack(new BishopAttack);
  pushBack(new RookAttack);
  pushBack(new BreakThreatmate);
  pushBack(new SendOff);
  pushBack(new CheckmateIfCapture);
  pushBack(new OpposingPawn);
  pushBack(new DropAfterOpposingPawn);
  pushBack(new LongRecapture);
  pushBack(new SacrificeAttack);
  pushBack(new AddEffectLong);
  pushBack(new King5x5Ptype);
  pushBack(new KingBlockade);
  pushBack(new CoverFork);
  pushBack(new ThreatmateByCapture);
  pushBack(new LureDefender);
  pushBack(new CoverPawn);
  pushBack(new PromotionBySacrifice);
  pushBack(new EscapeThreatened);
  pushBack(new BookMove);
  addFinished();
}



void gpsshogi::PredictionModelLight::
save(const char *filename, const valarray_t& weights)
{
  assert(weights.size() == dimension());
  FILE *fp = fopen(filename, "w");
  if (! fp)
    return;
  for (size_t i=0; i<weights.size(); ++i)
    fprintf(fp, "%.8f\n", weights[i]);
  fclose(fp);
}

bool gpsshogi::PredictionModelLight::
load(const char *filename, valarray_t& weights)
{
  weights.resize(dimension());
  weights = 0.0;
  std::ifstream is(filename);
  for (size_t i=0; i<dimension(); ++i) {
    is >> weights[i];
    if (! is) {
      std::cerr << "load failed at " << i << " in " << dimension() << "\n";
      break;
    }
  }
  return static_cast<bool>(is);
}

gpsshogi::weight_t gpsshogi::PredictionModelLight::
addGradientTakeBack(int progress8, Move next, Move target, double sum, const valarray_t& weights,
		    valarray_t& partial)
{
  return addGradient(next, target, sum, weights, partial, progress8*4 + 0);
}

gpsshogi::weight_t gpsshogi::PredictionModelLight::
addGradientSeePlus(const StateInfo& info, Move next, Move target, double sum, const valarray_t& weights,
		   valarray_t& partial)
{
  const int progress8 = info.progress8();
  return addGradient(next, target, sum, weights, partial, progress8*4 + 2);
}

gpsshogi::weight_t gpsshogi::PredictionModelLight::
addGradient(Move next, Move target, double sum, const valarray_t& weights,
	    valarray_t& partial, int offset)
{
  // p = a * r + b
  assert(partial.size() == dimension());
  assert(weights.size() == dimension());
  valarray_t x(dimension());
  x = 0.0;
  x[0+offset] = sum;
  x[1+offset] = 1.0;
  weight_t f = (x*weights).sum();
  weight_t p = 1/(1.0+exp(-f));
  if (target == next) {
    for (size_t j=0; j<x.size(); ++j) {
      assert(j < partial.size());
      partial[j] += x[j]*p*(1-p);
      assert(! isnan(partial[j]));
    }
  } else {
    for (size_t j=0; j<x.size(); ++j) {
      assert(j < partial.size());
      partial[j] -= x[j]*p*(1-p);
      assert(! isnan(partial[j]));
    }
  }
  return std::max(p, 1.0/(1<<12));
}

int gpsshogi::PredictionModelLight::
logProbTakeBack(const StateInfo& info, double sum, const valarray_t& w)
{
  return predict(info.progress8()*4 + 0, sum, w);
}
int gpsshogi::PredictionModelLight::
logProbSeePlus(const StateInfo& info, double sum, const valarray_t& w)
{
  return predict(info.progress8()*4 + 2, sum, w);
}
int gpsshogi::PredictionModelLight::
predict(int offset, double sum, const valarray_t& w)
{
  static const double scale = 100.0 / log(0.5);
  double x = w[offset] * sum + w[offset+1], p = 1/(1.0+exp(-x));
  return std::max(50, static_cast<int>(log(p)*scale));
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

