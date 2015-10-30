#include "boardAndListTabChild.h"
#include "moveTree.h"


#include "gpsshogi/gui/board.h"
#include "moveGeneratorDialog.h"
#include <qinputdialog.h>
#include <vector>

BoardAndListTabChild::BoardAndListTabChild(QWidget *parent)
  : BoardTabChild(parent), numMoves(0)
{
}

void BoardAndListTabChild::init()
{
  board->setManualMovement(false);
  moveTree->setRootIsDecorated(true);
  connect(moveTree, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
	  this, SLOT(updateBoardSlot(QTreeWidgetItem *)));
  connect(moveTree, SIGNAL(itemActivated(QTreeWidgetItem *, int)),
	  this, SLOT(updateBoardSlot(QTreeWidgetItem *)));
  connect(moveTree, SIGNAL(moveGenerate()),
	  this, SLOT(moveGenerateDialog()));
}

void BoardAndListTabChild::updateBoardSlot(QTreeWidgetItem *item)
{
  if (item)
    updateBoard(item);
}

void BoardAndListTabChild::updateBoard(QTreeWidgetItem *li)
{
  MoveTreeItem *item = (MoveTreeItem *)li;
  std::vector<osl::Move> moves;
  for (; item; item = (MoveTreeItem *)item->parent())
  {
    moves.push_back(item->getMove());
  }
  osl::NumEffectState newState(initialState);
  osl::Move last_moved;
  for (int i = moves.size() - 1; i >= 0; i--)
  {
    if (moves[i].isInvalid())
      continue;
    newState.makeMove(moves[i]);
    last_moved = moves[i];
  }
  numMoves = moves.size();
  board->setState(newState, last_moved); 
}

void BoardAndListTabChild::forward()
{
  tryForward();
  return;
}

bool BoardAndListTabChild::tryForward()
{
  QList<QTreeWidgetItem *> selected = moveTree->selectedItems();
  if (!selected.empty()) {
    MoveTreeItem *item = (MoveTreeItem *)selected[0];
    item->setNextMove();
    item->setExpanded(true);

    for (int i=0; i<item->childCount(); ++i)
    {
      MoveTreeItem *c = (MoveTreeItem *)item->child(i);
      if (c->isBestMove())
      {
        updateBoard(c);
        moveTree->setCurrentItem(c);
        moveTree->scrollToItem(c);
        return true;
      }
    }
  } else {
    for (int i=0; i<moveTree->topLevelItemCount(); ++i)
    {
      MoveTreeItem *c = (MoveTreeItem *)moveTree->topLevelItem(i);
      if (c->isBestMove())
      {
        updateBoard(c);
        moveTree->setCurrentItem(c);
        moveTree->scrollToItem(c);
        return true;
      }
    }
  }
  return false;
}

void BoardAndListTabChild::backward()
{
  QList<QTreeWidgetItem *> selected = moveTree->selectedItems();
  
  if (selected.empty())
    return;

  MoveTreeItem *item = (MoveTreeItem *)selected[0];
  if (item && item->parent())
  {
    updateBoard((MoveTreeItem *)item->parent());
    moveTree->setCurrentItem(item->parent());
    moveTree->scrollToItem(item->parent());
  }
  else
  {
    updateBoard(0);
    item->setSelected(false);
  }
}

osl::NumEffectState BoardAndListTabChild::getStateAndMovesToCurrent(std::vector<osl::Move> &moves)
{
  QList<QTreeWidgetItem *> selected = moveTree->selectedItems();
  if (selected.empty())
    return initialState;

  MoveTreeItem *item = (MoveTreeItem *)selected[0];
  if (item && item->getMove().isInvalid())
    return initialState;
  std::vector<osl::Move> movesOnTree;
  for (; item; item = (MoveTreeItem *)item->parent())
  {
    movesOnTree.push_back(item->getMove());
  }
  for (int i = movesOnTree.size() - 1; i >= 0; i--)
  {
    moves.push_back(movesOnTree[i]);
  }
  return initialState;
}

void BoardAndListTabChild::toLastState()
{
  while (tryForward())
    ;
}

