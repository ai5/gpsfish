#include <qlayout.h>
#include <stdexcept>
#include <iostream>

#include "openingEditor.h"
#include "movelist.h"
#include "moveHist.h"
#include "gpsshogi/gui/board.h"

#include "osl/record/compactBoard.h"
#include "osl/apply_move/applyMove.h"
#include "osl/record/csaRecord.h"
#include "gpsshogi/gui/util.h"
#include "showStatesDialog.h"

class BestMovesItem : public Q3ListBoxText
{
public:
  osl::Move move;

  BestMovesItem(osl::Move move)
    : Q3ListBoxText(gpsshogi::gui::Util::moveToString(move)),
      move(move)
  {
  }
};


OpeningEditor::OpeningEditor(const std::string& openingFile,
			     const char *csaFile,
			     QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  editor = new Editor(openingFile.c_str());
  osl::state::SimpleState initialState(osl::HIRATE);
  osl::vector<osl::record::opening::WMove> wmoves;

  hist = new MoveHist(this);
  list = new MoveListView(wmoves, this);
  bestMoves = new Q3ListBox(this);

  if (csaFile != NULL)
  {
    std::string file(csaFile);
    osl::record::csa::CsaFile csa_file(file);

    initialState = csa_file.getInitialState();

    const osl::record::Record &record = csa_file.getRecord();
    osl::stl::vector<osl::Move> moves = record.getMoves();

    for (size_t i = 0; i < moves.size(); i++)
    {
      hist->add(moves[i], 0);
    }
    for (size_t i = 0; i < moves.size(); i++)
    {
      hist->back();
    }
  }

  try
  {
    State state = editor->getState(initialState);
    wmoves = state.getMoves();
  } catch (KeyNotFoundException) {
    std::cerr << "Initial state not found" << std::endl;
  }
  sstate = initialState;
  board = new gpsshogi::gui::Board(sstate, this);
  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  QVBoxLayout *leftLayout = new QVBoxLayout;
  leftLayout->addWidget(hist);
  leftLayout->addWidget(bestMoves);
  mainLayout->addLayout(leftLayout);
  mainLayout->addWidget(board);
  mainLayout->addWidget(list);

  connect(bestMoves, SIGNAL(doubleClicked(Q3ListBoxItem *)),
	  this, SLOT(traceBestMoves(Q3ListBoxItem *)));
  connect(list, SIGNAL(selected(osl::Move, int)),
	  board, SLOT(move(osl::Move)));
  connect(list, SIGNAL(updated(MoveItem *)),
	  this, SLOT(updateWeight(MoveItem *)));
  connect(hist, SIGNAL(selected(osl::state::SimpleState)),
	  this, SLOT(setState(osl::state::SimpleState)));
  connect(hist, SIGNAL(selected(osl::state::SimpleState)),
	  board, SLOT(setState(osl::state::SimpleState)));
  connect(board, SIGNAL(moved(osl::Move)),
	  this, SLOT(move(osl::Move)));
}

OpeningEditor::~OpeningEditor()
{
  delete editor;
}

int OpeningEditor::moveCount() const
{
  return hist->count();
}

osl::Player OpeningEditor::turn() const
{
  return sstate.turn();
}

int OpeningEditor::blackWinCount()
{
  try
  {
    State state = editor->getState(sstate);
    return state.getBlackWinCount();
  } catch (KeyNotFoundException e)
  {
    return 0;
  }
}

int OpeningEditor::whiteWinCount()
{
  try
  {
    State state = editor->getState(sstate);
    return state.getWhiteWinCount();
  } catch (KeyNotFoundException e)
  {
    return 0;
  }
}

void OpeningEditor::move(osl::Move m)
{
  int weight = 0;
  try
  {
    State state = editor->getState(sstate);
    osl::vector<osl::record::opening::WMove> moves = state.getMoves();
    for (unsigned int i = 0; i < moves.size(); i++)
    {
      if (moves[i].getMove() == m)
      {
	weight = moves[i].getWeight();
	break;
      }
    }
  } catch (KeyNotFoundException e)
  {
  }
  hist->add(m, weight);
  osl::ApplyMoveOfTurn::doMove(sstate, m);
  updateState();
}

void OpeningEditor::traceBestMoves(Q3ListBoxItem *item)
{
  if (NULL == item) return;

  bool itemdone = false;
  for (BestMovesItem *cur = (BestMovesItem *)bestMoves->firstItem();
       !itemdone; cur = (BestMovesItem *)cur->next())
  {
    const osl::Move m = cur->move;
    int weight = 0;
    try
    {
      State state = editor->getState(sstate);
      osl::vector<osl::record::opening::WMove> moves = state.getMoves();
      for (unsigned int i = 0; i < moves.size(); i++)
      {
	if (moves[i].getMove() == m)
	{
	  weight = moves[i].getWeight();
	  break;
	}
      }
    } catch (KeyNotFoundException e)
    {
    }
    hist->add(m, weight);
    osl::ApplyMoveOfTurn::doMove(sstate, m);
    itemdone = cur == item;
  }
  board->setState(sstate);
  updateState();
}

void OpeningEditor::setState(osl::state::SimpleState s)
{
  if (! (sstate == s))
  {
    sstate = s;
    updateState();
  }
}

// update() is taken by QWidget...
void OpeningEditor::updateState()
{
  State state;
  osl::vector<osl::record::opening::WMove> moves;
  try
  {
    state = editor->getState(sstate);
    moves = state.getMoves();
  } catch (KeyNotFoundException e)
  {
  }
  list->update(moves);
  bestMoves->clear();

  {
    osl::state::SimpleState curState(sstate);
    while (moves.size() > 0)
    {
      int bestIndex = 0;
      int bestWeight = 0;
      for (size_t i = 0; i < moves.size(); i++)
      {
	if (moves[i].getWeight() > bestWeight)
	{
	  bestIndex = i;
	  bestWeight = moves[i].getWeight();
	}
      }
      osl::ApplyMoveOfTurn::doMove(curState, moves[bestIndex].getMove());
      bestMoves->insertItem(new BestMovesItem(moves[bestIndex].getMove()));

      try
      {
	state = editor->getState(curState);
	moves = state.getMoves();
      } catch (KeyNotFoundException e)
      {
	moves.clear();
      }
    }
  }
  emit statusChanged();
}

void OpeningEditor::initialState()
{
  hist->initialState();
}

void OpeningEditor::back()
{
  hist->back();
}

void OpeningEditor::forward()
{
  hist->forward();
}

void OpeningEditor::addMove()
{
  if (hist->count() == 0)
    return;

  osl::Move move = ((MoveHistItem *)hist->item(hist->count() - 1))->getMove();
  hist->back();

  osl::vector<osl::record::opening::WMove> moves;
  int black = 0;
  int white = 0;
  try
  {
    State state = editor->getState(sstate);
    moves = state.getMoves();
    for (unsigned int i = 0; i < moves.size(); i++)
    {
      // already exists
      if (moves[i].getMove() == move)
	return;
    }
    black = state.getBlackWinCount();
    white = state.getWhiteWinCount();
  } catch (KeyNotFoundException e)
  {
  }

  osl::record::opening::WMove wm(move, 0, 0);
  moves.push_back(wm);

  State newState(sstate, black, white, moves);
  editor->setState(newState);
  updateState();
}

void OpeningEditor::deleteMove()
{
  State state = editor->getState(sstate);
  Q3ListViewItemIterator it(list);
  osl::vector<osl::record::opening::WMove> moves;
  while (it.current())
  {
    MoveItem *mi = (MoveItem *) it.current();

    if (!it.current()->isSelected())
    {
      osl::record::opening::WMove wmove(mi->getMove(), 0, mi->getWeight());
      moves.push_back(wmove);
    }
    ++it;
  }
  State newState(sstate, state.getBlackWinCount(),
		 state.getWhiteWinCount(), moves);
  editor->setState(newState);
  updateState();

}

void OpeningEditor::clearWeight()
{
  State state = editor->getState(sstate);
  Q3ListViewItemIterator it(list);
  osl::vector<osl::record::opening::WMove> moves;
  while (it.current())
  {
    MoveItem *mi = (MoveItem *) it.current();

    if (it.current()->isSelected())
    {
      osl::record::opening::WMove wmove(mi->getMove(), 0, 0);
      moves.push_back(wmove);
    }
    else
    {
      osl::record::opening::WMove wmove(mi->getMove(), 0, mi->getWeight());
      moves.push_back(wmove);
    }
    ++it;
  }
  State newState(sstate, state.getBlackWinCount(),
		 state.getWhiteWinCount(), moves);
  editor->setState(newState);
  updateState();

}

void OpeningEditor::updateWeight(MoveItem *item)
{
  osl::Move move = item->getMove();

  State state = editor->getState(sstate);
  osl::vector<osl::record::opening::WMove> moves = state.getMoves();
  osl::vector<osl::record::opening::WMove> newMoves;
  newMoves.reserve(moves.size());
  for (unsigned int i = 0; i < moves.size(); i++)
  {
    if (moves[i].getMove() == move)
    {
      osl::record::opening::WMove wm(move, moves[i].getStateIndex(),
				     item->getWeight());
      newMoves.push_back(wm);
    }
    else
      newMoves.push_back(moves[i]);
  }
  State newState(sstate, state.getBlackWinCount(),
		 state.getWhiteWinCount(), newMoves);
  editor->setState(newState);
  updateState();
}

void OpeningEditor::normalizeWeight()
{
  State state = editor->getState(sstate);
  Q3ListViewItemIterator its(list, Q3ListViewItemIterator::Selected);
  int total_weight = 0;
  while (its.current())
  {
    MoveItem *mi = (MoveItem *) its.current();
    total_weight += mi->getWeight();

    ++its;
  }
  if (total_weight == 0)
    return;

  Q3ListViewItemIterator it(list);
  osl::vector<osl::record::opening::WMove> moves;

  while (it.current())
  {
    MoveItem *mi = (MoveItem *) it.current();

    if (it.current()->isSelected())
    {    
      int newWeight = 10000 * mi->getWeight() / total_weight;
      osl::record::opening::WMove wmove(mi->getMove(), 0, newWeight);
      moves.push_back(wmove);
    }
    else
    {
      osl::record::opening::WMove wmove(mi->getMove(), 0, mi->getWeight());
      moves.push_back(wmove);
    }
    ++it;
  }
  

  State newState(sstate, state.getBlackWinCount(),
		 state.getWhiteWinCount(), moves);
  editor->setState(newState);
  updateState();

}

void OpeningEditor::showStates()
{
  ShowStatesDialog *dialog = new ShowStatesDialog(editor, sstate, this);
  dialog->show();
  dialog->raise();
  dialog->activateWindow();
}
