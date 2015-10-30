#include "eval/progressEval.h"
#include "osl/eval/progressEval.h"
#include "osl/oslConfig.h"

#include <boost/filesystem.hpp>
#include <stack>
#include <iomanip>
#include <fstream>
#include <iostream>

typedef osl::eval::ProgressEval::progress_t progress_t;

gpsshogi::
ProgressEvalBase::ProgressEvalBase()
  : values(new int[dimension()])
{
  static bool initialized = false;
  if (! initialized) {
    initialized = true;
    osl::eval::ProgressEval::setUp();
  }
  std::fill(&values[0], &values[0]+dimension(), 0);
  std::string filename = OslConfig::home();
  filename += "/data/progresseval.txt";
  if (boost::filesystem::exists(filename.c_str())) {
    std::cerr << "loading " << filename << "\n";
    FILE *fp = fopen(filename.c_str(), "r");
    for (size_t i=0; i<dimension(); ++i) {
      if (fscanf(fp, "%d", &values[i]) != 1) {
	std::cerr << filename << " read failed " << i << "\n";
	break;
      }
    }
    fclose(fp);
  }  
}

gpsshogi::
ProgressEvalBase::~ProgressEvalBase()
{
}

int gpsshogi::ProgressEvalBase::maxProgress() const
{
  return 16;
}


int gpsshogi::ProgressEvalBase::progress(const NumEffectState &state) const
{
  progress_t progress(state);
  return this->progress(progress.progress16().value());
}

int gpsshogi::ProgressEvalBase::progress(int progress_value) const
{
  return progress_value;
}

size_t gpsshogi::
ProgressEvalBase::maxActive() const
{
  return 40*40;
}

class gpsshogi::ProgressEvalBase::Stack : public EvalValueStack
{
  struct Entry
  {
    osl::eval::ProgressEval osl_eval;
    Entry() : osl_eval(NumEffectState()) {}
  };
  std::stack<Entry> values;
public:  
  Stack(ProgressEvalBase *ev, const NumEffectState& state) { 
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
    e.osl_eval = osl::eval::ProgressEval(state);
    values.push(e);
  }
};

gpsshogi::EvalValueStack * gpsshogi::
ProgressEvalBase::newStack(const NumEffectState& state)
{
  return new Stack(this, state);
}

int gpsshogi::
ProgressEvalBase::pawnValue() const
{
  const int scale = maxProgress();
  return 128 * scale;
}

int gpsshogi::
ProgressEvalBase::openingValue(const NumEffectState& state) const
{
  osl::eval::ProgressEval osl_eval(state);
  return osl_eval.openingValue();
}
int gpsshogi::
ProgressEvalBase::endgameValue(const NumEffectState& state) const
{
  osl::eval::ProgressEval osl_eval(state);
  return osl_eval.endgameValue();
}

int gpsshogi::
ProgressEvalBase::eval(const NumEffectState& state) const
{
  osl::eval::ProgressEval osl_eval(state);
  return osl_eval.value();
}

size_t gpsshogi::
ProgressEvalBase::dimension() const
{
  return osl::eval::ProgressEval::AdjustableDimension + 1;
}
int gpsshogi::
ProgressEvalBase::flatValue(size_t index) const 
{
  if (index >= dimension())
    std::cerr << "flatValue " << index << ' ' << dimension() << "\n";
  assert(index < dimension());
  return values[index];
}

bool gpsshogi::
ProgressEvalBase::load(const char *filename)
{
  std::ifstream is(filename);
  boost::scoped_array<double> values(new double[dimension()]);
  for (size_t i=0; i<dimension(); ++i)
    is >> values[i];
  setWeight(&values[0]);
  return static_cast<bool>(is);
}

void gpsshogi::
ProgressEvalBase::save(const char *filename) const
{
  std::ofstream os(filename);
  for (size_t i=0; i<dimension(); ++i)
    os << values[i] << "\n";
  std::string f = filename;  
  if (f.find(".txt") != f.npos) {
    f = f.substr(0, f.find(".txt")) + "-info.txt";
    std::ofstream os(f.c_str());
    os << "#* opening\n";
    os << "PieceEval\n";
    os << "#* endgame\n";
    os << "AttackKing\n";
    os << "DefenseKing\n";    
  }
}

void gpsshogi::
ProgressEvalBase::setWeight(const double *w)
{
  double scale = 1.0;
  // assuming we have PieceEval
  scale = 128.0/w[PAWN];
  setWeightScale(w, scale);
}

void gpsshogi::
ProgressEvalBase::setWeightScale(const double *w, double scale)
{
  assert(abs(scale-1.0)<0.001);
  std::copy(w, w+dimension(), &values[0]);
  osl::eval::ProgressEval::resetWeights(&values[0]);
  values[dimension()-1] = 1;
}

void gpsshogi::
ProgressEvalBase::saveWeight(double *w) const
{
  std::copy(&values[0], &values[0]+dimension(), w);
}

void gpsshogi::
ProgressEvalBase::features(const NumEffectState& state, MoveData& data,
			   int offset) const
{
  std::vector<std::pair<int,double> >& out = data.diffs;
  
  using namespace osl::eval::endgame;
  osl::eval::ProgressEval osl_eval(state);
  data.value = osl_eval.value();
  double opening_weight = (16-osl_eval.progress16().value());
  double endgame_weight = osl_eval.progress16().value();
  // pieces
  int opening = 0, endgame = 0;
  CArray<int, PTYPE_SIZE> pieces = {{0}};
  for (int i=0; i<Piece::SIZE; ++i) {
    const Piece p = state.pieceOf(i);
    pieces[p.ptype()] += sign(p.owner());
  }
  for (size_t i=0; i<pieces.size(); ++i)
    if (pieces[i]) {
      out.push_back(std::make_pair(offset+i, pieces[i]*opening_weight));
      opening += values[i] * pieces[i];
    }
  // attack defense
  const CArray<Piece,2> kings = {{ state.kingPiece(BLACK), 
				   state.kingPiece(WHITE) }};
  for (int i=0; i<Piece::SIZE; ++i) {
    const Piece p = state.pieceOf(i);
    const Player player = p.owner();
    const Square sq = p.square();
    const Piece me = kings[player];
    const Piece op = kings[alt(player)];
    const int a = AttackKing::valueOf(op, p.ptypeO(), sq);
    const int d = DefenseKing::valueOf(me, p.ptypeO(), sq);
    int index_a = PTYPE_SIZE + KingPieceTable::effectiveIndexOf
      (op.square(), op.owner(), p.square(), p.ptype());
    int index_d = PTYPE_SIZE + KingPieceTable::effectiveIndexOf
      (me.square(), me.owner(), p.square(), p.ptype())
      + KingPieceTable::EffectiveDimension;
    assert(index_a < dimension());
    assert(index_d < dimension());
    assert(values[index_a] == a);
    assert(values[index_d] == d);
    if (eval::betterThan(player, a, d)) {
      out.push_back(std::make_pair(offset+index_a, endgame_weight));
      endgame += values[index_a];
    }
    else if (eval::betterThan(player, d, a)) {
      out.push_back(std::make_pair(offset+index_d, endgame_weight));
      endgame += values[index_d];
    }
    else {
      out.push_back(std::make_pair(offset+index_a, endgame_weight/2.0));
      out.push_back(std::make_pair(offset+index_d, endgame_weight/2.0));
      endgame += values[index_a];
    }
  }
  int estimate = opening*opening_weight + endgame*endgame_weight;
  out.push_back(std::make_pair(offset+dimension()-1, data.value-estimate));
#if 0
  std::cerr << state << "osl " << osl_eval.value()
	    << " opening " << osl_eval.openingValue() << ' ' << opening 
	    << " endgame " << osl_eval.endgameValue() << ' ' << endgame
	    << " diff " << osl_eval.value() << ' ' << estimate << ' ' << value-estimate
	    << " progress " << endgame_weight << "\n";
  assert(abs(osl_eval.openingValue()-opening) <= 512+abs(opening)/4+endgame_weight*128+state.inCheck()*512);
#endif
  assert(osl_eval.endgameValue() == endgame);
}

void gpsshogi::
ProgressEvalBase::showSummary(std::ostream& os) const
{
  for (int i=0; i<PTYPE_SIZE; ++i) {
    const Ptype ptype = static_cast<Ptype>(i);
    if (! isPiece(ptype) || ptype == KING)
      continue;
    const char *name = Ptype_Table.getCsaName(ptype);
    os << name << " " << values[i] << "  ";
  }
  os << std::endl;
}

void gpsshogi::
ProgressEvalBase::showAll(std::ostream& os) const
{
}

void gpsshogi::
ProgressEvalBase::setRandom()
{
}

bool gpsshogi::
ProgressEvalBase::hasPieceValue() const
{
  return false;
}

gpsshogi::KProgressEval::
KProgressEval() 
{
}

gpsshogi::KProgressEval::
~KProgressEval()
{
}

gpsshogi::StableProgressEval::
StableProgressEval() 
{
}

gpsshogi::StableProgressEval::
~StableProgressEval()
{
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

