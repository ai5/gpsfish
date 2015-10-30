#ifndef _VIEWER_H
#define _VIEWER_H
#include <qtabwidget.h>
#include "osl/simpleState.h"
#include <utility>
#include <vector>
namespace gpsshogi
{
  namespace gui
  {
    class Board;
  }
}

class MoveList;
class AnalysisViewer;
class CheckmateViewer;
class NtesukiViewer;
class KifuViewer;
class QuiescenceViewer;
class NetworkViewer;
class BoardEditor2;
class QPixmap;

namespace viewer
{  
struct EvalInfo
{
  int total;
  int opening, mid1, mid2, ending, tension;
  EvalInfo() : total(0), opening(0), mid1(0), mid2(0), ending(0), tension(0)
  {
  }
};
}


class Viewer : public QTabWidget
{
Q_OBJECT
public:
  QString dirPath;
  Viewer(QWidget *parent = 0);
  const viewer::EvalInfo evalState();
  void progressOfState(std::vector<int>&);
  std::pair<double,double> checkmateProbability();
  int moveCount() const;
  osl::Player turn() const;
  const osl::SimpleState& getState();
  void open(const QString& fileName); // csa
  void openUsi(const QString& fileName);
  void openKakinoki(const QString& fileName); // kakinoki
  void openKi2(const QString& fileName); // ki2
  void open(const QString& fileName, int index); // kisen
  void moveTo(int index); // only works on kifuView for now
  bool isSenteView() const;
  QString getFilename();
signals:
  void statusChanged();
  void orientationChanged(bool sente);
  void effectChanged(bool on);
  void analyzeOnlineDisabled();
public slots:
  void open();
  void nextFile();
  void prevFile();
  void watchFile(bool enable);
  void openUrl();
  void reloadUrl();
  void hirate();
  void network();
  void view();
  void viewInNewTab();
  void forward();
  void backward();
  void toInitialState();
  void toLastState();
  void analyze();
  void analyzeInNewTab();
  void checkmateSearch();
  void altCheckmateSearch();
  void checkmateSimulation();
  void quiescenceSearch();
  void quiescenceSearchHalf();
  void moveGenerateDialog();
  void toggleOrientation();
  void saveMovesToCurrent();
  void autoSaveMoves();
  void exportCurrent();
  void copy();
  void copyBoardAndMoves();
  void copyUsi();
  void copyBoardAndMovesUsi();
  void paste();
  void setOrientation(bool gote);
  void enableEffect(bool on);
  void highlightLastMove(bool on);
  void highlightBookMove(bool on);
  void showArrowMove(bool on);
  void setAnalyzeOnline(bool enable);
  void kifuAnalyze();
  void openEvalGraph();
  void editState();
  void testEvalDebug();
  void setState1();
  void setState2();
  void showEvalDiff();
  void showProgressDebug();
private slots:
  void closeTab();
  void notifyOrientation(int index);
  void notifyEffect(int index);
  void chatReceived();
  void chatDisplayed();
private:
  void openUrl(bool reload);
  void resetIcon();
  void setChatIcon();
  void analyze(bool newTab);
  QString getFilenameToSave();

  AnalysisViewer *analysisViewer;
  CheckmateViewer *checkmateViewer;
  NtesukiViewer *ntesukiViewer;
  QuiescenceViewer *quiescenceViewer;
  KifuViewer *kifuViewer;
  NetworkViewer *networkViewer;
  BoardEditor2 *editor;

  bool chatIcon;

  std::unique_ptr<osl::SimpleState> state1, state2;
};
#endif // _VIEWER_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
