#include "moveTree.h"
#include "viewer.h"
#include "osl/eval/pieceEval.h"

#if QT_VERSION >= 0x050000
#  include <QtWidgets>
#else
#  include <QtGui>
#endif

MoveTree::MoveTree(QWidget *parent)
  : QTreeWidget(parent)
{
  setRootIsDecorated(true);
}

MoveTree::~MoveTree()
{
}

void MoveTree::buildContextMenu(QMenu *menu)
{
  QAction *expandBestAction = new QAction(tr("Expand Best"), this);
  connect(expandBestAction, SIGNAL(triggered()),
	  this, SLOT(expandBest()));
  menu->addAction(expandBestAction);
  QAction *closeAllAction = new QAction(tr("Close All"), this);
  connect(closeAllAction, SIGNAL(triggered()),
	  this, SLOT(closeAll()));
  menu->addAction(closeAllAction);

  menu->addSeparator();
  QAction *moveGenerateAction = new QAction(tr("Show Generated &Moves"), this);
  connect(moveGenerateAction, SIGNAL(triggered()),
	  this, SIGNAL(moveGenerate()));
  menu->addAction(moveGenerateAction);
}

void MoveTree::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu(this);
  buildContextMenu(&menu);
  menu.exec(event->globalPos());
}

void MoveTree::closeAll()
{
  QTreeWidgetItemIterator it(this);
  while (*it)
  {
    (*it)->setExpanded(false);
    ++it;
  }
}

void MoveTree::expandBest()
{
  QList<QTreeWidgetItem *> selected = selectedItems();
  if (selected.empty())
    return;

  MoveTreeItem *item = (MoveTreeItem *)selected[0];
  expandBestChildren(item);
}

void MoveTree::expandBestChildren(MoveTreeItem *item)
{
  if (!item)
    return;

  item->setExpanded(true);
  for (int i=0; i<item->childCount(); ++i)
  {
    MoveTreeItem *child = (MoveTreeItem *)item->child(i);
    if (child->isBestMove())
    {
      child->setExpanded(true);
      expandBestChildren(child);
      break;
    }
  }
}

bool MoveTreeItem::operator<(const QTreeWidgetItem& other) const
{
  const int col = treeWidget()->sortColumn();
  if (col != 0)
  {
    return text(col) < other.text(col);
  }

  const MoveTreeItem& i = (const MoveTreeItem&)other;
  const osl::Move m2 = i.getMove();

  if ((! move.isNormal()) && (! m2.isNormal()))
    return true;
  else if (! move.isNormal())
    return false;
  else if (! m2.isNormal())
    return true;

  if (move.to().x() < m2.to().x())
    return true;
  else if (move.to().x() > m2.to().x())
    return false;
  else
  {
    if (move.to().y() < m2.to().y())
      return true;
    else if (move.to().y() > m2.to().y())
      return false;
    else
    {
      const int x1 = osl::eval::Ptype_Eval_Table.value(move.ptype()) +
                     osl::eval::Ptype_Eval_Table.value(osl::unpromote(move.ptype()));
      const int x2 = osl::eval::Ptype_Eval_Table.value(m2.ptype()) +
                     osl::eval::Ptype_Eval_Table.value(osl::unpromote(m2.ptype()));
      return (x1 < x2);
    }
  }
}
