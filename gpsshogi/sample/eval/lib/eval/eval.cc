/* eval.cc
 */
#include "eval/eval.h"
#include "eval/kingEval.h"
#include "eval/piecePair.h"
#include "eval/ptypeAttacked.h"
#include "osl/eval/pieceEval.h"
#include "osl/eval/progressEval.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stack>

gpsshogi::
EvalValueStack::~EvalValueStack()
{
}    

osl::Progress16 
gpsshogi::
EvalValueStack::progress16() const
{
  return Progress16(0);
}



gpsshogi::
EvalBase::~EvalBase()
{
}    

void gpsshogi::
EvalBase::showSummary(std::ostream&) const
{
}

void gpsshogi::
EvalBase::showAll(std::ostream& os) const
{
  showSummary(os);
}

int gpsshogi::
EvalBase::pieceValue(const NumEffectState&, Piece) const
{
  return 0;
}

bool gpsshogi::
EvalBase::hasPieceValue() const
{
  return false;
}

void gpsshogi::
EvalBase::showEvalSummary(const NumEffectState&) const
{
}

int gpsshogi::
EvalBase::pawnValue() const
{
  return 128;
}

size_t gpsshogi::
Eval::maxActive() const
{
  return dimension();
}    

const std::string gpsshogi::
Eval::describe(const std::string& feature, size_t local_index) const
{
  return "Eval not implemented";
}

const std::string gpsshogi::
Eval::describeAll(const std::string& feature) const
{
  return "Eval::describeAll not implemented";
}

std::tuple<std::string, int, int> 
gpsshogi::Eval::findFeature(size_t /*index*/) const
{
  return std::make_tuple("not_imlemented", 0, 0);
}

void gpsshogi::Eval::features(const NumEffectState& state, MoveData& out) const
{
  out.clear();
  out.reserve(maxActive()*2);
  features(state, out, 0);
}

void gpsshogi::Eval::featuresProgress(const NumEffectState&, MoveData&) const
{
}

/* ------------------------------------------------------------------------- */

gpsshogi::
PieceEval::PieceEval() : HasWeights<Eval>(PTYPE_SIZE)
{
  for (size_t i=0; i<dimension(); ++i)
    values[i] = eval::Ptype_Eval_Table.value(static_cast<Ptype>(i));
  values[KING] = 0;
}

gpsshogi::
PieceEval::~PieceEval()
{
}

struct IntValueStack : public gpsshogi::EvalValueStack
{
  std::stack<int> values;
  void pop() { assert(! values.empty()); values.pop(); }
  int value() const { assert(! values.empty()); return values.top(); }
};

class gpsshogi::PieceEval::Stack : public IntValueStack
{
  const PieceEval *eval;
public:  
  Stack(PieceEval *e, const NumEffectState& state) : eval(e) { 
    reset(state);
  }
  void push(const NumEffectState& /*new_state*/, Move moved)
  {
    values.push(values.top()+eval->diff(moved));
  }
  void reset(const NumEffectState& state)
  {
    while (! values.empty())
      pop();
    values.push(eval->eval(state));
  }
};
gpsshogi::EvalValueStack *gpsshogi::
PieceEval::newStack(const NumEffectState& state)
{
  return new Stack(this, state);
}
int gpsshogi::
PieceEval::diff(Move moved) const
{
  if (moved.isPass())
    return 0;
  int diff = value(moved.ptypeO()) - value(moved.oldPtypeO());
  if (moved.capturePtype() != PTYPE_EMPTY) {
    diff -= value(moved.capturePtypeO());
    diff += value(captured(moved.capturePtypeO()));
  }
  return diff;
}

int gpsshogi::
PieceEval::eval(const NumEffectState& state) const
{
  int sum = 0;
  for (int i=0; i<Piece::SIZE; ++i) {
    const Piece piece = state.pieceOf(i);
    sum += value(piece.ptypeO());
  }
  return sum;
}

bool gpsshogi::
PieceEval::load(const char *filename)
{
  std::ifstream is(filename);
  for (size_t i=0; i<dimension(); ++i)
    is >> values[i];
  return static_cast<bool>(is);
}

void gpsshogi::
PieceEval::save(const char *filename) const
{
  std::ofstream os(filename);
  for (size_t i=0; i<dimension(); ++i)
  {
    if (static_cast<Ptype>(i) == KING)
      os << 12800 << std::endl;
    else
      os << values[i] << std::endl;
  }
}

void gpsshogi::
PieceEval::setWeight(const double* w)
{
  setWeightScale(w, 128.0/w[PAWN]);
}

void gpsshogi::
PieceEval::features(const NumEffectState& state, 
		    MoveData& out, int offset) const
{
  // assert(diffs.size() == 0); // XXX: fix PieceEvalComponent in progress.cc
  double value = 0.0;
  for (size_t i=0; i<dimension(); ++i) {
    const double d = differential(state, i);
    if (d) {
      out.diffs.push_back(std::make_pair((int)i+offset, d));
      value += this->flatValue(i)*d;
    }
  }
  out.value = value;
}

double gpsshogi::
PieceEval::differential(const NumEffectState& state, size_t index) const
{
  const Ptype ptype = static_cast<Ptype>(index);
  if (! isPiece(ptype)
#ifdef FIX_PAWN
      || ptype == PAWN
#endif
    )
    return 0.0;
  double result = 0.0;
  for (int i=Ptype_Table.getIndexMin(unpromote(ptype)); 
       i<Ptype_Table.getIndexLimit(unpromote(ptype)); ++i) {
    const Piece piece = state.pieceOf(i);
    if (piece.ptype() != ptype) // promote
      continue;
	
    result += (piece.owner()==BLACK) ? 1.0 : -1.0;
  }
  return result;
}

void gpsshogi::
PieceEval::showSummary(std::ostream& os) const
{
  for (size_t i=0; i<dimension(); ++i) {
    const Ptype ptype = static_cast<Ptype>(i);
    if (! isPiece(ptype))
      continue;
    const char *name = Ptype_Table.getCsaName(ptype);
    os << name[0] << name[1] << " " << values[i] << "  ";
#if 0
    if (ptype == KING)
      os << std::endl;
#endif
  }
  os << std::endl;
}

/* ------------------------------------------------------------------------- */

int gpsshogi::
EvalComponent::evalWithUpdate(const NumEffectState& state, Move, int) const
{
  return eval(state);
}

void gpsshogi::
EvalComponent::features(const NumEffectState& state, double& value,
			std::vector<std::pair<int, double> >& out, int offset) const
{
  index_list_t values;
  featuresNonUniq(state, values, offset);
#ifndef L1BALL_NO_SORT
#  error "add uniqWrite here"
#endif
  convert(values, value, out, offset);
}

void gpsshogi::
EvalComponent::convert(const index_list_t& values, double& value,
		       std::vector<std::pair<int, double> >& out, int offset) const
{
  value = 0;
  for (size_t i=0; i<values.size(); ++i) {
    value += this->value(values[i].first - offset) * values[i].second;
    out.push_back(std::make_pair(values[i].first, (double)values[i].second));
  }
}

void gpsshogi::
EvalComponent::showSummary(std::ostream& os) const
{
  size_t count = value(0)>0, sum = std::abs(value(0));
  int minw=value(0), maxw=value(0);
  for (size_t i=1; i<dimension(); ++i)
  {
    minw = std::min(value(i), minw);
    maxw = std::max(value(i), maxw);
    sum += std::abs(value(i));
    if (value(i))
      ++count;
  }
  os << name() << " min " << minw << " max " << maxw
     << " nonzero " << count << " " << count*100.0/dimension() << "% "
     << " ave(abs) " << 1.0*sum/count << " " << 1.0*sum/dimension() << "\n";
}

const std::string gpsshogi::
EvalComponent::describe(size_t local_index) const
{
  return "EvalComponent not implemented";
}




gpsshogi::
EvalComponentMulti::~EvalComponentMulti()
{
}

osl::MultiInt gpsshogi::
EvalComponentMulti::eval(const NumEffectState& state, const MultiWeights& weights,
			 CArray<MultiInt,2>& /*saved_state*/) const
{
  index_list_t features;
  featuresNonUniq(state, features, 0);
  MultiInt result;
  for (size_t j=0; j<features.size(); ++j) {
    const MultiInt w = weights.value(features[j].first);
    result += EvalComponentStages::multiply(w, features[j].second);
  }
  return result;
}

osl::MultiInt gpsshogi::
EvalComponentMulti::evalWithUpdate(const NumEffectState& state, Move, MultiInt, const MultiWeights& weights,
      CArray<MultiInt,2>& saved_state) const
{
  return eval(state, weights, saved_state);
}

void gpsshogi::
EvalComponentMulti::showSummary(std::ostream& os, const MultiWeights& w) const
{
  MultiInt minw=w.value(0), maxw=w.value(0);
  MultiInt count, sum;
  for (size_t i=1; i<w.oneDimension(); ++i)
  {
    for (size_t s=0; s<MultiInt::size(); ++s)
    {
      minw[s] = std::min(w.value(i)[s], minw[s]);
      maxw[s] = std::max(w.value(i)[s], maxw[s]);
      sum[s] += std::abs(w.value(i)[s]);
      if (w.value(i)[s])
	++count[s];
    }
  }
  for (size_t s=0; s<MultiInt::size(); ++s)
    os << name() << " min " << minw[s] << " max " << maxw[s]
     << " nonzero " << count[s] << " " << count[s]*100.0/w.oneDimension() << "% "
     << " ave(abs) " << 1.0*sum[s]/count[s] << " " << 1.0*sum[s]/w.oneDimension() << "\n";
}

void gpsshogi::
EvalComponentMulti::showAll(std::ostream& os, const MultiWeights& w) const
{
  showSummary(os, w);
}

osl::MultiInt gpsshogi::
EvalComponentMulti::pieceValue(const NumEffectState&, Piece, const MultiWeights&) const
{
  return MultiInt();
}

const std::string gpsshogi::
EvalComponentMulti::describe(size_t local_index) const
{
  return "EvalComponentMulti";
}

/* ------------------------------------------------------------------------- */

gpsshogi::
FeaturesOneNonUniq::~FeaturesOneNonUniq()
{
}

size_t gpsshogi::
FeaturesOneNonUniq::maxActive() const
{
  return one_dim;
}

void gpsshogi::
FeaturesOneNonUniq::showSummary(const Weights& w, std::ostream &os) const
{
  int minw=w.value(0), maxw=w.value(0);
  size_t count = 0, sum = 0;
  for (size_t i=1; i<w.dimension(); ++i)
  {
    minw = std::min(w.value(i), minw);
    maxw = std::max(w.value(i), maxw);
    sum += std::abs(w.value(i));
    if (w.value(i))
      ++count;
  }
  os << name() << " min " << minw << " max " << maxw
     << " nonzero " << count << " " << count*100.0/w.dimension() << "% "
     << " ave(abs) " << 1.0*sum/count << " " << 1.0*sum/w.dimension() << "\n";
}

void gpsshogi::
FeaturesOneNonUniq::showAllOne(const Weights& w,
			       int /*n*/,
			       std::ostream &os) const
{
  showSummary(w, os);
}

osl::MultiInt gpsshogi::
FeaturesOneNonUniq::pieceValue(const NumEffectState&, Piece, const MultiWeights&) const
{
  return MultiInt();
}

const std::string gpsshogi::
FeaturesOneNonUniq::describe(size_t local_index) const
{
  return "FeaturesOneNonUniq";
}

/* ------------------------------------------------------------------------- */

gpsshogi::
RichEval::RichEval(int richness, bool fix)
  : fix_piece(fix)
{
  if (richness == 2) 
  {
    others.push_back(new PiecePair);
  } 

  // compute dimension
  {
    size_t sum = fix_piece ? (size_t)1 : piece.dimension();
    for (size_t i=0; i<others.size(); ++i)
      sum += others[i].dimension();
    dim = sum;
  }

  max_active = piece.maxActive();
  for (size_t i=0; i<others.size(); ++i)
    max_active += others[i].maxActive();
}

gpsshogi::
RichEval::~RichEval()
{
}

size_t gpsshogi::
RichEval::maxActive() const
{
  return max_active;
}

class gpsshogi::RichEval::Stack : public EvalValueStack
{
  const RichEval *eval;
  enum { OthersLimit = 16 };
  struct Entry
  {
    int sum;
    int piece;
    CArray<int,OthersLimit> others;
    Entry() : sum(0) {}
  };
  std::stack<Entry> values;
public:  
  Stack(RichEval *ev, const NumEffectState& state) : eval(ev) { 
    assert((int)eval->others.size() <= OthersLimit);
    reset(state);
  }
  void push(const NumEffectState& new_state, Move moved)
  {
    Entry e = values.top();
    e.piece += eval->piece.diff(moved);
    e.sum = e.piece;
    for (size_t i=0; i<eval->others.size(); ++i) {
      e.others[i] = eval->others[i].evalWithUpdate(new_state, moved, e.others[i]);
      // std::cerr << "push " << i << "  " << e.others[i] << "\n";
      e.sum += e.others[i];
    }
    values.push(e);
  }
  void pop() { assert(! values.empty()); values.pop(); }
  int value() const { assert(! values.empty()); return values.top().sum; }
  void reset(const NumEffectState& state) 
  {
    while (! values.empty())
      pop();
    Entry e;
    e.sum = e.piece = eval->piece.eval(state);
    for (size_t i=0; i<eval->others.size(); ++i) {
      e.others[i] = eval->others[i].eval(state);
      e.sum += e.others[i];
    }
    values.push(e);
  }  
};
gpsshogi::EvalValueStack * gpsshogi::
RichEval::newStack(const NumEffectState& state)
{
  return new Stack(this, state);
}

int gpsshogi::
RichEval::eval(const NumEffectState& state) const
{
  int sum = piece.eval(state);
  for (size_t i=0; i<others.size(); ++i) {
    const int value = others[i].eval(state);
    // std::cerr << "eval " << i << "  " << value << "\n";
    sum += value;
  }
  return sum;
}

bool gpsshogi::
RichEval::load(const char *filename)
{
  boost::scoped_array<double> values(new double[dimension()]);
  std::ifstream is(filename);
  for (size_t i=0; i<dimension(); ++i)
    is >> values[i];
  setWeight(&values[0]);
  return static_cast<bool>(is);
}

void gpsshogi::
RichEval::save(const char *filename) const
{
  std::ofstream os(filename);
  if (fix_piece) {
    os << 1 << std::endl;
  } else {
    for (size_t i=0; i<piece.dimension(); ++i)
      os << piece.flatValue(i) << std::endl;
  }
  for (size_t i=0; i<others.size(); ++i) {
    for (size_t j=0; j<others[i].dimension(); ++j)
      os << others[i].value(j) << std::endl;
  }
}

void gpsshogi::
RichEval::setWeightScale(const double*w, double scale)
{
  if (! fix_piece)
    piece.setWeightScale(&w[0], scale);
  int dim = fix_piece ? 1 : piece.dimension();
  for (size_t i=0; i<others.size(); ++i) {
    others[i].setWeightScale(&w[0]+dim, scale);
    dim += others[i].dimension();
  }
}

void gpsshogi::
RichEval::setWeight(const double *w)
{
  double scale = 1.0;
  if (fix_piece)
    scale = 1.0/w[0];
  else 
    scale = 128.0/w[PAWN];
  setWeightScale(w, scale);
}

void gpsshogi::
RichEval::saveWeight(double *w) const
{
  if (fix_piece)
    w[0] = 0.0;
  else
    piece.saveWeight(&w[0]);
  int dim = fix_piece ? 1 : piece.dimension();
  for (size_t i=0; i<others.size(); ++i) {
    others[i].saveWeight(&w[0]+dim);
    dim += others[i].dimension();
  }
}

void gpsshogi::
RichEval::features(const NumEffectState& state, MoveData& out, int offset) const
{
  double value = 0;
  if (fix_piece) {
    value = piece.eval(state);
    out.diffs.push_back(std::make_pair(0, value));
  } else {
    piece.features(state, out, offset);
    value = out.value;
  }
  int dim = fix_piece ? 1 : piece.dimension();
  for (size_t i=0; i<others.size(); ++i) {
    double his_value = 0.0;
    others[i].features(state, his_value, out.diffs, offset+dim);
    value += his_value;
    dim += others[i].dimension();
  }
  out.value = value;
}

void gpsshogi::
RichEval::showSummary(std::ostream& os) const
{
  piece.showSummary(os);
  for (size_t i=0; i<others.size(); ++i)
    others[i].showSummary(os);
}

void gpsshogi::
RichEval::showAll(std::ostream& os) const
{
  if (! fix_piece)
    piece.showAll(os);
  for (size_t i=0; i<others.size(); ++i)
    others[i].showAll(os);
}

void gpsshogi::
RichEval::setRandom()
{
  if (! fix_piece)
    piece.setRandom();
  for (size_t i=0; i<others.size(); ++i)
    others[i].setRandom();
}

int gpsshogi::
RichEval::pieceValue(const NumEffectState& state, Piece p) const
{
  int sum = 0;
  for (size_t i=0; i<others.size(); ++i)
    sum += others[i].pieceValue(state, p);
  return sum;
}

bool gpsshogi::
RichEval::hasPieceValue() const
{
  return true;
}

void gpsshogi::
RichEval::showEvalSummary(const NumEffectState& state) const
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


/* ------------------------------------------------------------------------- */

gpsshogi::
EvalComponentStages::~EvalComponentStages()
{
}

osl::MultiInt gpsshogi::
EvalComponentStages::evalMulti(const NumEffectState &state, CArray<MultiInt,2>& /*save_state*/) const
{
  MultiInt result;
  index_list_t values;
  featuresOneNonUniq(state, values);
  for (size_t i = 0; i < values.size(); ++i)
  {
    MultiInt weight = this->weight.value(values[i].first);
    result += multiply(weight, values[i].second);
  }
  return result;
}

void gpsshogi::
EvalComponentStages::setWeightScale(const double *w, const double& scale)
{
  for (size_t i=0; i<one_dim; ++i) {
    MultiInt value;
    for (size_t s=0; s<value.size(); ++s)
      value[s] = (int)round(w[one_dim*s + i] * scale);
    weight.setValue(i, value);
  }
}

const gpsshogi::Weights gpsshogi::
EvalComponentStages::convertToWeights() const
{
  Weights result(one_dim*MultiInt::size());
  for (size_t i=0; i<one_dim; ++i) {
    for (size_t s=0; s<MultiInt::size(); ++s)
      result.setValue(one_dim*s + i, weight.value(i)[s]);
  }
  return result;
}

osl::MultiInt gpsshogi::
EvalComponentStages::makeValue(const index_list_t& features) const
{
  MultiInt result;
  for (size_t i = 0; i < features.size(); ++i)
  {
    MultiInt weight = this->weight.value(features[i].first);
    result += multiply(weight, features[i].second);
  }
  return  result;
}

const std::string gpsshogi::
EvalComponentStages::describe(size_t local_index) const
{
  return delegate->describe(local_index % one_dim) + " stage "
    + std::to_string(local_index/one_dim);
}


/* ------------------------------------------------------------------------- */

osl::MultiInt gpsshogi::
EvalComponentStagesBW::evalMulti(const NumEffectState &state, CArray<MultiInt,2>& save_state) const
{
  save_state[BLACK] = evalBlack(state);
  save_state[WHITE] = evalWhite(state);
  return save_state[BLACK]+save_state[WHITE];
}

osl::MultiInt gpsshogi::
EvalComponentStagesBW::evalBlack(const NumEffectState &state) const
{
  MultiInt result;
  index_list_t values;
  featureOneBlack(state, values);
  for (size_t i = 0; i < values.size(); ++i)
  {
    MultiInt weight = this->weight.value(values[i].first);
    result += multiply(weight, values[i].second);
  }
  return  result;
}

osl::MultiInt gpsshogi::
EvalComponentStagesBW::evalWhite(const NumEffectState &state) const
{
  MultiInt result;
  index_list_t values;
  featureOneWhite(state, values);
  for (size_t i = 0; i < values.size(); ++i)
  {
    MultiInt weight = this->weight.value(values[i].first);
    result += multiply(weight, values[i].second);
  }
  return  result;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
