/* learn-piece.cc
 */
#include "osl/numEffectState.h"
#include "osl/record/csaRecord.h"
#include "osl/record/ki2.h"
#include "osl/record/kakinoki.h"
#include "osl/record/kisen.h"
#include "osl/eval/see.h"
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
using namespace osl;
using namespace std;

CArray<int,PTYPE_SIZE> weight, gradient;
void show() {
  for (size_t i=0; i<PieceStand::order.size(); ++i) {
    Ptype ptype = PieceStand::order[i];
    cout << csa::show(ptype) << ' ' << weight[ptype] << ' ';
    if (canPromote(ptype))
      cout << csa::show(promote(ptype)) << ' ' << weight[promote(ptype)] << ' ';
  }
  cout << endl;
#if 0
  for (size_t i=0; i<PieceStand::order.size(); ++i) {
    Ptype ptype = PieceStand::order[i];
    cout << csa::show(ptype) << ' ' << gradient[ptype] << ' ';
    if (canPromote(ptype))
      cout << csa::show(promote(ptype)) << ' ' << gradient[promote(ptype)] << ' ';
  }
  cout << endl;
#endif
}
int median() {
  std::vector<int> copy;
  for (int i=0; i<PTYPE_SIZE; ++i)
    if (gradient[i]!=0) copy.push_back(gradient[i]);
  sort(copy.begin(), copy.end());
  if (copy.size() == 1) return 0;
  if (copy.size()%2) return copy[copy.size()/2];
  return copy[copy.size()/2]-1;
}
void update() {
  std::vector<std::pair<int,Ptype> > gradient_ptype;
  for (size_t i=0; i<PieceStand::order.size(); ++i) {
    Ptype ptype = PieceStand::order[i];
    gradient_ptype.push_back(std::make_pair(gradient[ptype], ptype));
    if (canPromote(ptype)) {
      ptype = promote(ptype);
      gradient_ptype.push_back(std::make_pair(gradient[ptype], ptype));
    }
  }
  std::sort(gradient_ptype.begin(), gradient_ptype.end());
  // bonanza's robust update seems better than standard gradient descent methods, here
  // const int a[13] = { -1, -1, -1, -1, -1, -1, 0, 1, 1, 1, 1, 1, 1 }; 
  const int a[13] = { -3, -2, -2, -1, -1, -1, 0, 1, 1, 1, 2, 2, 3 };
  for (size_t i=0; i<gradient_ptype.size(); ++i)
    weight[gradient_ptype[i].second] += a[i];
}
void count(const NumEffectState& state, CArray<int,PTYPE_SIZE>& out) {
  out.fill(0);
  for (int i=0; i<Piece::SIZE; ++i) {
    Piece p = state.pieceOf(i);
    out[p.ptype()] += sign(p.owner());
  }
}
void compare(Player turn, const NumEffectState& selected, 
	     const NumEffectState& not_selected) {
  CArray<int,PTYPE_SIZE> c0, c1, diff;
  count(selected, c0);
  count(not_selected, c1);
  int evaldiff = 0;
  for (int i=0; i<PTYPE_SIZE; ++i) {
    diff[i] = (c0[i] - c1[i])*sign(turn);
    evaldiff += diff[i] * weight[i];
  }
  if (evaldiff > 0) return;
  for (int i=0; i<PTYPE_SIZE; ++i) 
    gradient[i] += diff[i];
}
Move greedymove(const NumEffectState& state) {
  MoveVector all;
  state.generateLegal(all);
  int best_see = 0;
  Move best_move;
  for (size_t i=0; i<all.size(); ++i) {
    if (! all[i].isCaptureOrPromotion()) continue;
    int see = See::see(state, all[i]);
    if (see <= best_see) continue;
    best_see = see;
    best_move = all[i];
  }
  return best_move;
}
void make_PV(const NumEffectState& src, Move prev, MoveVector& pv) {
  NumEffectState state(src);
  pv.clear(); 
  // todo: quiescence search
  while (true) {
    state.makeMove(prev);
    pv.push_back(prev);
    Move move = greedymove(state);
    if (! move.isNormal())
      return;
    prev = move;
  }
}
void make_moves(NumEffectState& state, const MoveVector& pv) {
  for (size_t i=0; i<pv.size(); ++i)
    state.makeMove(pv[i]);
}

void run(const std::vector<Move>& moves) {
  NumEffectState state;
  for (size_t i=0; i<moves.size(); ++i) {
    const Move selected = moves[i];
    MoveVector all;
    state.generateLegal(all);

    if (! state.hasEffectAt(alt(selected.player()), selected.to())) {
      MoveVector pv0;
      make_PV(state, selected, pv0);
      NumEffectState s0(state);
      make_moves(s0, pv0);
      for (size_t j=0; j<all.size(); ++j)
	if (all[j] != selected) {
	  MoveVector pv1;
	  make_PV(state, all[j], pv1);
	  NumEffectState s1(state);
	  make_moves(s1, pv1);
	  compare(state.turn(), s0, s1);
	}
    }
    state.makeMove(selected);
  }
}
int main(int argc, char **argv) {
  weight.fill(500);
  for (int t=0; t<1024; ++t) {
    show();
    gradient.fill(0);
    for (int i=1; i<argc; ++i) {
      const char *filename = argv[i];
      if (boost::algorithm::iends_with(filename, ".csa")) {
	const CsaFile csa(filename);
	run(csa.moves());
      }
      else if (boost::algorithm::iends_with(filename, ".ki2")) {
	const Ki2File ki2(filename);
	run(ki2.moves());
      }
      else if (boost::algorithm::iends_with(filename, ".kif")
	       && KakinokiFile::isKakinokiFile(filename)) {
	const KakinokiFile kif(filename);
	run(kif.moves());
      }
      else if (boost::algorithm::iends_with(filename, ".kif")) {
	KisenFile kisen(filename);
	for (size_t j=0; j<kisen.size(); ++j)
	  run(kisen.moves(j));
      }
      else {
	cerr << "Unknown file type: " << filename << "\n";
	continue;
      }
    }
    update();
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
