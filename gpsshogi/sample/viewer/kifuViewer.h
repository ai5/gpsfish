#ifndef _KIFU_VIEWER_H
#define _KIFU_VIEWER_H
#include "boardTabChild.h"
#include "osl/basic_type.h"
#include "osl/numEffectState.h"
#include "osl/numEffectState.h"
#include "osl/record/searchInfo.h"
#include "osl/hashKey.h"
#include "kifuFile.h"
#include <QFile>
#include <QMutex>
#include <QTreeWidget>
#include <QDialog>
#include <vector>

class MoveList;
class QTableWidget;
class QListWidget;
class QTimer;
class QTextBrowser;
class QWidget;

class KifuViewer : public BoardTabChild
{
Q_OBJECT
public:
  KifuViewer(QWidget *parent = 0);
  ~KifuViewer();
  void forward();
  void backward();
  void open(const osl::SimpleState& state,
	    const std::vector<osl::Move>& board_moves);
  void open(const osl::SimpleState& state,
	    const std::vector<osl::Move>& board_moves,
	    const std::vector<int>& consumed_time);
  void open(KifuFile *file);
  void toInitialState();
  void toLastState();
  void watchFile(bool enable);

  int moveCount() const {
    return index + 1;
  }
  osl::NumEffectState getStateAndMovesToCurrent(std::vector<osl::Move> &moves);
  const KifuFile *getKifuFile();
  QString getFilename() {
    return filename;
  }
  QWidget *moveGenerateDialog();
  static void setAnalyzeOnline(bool enable);
  void analyze();
  void setAutoSaveFile(const QString& filename);
public slots:
  void updateIndex(int i);
  void openEvalGraph(bool force);
  void updateStatusForAnalysis();
  void prepareUpdateForAnalysis(const osl::SimpleState &state,
				const std::vector<osl::Move> &moves,
				int limit, osl::Move next_move);
protected:
  osl::Move getNextMove();
private slots:
  void boardMoved(osl::Move move);
  void updateTime();
  void reloadFile();
  void stopAnalysisOnline();
signals:
  void stateReset();  
  void indexUpdate(int);  
  void analyzeOnlineDisabled();
private:
  int consumedTime(osl::Player player) const;
  QString secondToTime(int time);
  void updateStateToIndex(int n);
  void updateState();
  void resetState();
  void clearState();
  void doAutoSaveMoves();

  MoveList *list;
  QTableWidget *timeView;
  QListWidget *players;
  QListWidget *moveInfoView;
  QTextBrowser *commentView;
  QTimer *timer;
  uint lastReadTime;
  int index;
  std::vector<osl::Move> manualMoves;
  int manualIndex;
  std::vector<QString> comments;
  std::vector<osl::record::SearchInfo> move_info;
  std::vector<int> move_eval;
  std::vector<double> move_progress;
  QString filename;
  std::unique_ptr<KifuFile> kifuFile;
  QMutex kifuFileMutex;
  std::unique_ptr<QFile> autoSaveFile;
  bool ignoreMoveSignal;
  static volatile bool analyzeOnlineEnabled, stateForAnalysisChanged;
  volatile bool watchFileEnabled;
  struct AnalysisOnlineThread;
  static std::unique_ptr<AnalysisOnlineThread> thread;
  static std::unique_ptr<osl::NumEffectState> stateForAnalysis;
public:
  class AnalysisOnlineDialog;
  static AnalysisOnlineDialog *analysisOnlineDialog;
};

class KifuViewer::AnalysisOnlineDialog : public QDialog
{
Q_OBJECT
  QTreeWidget *pvList;
  QTextCodec *codec;
  osl::NumEffectState root;
  osl::HashKey root_key;
  osl::Move cur_move;
  size_t node_count;
  double elapsed;
  static QRect lastGeometry;
protected:
  void contextMenuEvent(QContextMenuEvent *event);
public:
  volatile bool closed;
  explicit AnalysisOnlineDialog(const osl::NumEffectState& state, 
				QWidget *parent = 0);
  void setState(const osl::NumEffectState& state)
  {
    root = state;
    root_key = osl::HashKey(state);
    cur_move = osl::Move();
    node_count = 0;
    elapsed = 0;
    pvList->setUpdatesEnabled(false);
    pvList->clear();
    pvList->setUpdatesEnabled(true);
  }
  void showPV(unsigned long key,
              int depth, size_t node_count, double elapsed, int value,
              const std::vector<osl::Move>& pv, const char *additional_message,
              const std::vector<char> *pv_threatmate);
public slots:
  void updateStatus(const QString& status);
  void showPVInt(unsigned long key,
                 int depth, unsigned long node_count, double elapsed, int value, const std::vector<int>&);
  void rootMove(unsigned long key, int cur_move)
  {
    if (closed || key != root_key.signature())
      return;
    this->cur_move = osl::Move::makeDirect(cur_move);
    // Too many rootMove() can be called in a short period.
    // Thus, updateStatus() will be delayed until timeInfo() comes.
    // updateStatus();
  }
  void rootForcedMove(unsigned long key, int the_move)
  {
    if (closed || key != root_key.signature())
      return;
    std::vector<osl::Move> pv;
    pv.push_back(osl::Move::makeDirect(the_move));
    showPV(key, 100, this->node_count, this->elapsed, 0, pv, " (forced move)", nullptr);
  }
  void rootLossByCheckmate(unsigned long key)
  {
    if (closed || key != root_key.signature())
      return;
    std::vector<osl::Move> pv;
    showPV(key, 100, this->node_count, this->elapsed, -10000000, pv, "(loss by checkmate)", nullptr);
  }
  void timeInfo(unsigned long key, unsigned long node_count, double elapsed)
  {
    if (closed || key != root_key.signature())
      return;
    this->node_count = node_count;
    this->elapsed = elapsed;
  }
  void showEvent(QShowEvent *event)
  {
    setGeometry(lastGeometry);
    if (geometry().width() < 100 || geometry().height() < 100)
      resize(std::max(geometry().width(), 100), 
	     std::max(geometry().height(),100));
    QDialog::showEvent(event);
  }
  void hideEvent(QHideEvent *event)
  {
    lastGeometry = geometry();
    QDialog::hideEvent(event);
  }
  void onCopyToClipBoard();
};

#endif // _KIFU_VIEWER_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
