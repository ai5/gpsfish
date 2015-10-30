#ifndef _MOVELIST_H
#define _MOVELIST_H
#include <qglobal.h>
#if QT_VERSION >= 0x040000
#include <Q3ListView>
#else
#include <qlistview.h>
#define Q3ListView QListView
#define Q3ListViewItem QListViewItem
#endif
#include <qpoint.h>
#include <osl/stl/vector.h>
#include <osl/record/opening/openingBook.h>

class MoveItem : public Q3ListViewItem
{
public:
  MoveItem(osl::Move m, int weight, Q3ListView *parent);
  osl::Move getMove() const {
    return move;
  }
  int getWeight() const {
    return weight;
  }
  void setWeight(int newWeight) {
    weight = newWeight;
  }
  virtual int compare(Q3ListViewItem *i, int col, bool ascending) const;
private:
  osl::Move move;
  int weight;
};

class MoveListView : public Q3ListView
{
Q_OBJECT
public:
  MoveListView(osl::vector<osl::record::opening::WMove>& moves,
	       QWidget *parent = 0, const char *name = 0);
  void update(osl::vector<osl::record::opening::WMove> moves);
signals:
  void selected(osl::Move m, int weight);
  void updated(MoveItem *item);
private slots:
  void update(Q3ListViewItem *item);
  void renamed(Q3ListViewItem *item, int column, const QString& text);
  void startEdit(Q3ListViewItem *item, const QPoint& p, int c);
};

#endif // _MOVELIST_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
