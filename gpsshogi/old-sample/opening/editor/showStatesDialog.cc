#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <QInputDialog>

#include "showStatesDialog.h"
#include "osl/apply_move/applyMove.h"

void ShowStatesDialog::addMoves(
  osl::Player player,
  int count,
  osl::apply_move::DoUndoMoveStack<osl::state::SimpleState> &stack,
  osl::state::SimpleState &state,
  osl::stl::vector<osl::record::opening::WMove> &moves)
{
  State openingState;
  try {
    openingState = editor->getState(state);
  } catch (KeyNotFoundException) {
    count = 0;
  }

  if (count == 0)
  {
    StateMove sm(state, moves);
    states.push_back(sm);
    return;
  }

  osl::vector<osl::record::opening::WMove> wmoves = openingState.getMoves();
  if (wmoves.size() == 0)
  {
    StateMove sm(state, moves);
    states.push_back(sm);
    return;
  }

  if (player != state.turn())
  {
    for (size_t i = 0; i < wmoves.size(); i++)
    {
      stack.push(state, wmoves[i].getMove());
      moves.push_back(wmoves[i]);
      addMoves(player, count - 1, stack, state, moves);
      stack.pop();
      moves.pop_back();
    }
  }
  else
  {
    int bestIndex = 0;
    int bestWeight = 0;
    for (size_t i = 0; i < wmoves.size(); i++)
    {
      if (wmoves[i].getWeight() > bestWeight)
      {
	bestIndex = i;
	bestWeight = wmoves[i].getWeight();
      }
    }
    stack.push(state, wmoves[bestIndex].getMove());
    moves.push_back(wmoves[bestIndex]);
    addMoves(player, count - 1, stack, state, moves);
    stack.pop();
    moves.pop_back();
  }
}

ShowStatesDialog::ShowStatesDialog(Editor *editor,
				   osl::state::SimpleState state,
				   QWidget *parent)
  : QDialog(parent), editor(editor), state(state), index(0)
{
  int i = QInputDialog::getInteger(this, "Number of moves ahead",
				   "Show state after this many number of moves",
				   0, 1, 200);

  osl::apply_move::DoUndoMoveStack<osl::state::SimpleState> stack;
  osl::stl::vector<osl::record::opening::WMove> moves;
  addMoves(state.turn(), i, stack, state, moves);

  if (states.size() == 0)
    return;

  board = new gpsshogi::gui::Board(states[0].state, this);
  list = new MoveListView(states[0].moves, this);
  list->setSorting(-1);
  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  QVBoxLayout *boardLayout = new QVBoxLayout;
  boardLayout->addWidget(board);
  countLabel = new QLabel(QString("%1/%2").arg(index).arg(states.size()), this);
  boardLayout->addWidget(countLabel, Qt::AlignVCenter);
  QPushButton *next = new QPushButton(this);
  next->setText("&Next");
  boardLayout->addWidget(next);
  QPushButton *previous = new QPushButton(this);
  previous->setText("&Previous");
  boardLayout->addWidget(previous);

  mainLayout->addLayout(boardLayout);
  mainLayout->addWidget(list);

  connect(next, SIGNAL(clicked()), this, SLOT(next()));
  connect(previous, SIGNAL(clicked()), this, SLOT(previous()));

  resize(QSize(800, 600));
}

void ShowStatesDialog::next()
{
  index = (index + 1) % states.size();
  updateStatus();
}

void ShowStatesDialog::previous()
{
  index = (index - 1) % states.size();
  updateStatus();
}

void ShowStatesDialog::updateStatus()
{
  if (states.size() > 0)
  {
    board->setState(states[index].state);
    list->update(states[index].moves);
    countLabel->setText(QString("%1/%2").arg(index).arg(states.size()));
  }
}

