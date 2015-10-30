#ifndef _OPENING_BOOK_OPENINGEDITOR_H
#define _OPENING_BOOK_OPENINGEDITOR_H
#include <qwidget.h>
#include <Qt3Support/q3listbox.h>
#include "osl/stl/vector.h"
#include "../editor.h"

namespace gpsshogi
{
  namespace gui
  {
    class Board;
  }
}

class MoveListView;
class MoveHist;
class MoveItem;
class Q3ListBox;

class OpeningEditor : public QWidget
{
Q_OBJECT
public:
  explicit OpeningEditor(const std::string& fileName,
			 const char *csaFile,
			 QWidget *parent = 0, const char *name = 0);
  ~OpeningEditor();
  int moveCount() const;
  osl::Player turn() const;
  int blackWinCount();
  int whiteWinCount();
signals:
  void statusChanged();
public slots:
  void addMove();
  void deleteMove();
  void clearWeight();
  void normalizeWeight();
  void showStates();
private slots:
  void move(osl::Move m);
  void traceBestMoves(Q3ListBoxItem *);
  void back();
  void forward();
  void initialState();
  void setState(osl::state::SimpleState sstate);
  void updateWeight(MoveItem *item);

private:
  Editor *editor;
  MoveListView *list;
  MoveHist *hist;
  Q3ListBox *bestMoves;
  gpsshogi::gui::Board *board;
  osl::state::SimpleState sstate;
  OpeningEditor(const OpeningEditor&);

  void updateState();
};

#endif // _OPENING_BOOK_OPENINGEDITOR_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
