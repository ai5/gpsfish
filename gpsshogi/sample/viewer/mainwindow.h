#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H
#include <qmainwindow.h>

class Viewer;
class MoveListView;
class QLabel;
class CopyLabel;
class CopyRateLabel;
class QAction;
class QPixmap;
class MainWindow : public QMainWindow
{
Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);
  void setUsiProxyAvailable(bool b);
  bool isUsiProxyAvailable() const;
private slots:
  void updateStatusBar();
  void updateFlipButton(bool sente);
  void updateEffectButton(bool on);
  void turnOffAnalyzeOnlineButton();
private:
  QPixmap loadPixmap(const QString &name);
  Viewer *viewer;
  QLabel *moveLabel;
  QLabel *turnLabel;
  CopyRateLabel *evalLabel;
  CopyRateLabel *progressLabel;
  CopyRateLabel *threatmateLabel;
  QAction *flipAction;
  QAction *effectAction, *lastMoveAction, *bookMoveAction, *arrowMoveAction, *analyzeOnlineAction;
  CopyLabel *filenameLabel;
  CopyRateLabel *proDBLabel, *blackGpsDBLabel, *whiteGpsDBLabel;
  bool usiProxyAvailable;
};

#endif // _MAINWINDOW_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
