/* analyzer.cc
 */
#include "analyzer.h"
#include "eval/eval.h"
#include "quiesce.h"
#include "osl/progress/effect5x3.h"
#include "osl/eval/pieceEval.h"
#include <iostream>
#include <cstdio>

// #define DEBUG_ALL

constexpr double gpsshogi::SigmoidUtil::eps;

/* ------------------------------------------------------------------------- */

std::ostream& gpsshogi::operator<<(std::ostream& os, const gpsshogi::MoveData& md)
{
  os << "value " << md.value << " data";
  for (size_t i=0; i<md.diffs.size(); ++i) {
    os << "  " << md.diffs[i].first << " " << md.diffs[i].second;
  }
  return os << "\n";
}

/* ------------------------------------------------------------------------- */

void gpsshogi::
Analyzer::makeLeaf(NumEffectState& state, const PVVector& pv)
{
  for (Move m: pv)
    state.makeMove(m);
}

void gpsshogi::
Analyzer::analyzeLeaf(const NumEffectState& state_org, 
		      const PVVector& pv, Eval& eval,
		      MoveData& data, bool bonanza_compatible)
{
  NumEffectState state(state_org);
  makeLeaf(state, pv);

  if (! bonanza_compatible) {
    assert(! state.inCheck(BLACK));
    assert(! state.inCheck(WHITE));
  }
#ifdef DEBUG_ALL
  std::cerr << state;
  std::cerr << value << "\n";
#endif  
  eval.features(state, data);
#ifdef DEBUG_ALL
  std::cerr << "\n";
#endif
}

int gpsshogi::
Analyzer::leafValue(const NumEffectState& state_org, 
		    const PVVector& pv, Eval& eval)
{
  NumEffectState state(state_org);
  makeLeaf(state, pv);

  return eval.eval(state);
}

void gpsshogi::
Analyzer::makeInstanceSorted
(double turn_coef, 
 const sparse_vector_t& selected, const sparse_vector_t& sibling,
 const std::vector<size_t>& frequency, int min_frequency, InstanceData& instance)
{
  assert(std::is_sorted(selected.begin(), selected.end()));
  assert(std::is_sorted(sibling.begin(), sibling.end()));

  instance.index.reserve(selected.size()+sibling.size()+1);
  instance.value.reserve(selected.size()+sibling.size()+1);
  // NOTE: black = 0, white = 1 for backward compatibility
  instance.y = 1-std::max(0.0, turn_coef); 

  // NOTE: sibling - selected
  sparse_vector_t::const_iterator p = sibling.begin();
  sparse_vector_t::const_iterator q = selected.begin();

  while (p != sibling.end() && q != selected.end()) {
    if (min_frequency) {
      if (frequency[p->first] < min_frequency) {
	++p;
	continue;
      }
      if (frequency[q->first] < min_frequency) {
	++q;
	continue;
      }
    }
    const int index = std::min(p->first, q->first);
    double diff;		// sibling[i] - selected[i];
    if (p->first == q->first) {
      diff = p->second - q->second;
      ++p, ++q;
    }
    else if (p->first < q->first) {
      diff = p->second;		// seleceted[index] == 0
      ++p;
    }
    else {
      assert(p->first > q->first);
      diff = - (q->second);	// sibling[index] == 0
      ++q;
    }
    if (fabs(diff) > SigmoidUtil::eps) {
      instance.index.push_back(index);
      instance.value.push_back(diff);
    }
  }
  while (p != sibling.end()) {
    const int index = p->first;
    const double diff = p->second; // seleceted[index] == 0
    ++p;
    if (fabs(diff) > SigmoidUtil::eps) {
      instance.index.push_back(index);
      instance.value.push_back(diff);
    }
  }
  while (q != selected.end()) {
    const int index = q->first;
    const double diff = - (q->second); // sibling[index] == 0
    ++q;
    if (fabs(diff) > SigmoidUtil::eps) {
      instance.index.push_back(index);
      instance.value.push_back(diff);
    }
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
