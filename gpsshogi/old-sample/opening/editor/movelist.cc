#include "movelist.h"
#include "gpsshogi/gui/util.h"

MoveItem::MoveItem(osl::Move m, int weight, Q3ListView *parent)
  : Q3ListViewItem(parent, gpsshogi::gui::Util::moveToString(m),
		  QString("%1").arg(weight)),
    move(m), weight(weight)
{
  setRenameEnabled(1, true);
}

int MoveItem::compare(Q3ListViewItem *i, int col, bool ascending) const
{
  if (col == 1)
  {
    MoveItem *mi = (MoveItem *)i;
    if (weight == mi->weight)
      return 0;
    else if (weight < mi->weight)
      return -1;
    else
      return 1;
  }
  else
    return key(col, ascending).compare(i->key(col, ascending));
}

MoveListView::MoveListView(osl::vector<osl::record::opening::WMove>& moves,
			   QWidget *parent, const char *name)
  : Q3ListView(parent, name)
{
  setSelectionMode(Extended);
  addColumn(tr("Move"));
  addColumn(tr("Weight"));

  for (int i = moves.size() - 1; i >= 0; i--)
  {
    new MoveItem(moves[i].getMove(), moves[i].getWeight(), this);
  }
  connect(this, SIGNAL(doubleClicked(Q3ListViewItem *, const QPoint&, int)),
	  SLOT(update(Q3ListViewItem *)));
  connect(this, SIGNAL(itemRenamed(Q3ListViewItem *, int, const QString&)),
	  SLOT(renamed(Q3ListViewItem *, int, const QString&)));
  connect(this, SIGNAL(clicked(Q3ListViewItem *, const QPoint&, int)),
	  SLOT(startEdit(Q3ListViewItem *, const QPoint&, int)));
}

void MoveListView::update(Q3ListViewItem *item)
{
  if (!item)
    return;

  MoveItem *m = (MoveItem *)item;
  emit selected(m->getMove(), m->getWeight());
}

void MoveListView::update(osl::vector<osl::record::opening::WMove> moves)
{
  clear();
  for (int i = moves.size() - 1; i >= 0; i--)
  {
    new MoveItem(moves[i].getMove(), moves[i].getWeight(), this);
  }
}

void MoveListView::renamed(Q3ListViewItem *item, int column, const QString& text)
{
  assert(column == 1);
  MoveItem *mi = (MoveItem *)item;
  bool ok;
  int newValue = text.toInt(&ok);
  if (!ok || newValue < 0 || newValue > 10000)
  {
    mi->setText(1, QString("%1").arg(mi->getWeight()));
  }
  else
  {
    mi->setWeight(newValue);
    emit updated((MoveItem *)item);
  }
}

void MoveListView::startEdit(Q3ListViewItem *item, const QPoint&, int c)
{
  if (item && c == 1)
  {
    item->startRename(c);
  }
}
