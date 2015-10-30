#ifndef _QUIESCENSE_TREE_H
#define _QUIESCENSE_TREE_H
#include "moveTree.h"

#include "osl/hashKey.h"

namespace osl
{
  namespace search
  {
    class SimpleHashTable;
    class SimpleHashRecord;
  }
}

class QuiescenceItem;

class QuiescenceTree : public MoveTree
{
  Q_OBJECT
public:
  QuiescenceTree(QWidget *parent = 0);
  const osl::SimpleState initialState() const { return initial_state; }
  void showRecord(const osl::hash::HashKey& key, const osl::SimpleState&,
		  const osl::search::SimpleHashTable *table);
protected:
  void contextMenuEvent(QContextMenuEvent *);
private slots:
  void showRecord();
  void expandMove(QTreeWidgetItem *item);
private:
  void addItem(int depth, QuiescenceItem *parent,
	       const osl::hash::HashKey& key, const osl::SimpleState&,
	       const osl::search::SimpleHashTable *table,
	       const osl::search::SimpleHashRecord *record);
  osl::SimpleState initial_state;
};


#endif // _QUIESCENCE_TREE_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
