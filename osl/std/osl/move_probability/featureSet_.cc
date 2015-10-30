/* featureSet.cc
 */
#include "osl/move_probability/featureSet.h"
#include "osl/move_probability/feature.h"
#include "osl/container.h"
#include "osl/csa.h"
#include "osl/bits/binaryIO.h"
#include "osl/oslConfig.h"
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/format.hpp>
#include <mutex>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cstdio>
#include <cmath>

osl::move_probability::
FeatureSet::FeatureSet()
{
}

osl::move_probability::
FeatureSet::~FeatureSet()
{
}

void osl::move_probability::
FeatureSet::pushBack(Feature *f, bool light)
{
  features.push_back(f);
  if (light)
    light_features.push_back(features.size()-1);
}

void osl::move_probability::
FeatureSet::addFinished()
{
  offsets.resize(features.size()+1);
  offsets[0] = 0;
  for (size_t i=0; i<features.size(); ++i)
    offsets[i+1] = offsets[i] + features[i].dimension();
}

double osl::move_probability::FeatureSet::
matchNoExp(const StateInfo& state, Move move, const double * weights) const
{
  MoveInfo info(state, move);
  assert(offsets.size() == features.size()+1);
  double sum = 0.0;
  for (size_t i=0; i<features.size(); ++i) {
    sum += features[i].match(state, info, offsets[i], weights);
  }
  return sum;
}

double osl::move_probability::FeatureSet::
matchExp(const StateInfo& state, Move move, const double * weights) const
{
  return exp(matchNoExp(state, move, weights));
}

double osl::move_probability::FeatureSet::
matchLight(const StateInfo& state, Move move, const double * weights) const
{
  MoveInfo info(state, move);
  assert(offsets.size() == features.size()+1);
  double sum = 0.0;
  for (size_t i: light_features) {
    sum += features[i].match(state, info, offsets[i], weights);
  }
  return sum;
}


void osl::move_probability::FeatureSet::
analyze(const StateInfo& state, Move move, const double * weights) const
{
  MoveInfo info(state, move);
  std::cerr << csa::show(move) << "\n";
  std::vector<std::pair<double, std::string> > out;
  for (size_t i=0; i<features.size(); ++i) {
    double s = features[i].match(state, info, offsets[i], weights);
    if (s)
      out.push_back(make_pair(s, features[i].name()));
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

double osl::move_probability::FeatureSet::
generateRating(const StateInfo& state, WeightedMoveVector& out,
	       const double * weights) const
{
  assert(! state.dirty);
  MoveVector moves;
  state.state->generateLegal(moves);
  double sum = 0.0;
  FixedCapacityVector<Move,128> unpromote_moves;
  for (Move move: moves) {
    double score = matchExp(state, move, weights);
    out.push_back(WeightedMove(score, move));
    sum += score;
  }
  return sum;
}

void osl::move_probability::FeatureSet::
ratingToLogProb(const WeightedMoveVector& rating,
		double sum, MoveLogProbVector& out)
{
  static const double scale = 100.0 / log(0.5);
  for (WeightedMove move: rating) {
    double p = move.first/sum;
    if (std::isnan(p) || p <= 1.0/(1<<12)) 
      p = 1.0/(1<<12);
    const int logp = std::max(50, static_cast<int>(log(p)*scale));
    out.push_back(MoveLogProb(move.second, logp));
  }
  out.sortByProbability();
}

void osl::move_probability::FeatureSet::
generateLogProb(const StateInfo& state, MoveLogProbVector& out,
		const double * weights) const
{
  WeightedMoveVector moves;
  double sum = generateRating(state, moves, weights);
  ratingToLogProb(moves, sum, out);
}

bool osl::move_probability::FeatureSet::
load(const char *base_filename, double * weights) const
{
  std::string filename = std::string(base_filename) + ".txt";
  std::fill(weights, weights+dimension(), 0.0);
  std::ifstream is(filename.c_str());
  for (int i=0; i<dimension(); ++i) {
    is >> weights[i];
    if (! is) {
      std::cerr << "load failed at " << i << " in " << dimension()
		<< " file " << filename << "\n";
      break;
    }
  }
  return static_cast<bool>(is);
}

bool osl::move_probability::FeatureSet::
load_binary(const char *base_filename, double * weights) const
{
  std::string filename = std::string(base_filename) + ".bin";
  std::fill(weights, weights+dimension(), 0.0);
  std::ifstream is(filename.c_str(), std::ios_base::binary);
  misc::BinaryElementReader<double> reader(is);
  for (int i=0; i<dimension(); ++i) {
    if (! reader.hasNext()) {
      std::cerr << "load failed at " << i << " in " << dimension()
		<< " file " << filename << "\n";
      return false;
    }
    double value = reader.read();
    weights[i] = value;
  }
  return true;
}

void osl::move_probability::FeatureSet::
showSummary(const double * weights) const
{
  for (size_t i=0; i<features.size(); ++i) {
    const Feature& f = features[i];
#if (__GNUC_MINOR__ < 5)
    using namespace boost::accumulators;
    accumulator_set<double, stats<tag::mean, tag::min, tag::max> > acc;
#endif
    int zero = 0;
    for (int j=offsets[i]; j<offsets[i+1]; ++j)
      if (weights[j]) {
#if (__GNUC_MINOR__ < 5)
	acc(weights[j]);
#endif
      }
      else 
	++zero;
    std::cerr << std::setw(16) << f.name() 
	      << " dim " << std::setw(5) << f.dimension() - zero
	      << "/" << std::setw(5) << f.dimension()
#if (__GNUC_MINOR__ < 5)
	      << " min " << std::setw(6) << min(acc)
	      << " max " << std::setw(6) << max(acc)
	      << " mean " << std::setw(6) << mean(acc)
#endif
	      << "\n";
  }
}



boost::scoped_array<double> osl::move_probability::
StandardFeatureSet::weights;
boost::scoped_array<double> osl::move_probability::
StandardFeatureSet::tactical_weights;

osl::move_probability::StandardFeatureSet::
StandardFeatureSet() : initialized(false)
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

osl::move_probability::StandardFeatureSet::
~StandardFeatureSet()
{
}

const osl::move_probability::StandardFeatureSet& 
osl::move_probability::StandardFeatureSet::
instance(bool verbose)
{
  static StandardFeatureSet the_instance;
  the_instance.setUp(verbose);
  return the_instance;
}

bool osl::move_probability::StandardFeatureSet::
healthCheck()
{
  return instance(true).ok();
}

namespace osl
{
  namespace move_probability 
  {
    std::mutex standardfeatureset_lock;
  }
}
bool osl::move_probability::StandardFeatureSet::
setUp(bool verbose)
{
  std::lock_guard<std::mutex> lk(standardfeatureset_lock);
  static bool initialized = false;
  if (initialized)
    return true;
  initialized = true;
  weights.reset(new double[dimension()]);
  std::string filename = OslConfig::home();
  filename += "/data/move-order";
  if (verbose)
    std::cerr << "loading " << filename << ".bin ";
  const bool success = load_binary(filename.c_str(), &weights[0]);
  if (verbose)
    std::cerr << (success ? "success" : "failed\a") << "\n";

  filename = OslConfig::home();
  filename += "/data/move-tactical.txt";
  const int tactical_dimension = 8*4;
  tactical_weights.reset(new double[tactical_dimension]);
  if (verbose)
    std::cerr << "loading " << filename << " ";
  std::ifstream is(filename.c_str());
  for (int i=0; i<tactical_dimension; ++i)
    is >> tactical_weights[i];
  if (verbose)
    std::cerr << (is ? "success" : "failed\a") << "\n";
  this->initialized = success && is;
  return this->initialized;
}

void osl::move_probability::StandardFeatureSet::
generateLogProb(const StateInfo& state, MoveLogProbVector& out) const
{
  FeatureSet::generateLogProb(state, out, &weights[0]);
}

void osl::move_probability::StandardFeatureSet::
generateLogProb2(const StateInfo& state, MoveLogProbVector& out) const
{
  WeightedMoveVector moves;
  double sum = FeatureSet::generateRating(state, moves, &weights[0]);
  double elapsed = 0.0, welapsed = 0.0, last_p = 1.0;
  std::sort(moves.begin(), moves.end());
  for (int i=moves.size()-1; i>=0; --i) {
    WeightedMove move = moves[i];
    static const double scale = 100.0 / log(0.5);
    if (i+1<(int)moves.size())
      welapsed = std::max(welapsed, std::min(moves[i+1].first,move.first*4));
    double p = move.first/(sum-elapsed+welapsed);
    if (std::isnan(p) || p <= 1.0/(1<<12)) 
      p = 1.0/(1<<12);
    else
      p = std::min(last_p, p);
    int logp = std::max(50, static_cast<int>(log(p)*scale));
    if (moves.size() - i <= 8)
      logp = std::min(logp, 300);
    else if (moves.size() - i <= 16)
      logp = std::min(logp, 500);
    out.push_back(MoveLogProb(move.second, logp));
    elapsed += move.first;
    welapsed = (welapsed+move.first)*(moves.size()-i)/moves.size();
  }
}

void osl::move_probability::StandardFeatureSet::
generateLogProb(const StateInfo& state, int /*limit*/, MoveLogProbVector& out, bool /*in_pv*/) const
{
  generateLogProb2(state, out);
}

double osl::move_probability::StandardFeatureSet::
matchLight(const StateInfo& state, Move move) const
{
  return FeatureSet::matchLight(state, move, &weights[0]);
}

double osl::move_probability::StandardFeatureSet::
matchExp(const StateInfo& state, Move move) const
{
  return FeatureSet::matchExp(state, move, &weights[0]);
}

double osl::move_probability::StandardFeatureSet::
matchNoExp(const StateInfo& state, Move move) const
{
  return FeatureSet::matchNoExp(state, move, &weights[0]);
}

int osl::move_probability::StandardFeatureSet::
logProbTakeBack(const StateInfo& state, Move target) const
{
  const int progress8 = state.progress8();
  const double sum = matchLight(state, target);
  return tacticalLogProb(progress8*4 + 0, sum);
}

int osl::move_probability::StandardFeatureSet::
logProbSeePlus(const StateInfo& state, Move target) const
{
  const int progress8 = state.progress8();
  const double sum = matchLight(state, target);
  return tacticalLogProb(progress8*4 + 2, sum);
}

int osl::move_probability::StandardFeatureSet::
tacticalLogProb(int offset, double sum) const
{
  static const double scale = 100.0 / log(0.5);
  double x = tactical_weights[offset] * sum + tactical_weights[offset+1];
  double p = 1/(1.0+exp(-x));
  return std::max(50, static_cast<int>(log(p)*scale));
}




// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

