#ifndef _MOVEHIST_H
#define _MOVEHIST_H
#include <qglobal.h>
#if QT_VERSION >= 0x040000
#include <Q3ListBox>
#else
#include <qlistbox.h>
#define Q3ListBox QListBox
#define Q3ListBoxText QListBoxText
#endif
#include "osl/stl/vector.h"
#include "osl/move.h"
#include "osl/state/simpleState.h"
#include "osl/record/opening/openingBook.h"

class MoveHistItem : public Q3ListBoxText
{
public:
  MoveHistItem(osl::Move m, int weight);
  osl::Move getMove() const {
    return move;
  }
  int getWeight() const {
    return weight;
  }
private:
  osl::Move move;
  int weight;
};

class MoveHist : public Q3ListBox
{
Q_OBJECT
public:
  MoveHist(QWidget *parent = 0, const char *name = 0);
signals:
  void selected(osl::state::SimpleState state);
public slots:
  void add(osl::Move m, int weight);
  void back();
  void forward();
  void initialState();
private slots:
  void update(int index);
private:
  void updateToEnd();

  osl::vector<osl::record::opening::WMove> redo;
};

#endif // _MOVEHIST_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
