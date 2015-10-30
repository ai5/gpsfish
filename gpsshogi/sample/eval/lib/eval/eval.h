/* eval.h
 */
#ifndef GPSSHOGI_LEARN_EVAL_H
#define GPSSHOGI_LEARN_EVAL_H

#include "moveData.h"
#include "eval/indexCache.h"
#include "osl/numEffectState.h"
#include "osl/eval/weights.h"
#include "osl/progress.h"
#include "osl/random.h"
#include <boost/scoped_array.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <tuple>
#include <memory>
#include <utility>
#include <iosfwd>

namespace gpsshogi
{
  using namespace osl;
  enum { MaxActiveWithDuplication = 6144 }; // XXX: MaxActive + alpha
  typedef IndexCacheI<MaxActiveWithDuplication> index_list_t;
  class EvalValueStack
  {
  public:
    virtual ~EvalValueStack();
    virtual void push(const NumEffectState& new_state, Move moved)=0;
    virtual void pop()=0;
    virtual int value() const=0;
    virtual Progress16 progress16() const;
    virtual void reset(const NumEffectState& new_state)=0;
  };

  class EvalBase
  {
  public:
    virtual ~EvalBase();
    virtual int eval(const NumEffectState&) const=0;

    virtual void setWeight(const double*)=0;
    virtual void saveWeight(double*) const=0;

    virtual void showSummary(std::ostream&) const;
    virtual void showAll(std::ostream&) const;
    virtual void setRandom()=0;

    virtual int pieceValue(const NumEffectState& state, Piece p) const;
    virtual bool hasPieceValue() const;
    virtual void showEvalSummary(const NumEffectState&) const;
    
    virtual int pawnValue() const;
  };

  class Eval : public EvalBase
  {
  public:
    virtual bool load(const char *filename)=0;
    virtual void save(const char *filename) const=0;
    virtual void setWeightScale(const double*, double)=0;
    virtual size_t lambdaStart() const=0;
    virtual int flatValue(size_t index) const=0;
    virtual const std::string describe(const std::string& feature, size_t local_index) const;
    virtual const std::string describeAll(const std::string& feature) const;
    virtual std::tuple<std::string, int, int> findFeature(size_t index) const;
    virtual size_t dimension() const=0;
    virtual size_t maxActive() const;

    virtual EvalValueStack *newStack(const NumEffectState& state)=0;
    virtual void features(const NumEffectState&, MoveData&, int offset) const=0;
    void features(const NumEffectState& state, MoveData& out) const;
    /** 進行度に対する偏微分 */
    virtual void featuresProgress(const NumEffectState&, MoveData&) const;
    virtual int roundUp() const { return 0; }
    virtual int maxProgress() const { return 1; }
  };

  typedef osl::eval::ml::Weights Weights;
  typedef osl::eval::ml::MultiWeights MultiWeights;

  template <class EvalOrEvalBase>
  class HasWeights : public EvalOrEvalBase, public Weights
  {
  public:
    explicit HasWeights(size_t dim=0);
    ~HasWeights();
    
    int value(size_t index) const { return Weights::value(index); }
    size_t dimension() const { return Weights::dimension(); }
    virtual size_t maxActive() const;

    virtual void setWeightScale(const double*, double);
    virtual void setWeight(const double *w) { setWeightScale(w, 1.0); }
    virtual void saveWeight(double*) const;
    virtual void setRandom() { Weights::setRandom(); }
  };

  class PieceEval : public HasWeights<Eval>
  {
  public:
    PieceEval();
    ~PieceEval();
    int eval(const NumEffectState&) const;
    int newValue(const NumEffectState& new_state, Move moved, int old_value) const;
    bool load(const char *filename);
    void save(const char *filename) const;
    void setWeight(const double*);

    int flatValue(size_t index) const { return HasWeights<Eval>::value(index); }
    int value(PtypeO ptypeo) const { 
      const int v = flatValue(getPtype(ptypeo));
      return getOwner(ptypeo) == BLACK ? v : -v;
    }
    int value(Ptype ptype) const { return flatValue(ptype); }

    void features(const NumEffectState&, MoveData&, int) const;
    double differential(const NumEffectState& state, size_t index) const;
    void showSummary(std::ostream&) const;
    size_t lambdaStart() const { return dimension(); };

    int diff(Move) const;
    EvalValueStack *newStack(const NumEffectState& state);
    class Stack;
    friend class Stack;
  };

  class EvalComponent : public HasWeights<EvalBase> 
  {
  public:
    explicit EvalComponent(size_t dim=0) : HasWeights<EvalBase>(dim)
    {
    }
    /** 可能なら差分計算 */
    virtual int evalWithUpdate(const NumEffectState& state, Move moved, int last_value) const;
    virtual const std::string name() const=0;
    void features(const NumEffectState&, double& value,
		  std::vector<std::pair<int, double> >&, int offset) const;
    virtual void featuresNonUniq(const NumEffectState& state, index_list_t&, int offset) const = 0;
    void convert(const index_list_t&, double& value,
		 std::vector<std::pair<int, double> >&, int offset) const;
    void showSummary(std::ostream&) const;
    virtual const std::string describe(size_t local_index) const;
  };

  /** MultiIntを使うfeature 重みは外部に持つ */
  class EvalComponentMulti
  {
  protected:
    size_t one_dim;
  public:
    explicit EvalComponentMulti(size_t o) : one_dim(o) {}
    size_t dimensionOne() const { return one_dim; }
    virtual ~EvalComponentMulti();
    virtual const std::string name() const=0;
    virtual size_t maxActiveOne() const { return one_dim; }
    virtual void featuresNonUniq(const NumEffectState& state, index_list_t&, int offset) const = 0;

    virtual MultiInt eval(const NumEffectState& state, const MultiWeights& w,
			  CArray<MultiInt,2>& /*save_state*/) const;
    virtual MultiInt evalWithUpdate(const NumEffectState& state, Move moved, MultiInt last_value, const MultiWeights& w,
				    CArray<MultiInt,2>& /*saved_state*/) const;

    virtual void showSummary(std::ostream&, const MultiWeights& weights) const;
    virtual void showAll(std::ostream&, const MultiWeights& weights) const;
    virtual MultiInt pieceValue(const NumEffectState&, Piece, const MultiWeights&) const;
    virtual const std::string describe(size_t local_index) const;
  };

  class FeaturesOneNonUniq
  {
    size_t one_dim;
  public:
    explicit FeaturesOneNonUniq(size_t dim) : one_dim(dim) {}
    virtual ~FeaturesOneNonUniq();
    virtual void featuresOneNonUniq(const NumEffectState &state,
				    index_list_t &out) const=0;
    typedef FixedCapacityVector<std::pair<int, int>, MaxActiveWithDuplication> features_one_t;
    virtual void featuresOne(const NumEffectState &state, features_one_t&out) const
    {
      index_list_t features;
      featuresOneNonUniq(state, features);
      features.output(out, 0);
    }
    virtual const std::string name() const=0;
    virtual size_t maxActive() const;
    size_t dimension() const { return one_dim; }
    virtual void showSummary(const Weights&, std::ostream &os) const;
    virtual void showAllOne(const Weights&,
			    int n,
			    std::ostream &os) const;
    virtual MultiInt pieceValue(const NumEffectState&, Piece, const MultiWeights&) const;
    virtual const std::string describe(size_t local_index) const;
  };

  /** MultiIntを使うfeature 重みは自分で持つ FeaturesOneNonUniq を活用 */
  class EvalComponentStages
  {
  protected:
    std::unique_ptr<FeaturesOneNonUniq> delegate;
    MultiWeights weight;
    size_t one_dim;
  public:
    explicit EvalComponentStages(FeaturesOneNonUniq *f) 
      : weight(f->dimension()), one_dim(weight.oneDimension())
    {
      delegate.reset(f);
    }
    virtual ~EvalComponentStages();
    void featuresOneNonUniq(const NumEffectState &state,
			    index_list_t &out) const
    {
      delegate->featuresOneNonUniq(state, out);
    }
    const std::string name() const { return delegate->name(); }
    size_t dimension() const { return one_dim*MultiInt::size(); }
    size_t dimensionOne() const { return one_dim; }
    size_t maxActive() const { return delegate->maxActive()*MultiInt::size(); }
    size_t maxActiveOne() const { return delegate->maxActive(); }
    MultiInt pieceValue(const NumEffectState& state, Piece p) const { return delegate->pieceValue(state, p, weight); }
    void showSummary(std::ostream &os) const { 
      const Weights w = convertToWeights();
      delegate->showSummary(w, os); 
    }
    void showAll(std::ostream &os) const
    {
      const Weights w = convertToWeights();
      for (size_t i = 0; i < MultiInt::size(); ++i)
      {
	delegate->showAllOne(w, i, os);
      };
    }
    /** @param save_state evalWithUpdateMulti等で利用する値 King別, 大ごまのどちらかなど. */
    virtual MultiInt evalMulti(const NumEffectState &state, CArray<MultiInt,2>& /*save_state*/) const;
    /** @param saved_state 前の局面の計算で保存された値 */
    virtual MultiInt evalWithUpdateMulti(
      const NumEffectState& state,
      Move /*moved */,
      const MultiInt&/*last_values*/,
      CArray<MultiInt,2>& saved_state) const
    {
      return evalMulti(state, saved_state);
    }

    // compatibility: 
    // int stageValue(int index, int stage) const { return value(index + one_dim*stage); }
    // value(index + one_dim*stage) == weight.value(index)[stage] }
    void setWeightScale(const double *w, const double& scale);
    int value(size_t index) const { 
      const int index_one = index % one_dim;
      const int stage = index / one_dim;
      return weight.value(index_one)[stage]; 
    }
    MultiInt valueMulti(size_t index_one) const
    {
      return weight.value(index_one); 
    }
    const Weights convertToWeights() const;
    void setRandom() { weight.setRandom(); }
    static MultiInt multiply(const MultiInt& a, int b)
    {
#ifdef OSL_USE_SSE41
      return a * b;
#else
      MultiInt result;
      for (int i=0; i<b; ++i)
	result += a;
      for (int i=0; i>b; --i)
	result -= a;
      return result;
#endif
    }
    void saveWeight(double *w) const
    {
      for (size_t i=0; i<one_dim*MultiInt::size(); ++i)
	w[i] = value(i);
    }
  public:
    void featuresMulti(const NumEffectState &state,
		       MultiInt& value,
		       std::vector<std::pair<int, double> > &diffs,
		       int offset,
		       int progress, int progress_max) const
    {
#ifndef L1BALL_NO_SORT
#  errer "not supported"
#endif
      // c = progress_max/2
      // [0, c-1]         : 2*(opening * (c - progress) + mid * progress)
      // [c, progress_max]: 2*(mid * (progress_max - progress) + ending * (progress - c))
      value.clear();
      index_list_t values;
      featuresOneNonUniq(state, values);
      for (size_t i = 0; i < values.size(); ++i)
      {
	MultiInt weight = this->weight.value(values[i].first);
	value += multiply(weight, values[i].second);
#ifdef EVAL_QUAD
	const int c0 = progress_max/3, c1 = c0*2, w2 = progress_max - c1;
	if (progress < c0) 
	{
	  diffs.push_back(std::make_pair(values[i].first + offset,
					 values[i].second * w2 * (c0 - progress)));
	  if (progress > 0)
	    diffs.push_back(std::make_pair(values[i].first + offset + one_dim,
					   values[i].second * w2 * progress));
	}
	else if (progress < c1) 
	{
	  diffs.push_back(std::make_pair(values[i].first + offset + one_dim,
					 values[i].second * w2 * (c1 - progress)));
	  if (progress > c0)
	    diffs.push_back(std::make_pair(values[i].first + offset + one_dim*2,
					   values[i].second * w2 * (progress - c0)));
	}
	else 
	{
	  diffs.push_back(std::make_pair(values[i].first + offset + one_dim*2,
					 values[i].second * c0 * (progress_max - progress)));
	  if (progress > c1)
	    diffs.push_back(std::make_pair(values[i].first + offset + one_dim*3,
					   values[i].second * c0 * (progress - c1)));
	}
#else
	const int c = progress_max/2;
	if (progress < c) 
	{
	  diffs.push_back(std::make_pair(values[i].first + offset,
					 values[i].second * 2 * (c - progress)));
	  diffs.push_back(std::make_pair(values[i].first + offset + one_dim,
					 values[i].second * 2 * progress));
	}
	else 
	{
	  diffs.push_back(std::make_pair(values[i].first + offset + one_dim,
					 values[i].second * 2 * (progress_max - progress)));
	  diffs.push_back(std::make_pair(values[i].first + offset + one_dim*2,
					 values[i].second * 2 * (progress - c)));
	}
#endif
      }
    }
    MultiInt makeValue(const index_list_t&) const;
    const std::string describe(size_t local_index) const;
  };

  class EvalComponentStagesBW : public EvalComponentStages
  {
  public:
    explicit EvalComponentStagesBW(FeaturesOneNonUniq *f) : EvalComponentStages(f)
    {
    }
    MultiInt evalMulti(const NumEffectState &state, CArray<MultiInt,2>& /*save_state*/) const;
  protected:
    MultiInt evalBlack(const NumEffectState &state) const;
    MultiInt evalWhite(const NumEffectState &state) const;    
    virtual void featureOneBlack(const NumEffectState &state, index_list_t&) const=0;
    virtual void featureOneWhite(const NumEffectState &state, index_list_t&) const=0;    
  };


  class RichEval : public Eval
  {
    PieceEval piece;
    boost::ptr_vector<EvalComponent> others;
    bool fix_piece;
    size_t dim, max_active;
  public:
    RichEval(int richness, bool fix_piece=false);
    ~RichEval();
    int eval(const NumEffectState&) const;
    int newValue(const NumEffectState& new_state, Move moved, int old_value) const;

    bool load(const char *filename);
    void save(const char *filename) const;
    void setWeight(const double*w);
    void setWeightScale(const double*, double);
    void saveWeight(double*) const;

    int flatValue(size_t index) const {
      if (fix_piece) {
	if (index == 0)
	  return 1;
	--index;
      } else {
	size_t pdim = piece.dimension();
	if (index < pdim)
	  return piece.flatValue(index);
	index -= pdim;
      }
      for (size_t i=0; i<others.size(); ++i) {
	if (index < others[i].dimension())
	  return others[i].value(index);
	index -= others[i].dimension();
      }
      assert(0);
      return 0;
    }
    size_t dimension() const {
      return dim;
    }
    size_t lambdaStart() const { return fix_piece ? 1 : piece.dimension(); };
    void features(const NumEffectState&, MoveData&, int) const;
    void showSummary(std::ostream&) const;
    void showAll(std::ostream&) const;
    void setRandom();

    size_t maxActive() const;

    int pieceValue(const NumEffectState& state, Piece p) const;
    bool hasPieceValue() const;
    void showEvalSummary(const NumEffectState&) const;

    class Stack;
    friend class Stack;
    EvalValueStack *newStack(const NumEffectState& state);
  };

} // namespace gpsshogi

template <class EvalOrEvalBase>
gpsshogi::
HasWeights<EvalOrEvalBase>::HasWeights(size_t idim) 
  : Weights(idim)
{
}

template <class EvalOrEvalBase>
gpsshogi::
HasWeights<EvalOrEvalBase>::~HasWeights()
{
}

template <class EvalOrEvalBase>
void gpsshogi::HasWeights<EvalOrEvalBase>::
setWeightScale(const double *w, double scale) 
{
  for (size_t i=0; i<dim; ++i)
    values[i] = (int)round(w[i]*scale);
}

template <class EvalOrEvalBase>
void gpsshogi::HasWeights<EvalOrEvalBase>::
saveWeight(double *w) const
{
  for (size_t i=0; i<dim; ++i)
    w[i] = values[i];
}

template <class EvalOrEvalBase>
size_t gpsshogi::HasWeights<EvalOrEvalBase>::
maxActive() const
{
  return dimension();
}

#endif /* GPSSHOGI_LEARN_EVAL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
