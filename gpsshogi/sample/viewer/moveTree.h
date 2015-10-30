#ifndef _MOVE_TREE_H
#define _MOVE_TREE_H
#include "osl/basic_type.h"
#include <QTreeWidget>

class MoveTreeItem;
class QMenu;

class MoveTree : public QTreeWidget
{
Q_OBJECT
public:
  MoveTree(QWidget *parent = 0);
  virtual ~MoveTree();
public slots:
  void expandBest();
  void closeAll();
signals:
  void moveGenerate();
protected:
  void expandBestChildren(MoveTreeItem *item);
  void contextMenuEvent(QContextMenuEvent *event);
  virtual void buildContextMenu(QMenu *menu);
};

class MoveTreeItem : public QTreeWidgetItem
{
  // Str1 must correspond to m.
public:
  MoveTreeItem(MoveTree *parent, osl::Move m, const QString& s1 = "")
    : QTreeWidgetItem(parent, QStringList(s1)), move(m), depth(1)
  {
  }
  MoveTreeItem(MoveTreeItem *parent, osl::Move m, const QString& s1 = "")
    : QTreeWidgetItem(parent, QStringList(s1)), move(m), depth(parent->getDepth() + 1)
  {
  }
  osl::Move getMove() const {
    return move;
  }
  virtual bool isBestMove() const {
    return true;
  }
  virtual void setNextMove() = 0;
  bool operator<(const QTreeWidgetItem& other) const;
  int getDepth() const
  {
    return depth;
  }
protected:
  virtual void setCellColor() = 0;
private:
  const osl::Move move;
  const int depth;
};

#endif // _MOVE_TREE_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
