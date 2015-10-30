#include "osl/apply_move/applyMove.h"
#include "moveHist.h"
#include "gpsshogi/gui/util.h"

MoveHistItem::MoveHistItem(osl::Move m, int weight)
  : Q3ListBoxText(QString("%1 %2").
		 arg(gpsshogi::gui::Util::moveToString(m)).arg(weight)),
    move(m), weight(weight)
{
}

MoveHist::MoveHist(QWidget *parent, const char *name)
  : Q3ListBox(parent, name)
{
  connect(this, SIGNAL(selected(int)), SLOT(update(int)));
}

void MoveHist::add(osl::Move m, int weight)
{
  if (!redo.empty())
  {
    if (m == redo.back().getMove())
      redo.pop_back();
    else
      redo.clear();
  }
  insertItem(new MoveHistItem(m, weight));
}

void MoveHist::initialState()
{
  update(-1);
}

void MoveHist::update(int index)
{
  redo.clear();
  for (int i = count() - 1; i > index; i--)
  {
    MoveHistItem *mi = (MoveHistItem *)item(i);
    osl::record::opening::WMove wm(mi->getMove(), 0, mi->getWeight());
    redo.push_back(wm);
    removeItem(i);
  }
  updateToEnd();
}

void MoveHist::updateToEnd()
{
  osl::state::SimpleState state(osl::HIRATE);
  for (unsigned int i = 0; i < count() ; i++)
  {
    MoveHistItem *histItem = (MoveHistItem *) item(i);
    osl::ApplyMoveOfTurn::doMove(state, histItem->getMove());
  }
  emit selected(state);
}

void MoveHist::back()
{
  if (count() > 0)
  {
    MoveHistItem *mi = (MoveHistItem *)item(count() - 1);
    osl::record::opening::WMove wm(mi->getMove(), 0, mi->getWeight());
    redo.push_back(wm);
    removeItem(count() - 1);
    updateToEnd();
  }
}

void MoveHist::forward()
{
  if (redo.empty())
    return;
  osl::record::opening::WMove wm = redo.back();
  insertItem(new MoveHistItem(wm.getMove(), wm.getWeight()));
  redo.pop_back();
  updateToEnd();
}
