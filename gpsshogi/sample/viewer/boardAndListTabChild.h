#ifndef _BOARD_AND_LIST_TABCHILD_H
#define _BOARD_AND_LIST_TABCHILD_H
#include "boardTabChild.h"
#define QListViewItem Q3ListViewItem

class MoveTree;
class QTreeWidgetItem;

class BoardAndListTabChild : public BoardTabChild
{
Q_OBJECT
public:
  BoardAndListTabChild(QWidget *parent = 0);
  void forward();
  void backward();
  int moveCount() const {
    return numMoves;
  }
  osl::NumEffectState getStateAndMovesToCurrent(std::vector<osl::Move> &moves);
  void toLastState();
private slots:
  void updateBoardSlot(QTreeWidgetItem *item);

protected:
  virtual void updateBoard(QTreeWidgetItem *item);
  void init();
  bool tryForward();

  MoveTree *moveTree;
  int numMoves;
};

#endif // _BOARD_AND_LIST_TABCHILD_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
