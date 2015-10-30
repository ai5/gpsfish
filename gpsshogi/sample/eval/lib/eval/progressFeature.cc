/* progressFeature.cc
 */
#include "eval/progressFeature.h"
#include "eval/openMidEnding.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress/effect5x3.h"
#include "osl/additionalEffect.h"
#include "osl/csa.h"
#include <functional>
#include <numeric>
#include <fstream>
#include <iostream>
#include <stack>
#include <cmath>

typedef osl::eval::ml::OpenMidEndingEval ome_eval_t;

gpsshogi::
ProgressFeatureBase::ProgressFeatureBase()
  : max_progress(1023), scale(1.0)
{
  assert(ome_eval_t::initialized());
  static bool initialized = false;
  if (! initialized) {
    ome_eval_t::setUp();
    initialized = true;
  }
}

gpsshogi::
ProgressFeatureBase::~ProgressFeatureBase()
{
}

void gpsshogi::
ProgressFeatureBase::addFinished()
{
  dim = max_active = 0;
  for (size_t i = 0; i < all.size(); ++i)
  {
    dim += all[i].dimension();
    max_active += all[i].maxActive();
  }
  // for max_progress
  ++dim;
  ++max_active;
}

size_t gpsshogi::
ProgressFeatureBase::maxActive() const
{
  return max_active;
}

class gpsshogi::ProgressFeatureBase::Stack : public EvalValueStack
{
  const ProgressFeatureBase *eval;
  enum { OthersLimit = 16 };
  struct Entry
  {
    ome_eval_t test_eval;
    int eval_value;
    CArray<int,OthersLimit> all;
    explicit Entry(const NumEffectState& state=NumEffectState(SimpleState(HIRATE))) 
      : test_eval(state), eval_value(0) 
    {
    }
  };
  std::stack<Entry> values;
public:  
  Stack(ProgressFeatureBase *ev, const NumEffectState& state) : eval(ev) { 
    assert((int)eval->all.size() <= OthersLimit);
    reset(state);
  }
  void push(const NumEffectState& new_state, Move moved)
  {
    Entry e = values.top();
    e.test_eval.update(new_state, moved);
    int progress = 0;
    for (size_t i=0; i<eval->all.size(); ++i) {
      e.all[i] =
	eval->all[i].evalWithUpdate(new_state, moved, e.all[i]);
      progress += e.all[i];
    }
    int max = eval->maxProgress();
    e.eval_value = compose(progress, max,
			   e.test_eval.progressIndependentValue(),
			   e.test_eval.stageValue());
    values.push(e);
  }
  void pop() { assert(! values.empty()); values.pop(); }
  int value() const { assert(! values.empty()); return values.top().eval_value; }
  void reset(const NumEffectState& state)
  {
    while (! values.empty())
      pop();
    Entry e(state);
    int progress = 0;
    for (size_t i=0; i<eval->all.size(); ++i) {
      e.all[i] = eval->all[i].eval(state);
      progress += e.all[i];
    }
    int max = eval->maxProgress();
    e.eval_value = compose(progress, max,
			   e.test_eval.progressIndependentValue(),
			   e.test_eval.stageValue());
    values.push(e);
  }
};

gpsshogi::EvalValueStack * gpsshogi::
ProgressFeatureBase::newStack(const NumEffectState& state)
{
  return new Stack(this, state);
}

int gpsshogi::
ProgressFeatureBase::pawnValue() const
{
  return ome_eval_t::captureValue(newPtypeO(WHITE, PAWN))/2 
    * maxProgress() / osl::progress::ml::NewProgress::maxProgress(); // should be consistent with osl::progress::ml::NewProgress
}

int gpsshogi::
ProgressFeatureBase::progress(const NumEffectState& state) const
{
  int progress = 0;
  for (size_t i=0; i<all.size(); ++i) {
    const int value = all[i].eval(state);
    progress += value;
  }
  return progress;
}

int gpsshogi::
ProgressFeatureBase::eval(const NumEffectState& state) const
{
  ome_eval_t eval(state);
  int progress = 0;
  for (size_t i=0; i<all.size(); ++i) {
    const int value = all[i].eval(state);
    progress += value;
  }
  return compose(progress, maxProgress(),
		 eval.progressIndependentValue(),
		 eval.stageValue());
}

bool gpsshogi::
ProgressFeatureBase::load(const char *filename)
{
  boost::scoped_array<double> values(new double[dimension()]);
  std::ifstream is(filename);
  for (size_t i=0; i<dimension(); ++i)
    is >> values[i];
  setWeight(&values[0]);
  return static_cast<bool>(is);
}

void gpsshogi::
ProgressFeatureBase::save(const char *filename) const
{
  std::ofstream os(filename);
  for (size_t i=0; i<all.size(); ++i) {
    for (size_t j=0; j<all[i].dimension(); ++j)
      os << all[i].value(j) << std::endl;
  }
  os << max_progress << std::endl;

  std::string f = filename;
  std::cerr << f << " " << f.find(".txt") << " " << f.substr(0, f.find(".txt")) + "-info.txt" << std::endl;
  
  if (f.find(".txt") != f.npos) {
    f = f.substr(0, f.find(".txt")) + "-info.txt";
    std::ofstream os(f.c_str());

    const time_t now = time(0);
    char ctime_buf[64];
    os << "# generated " << ctime_r(&now, ctime_buf);
    os << "#* all" << std::endl;
    for (size_t i=0; i<all.size(); ++i) 
      os << all[i].name() << " " << all[i].dimension() << std::endl;
    os << "MaxProgress 1" << std::endl;
  }
}

void gpsshogi::
ProgressFeatureBase::setWeightScale(const double *w, double scale)
{
  this->scale = scale;
  int dim = 0;
  for (size_t i=0; i<all.size(); ++i) {
    all[i].setWeightScale(&w[0]+dim, scale);
    dim += all[i].dimension();
  }
  max_progress = round(w[dimension()-1]*scale);
}

void gpsshogi::
ProgressFeatureBase::setWeight(const double *w)
{
  if (! (w[dimension()-1] > 0))
    std::cerr << "error ProgressFeatureBase::setWeight max_progress is not positive.\n" << " " << w[dimension()-1] << "\n";
  assert(w[dimension()-1] > 0);
  setWeightScale(w, maxProgressConstraint()/w[dimension()-1]);
}

void gpsshogi::
ProgressFeatureBase::saveWeight(double *w) const
{
  int dim = 0;
  for (size_t i=0; i<all.size(); ++i) {
    all[i].saveWeight(&w[0]+dim);
    dim += all[i].dimension();
  }
  w[dimension()-1] = max_progress;
}

int gpsshogi::
ProgressFeatureBase::compose(int progress, int progress_max,
			     int flat, const MultiInt& stage_value)
{
  if (progress_max == 1024)
    progress_max = 1023;	// xxx
  return StableOpenMidEnding::compose
    (flat, stage_value, progress, progress_max, false);
}

int gpsshogi::
ProgressFeatureBase::featuresCommon(const NumEffectState& state, 
				    MoveData& data, int offset) const
{
  std::vector<std::pair<int,double> >& out = data.diffs;
  int dim = 0, progress = 0;
  for (size_t i=0; i<all.size(); ++i) {
    double his_value = 0.0;
    all[i].features(state, his_value, out, offset+dim);
    progress += his_value;
    dim += all[i].dimension();
  }

  const int progress_max = maxProgress();
  if (progress < 0)
    progress = 0;
  if (progress >= progress_max) 
    progress = progress_max-1;
  return progress;
}

void gpsshogi::
ProgressFeatureBase::featuresProgress(const NumEffectState& state, 
				      MoveData& data) const
{
  data.clear();
  data.reserve(maxActive()*2);
  data.progress = featuresCommon(state, data, 0);
  data.value = 0;
}

void gpsshogi::
ProgressFeatureBase::features(const NumEffectState& state, 
			      MoveData& data, int offset) const
{
  std::vector<std::pair<int,double> >& out = data.diffs;
  const ome_eval_t eval(state);
  const int progress = featuresCommon(state, data, 0);
  const int progress_max = maxProgress();
  const int flat_value = eval.progressIndependentValue();
  const MultiInt stage_value = eval.stageValue();

  const int c0 = progress_max/3, c1 = c0*2;
  if (progress < c0) {
    const double diff = stage_value[1] - stage_value[0];
    for (size_t i = 0; i < out.size(); ++i)
      out[i].second *= diff;
    out.push_back(std::make_pair(dimension()-1, (stage_value[0]+flat_value)/3.0));
  } else if (progress < c1) {
    const double diff = stage_value[2] - stage_value[1];
    for (size_t i = 0; i < out.size(); ++i)
      out[i].second *= diff;
    out.push_back(std::make_pair(dimension()-1, (2*stage_value[1]-stage_value[2]+flat_value)/3.0));
  } else {
    assert(c1 <= progress && c1 < progress_max);
    const double diff = stage_value[3] - stage_value[2];
    for (size_t i = 0; i < out.size(); ++i)
      out[i].second *= diff;
    out.push_back(std::make_pair(dimension()-1, (stage_value[2]*3-stage_value[3]*2 +flat_value)/3.0));
  }

  data.value = compose(progress, progress_max, flat_value, stage_value);
  data.progress = progress;
}

void gpsshogi::
ProgressFeatureBase::showSummary(std::ostream& os) const
{
  for (size_t i=0; i<all.size(); ++i)
    all[i].showSummary(os);
  os << "max_progress " << max_progress << "\n";
}

void gpsshogi::
ProgressFeatureBase::showAll(std::ostream& os) const
{
  for (size_t i=0; i<all.size(); ++i)
    all[i].showAll(os);
  os << "max_progress " << max_progress << "\n";
}

void gpsshogi::
ProgressFeatureBase::setRandom()
{
  for (size_t i=0; i<all.size(); ++i)
    for (size_t j=0; j<all[i].dimension(); ++j)
      all[i].setValue(j, osl::random()%4);
}

std::tuple<std::string, int, int> 
gpsshogi::ProgressFeatureBase::findFeature(size_t index) const
{
  for (size_t i=0; i<all.size(); ++i) {
    if (index < all[i].dimension())
      return std::make_tuple(all[i].name(), index, all[i].dimension());
    index -= all[i].dimension();
  }
  return std::make_tuple("max_progress", index, 1);
}

const std::string gpsshogi::
ProgressFeatureBase::describe(const std::string& feature, size_t local_index) const
{
  for (size_t i=0; i<all.size(); ++i) {
    if (all[i].name() == feature) {
      if (local_index >= all[i].dimension())
	return "index error";
      return all[i].describe(local_index);
    }
  }
  if (feature == "max_progress")
    return "max_progress";
  return "not found";
}

const std::string gpsshogi::
ProgressFeatureBase::describeAll(const std::string& feature) const
{
  for (size_t i=0; i<all.size(); ++i) {
    if (all[i].name() == feature) {
      std::string out;
      for (size_t j=0; j<all[i].dimension(); ++j) {
	int value = all[i].value(j);
	if (value != 0)
	  out += all[i].describe(j)
	    + " value " + std::to_string(value) + "\n";
      }
      return out;
    }
  }
  if (feature == "max_progress")
    return "max_progress";
  return "not found";
}

/* ------------------------------------------------------------------------- */

gpsshogi::NullProgressFeatureEval::NullProgressFeatureEval()
{
  addFinished();
}

gpsshogi::NullProgressFeatureEval::~NullProgressFeatureEval()
{
}

double gpsshogi::NullProgressFeatureEval::maxProgressConstraint() const
{
  return 16;
}

/* ------------------------------------------------------------------------- */

#if (defined LEARN_TEST_PROGRESS)
struct gpsshogi::HandProgressFeatureEval::HandProgress : public EvalComponent
{
  HandProgress() : EvalComponent(16)
  {
    for (int i=0; i<16; ++i)
      values[i] = i; // 1;
  }
  int eval(const osl::NumEffectState& state) const
  {
    return values[progress::Effect5x3(state).progress16().value()];
  }
  void featuresNonUniq(const osl::NumEffectState& state, index_list_t& out, int offset) const
  {
    const int progress = progress::Effect5x3(state).progress16().value();
    out.add(progress+offset, 1);
  }
  const std::string name() const
  {
    return "HandProgress";
  }
  void showSummary(std::ostream& os) const
  {
    os << name();
    for (int i=0; i<16; ++i)
      os << ' ' << values[i];
    os << "\n";
  }
};

gpsshogi::HandProgressFeatureEval::HandProgressFeatureEval()
{
  all.push_back(new HandProgress());
  addFinished();
}

gpsshogi::HandProgressFeatureEval::~HandProgressFeatureEval()
{
}

double gpsshogi::HandProgressFeatureEval::maxProgressConstraint() const
{
  return 16;
}
#endif


namespace gpsshogi
{
  using namespace osl;

  class ProgressPieceStand : public EvalComponent
  {
  public:
    ProgressPieceStand() : EvalComponent(osl::Piece::SIZE) {
      for (size_t i = 0; i < dimension(); ++i)
	values[i] = 1;
    }
    int eval(const osl::NumEffectState &state) const;
    int evalWithUpdate(const osl::NumEffectState &state,
		       osl::Move moved, int last_value) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
			 index_list_t&diffs, int offset) const;
    void showSummary(std::ostream &os) const;
    const std::string name() const { return "ProgressPieceStand"; };
    const std::string describe(size_t local_index) const;
  };

  class ProgressPieceKingRelative : public EvalComponent
  {
  public:
    enum { ONE_DIM = 2142 };
    ProgressPieceKingRelative() : EvalComponent(ONE_DIM * 2) { }
    static int index(const Player player, const Square king,
		     const Ptype ptype, const Square pos)
    {
      const int x = std::abs(pos.x() - king.x());
      const int y = (king.y() - pos.y()) *
	(player == osl::BLACK ? 1 : -1) + 8;
      return (ptype - osl::PTYPE_PIECE_MIN) * 17 * 9 + (x * 17 + y);
    }

    static int index(const Player player, const Square king,
		     const Piece piece)
    {
      return index(player, king, piece.ptype(), piece.square());
    }
    size_t maxActive() const { return 160; }
    const std::string name() const { return "ProgressPieceKingRelative"; };
    void featuresNonUniq(const NumEffectState &state,
			 index_list_t &diffs, int offset) const
    {
      const CArray<Square,2> kings = {{ 
	  state.kingSquare(BLACK),
	  state.kingSquare(WHITE),
	}};
      for (int i = 0; i < osl::Piece::SIZE; ++i)
      {
	const osl::Piece piece = state.pieceOf(i);
	if (piece.ptype() == osl::KING || !piece.isOnBoard())
	  continue;
	Player pl = piece.owner();
	const int index_attack = index(piece.owner(), kings[alt(pl)],
				       piece);
	const int index_defense = index(piece.owner(), kings[pl],
					piece) + ONE_DIM;
	diffs.add(index_attack + offset, 1);
	diffs.add(index_defense + offset, 1);
      }
    }
    int eval(const NumEffectState &state) const
    {
      index_list_t values;
      int result = 0;
      featuresNonUniq(state, values, 0);
      for (size_t i = 0; i < values.size(); ++i)
      {
	result += this->value(values[i].first) * values[i].second;
      }
      return result;
    }
#if 0
    int evalWithUpdate(const NumEffectState& state,
		       Move moved,
		       const int &last_values) const;
#endif
  };

  class AttackedPieces : public EvalComponent
  {
  public:
    AttackedPieces() : EvalComponent(21 + 40) { }

    size_t maxActive() const { return 4; }
    const std::string name() const { return "AttackedPieces"; };
    template <Player Attacked>
    void countOne(const NumEffectState &state,
		  int &pawn, int &non_pawn) const
    {
      PieceMask mask = (state.effectedMask(alt(Attacked)) &
			state.piecesOnBoard(Attacked));
      mask.reset(state.kingPiece<Attacked>().number());
      mask_t black_ppawn = (state.promotedPieces().getMask<PAWN>() &
			    mask.selectBit<PAWN>());

      pawn = mask.selectBit<PAWN>().countBit();
      mask.clearBit<PAWN>();
      non_pawn = black_ppawn.countBit() + mask.countBit();
    }
    void featuresNonUniq(const NumEffectState &state,
			 index_list_t &diffs, int offset) const
    {
      int black_pawn, black_non_pawn;
      countOne<BLACK>(state, black_pawn, black_non_pawn);
      diffs.add(black_pawn + offset, 1);
      diffs.add(black_non_pawn + 21 + offset, 1);
      int white_pawn, white_non_pawn;
      countOne<WHITE>(state, white_pawn, white_non_pawn);
      diffs.add(white_pawn + offset, 1);
      diffs.add(white_non_pawn + 21 + offset, 1);
    }
    int eval(const NumEffectState &state) const
    {
      index_list_t values;
      int result = 0;
      featuresNonUniq(state, values, 0);
      for (size_t i = 0; i < values.size(); ++i)
      {
	result += this->value(values[i].first) * values[i].second;
      }
      return result;
    }
  };


  class ProgressNonPawnAttackedPtypePair : public EvalComponent
  {
  public:
    // (ptype, with self support, attacking ptype)^2
    ProgressNonPawnAttackedPtypePair()
      : EvalComponent((PTYPE_SIZE * 2 * PTYPE_SIZE)*(PTYPE_SIZE * 2 * PTYPE_SIZE))
    { 
      for (size_t i = 0; i < dimension(); ++i)
	values[i] = 1;
    }
    void countOne(PieceMask attacked, const NumEffectState& state,
		  index_list_t &features, int offset) const
    {
      // 評価関数用との違いは、後手番の符号
      PieceVector pieces;
      while (attacked.any())
      {
	const Piece piece = state.pieceOf(attacked.takeOneBit());
	pieces.push_back(piece);
      }
      for (size_t i=0; i<pieces.size(); ++i) {
	const int i0 = index1(state, pieces[i]);
	features.add(index2(0, i0) + offset, 1);	
	for (size_t j=i+1; j<pieces.size(); ++j) {
	  const int i1 = index1(state, pieces[j]);
	  features.add(index2(i0, i1) + offset, 1);
	}
      }
    }
    void featuresNonUniq(const NumEffectState &state,
			 index_list_t &features, int offset) const
    {
      PieceMask black_attacked = state.effectedMask(WHITE) & state.piecesOnBoard(BLACK);
      black_attacked.reset(state.kingPiece<BLACK>().number());
      mask_t black_ppawn = state.promotedPieces().getMask<PAWN>() & black_attacked.selectBit<PAWN>();
      black_attacked.clearBit<PAWN>();
      black_attacked.orMask(PtypeFuns<PAWN>::indexNum, black_ppawn);
      countOne(black_attacked, state, features, offset);

      PieceMask white_attacked = state.effectedMask(BLACK) & state.piecesOnBoard(WHITE);
      white_attacked.reset(state.kingPiece<WHITE>().number());
      mask_t white_ppawn = state.promotedPieces().getMask<PAWN>() & white_attacked.selectBit<PAWN>();
      white_attacked.clearBit<PAWN>();
      white_attacked.orMask(PtypeFuns<PAWN>::indexNum, white_ppawn);
      countOne(white_attacked, state, features, offset);
    }
    const std::string name() const { return "ProgressNonPawnAttackedPtypePair"; }
    size_t maxActive() const { return 40*40*2; }

    // 重複
    int eval(const NumEffectState &state) const
    {
      index_list_t values;
      int result = 0;
      featuresNonUniq(state, values, 0);
      for (size_t i = 0; i < values.size(); ++i)
      {
	result += this->value(values[i].first) * values[i].second;
      }
      return result;
    }
    const std::string describe(size_t local_index) const
    {
      const int i0 = local_index / (PTYPE_SIZE * 2 * PTYPE_SIZE);
      const int i1 = local_index % (PTYPE_SIZE * 2 * PTYPE_SIZE);
      return describeOne(i0) + " " + describeOne(i1);
    }
  private:
    static int index1(const NumEffectState &state, Piece piece)
    {
      const Ptype attack_ptype
	= state.findCheapAttack(alt(piece.owner()), piece.square()).ptype();
      const bool has_support = state.hasEffectAt(piece.owner(),
						 piece.square());
      return (piece.ptype() + 
	      (has_support ? 0 : PTYPE_SIZE)) * PTYPE_SIZE + attack_ptype;
    }
    static int index2(int i0, int i1) 
    {
      if (i0 > i1) 
	std::swap(i0, i1);
      return i0 * PTYPE_SIZE * 2 * PTYPE_SIZE + i1;
    }
    const std::string describeOne(size_t index) const
    {
      if (index == 0)
	return "--";
      const Ptype attack = static_cast<Ptype>(index%PTYPE_SIZE);
      index /= PTYPE_SIZE;
      const Ptype ptype = static_cast<Ptype>(index%PTYPE_SIZE);
      const bool has_support = index > PTYPE_SIZE;
      return csa::show(attack) + "=>" + csa::show(ptype)
	+ (has_support ? "#" : ".");
    }
  };

  // 
  class ProgressPawnFacing : public EvalComponent
  {
  public:
    ProgressPawnFacing() : EvalComponent(10)
    { 
      for (size_t i = 0; i < dimension(); ++i)
	values[i] = 1;
    }
    void featuresNonUniq(const NumEffectState &state,
			 index_list_t &features, int offset) const
    {
      PieceMask attacked = state.effectedMask(WHITE) & state.piecesOnBoard(BLACK);
      mask_t pawn = attacked.selectBit<PAWN>()
	& ~(state.promotedPieces().getMask<PAWN>());
      int count = 0;
      while (pawn.any()) {
	const Piece p(state.pieceOf(pawn.takeOneBit()+PtypeFuns<PAWN>::indexNum*32));
	if (state.hasEffectByPtypeStrict<PAWN>(WHITE, p.square()))
	  ++count;
      }
      if (count == 0)
	features.add(offset, 1);
      for (int i=1; i<=count; ++i)
	features.add(offset+i, 1);
    }
    const std::string name() const { return "ProgressPawnFacing"; }
    size_t maxActive() const { return 9; }

    int eval(const NumEffectState &state) const
    {
      index_list_t values;
      int result = 0;
      featuresNonUniq(state, values, 0);
      for (size_t i = 0; i < values.size(); ++i)
      {
	result += this->value(values[i].first) * values[i].second;
      }
      return result;
    }
    void showSummary(std::ostream &os) const
    {
      os << "PawnFacing ";
      for (size_t i = 0; i < dimension(); ++i)
	os << this->value(i) << " ";
      os << std::endl;
    }
  };
  // 
  class ProgressPromotion37 : public EvalComponent
  {
  public:
    ProgressPromotion37() : EvalComponent(PTYPE_SIZE)
    { 
      for (size_t i = 0; i < dimension(); ++i)
	values[i] = 1;
    }
    void addFeatures(const NumEffectState &state,
		     index_list_t &features, int offset,
		     Player attack, int rank) const
    {
      CArray<int,PTYPE_SIZE> count = {{ 0 }};
      for (int x=1; x<=9; ++x) {
	const Square target(x, rank);
	if (! state[target].isEmpty())
	  continue;
	int a = state.countEffect(attack, target);
	const int d = state.countEffect(alt(attack), target);
	if (a > 0 && a == d)
	  a += AdditionalEffect::hasEffect(state, target, attack);
	if (a <= d)
	  continue;
	const Ptype ptype = state.findCheapAttack(attack, target).ptype();
	if (isPiece(ptype) && ! isPromoted(ptype))
	  count[ptype]++;
      }
      for (int p=PTYPE_BASIC_MIN; p<=PTYPE_MAX; ++p) {
	if (count[p] > 0)
	  features.add(offset+p, 1);
	if (count[p] > 1)
	  features.add(offset+p-8, (count[p]-1));
      }
    }
    void featuresNonUniq(const NumEffectState &state,
			 index_list_t &features, int offset) const
    {
      // black
      addFeatures(state, features, offset, BLACK, 3);
      // white
      addFeatures(state, features, offset, WHITE, 7);
    }
    const std::string name() const { return "ProgressPromotion37"; }
    size_t maxActive() const { return PTYPE_SIZE*2; }

    int eval(const NumEffectState &state) const
    {
      index_list_t values;
      int result = 0;
      featuresNonUniq(state, values, 0);
      for (size_t i = 0; i < values.size(); ++i)
      {
	result += this->value(values[i].first) * values[i].second;
      }
      return result;
    }
    void showSummary(std::ostream &os) const
    {
      os << "ProgressPromotion37 ";
      for (size_t i = 0; i < dimension(); ++i)
	os << this->value(i) << " ";
      os << std::endl;
    }
  };
  // 
  class ProgressPieceStand7 : public EvalComponent
  {
  public:
    enum { StandRank = 8 };
    ProgressPieceStand7() : EvalComponent(StandRank*7)
    { 
      for (size_t i = 0; i < dimension(); ++i)
	values[i] = 1;
    }
    void addFeatures(const NumEffectState &state,
		     index_list_t &features, int offset,
		     Player attack) const
    {
      CArray<int,7> stand = {{ 0 }};
      int filled = 0;
      for (Ptype ptype: PieceStand::order)
	if (state.hasPieceOnStand(attack, ptype))
	  stand[filled++] = ptype-PTYPE_BASIC_MIN;
      for (int i=0; i<std::min(7,filled+1); ++i) {
	assert(stand[i] < StandRank);
	features.add(offset+(stand[i] + StandRank*i), 1);
      }
    }
    void featuresNonUniq(const NumEffectState &state,
			 index_list_t &features, int offset) const
    {
      // black
      addFeatures(state, features, offset, BLACK);
      // white
      addFeatures(state, features, offset, WHITE);
    }
    const std::string name() const { return "ProgressPieceStand7"; }
    size_t maxActive() const { return 7*2; }

    int eval(const NumEffectState &state) const
    {
      index_list_t values;
      int result = 0;
      featuresNonUniq(state, values, 0);
      for (size_t i = 0; i < values.size(); ++i)
      {
	result += this->value(values[i].first) * values[i].second;
      }
      return result;
    }
    void showSummary(std::ostream &os) const
    {
      os << "ProgressPieceStand7\n";
      for (int i=0; i<7; ++i) {
	for (Ptype ptype: PieceStand::order)
	  os << "  " << csa::show(ptype) << ' '
	     << this->value(ptype-PTYPE_BASIC_MIN + StandRank*i);
	os << "\n";
      }
    }
    const std::string describeOne(size_t index) const
    {
      Ptype ptype = static_cast<Ptype>((index%StandRank)+PTYPE_PIECE_MIN);
      if (ptype == KING)
	ptype = PTYPE_EMPTY;
      return csa::show(ptype)
	+ "(" + static_cast<char>('0'+(index/StandRank)) + ")";
    }
  };
}

int gpsshogi::ProgressPieceStand::evalWithUpdate(
  const osl::NumEffectState &state, osl::Move moved,
  int last_value) const
{
  if (moved.isPass())
    return last_value;
  osl::Ptype captured = moved.capturePtype();
  if (moved.isDrop())
  {
    int count = state.countPiecesOnStand(moved.player(), moved.ptype()) + 1;
    return last_value -
      value(Ptype_Table.getIndexMin(moved.ptype()) + count - 1);
  }
  else if (captured != PTYPE_EMPTY)
  {
    Ptype ptype = unpromote(captured);
    int count = state.countPiecesOnStand(moved.player(), ptype);
    return last_value +
      value(Ptype_Table.getIndexMin(ptype) + count - 1) ;
  }
  else
    return last_value;
}

int gpsshogi::ProgressPieceStand::eval(
  const osl::NumEffectState &state) const
{
  int result = 0;
  for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
  {
    const Ptype ptype = osl::PieceStand::order[i];
    const int black_count =
      state.countPiecesOnStand(osl::BLACK, osl::PieceStand::order[i]);
    const int white_count =
      state.countPiecesOnStand(osl::WHITE, osl::PieceStand::order[i]);
    for (int j = 0; j < black_count; ++j)
    {
      result += value(Ptype_Table.getIndexMin(ptype) + j);
    }
    for (int j = 0; j < white_count; ++j)
    {
      result += value(Ptype_Table.getIndexMin(ptype) + j);
    }
  }
  return result;
}

const std::string gpsshogi::
ProgressPieceStand::describe(size_t local_index) const
{
  for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
  {
    const Ptype ptype = osl::PieceStand::order[i];
    if (local_index >= Ptype_Table.getIndexMin(ptype)
	&& local_index < Ptype_Table.getIndexLimit(ptype))
      return Ptype_Table.getCsaName(ptype)
	+ std::to_string(local_index - Ptype_Table.getIndexMin(ptype));
  }
  return "ProgressPieceStand error";
}

void gpsshogi::ProgressPieceStand::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
  {
    const Ptype ptype = osl::PieceStand::order[i];
    const int black_count =
      state.countPiecesOnStand(osl::BLACK, osl::PieceStand::order[i]);
    const int white_count =
      state.countPiecesOnStand(osl::WHITE, osl::PieceStand::order[i]);
    const int max = std::max(black_count, white_count);
    const int min = std::min(black_count, white_count);
    for (int j = 0; j < min; ++j)
    {
      const int index = Ptype_Table.getIndexMin(ptype) + j;
      diffs.add(offset + index, 2);
    }
    for (int j = min; j < max; ++j)
    {
      const int index = Ptype_Table.getIndexMin(ptype) + j;
      diffs.add(offset + index, 1);
    }
  }
}

void gpsshogi::ProgressPieceStand::showSummary(std::ostream &os) const
{
  os << "Stand ";
  for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
  {
    Ptype ptype = osl::PieceStand::order[i];
    os << ptype << " ";
    for (int j = Ptype_Table.getIndexMin(ptype);
	 j < Ptype_Table.getIndexLimit(ptype);
	 ++j)
    {
      os << this->value(j) << " ";
    }
    os << std::endl;
  }
}

#if (defined LEARN_TEST_PROGRESS)
gpsshogi::EffectProgressFeatureEval::EffectProgressFeatureEval()
{
  all.push_back(new Effect5x3Features);
  all.push_back(new ProgressPieceStand);
  all.push_back(new Effect5x5Features);
  all.push_back(new ProgressPieceKingRelative);
  all.push_back(new ProgressNonPawnAttackedPtypePair);
  all.push_back(new ProgressPawnFacing);
  all.push_back(new ProgressPromotion37);
  all.push_back(new ProgressPieceStand7);
  addFinished();
}

double gpsshogi::EffectProgressFeatureEval::maxProgressConstraint() const
{
  return 1023;
}
#endif
gpsshogi::StableEffectProgressFeatureEval::StableEffectProgressFeatureEval()
{
  all.push_back(new Effect5x3Features);
  all.push_back(new ProgressPieceStand);
  all.push_back(new Effect5x5Features);
  all.push_back(new ProgressPieceKingRelative);
  all.push_back(new ProgressNonPawnAttackedPtypePair);
  all.push_back(new ProgressPawnFacing);
  all.push_back(new ProgressPromotion37);
  all.push_back(new ProgressPieceStand7);
  addFinished();
}

double gpsshogi::StableEffectProgressFeatureEval::maxProgressConstraint() const
{
  return 1023;
}

void gpsshogi::
StableEffectProgressFeatureEval::setWeight(const double *w)
{
  scale = 1.0;
  int dim = 0;
  for (size_t i=0; i<all.size(); ++i) {
    all[i].setWeightScale(&w[0]+dim, 1.0);
    dim += all[i].dimension();
  }
  max_progress = w[dimension()-1];
#ifdef EVAL_QUAD
  while (((max_progress/osl::progress::ml::NewProgress::ProgressScale) % 3) && max_progress > 0)
    --max_progress;
#endif
}

/* ------------------------------------------------------------------------- */

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
