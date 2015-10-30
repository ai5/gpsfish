#include "quiescenceViewer.h"
#include "quiescenceTree.h"

#include "gpsshogi/gui/board.h"

#include "osl/eval/openMidEndingEval.h"
#include "osl/search/quiescenceSearch2.h"

#include <qcombobox.h>
#include <qlayout.h>
#include <iostream>

QuiescenceViewer::QuiescenceViewer(QWidget *parent)
  : BoardAndListTabChild(parent)
{
  moveTree = new QuiescenceTree(this);

  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(board);
  mainLayout->addWidget(moveTree);

  init();
}

bool QuiescenceViewer::analyze(const osl::SimpleState& s,
			       const std::vector<osl::Move>& moves)
{
  return analyze(s, moves, osl::search::QSearchTraits::MaxDepth);
}


bool QuiescenceViewer::analyzeHalfDepth(const osl::SimpleState& s,
					const std::vector<osl::Move>& moves)
{
  return analyze(s, moves, osl::search::QSearchTraits::MaxDepth / 2);
}

bool QuiescenceViewer::analyze(const osl::SimpleState& s,
			       const std::vector<osl::Move>& moves,
			       int depth)
{
  initialState.copyFrom(osl::NumEffectState(s));
  for (size_t i = 0; i < moves.size(); i++)
  {
    initialState.makeMove(moves[i]);
  }
  board->setState(initialState);
  const osl::Move last_move = (moves.size() > 0) 
      ? moves[moves.size() - 1] 
      : osl::Move::PASS(osl::alt(initialState.turn()));
 
  analyze<osl::eval::ml::OpenMidEndingEval>(initialState, last_move, depth);

  return true;
}

template <class Eval>
void QuiescenceViewer::analyze(const osl::SimpleState& s,
			       const osl::Move last_move,
			       int depth)
{
  typedef osl::search::QuiescenceSearch2<Eval> qsearch_t;
  osl::NumEffectState state(s);
  table.reset(new osl::search::SimpleHashTable(100000, -16, false));
  osl::search::SearchState2Core::checkmate_t checkmate_searcher;
  osl::search::SearchState2Core core(state, checkmate_searcher);
  qsearch_t qs(core, *table);

  Eval ev(state);
  assert(! last_move.isInvalid());
  qs.search(state.turn(), ev, last_move, depth);
  std::cerr << "#quiescence " << qs.nodeCount() << "\n";
  const osl::hash::HashKey key(state);
  ((QuiescenceTree *)moveTree)->showRecord(key, state, &*table);
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
