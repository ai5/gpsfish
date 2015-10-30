/* openMidEnding.cc
 */
#include "eval/openMidEnding.h"
#include "eval/progress.h"
#include "eval/pieceStand.h"
#include "eval/pin.h"
#include "eval/piecePair.h"
#include "eval/mobility.h"
#include "eval/piecePair.h"
#include "eval/kingEval.h"
#include "eval/minorPiece.h"
#include "eval/majorPiece.h"
#include "eval/ptypeAttacked.h"
#include "eval/ptypeEval.h"
#include "eval/pieceEvalComponent.h"
#include "eval/ptypeEval.h"
#include "eval/piecePairKing.h"
#include "osl/progress.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/bits/binaryIO.h"
#include "osl/oslConfig.h"

#include <boost/filesystem.hpp>
#include <iomanip>
#include <fstream>
#include <iostream>

gpsshogi::
OpenMidEndingBase::OpenMidEndingBase(bool p)
  : progress_max_16(p)
{
  assert(osl::progress::ml::NewProgress::initialized());
}

gpsshogi::
OpenMidEndingBase::~OpenMidEndingBase()
{
}

void gpsshogi::
OpenMidEndingBase::addFinished()
{
  dim = max_active = legacy_one_dim = 0;
  for (size_t i = 0; i < flat.size(); ++i)
  {
    dim += flat[i].dimension();
    max_active += flat[i].maxActive();
  }
  for (size_t i = 0; i < legacy.size(); ++i)
  {
    legacy_one_dim += legacy[i].dimensionOne();
    dim += legacy[i].dimensionOne() * MultiInt::size();
    max_active += legacy[i].maxActiveOne() * MultiInt::size();
    legacy_weights.push_back(new MultiWeights(legacy[i].dimensionOne()));
  }
  for (size_t i = 0; i < triple.size(); ++i)
  {
    dim += triple[i].dimension();
    max_active += triple[i].maxActive();
  }
}

int gpsshogi::OpenMidEndingBase::maxProgress() const
{
  if (progress_max_16)
    return 16;
  return osl::progress::ml::NewProgress::maxProgress();
}


int gpsshogi::OpenMidEndingBase::progress(const NumEffectState &state) const
{
  osl::progress::ml::NewProgress progress(state);
  return this->progress(progress.progress());
}

int gpsshogi::OpenMidEndingBase::progress(int progress_value) const
{
  if (progress_max_16)
    return progress_value*16/(1+progress::ml::NewProgress::maxProgress()); // [0:15]
  return progress_value;
}

size_t gpsshogi::
OpenMidEndingBase::maxActive() const
{
  return max_active;
}

std::tuple<std::string, int, int> gpsshogi::
OpenMidEndingBase::findFeature(size_t index) const
{
  for (size_t i=0; i<flat.size(); ++i) {
    if (index < flat[i].dimension())
      return std::make_tuple(flat[i].name(), index, flat[i].dimension());
    index -= flat[i].dimension();
  }
  for (size_t s=0; s<MultiInt::size(); ++s) {
    for (size_t i=0; i<legacy.size(); ++i) {
      if (index < legacy[i].dimensionOne())
	return std::make_tuple(legacy[i].name(), index + legacy[i].dimensionOne()*s, legacy[i].dimensionOne());
      index -= legacy[i].dimensionOne();
    }
  }
  for (size_t i=0; i<triple.size(); ++i) {
    if (index < triple[i].dimension())
      return std::make_tuple(triple[i].name(), index, triple[i].dimensionOne());
    index -= triple[i].dimension();
  }
  assert(0);
  return std::make_tuple("not found", 0, 0);
}

namespace gpsshogi
{
  template <class Eval>
  struct OMDStackBase : public EvalValueStack
  {
    const Eval *eval;
    enum { OthersLimit = 128 };
    struct Entry
    {
      int sum;
      progress::ml::NewProgress progress;
      CArray<int,OthersLimit> flat;
      CArray<MultiInt,OthersLimit> legacy;
      CArray<MultiInt ,OthersLimit> triple;
      CArray<CArray<MultiInt,2> ,OthersLimit> legacy_state;
      CArray<CArray<MultiInt,2> ,OthersLimit> triple_state;
      Entry() : sum(0), progress(NumEffectState()) {}
    };
    std::stack<Entry> values;
    OMDStackBase(Eval *ev, const NumEffectState& state) : eval(ev) { 
      assert((int)eval->flat.size() <= OthersLimit);
      assert((int)eval->legacy.size() <= OthersLimit);
      assert((int)eval->triple.size() <= OthersLimit);
      reset(state);
    }
    void push(const NumEffectState& new_state, Move moved)
    {
      Entry e = values.top();
      e.sum = 0;
      e.progress.update(new_state, moved);
      const int progress = eval->progress(e.progress.progress());
      for (size_t i=0; i<eval->flat.size(); ++i)
      {
	if (const EvalProgress *p = dynamic_cast<const EvalProgress*>(&(eval->flat[i])))
	  e.flat[i] = p->eval(new_state, e.progress);
	else
	  e.flat[i] = eval->flat[i].evalWithUpdate(new_state, moved, e.flat[i]);
      }
      MultiInt multi_sum;
      for (size_t i=0; i<eval->legacy.size(); ++i) {
	e.legacy[i] = eval->legacy[i].evalWithUpdate(new_state, moved, e.legacy[i], eval->legacy_weights[i], e.legacy_state[i]);
	multi_sum += e.legacy[i];
      }
      for (size_t i=0; i<eval->triple.size(); ++i) {
	e.triple[i] = eval->triple[i].evalWithUpdateMulti(new_state, moved, e.triple[i], e.triple_state[i]);
	multi_sum += e.triple[i];
      }
      // sum
      for (size_t i=0; i<eval->flat.size(); ++i)
	e.sum += e.flat[i];
      e.sum = eval->compose(e.sum, multi_sum, progress);
      values.push(e);
    }
    void pop() { assert(! values.empty()); values.pop(); }
    int value() const { assert(! values.empty()); return values.top().sum; }
    Progress16 progress16() const {
      assert(! values.empty());
      return values.top().progress.progress16();
    }
    void reset(const NumEffectState& state)
    {
      while(! values.empty())
	pop();
      Entry e;
      e.sum = 0;
      e.progress = progress::ml::NewProgress(state);
      const int progress = eval->progress(e.progress.progress());
      for (size_t i=0; i<eval->flat.size(); ++i)
	e.flat[i] = eval->flat[i].eval(state);
      MultiInt multi_sum;
      for (size_t i=0; i<eval->legacy.size(); ++i) {
	e.legacy[i] = eval->legacy[i].eval(state, eval->legacy_weights[i], e.legacy_state[i]);
	multi_sum += e.legacy[i];
      }
      for (size_t i=0; i<eval->triple.size(); ++i) {
	e.triple[i] = eval->triple[i].evalMulti(state, e.triple_state[i]);
	multi_sum += e.triple[i];
      }
      // sum
      for (size_t i=0; i<eval->flat.size(); ++i)
	e.sum += e.flat[i];
      e.sum = eval->compose(e.sum, multi_sum, progress);
      values.push(e);
    }
  };
}

gpsshogi::EvalValueStack * gpsshogi::
OpenMidEndingBase::newStack(const NumEffectState& state)
{
  return new OMDStackBase<OpenMidEndingBase>(this, state);
}

int gpsshogi::
OpenMidEndingBase::pawnValue() const
{
#ifdef EVAL_QUAD
  const int progress_max = maxProgress();
  const int c0 = progress_max/3, c1 = c0*2, w2 = progress_max - c1;
  int c0_c = c0, w2_c = w2;
  if (! progress_max_16) {
    c0_c = w2_c = 1;
    assert(c0 == w2);
  }
  const int scale = c0_c * w2;
#else
  const int scale = maxProgress();
#endif
  return 128.0 * scale;
}

int gpsshogi::
OpenMidEndingBase::progressIndependentValue(const NumEffectState& state) const
{
  int sum = 0;
  for (size_t i=0; i<flat.size(); ++i) 
  {
    int value = flat[i].eval(state);
    sum += value;
  }
  return sum;
}
int gpsshogi::
OpenMidEndingBase::openingValue(const NumEffectState& state) const
{
  int sum = 0;
  for (size_t i=0; i<legacy.size(); ++i)
    sum += legacyValue(i, state)[0];
  CArray<MultiInt,2> tmp;
  for (size_t i = 0; i < triple.size(); ++i)
    sum += triple[i].evalMulti(state, tmp)[0];
  return sum;
}
int gpsshogi::
OpenMidEndingBase::midgameValue(const NumEffectState& state) const
{
  int sum = 0;
  for (size_t i=0; i<legacy.size(); ++i)
    sum += legacyValue(i, state)[1];
  CArray<MultiInt,2> tmp;
  for (size_t i = 0; i < triple.size(); ++i)
    sum += triple[i].evalMulti(state, tmp)[1];
  return sum;
}
#ifdef EVAL_QUAD
int gpsshogi::
OpenMidEndingBase::midgame2Value(const NumEffectState& state) const
{
  int sum = 0;
  CArray<MultiInt,2> tmp;
  for (size_t i=0; i<legacy.size(); ++i)
    sum += legacyValue(i, state)[2];
  for (size_t i = 0; i < triple.size(); ++i)
    sum += triple[i].evalMulti(state, tmp)[2];
  return sum;
}
#endif
int gpsshogi::
OpenMidEndingBase::endgameValue(const NumEffectState& state) const
{
  int sum = 0;
  for (size_t i=0; i<legacy.size(); ++i)
    sum += legacyValue(i, state)[EndgameIndex];
  CArray<MultiInt,2> tmp;
  for (size_t i = 0; i < triple.size(); ++i)
    sum += triple[i].evalMulti(state, tmp)[EndgameIndex];
  return sum;
}

osl::MultiInt gpsshogi::
OpenMidEndingBase::legacyValue(size_t i, const NumEffectState& state) const
{
  CArray<MultiInt,2> tmp;
  assert(i < legacy.size());
  return legacy[i].eval(state, legacy_weights[i], tmp);
}

int gpsshogi::
OpenMidEndingBase::eval(const NumEffectState& state) const
{
  // TODO: optimize this to use "tri" more effectively.
  const int progress_max = maxProgress();
  const int progress = this->progress(state);
#ifdef EVAL_QUAD
  const int c0 = progress_max/3, c1 = c0*2, w2 = progress_max - c1;
  int c0_c = c0, w2_c = w2;
  if (! progress_max_16) {
    c0_c = w2_c = 1;
    assert(c0 == w2);
  }
  int sum = progressIndependentValue(state) * c0_c * w2;
  if (progress < c0) {
    const int scale = w2_c * (c0 - progress);
    sum += openingValue(state) * scale;
  }
  if (progress > 0 && progress < c1) {
    const int scale = (progress < c0) ? (w2_c * progress) : (w2_c * (c1 - progress));
    sum += midgameValue(state) * scale;
  }
  if (progress > c0) {
    const int scale = (progress < c1) ? (w2_c * (progress-c0)) : (c0_c * (progress_max - progress));
    sum += midgame2Value(state) * scale;
  }
  if (progress > c1) {
    const int scale = c0_c * (progress - c1);
    sum += endgameValue(state) * scale;
  }
#else
  const int c = progress_max/2;
  int sum = progressIndependentValue(state) * progress_max;
  if (progress < c) 
  {
    sum += openingValue(state) * 2*(c - progress);
    sum += midgameValue(state) * 2*progress;
  }
  else 
  {
    sum += midgameValue(state) * 2*(progress_max - progress);
    sum += endgameValue(state) * 2*(progress - c);
  }
#endif
  return sum;
}

bool gpsshogi::
OpenMidEndingBase::load(const char *filename)
{
  boost::scoped_array<double> values(new double[dimension()]);
  std::ifstream is(filename);
  for (size_t i=0; i<dimension(); ++i)
    is >> values[i];
  setWeight(&values[0]);
  return static_cast<bool>(is);
}

void gpsshogi::
OpenMidEndingBase::save(const char *filename) const
{
  std::ofstream os(filename);
  for (size_t i=0; i<flat.size(); ++i) {
    for (size_t j=0; j<flat[i].dimension(); ++j)
      os << flat[i].value(j) << std::endl;
  }
  for (size_t s=0; s<MultiInt::size(); ++s) {
    for (size_t i=0; i<legacy.size(); ++i) {
      for (size_t j=0; j<legacy[i].dimensionOne(); ++j)
	os << legacy_weights[i].value(j)[s] << std::endl;
    }
  }
  for (size_t i=0; i<triple.size(); ++i) {
    for (size_t j=0; j<triple[i].dimension(); ++j)
      os << triple[i].value(j) << std::endl;
  }
  std::string f = filename;  
  if (f.find(".txt") != f.npos) {
    f = f.substr(0, f.find(".txt")) + "-info.txt";
    std::ofstream os(f.c_str());

    const time_t now = time(0);
    char ctime_buf[64];
    os << "# generated " << ctime_r(&now, ctime_buf);
    os << "#* flat" << std::endl;
    for (size_t i=0; i<flat.size(); ++i) 
      os << flat[i].name() << " " << flat[i].dimension() << std::endl;
    os << "#* opening" << std::endl;
    for (size_t i=0; i<legacy.size(); ++i) 
      os << legacy[i].name() << " " << legacy[i].dimensionOne() << std::endl;
    os << "#* midgame" << std::endl;
    for (size_t i=0; i<legacy.size(); ++i) 
      os << legacy[i].name() << " " << legacy[i].dimensionOne() << std::endl;
#ifdef EVAL_QUAD
    os << "#* midgame2" << std::endl;
    for (size_t i=0; i<legacy.size(); ++i) 
      os << legacy[i].name() << " " << legacy[i].dimensionOne() << std::endl;
#endif
    os << "#* endgame" << std::endl;
    for (size_t i=0; i<legacy.size(); ++i) 
      os << legacy[i].name() << " " << legacy[i].dimensionOne() << std::endl;
#ifdef EVAL_QUAD
    os << "#* quadruple" << std::endl;
#else
    os << "#* triple" << std::endl;
#endif
    for (size_t i=0; i<triple.size(); ++i) 
      os << triple[i].name() << " " << triple[i].dimension() << std::endl;
  }
}

void gpsshogi::
OpenMidEndingBase::setWeight(const double *w)
{
  double scale = 1.0;
  // assuming we have PieceEval
  scale = 128.0/w[PAWN];
  setWeightScale(w, scale);
}

void gpsshogi::
OpenMidEndingBase::setWeightScale(const double *w, double scale)
{
  int dim = 0;
  for (size_t i=0; i<flat.size(); ++i) {
    flat[i].setWeightScale(&w[0]+dim, scale);
    dim += flat[i].dimension();
  }
  for (size_t i=0; i<legacy.size(); ++i) {
    for (size_t j=0; j<legacy[i].dimensionOne(); ++j) {
      MultiInt value;
      for (size_t s=0; s<value.size(); ++s)
	value[s] = (int)round(w[legacy_one_dim*s + dim + j] * scale);
      legacy_weights[i].setValue(j, value);
    }
    dim += legacy[i].dimensionOne();
  }
  dim += legacy_one_dim*(MultiInt::size()-1);
  for (size_t i=0; i<triple.size(); ++i) {
    triple[i].setWeightScale(&w[0]+dim, scale);
    dim += triple[i].dimension();
  }
}

void gpsshogi::
OpenMidEndingBase::saveWeight(double *w) const
{
  int dim = 0;
  for (size_t i=0; i<flat.size(); ++i) {
    flat[i].saveWeight(&w[0]+dim);
    dim += flat[i].dimension();
  }
  for (size_t s=0; s<MultiInt::size(); ++s) {
    for (size_t i=0; i<legacy.size(); ++i) {
      for (size_t j=0; j<legacy[i].dimensionOne(); ++j)
	w[j+dim] = legacy_weights[i].value(j)[s];
      dim += legacy[i].dimensionOne();
    }
  }
  for (size_t i=0; i<triple.size(); ++i) {
    triple[i].saveWeight(&w[0]+dim);
    dim += triple[i].dimension();
  }
}

void gpsshogi::
OpenMidEndingBase::features(const NumEffectState& state, 
			    MoveData& data, int offset) const
{
  std::vector<std::pair<int,double> >& out = data.diffs;
  const int progress_max = maxProgress();
  const osl::progress::ml::NewProgress raw_progress(state);
  const int progress = this->progress(raw_progress.progress());
#ifdef EVAL_QUAD
  const int c0 = progress_max/3, c1 = c0*2, w2 = progress_max - c1;
  const int flat_scale = c0 * w2;
#else
  const int c = progress_max/2;
  const int flat_scale = progress_max;
#endif
  double value = 0.0;
  int dim = 0;
  for (size_t i=0; i<flat.size(); ++i) {
    double his_value = 0.0;
    if (const EvalProgress *p = dynamic_cast<const EvalProgress*>(&flat[i])) 
    {
      index_list_t his_features;
      p->featuresNonUniq(state, raw_progress, his_features, offset+dim);
      flat[i].convert(his_features, his_value, out, offset+dim);
    }
    else
      flat[i].features(state, his_value, out, offset+dim);
    value += his_value;
    dim += flat[i].dimension();
  }
  size_t start = out.size();
  for (size_t i = 0; i < start; ++i) {
    out[i].second *= flat_scale;
  }
  start = out.size();
  MultiInt multi_sum;
  for (size_t i=0; i<legacy.size(); ++i) {
    index_list_t his_features;
    legacy[i].featuresNonUniq(state, his_features, 0);
    for (size_t j=0; j<his_features.size(); ++j) {
      MultiInt w = legacy_weights[i].value(his_features[j].first);
      multi_sum += EvalComponentStages::multiply(w, (int)his_features[j].second);
#ifdef EVAL_QUAD
      // [0, c0]  : w2*(opening * (c0 - progress) + midgame * progress)
      // [c0,c1]  : w2*(midgame * (c1 - progress) + midgame2 * (progress-c0))
      // [c1,pmax]: c0*(midgame2 * (pmax - progress) + endgame * (progress - c1))
      if (progress < c0) 
      {
	out.push_back(std::make_pair(his_features[j].first + offset + dim,
				     his_features[j].second * w2 * (c0 - progress)));
	if (progress > 0)
	  out.push_back(std::make_pair(his_features[j].first + offset + dim + legacy_one_dim,
				       his_features[j].second * w2 * progress));
      }
      else if (progress < c1) 
      {
	out.push_back(std::make_pair(his_features[j].first + offset + dim + legacy_one_dim,
				     his_features[j].second * w2 * (c1 - progress)));
	if (progress > c0)
	  out.push_back(std::make_pair(his_features[j].first + offset + dim + legacy_one_dim*2,
				       his_features[j].second * w2 * (progress - c0)));
      }
      else 
      {
	out.push_back(std::make_pair(his_features[j].first + offset + dim + legacy_one_dim*2,
				     his_features[j].second * c0 * (progress_max - progress)));
	if (progress > c1)
	  out.push_back(std::make_pair(his_features[j].first + offset + dim + legacy_one_dim*3,
				       his_features[j].second * c0 * (progress - c1)));
      }
#else
      if (progress < c) 
      {
	out.push_back(std::make_pair(his_features[j].first + offset + dim,
				     his_features[j].second * 2 * (c - progress)));
	out.push_back(std::make_pair(his_features[j].first + offset + dim + legacy_one_dim,
				     his_features[j].second * 2 * progress));
      }
      else 
      {
	out.push_back(std::make_pair(his_features[j].first + offset + dim + legacy_one_dim,
				     his_features[j].second * 2 * (progress_max - progress)));
	out.push_back(std::make_pair(his_features[j].first + offset + dim + legacy_one_dim*2,
				     his_features[j].second * 2 * (progress - c)));
      }
#endif
    }
    dim += legacy[i].dimensionOne();
  }
  dim += legacy_one_dim * (MultiInt::size()-1);
  start = out.size();
  for (size_t i=0; i<triple.size(); ++i) {
    MultiInt his_value;
    triple[i].featuresMulti(state, his_value, out, offset+dim,
			    progress, progress_max);
    multi_sum += his_value;
    dim += triple[i].dimension();
  }
  assert(out.size() <= maxActive());
#ifdef EVAL_QUAD
  if (c0 == w2)
    for (size_t i = 0; i < out.size(); ++i) 
      out[i].second /= c0;
#endif

  data.value = compose(value, multi_sum, progress);
  data.progress = progress;
}

void gpsshogi::
OpenMidEndingBase::showSummary(std::ostream& os) const
{
  for (size_t i=0; i<flat.size(); ++i)
    flat[i].showSummary(os);
  for (size_t s=0; s<MultiInt::size(); ++s) {
    for (size_t i=0; i<legacy.size(); ++i) {
      legacy[i].showSummary(os, legacy_weights[i]);
    }
  }
  for (size_t i=0; i<triple.size(); ++i)
    triple[i].showSummary(os);
}

void gpsshogi::
OpenMidEndingBase::showAll(std::ostream& os) const
{
  for (size_t i=0; i<flat.size(); ++i)
    flat[i].showAll(os);
  for (size_t s=0; s<MultiInt::size(); ++s) {
    for (size_t i=0; i<legacy.size(); ++i) {
      legacy[i].showAll(os, legacy_weights[i]);
    }
  }
  for (size_t i=0; i<triple.size(); ++i)
    triple[i].showAll(os);
}

void gpsshogi::
OpenMidEndingBase::setRandom()
{
  for (size_t i=0; i<flat.size(); ++i)
    flat[i].setRandom();
  for (size_t i=0; i<legacy.size(); ++i)
    legacy_weights[i].setRandom();
  for (size_t i=0; i<triple.size(); ++i)
    triple[i].setRandom();
}

int gpsshogi::
OpenMidEndingBase::pieceValue(const NumEffectState& state, Piece p) const
{
  int sum = 0;
  MultiInt multi_sum;
  for (size_t i=0; i<flat.size(); ++i)
    sum += flat[i].pieceValue(state, p);
  for (size_t s=0; s<MultiInt::size(); ++s) {
    for (size_t i=0; i<legacy.size(); ++i) {
      multi_sum += legacy[i].pieceValue(state, p, legacy_weights[i]);
    }
  }
  for (size_t i=0; i<triple.size(); ++i)
    multi_sum += triple[i].pieceValue(state, p);
  return compose(sum, multi_sum, progress(state));
}

bool gpsshogi::
OpenMidEndingBase::hasPieceValue() const
{
  return true;
}

void gpsshogi::
OpenMidEndingBase::showEvalSummary(const NumEffectState& state) const
{
  for (int y=1; y<=9; ++y) {
    for (int x=9; x>=1; --x) {
      const Square position(x,y);
      const Piece piece = state.pieceOnBoard(position);
      if (piece.isPiece())
	std::cerr << std::setw(4) << pieceValue(state, piece);
      else
	std::cerr << "    ";
    }
    std::cerr << "\n";
  }
}

void gpsshogi::
OpenMidEndingBase::debug(const NumEffectState& state) const
{
  for (size_t i=0; i<legacy.size(); ++i)
    std::cerr << legacy[i].name() << " " << legacyValue(i, state)[0] << "\n";
  CArray<MultiInt,2> tmp;
  for (size_t i = 0; i < triple.size(); ++i)
    std::cerr << triple[i].name() << " " << triple[i].evalMulti(state, tmp)[0] << "\n";
}

const std::string gpsshogi::
OpenMidEndingBase::describe(const std::string& feature, size_t local_index) const
{
  for (size_t i=0; i<flat.size(); ++i) {
    if (flat[i].name() == feature) {
      if (local_index >= flat[i].dimension())
	return "index error";
      return flat[i].describe(local_index)
	+ " value " + std::to_string(flat[i].value(local_index));
    }
  }
  for (size_t i=0; i<legacy.size(); ++i) {
    if (legacy[i].name() == feature) {
      const int dim = legacy[i].dimensionOne();
      if (local_index >= dim * EvalStages)
	return "index error";
      const int value = legacy_weights[i].value(local_index%dim)
	[local_index/dim];	  
      return legacy[i].describe(local_index)
	+ " stage " + std::to_string(local_index/dim)
	+ " value " + std::to_string(value);
    }
  }
  for (size_t i=0; i<triple.size(); ++i) {
    if (triple[i].name() == feature) {
      if (local_index >= triple[i].dimension())
	return "index error";
      return triple[i].describe(local_index)
	+ " value " + std::to_string(triple[i].value(local_index));
    }
  }
  return "not found";
}

const std::string gpsshogi::
OpenMidEndingBase::describeAll(const std::string& feature) const
{
  for (size_t i=0; i<flat.size(); ++i) {
    if (flat[i].name() == feature) {
      std::string all;
      for (size_t j=0; j<flat[i].dimension(); ++j)
	all += describe(feature, j) + "\n";
      return all;
    }
  }
  for (size_t i=0; i<legacy.size(); ++i) {
    if (legacy[i].name() == feature) {
      const int dim = legacy[i].dimensionOne();
      std::string all;
      for (int s=0; s<EvalStages; ++s) 
	for (int j=0; j<dim; ++j)
	  all += describe(feature, s*dim + j) + "\n";
      return all;
    }
  }
  for (size_t i=0; i<triple.size(); ++i) {
    if (triple[i].name() == feature) {
      std::string all;
      for (int j=0; j<triple[i].dimension(); ++j)
	all += describe(feature, j) + "\n";
      return all;
    }
  }
  return "not found";
}

/* ------------------------------------------------------------------------- */

gpsshogi::OpenMidEndingForTest::
OpenMidEndingForTest(int /*richness*/) : OpenMidEndingBase(true)
{
  flat.push_back(new PieceEvalComponent);
  // legacy.push_back(new PieceEvalComponent);
  triple.push_back(new EvalComponentStages(new RookRook));
  addFinished();
}

gpsshogi::OpenMidEndingForTest::
~OpenMidEndingForTest()
{
}

/* ------------------------------------------------------------------------- */

gpsshogi::KOpenMidEnding::
KOpenMidEnding() : OpenMidEndingBase(true)
{
  flat.push_back(new PieceEvalComponent);
  flat.push_back(new PiecePair);
  flat.push_back(new King25EffectAttack);
  flat.push_back(new King25EffectYAttack);
  flat.push_back(new PiecePairKingFlat);
  flat.push_back(new BishopExchangeSilverKing);
  flat.push_back(new EnterKingDefense);
  // 
  legacy.push_back(new PieceStand);
  legacy.push_back(new King25EffectEach);
  legacy.push_back(new PawnDrop);
  legacy.push_back(new PawnDrop(false));
  legacy.push_back(new NoPawnOnStand);
  legacy.push_back(new GoldRetreat);
  legacy.push_back(new SilverRetreat);
  legacy.push_back(new KnightAdvance);
  legacy.push_back(new AllMajor);
  legacy.push_back(new KingXBlocked);
  legacy.push_back(new KingXBlockedY);
  legacy.push_back(new AllGold);
  legacy.push_back(new PtypeX);
  legacy.push_back(new PtypeY);
  legacy.push_back(new AnagumaEmpty);
  legacy.push_back(new NonPawnPieceStand);
  legacy.push_back(new King25EffectDefense);
  legacy.push_back(new King25EffectYDefense);
  legacy.push_back(new RookMobility);
  legacy.push_back(new BishopMobility);
  legacy.push_back(new LanceMobility);
  legacy.push_back(new RookEffect);
  legacy.push_back(new RookEffect(false));
  legacy.push_back(new BishopEffect);
  legacy.push_back(new BishopEffect(false));
  legacy.push_back(new PawnAdvance);
  legacy.push_back(new PawnDropY(false));
  legacy.push_back(new PawnDropY);
  legacy.push_back(new KnightCheck);
  // 
  triple.push_back(new PieceKingRelativeStages);
  triple.push_back(new EvalComponentStages(new NonPawnPieceStandTurn));
  triple.push_back(new King25EffectEachXStages);
  triple.push_back(new King25EffectEachYStages);
  triple.push_back(new EvalComponentStages(new RookPawnY));
  triple.push_back(new RookEffectPieceStages);
  triple.push_back(new BishopEffectPieceStages);
  triple.push_back(new EvalComponentStages(new PieceStandY));
  triple.push_back(new RookEffectPieceKingRelative);
  triple.push_back(new BishopEffectPieceKingRelative);
  triple.push_back(new EvalComponentStages(new RookPawnYX));
  triple.push_back(new PawnPtypeOPtypeOStages);
  triple.push_back(new EvalComponentStages(new PromotedMinorPieces));
  triple.push_back(new EvalComponentStages(new PieceKingRelativeNoSupport));
  triple.push_back(new EvalComponentStages(new NonPawnAttacked));
  triple.push_back(new PtypeYYStages);
  triple.push_back(new PawnPtypeOPtypeOYStages);
  triple.push_back(new EvalComponentStages(new PawnDropX));
  triple.push_back(new EvalComponentStages(new King3Pieces));
  triple.push_back(new EvalComponentStages(new King3PiecesXY));
  triple.push_back(new King25EffectEachXYStages);
  triple.push_back(new EvalComponentStages(new BishopHead));
  triple.push_back(new EvalComponentStages(new BishopHeadKingRelative));
  triple.push_back(new EvalComponentStages(new KnightCheckY));
  triple.push_back(new EvalComponentStages(new KnightHead));
  triple.push_back(new EvalComponentStages(new RookPromoteDefense));
  triple.push_back(new EvalComponentStages(new PawnDropPawnStand));
  triple.push_back(new EvalComponentStages(new PawnDropPawnStandX));
  triple.push_back(new EvalComponentStages(new PawnDropPawnStandY));
  triple.push_back(new EvalComponentStages(new KnightHeadOppPiecePawnOnStand));
  triple.push_back(new EvalComponentStages(new KingXBothBlocked));
  triple.push_back(new EvalComponentStages(new KingXBothBlockedY));
  triple.push_back(new EvalComponentStages(new KingRookBishop));
  triple.push_back(new EvalComponentStages(new PromotedMinorPiecesY));
  triple.push_back(new EvalComponentStages(new King25EffectSupported));
  triple.push_back(new EvalComponentStages(new King25EffectSupportedY));
  triple.push_back(new EvalComponentStages(new NonPawnAttackedKingRelatve));
  triple.push_back(new EvalComponentStages(new NonPawnAttackedPtype));
  triple.push_back(new PtypeCount);
  triple.push_back(new EvalComponentStages(new KingXBlocked3));
  triple.push_back(new EvalComponentStages(new KingXBlocked3Y));
  triple.push_back(new PtypeCountXY);
  triple.push_back(new PtypeCountXYAttack);
  triple.push_back(new EvalComponentStages(new LanceEffectPieceKingRelative));
  triple.push_back(new EvalComponentStages(new KingMobility));
  triple.push_back(new EvalComponentStages(new KingMobilitySum));
  triple.push_back(new PtypeYPawnY);
  triple.push_back(new EvalComponentStages(new GoldAndSilverNearKing));
  triple.push_back(new EvalComponentStages(new PtypeCombination));
  triple.push_back(new EvalComponentStages(new PieceStandCombinationBoth));
  triple.push_back(new King25BothSide);
  triple.push_back(new King25BothSideX);
  triple.push_back(new King25BothSideY);
  triple.push_back(new EvalComponentStages(new GoldAndSilverNearKingCombination));
  triple.push_back(new EvalComponentStages(new KingMobilityWithRook));
  triple.push_back(new EvalComponentStages(new KingMobilityWithBishop));
  triple.push_back(new EvalComponentStages(new NumPiecesBetweenBishopAndKingSelf));
  triple.push_back(new EvalComponentStages(new NumPiecesBetweenBishopAndKingOpp));
  triple.push_back(new EvalComponentStages(new NumPiecesBetweenBishopAndKingAll));
  triple.push_back(new EvalComponentStages(new King25Effect3));
  triple.push_back(new EvalComponentStages(new SilverHeadPawnKingRelative));
  triple.push_back(new EvalComponentStages(new GoldKnightKingRelative));
  triple.push_back(new EvalComponentStages(new RookMobilitySum));
  triple.push_back(new EvalComponentStages(new RookMobilityX));
  triple.push_back(new EvalComponentStages(new RookMobilityY));
  triple.push_back(new EvalComponentStages(new RookMobilitySumKingX));
  triple.push_back(new EvalComponentStages(new RookMobilityXKingX));
  triple.push_back(new EvalComponentStages(new PinPtype));
  triple.push_back(new EvalComponentStages(new PinPtypeDistance));
  triple.push_back(new EvalComponentStages(new BishopMobilityEach));
  triple.push_back(new EvalComponentStages(new BishopBishopPiece));
  triple.push_back(new EvalComponentStages(new NonPawnPieceStandCombinationEach));
  triple.push_back(new EvalComponentStages(new CanCheckNonPawnPieceStandCombinationEach));
  triple.push_back(new EvalComponentStages(new King25Effect3Y));
  triple.push_back(new EvalComponentStages(new RookRook));
  triple.push_back(new EvalComponentStages(new RookRookPiece));
  triple.push_back(new EvalComponentStages(new PinPtypePawnAttack));
  triple.push_back(new King25KingMobility);
  triple.push_back(new King25KingMobilityX);
  triple.push_back(new King25KingMobilityY);
  triple.push_back(new EvalComponentStages(new King25EffectCountCombination));
  triple.push_back(new EvalComponentStages(new GoldSideMove));
  triple.push_back(new EvalComponentStages(new King25EffectCountCombinationY));
  triple.push_back(new EvalComponentStages(new RookPromoteDefenseRookH));
  triple.push_back(new EvalComponentStages(new BishopHeadX));
  triple.push_back(new EvalComponentStages(new PawnDropNonDrop));
  triple.push_back(new EvalComponentStages(new PawnStateKingRelative));
  triple.push_back(new EvalComponentStages(new SilverFork));
  triple.push_back(new EvalComponentStages(new BishopRookFork));
  triple.push_back(new EvalComponentStages(new BishopStandRank5));
  triple.push_back(new EvalComponentStages(new KnightFork));
  triple.push_back(new EvalComponentStages(new NonPawnAttackedPtypePair));
  triple.push_back(new EvalComponentStages(new MajorCheckWithCapture));
  triple.push_back(new EvalComponentStages(new SilverAdvance26));
  triple.push_back(new EvalComponentStages(new RookSilverKnight));
  triple.push_back(new EvalComponentStages(new BishopSilverKnight));
  triple.push_back(new EvalComponentStages(new AttackMajorsInBase));
  triple.push_back(new EvalComponentStages(new CheckShadowPtype));
  triple.push_back(new EvalComponentStages(new Promotion37));
  addFinished();
}

gpsshogi::KOpenMidEnding::
~KOpenMidEnding()
{
}

/* ------------------------------------------------------------------------- */

gpsshogi::StableOpenMidEnding::
StableOpenMidEnding() : OpenMidEndingBase(false)
{
  flat.push_back(new PieceEvalComponent);
  flat.push_back(new PiecePair);
  flat.push_back(new King25EffectAttack);
  flat.push_back(new King25EffectYAttack);
  flat.push_back(new PiecePairKingFlat);
  flat.push_back(new BishopExchangeSilverKing);
  flat.push_back(new EnterKingDefense);
  // 
  legacy.push_back(new PieceStand);
  legacy.push_back(new King25EffectEach);
  legacy.push_back(new PawnDrop);
  legacy.push_back(new PawnDrop(false));
  legacy.push_back(new NoPawnOnStand);
  legacy.push_back(new GoldRetreat);
  legacy.push_back(new SilverRetreat);
  legacy.push_back(new KnightAdvance);
  legacy.push_back(new AllMajor);
  legacy.push_back(new KingXBlocked);
  legacy.push_back(new KingXBlockedY);
  legacy.push_back(new AllGold);
  legacy.push_back(new PtypeX);
  legacy.push_back(new PtypeY);
  legacy.push_back(new AnagumaEmpty);
  legacy.push_back(new NonPawnPieceStand);
  legacy.push_back(new King25EffectDefense);
  legacy.push_back(new King25EffectYDefense);
  legacy.push_back(new RookMobility);
  legacy.push_back(new BishopMobility);
  legacy.push_back(new LanceMobility);
  legacy.push_back(new RookEffect);
  legacy.push_back(new RookEffect(false));
  legacy.push_back(new BishopEffect);
  legacy.push_back(new BishopEffect(false));
  legacy.push_back(new PawnAdvance);
  legacy.push_back(new PawnDropY(false));
  legacy.push_back(new PawnDropY);
  legacy.push_back(new KnightCheck);
  // 
  triple.push_back(new PieceKingRelativeStages);
  triple.push_back(new EvalComponentStages(new NonPawnPieceStandTurn));
  triple.push_back(new King25EffectEachXStages);
  triple.push_back(new King25EffectEachYStages);
  triple.push_back(new EvalComponentStages(new RookPawnY));
  triple.push_back(new RookEffectPieceStages);
  triple.push_back(new BishopEffectPieceStages);
  triple.push_back(new EvalComponentStages(new PieceStandY));
  triple.push_back(new RookEffectPieceKingRelative);
  triple.push_back(new BishopEffectPieceKingRelative);
  triple.push_back(new EvalComponentStages(new RookPawnYX));
  triple.push_back(new PawnPtypeOPtypeOStages);
  triple.push_back(new EvalComponentStages(new PromotedMinorPieces));
  triple.push_back(new EvalComponentStages(new PieceKingRelativeNoSupport));
  triple.push_back(new EvalComponentStages(new NonPawnAttacked));
  triple.push_back(new PtypeYYStages);
  triple.push_back(new PawnPtypeOPtypeOYStages);
  triple.push_back(new EvalComponentStages(new PawnDropX));
  triple.push_back(new EvalComponentStages(new King3Pieces));
  triple.push_back(new EvalComponentStages(new King3PiecesXY));
  triple.push_back(new King25EffectEachXYStages);
  triple.push_back(new EvalComponentStages(new BishopHead));
  triple.push_back(new EvalComponentStages(new BishopHeadKingRelative));
  triple.push_back(new EvalComponentStages(new KnightCheckY));
  triple.push_back(new EvalComponentStages(new KnightHead));
  triple.push_back(new EvalComponentStages(new RookPromoteDefense));
  triple.push_back(new EvalComponentStages(new PawnDropPawnStand));
  triple.push_back(new EvalComponentStages(new PawnDropPawnStandX));
  triple.push_back(new EvalComponentStages(new PawnDropPawnStandY));
  triple.push_back(new EvalComponentStages(new KnightHeadOppPiecePawnOnStand));
  triple.push_back(new EvalComponentStages(new KingXBothBlocked));
  triple.push_back(new EvalComponentStages(new KingXBothBlockedY));
  triple.push_back(new EvalComponentStages(new KingRookBishop));
  triple.push_back(new EvalComponentStages(new PromotedMinorPiecesY));
  triple.push_back(new EvalComponentStages(new King25EffectSupported));
  triple.push_back(new EvalComponentStages(new King25EffectSupportedY));
  triple.push_back(new EvalComponentStages(new NonPawnAttackedKingRelatve));
  triple.push_back(new EvalComponentStages(new NonPawnAttackedPtype));
  triple.push_back(new PtypeCount);
  triple.push_back(new EvalComponentStages(new KingXBlocked3));
  triple.push_back(new EvalComponentStages(new KingXBlocked3Y));
  triple.push_back(new PtypeCountXY);
  triple.push_back(new PtypeCountXYAttack);
  triple.push_back(new EvalComponentStages(new LanceEffectPieceKingRelative));
  triple.push_back(new EvalComponentStages(new KingMobility));
  triple.push_back(new EvalComponentStages(new KingMobilitySum));
  triple.push_back(new PtypeYPawnY);
  triple.push_back(new EvalComponentStages(new GoldAndSilverNearKing));
  triple.push_back(new EvalComponentStages(new PtypeCombination));
  triple.push_back(new EvalComponentStages(new PieceStandCombinationBoth));
  triple.push_back(new King25BothSide);
  triple.push_back(new King25BothSideX);
  triple.push_back(new King25BothSideY);
  triple.push_back(new EvalComponentStages(new GoldAndSilverNearKingCombination));
  triple.push_back(new EvalComponentStages(new KingMobilityWithRook));
  triple.push_back(new EvalComponentStages(new KingMobilityWithBishop));
  triple.push_back(new EvalComponentStages(new NumPiecesBetweenBishopAndKingSelf));
  triple.push_back(new EvalComponentStages(new NumPiecesBetweenBishopAndKingOpp));
  triple.push_back(new EvalComponentStages(new NumPiecesBetweenBishopAndKingAll));
  triple.push_back(new EvalComponentStages(new King25Effect3));
  triple.push_back(new EvalComponentStages(new SilverHeadPawnKingRelative));
  triple.push_back(new EvalComponentStages(new GoldKnightKingRelative));
  triple.push_back(new EvalComponentStages(new RookMobilitySum));
  triple.push_back(new EvalComponentStages(new RookMobilityX));
  triple.push_back(new EvalComponentStages(new RookMobilityY));
  triple.push_back(new EvalComponentStages(new RookMobilitySumKingX));
  triple.push_back(new EvalComponentStages(new RookMobilityXKingX));
  triple.push_back(new EvalComponentStages(new PinPtype));
  triple.push_back(new EvalComponentStages(new PinPtypeDistance));
  triple.push_back(new EvalComponentStages(new BishopMobilityEach));
  triple.push_back(new EvalComponentStages(new BishopBishopPiece));
  triple.push_back(new EvalComponentStages(new NonPawnPieceStandCombinationEach));
  triple.push_back(new EvalComponentStages(new CanCheckNonPawnPieceStandCombinationEach));
  triple.push_back(new EvalComponentStages(new King25Effect3Y));
  triple.push_back(new EvalComponentStages(new RookRook));
  triple.push_back(new EvalComponentStages(new RookRookPiece));
  triple.push_back(new EvalComponentStages(new PinPtypePawnAttack));
  triple.push_back(new King25KingMobility);
  triple.push_back(new King25KingMobilityX);
  triple.push_back(new King25KingMobilityY);
  triple.push_back(new EvalComponentStages(new King25EffectCountCombination));
  triple.push_back(new EvalComponentStages(new GoldSideMove));
  triple.push_back(new EvalComponentStages(new King25EffectCountCombinationY));
  triple.push_back(new EvalComponentStages(new RookPromoteDefenseRookH));
  triple.push_back(new EvalComponentStages(new BishopHeadX));
  triple.push_back(new EvalComponentStages(new PawnDropNonDrop));
  triple.push_back(new EvalComponentStages(new PawnStateKingRelative));
  triple.push_back(new EvalComponentStages(new SilverFork));
  triple.push_back(new EvalComponentStages(new BishopRookFork));
  triple.push_back(new EvalComponentStages(new BishopStandRank5));
  triple.push_back(new EvalComponentStages(new KnightFork));
  triple.push_back(new EvalComponentStages(new NonPawnAttackedPtypePair));
  triple.push_back(new EvalComponentStages(new MajorCheckWithCapture));
  triple.push_back(new EvalComponentStages(new SilverAdvance26));
  triple.push_back(new EvalComponentStages(new RookSilverKnight));
  triple.push_back(new EvalComponentStages(new BishopSilverKnight));
  triple.push_back(new EvalComponentStages(new AttackMajorsInBase));
  triple.push_back(new EvalComponentStages(new CheckShadowPtype));
  triple.push_back(new EvalComponentStages(new Promotion37));
  addFinished();
}

gpsshogi::StableOpenMidEnding::
~StableOpenMidEnding()
{
}

/* ------------------------------------------------------------------------- */

gpsshogi::
OslOpenMidEnding::OslOpenMidEnding()
  : stable_eval(new StableOpenMidEnding), values(new int[dimension()])
{
  static bool initialized = false;
  if (! initialized) {
    initialized = true;
    osl::eval::ml::OpenMidEndingEval::setUp();
  }
  std::fill(&values[0], &values[0]+dimension(), 0);
  std::string filename = OslConfig::home();
  filename += "/data/eval.bin";

  if (boost::filesystem::exists(filename.c_str())) {
    std::cerr << "loading " << filename << "\n";
    std::ifstream is(filename.c_str());
    std::vector<double> w(dimension());
    typedef osl::misc::BinaryElementReader<int> reader_t;
    reader_t reader(is);
    for (size_t i=0; i<dimension(); ++i) {
      if (! reader.hasNext()) {
	std::cerr << filename << " read failed " << i << "\n";
	break;
      }
      w[i] = values[i] = reader.read();
    }
    stable_eval->setWeightScale(&w[0], 1.0);

    for (size_t i=0; i<dimension(); ++i) {
      if (std::get<0>(findFeature(i)) == "PiecePair")
	values[i] = stable_eval->flatValue(i);
      assert(values[i] == stable_eval->flatValue(i));
    }
  }  
}

gpsshogi::
OslOpenMidEnding::~OslOpenMidEnding()
{
}

int gpsshogi::OslOpenMidEnding::maxProgress() const
{
  return progress::ml::NewProgress::maxProgress();
}


int gpsshogi::OslOpenMidEnding::progress(const NumEffectState &state) const
{
  progress::ml::NewProgress progress(state);
  return this->progress(progress.progress());
}

int gpsshogi::OslOpenMidEnding::progress(int progress_value) const
{
  return progress_value;
}

size_t gpsshogi::
OslOpenMidEnding::maxActive() const
{
  return stable_eval->maxActive();
}

class gpsshogi::OslOpenMidEnding::Stack : public EvalValueStack
{
  struct Entry
  {
    osl::eval::ml::OpenMidEndingEval osl_eval;
    Entry() : osl_eval(NumEffectState()) {}
  };
  std::stack<Entry> values;
public:  
  Stack(OslOpenMidEnding *ev, const NumEffectState& state) { 
    reset(state);
  }
  void push(const NumEffectState& new_state, Move moved)
  {
    Entry e = values.top();
    e.osl_eval.update(new_state, moved);
    values.push(e);
  }
  void pop() { assert(! values.empty()); values.pop(); }
  int value() const {
    assert(! values.empty()); return values.top().osl_eval.value(); 
  }
  Progress16 progress16() const {
    assert(! values.empty());
    return values.top().osl_eval.progress16();
  }
  void reset(const NumEffectState& state)
  {
    while(! values.empty())
      pop();
    Entry e;
    e.osl_eval = osl::eval::ml::OpenMidEndingEval(state);
    values.push(e);
  }
};

gpsshogi::EvalValueStack * gpsshogi::
OslOpenMidEnding::newStack(const NumEffectState& state)
{
  return new Stack(this, state);
}

int gpsshogi::
OslOpenMidEnding::pawnValue() const
{
  const int scale = maxProgress();
  return 128 * scale;
}

int gpsshogi::
OslOpenMidEnding::openingValue(const NumEffectState& state) const
{
  osl::eval::ml::OpenMidEndingEval osl_eval(state);
  return osl_eval.openingValue();
}
int gpsshogi::
OslOpenMidEnding::midgameValue(const NumEffectState& state) const
{
  osl::eval::ml::OpenMidEndingEval osl_eval(state);
  return osl_eval.midgameValue();
}
#ifdef EVAL_QUAD
int gpsshogi::
OslOpenMidEnding::midgame2Value(const NumEffectState& state) const
{
  osl::eval::ml::OpenMidEndingEval osl_eval(state);
  return osl_eval.midgame2Value();
}
#endif
int gpsshogi::
OslOpenMidEnding::endgameValue(const NumEffectState& state) const
{
  osl::eval::ml::OpenMidEndingEval osl_eval(state);
  return osl_eval.endgameValue();
}
int gpsshogi::
OslOpenMidEnding::progressIndependentValue(const NumEffectState& state) const
{
  osl::eval::ml::OpenMidEndingEval osl_eval(state);
  return osl_eval.progressIndependentValue();
}

int gpsshogi::
OslOpenMidEnding::eval(const NumEffectState& state) const
{
  osl::eval::ml::OpenMidEndingEval osl_eval(state, false);
  return osl_eval.value();
}

size_t gpsshogi::
OslOpenMidEnding::dimension() const
{
  return stable_eval->dimension();
}
int gpsshogi::
OslOpenMidEnding::flatValue(size_t index) const 
{
  if (index >= dimension())
    std::cerr << "flatValue " << index << ' ' << dimension() << "\n";
  assert(index < dimension());
  return values[index];
}

bool gpsshogi::
OslOpenMidEnding::load(const char *filename)
{
  std::ifstream is(filename);
  boost::scoped_array<double> values(new double[dimension()]);
  for (size_t i=0; i<dimension(); ++i)
    is >> values[i];
  setWeight(&values[0]);
  return static_cast<bool>(is);
}

void gpsshogi::
OslOpenMidEnding::save(const char *filename) const
{
  stable_eval->save(filename);	// for "-info.txt"
  std::ofstream os(filename);
  for (size_t i=0; i<dimension(); ++i)
    os << values[i] << "\n";
  std::string f = filename;  
}

void gpsshogi::
OslOpenMidEnding::setWeight(const double *w)
{
  double scale = 1.0;
  // assuming we have PieceEval
  scale = 128.0/w[PAWN];
  setWeightScale(w, scale);
}

void gpsshogi::
OslOpenMidEnding::setWeightScale(const double *w, double scale)
{
  for (size_t i=0; i<dimension(); ++i)
    values[i] = round(w[i]*scale);
  osl::eval::ml::OpenMidEndingEval::resetWeights(&values[0], dimension());
  stable_eval->setWeightScale(w, scale);
}

void gpsshogi::
OslOpenMidEnding::saveWeight(double *w) const
{
  std::copy(&values[0], &values[0]+dimension(), w);
}

void gpsshogi::
OslOpenMidEnding::features(const NumEffectState& state, MoveData& out,
			   int offset) const
{
  assert(stable_eval->maxProgress() == maxProgress());
  stable_eval->features(state, out, offset);
}

void gpsshogi::
OslOpenMidEnding::showSummary(std::ostream& os) const
{
  stable_eval->showSummary(os);
}

void gpsshogi::
OslOpenMidEnding::showAll(std::ostream& os) const
{
  stable_eval->showAll(os);
}

void gpsshogi::
OslOpenMidEnding::setRandom()
{
}

bool gpsshogi::
OslOpenMidEnding::hasPieceValue() const
{
  return false;
}

std::tuple<std::string, int, int> gpsshogi::
OslOpenMidEnding::findFeature(size_t index) const
{
  return stable_eval->findFeature(index);
}

const std::string gpsshogi::
OslOpenMidEnding::describe(const std::string& feature, size_t local_index) const
{
  return stable_eval->describe(feature, local_index);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

