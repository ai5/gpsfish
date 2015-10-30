#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H
#include <qmainwindow.h>

class OpeningEditor;
class MoveListView;
class QLabel;
class MainWindow : public QMainWindow
{
Q_OBJECT
public:
  MainWindow(QWidget *parent = 0, const char *name = 0);
private slots:
  void updateStatusBar();
private:
  OpeningEditor *editor;
  QLabel *moveLabel;
  QLabel *turnLabel;
  QLabel *winCountLabel;
};

#endif // _MAINWINDOW_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
