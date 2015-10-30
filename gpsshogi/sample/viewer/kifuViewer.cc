#include "kifuViewer.h"
#include "moveList.h"
#include "mainwindow.h"
#include "searchMonitor.h"

#include "moveGeneratorDialog.h"
#include "kifuAnalyzer.h"
#include "dualEvaluationDialog.h"

#include "gpsshogi/gui/board.h"
#include "gpsshogi/gui/util.h"

#include "osl/search/searchMonitor.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/record/csaRecord.h"
#include "osl/record/kisen.h"
#include "osl/record/ki2.h"
#include "osl/usi.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>
#include <QListWidget>
#include <QTableWidget>
#include <QThread>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMenu>
#include <QWidget>
#include <QTextCodec>
#include <QTimer>
#include <QFile>
#include <QTextBrowser>

#include <iostream>

volatile bool KifuViewer::analyzeOnlineEnabled = false;
volatile bool KifuViewer::stateForAnalysisChanged = false;
std::unique_ptr<osl::NumEffectState> KifuViewer::stateForAnalysis;
std::unique_ptr<KifuViewer::AnalysisOnlineThread> KifuViewer::thread;
KifuViewer::AnalysisOnlineDialog *KifuViewer::analysisOnlineDialog = 0;


KifuViewer::KifuViewer(QWidget *parent)
  : BoardTabChild(parent), lastReadTime(0),
    index(-1), manualIndex(-1), kifuFileMutex(QMutex::Recursive), ignoreMoveSignal(false),
    watchFileEnabled(false)
{
  list = new MoveList(this);
  players = new QListWidget(this);
  timeView = new QTableWidget(2, 1, this);
  moveInfoView = new QListWidget(this);
  commentView = new QTextBrowser(this);
  updateTime();

  QVBoxLayout *eastLayout = new QVBoxLayout;
  eastLayout->addWidget(players, 4);
  eastLayout->addWidget(timeView, 3);
  eastLayout->addWidget(moveInfoView, 6);
  eastLayout->addWidget(commentView, 2);
  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(list, 1);
  mainLayout->addWidget(board, 4);
  mainLayout->addLayout(eastLayout, 2);

  timer = new QTimer(this);

  connect(list, SIGNAL(currentRowChanged(int)),
	  this, SLOT(updateIndex(int)));
  connect(board, SIGNAL(moved(osl::Move)),
	  this, SLOT(boardMoved(osl::Move)));
  connect(board, SIGNAL(statusChanged()),
	  this, SLOT(updateTime()));
  connect(board, SIGNAL(statusChanged()),
	  this, SLOT(notifyState()));
  connect(timer, SIGNAL(timeout()),
	  this, SLOT(reloadFile()));
  connect(timer, SIGNAL(timeout()),
	  this, SLOT(updateStatusForAnalysis()));
  connect(this, SIGNAL(statusChanged(const osl::SimpleState &,
				     const std::vector<osl::Move> &,
				     int, osl::Move)),
	  this, SLOT(prepareUpdateForAnalysis(const osl::SimpleState &,
					      const std::vector<osl::Move> &,
					      int, osl::Move)));
  connect(this, SIGNAL(analyzeOnlineDisabled()), this, SLOT(stopAnalysisOnline()));

  timer->setSingleShot(false);
  timer->start(1000);
}

KifuViewer::~KifuViewer()
{
  setAnalyzeOnline(false);
}

void KifuViewer::forward()
{
  if (index >= (int) list->numMoves() - 1)
    return;

  if (manualMoves.size() > 0)
  {
    updateStateToIndex(index);
  }

  manualMoves.clear();

  osl::Move move = list->getMove(++index);

  if (!move.isPass())
  {
    ignoreMoveSignal = true;
    board->move(move);
    ignoreMoveSignal = false;
    list->setCurrentItem(list->item(index));
  }
  manualIndex = -1;
  emit indexUpdate(index);
}

void KifuViewer::backward()
{
  if (manualIndex > 0)
  {
    manualIndex--;
    manualMoves.pop_back();
    updateState();
    return;
  }
  else if (manualIndex == 0)
  {
    manualIndex = -1;
    manualMoves.clear();
    updateState();
    return;
  }

  if (index < 0)
  {
    return;
  }

  updateStateToIndex(--index);
  emit indexUpdate(index);
}

void KifuViewer::updateStateToIndex(int n)
{
  if (n < (int)list->numMoves())
  {
    osl::NumEffectState state(initialState);
    for (int i = 0; i <= n; i++)
    {
      state.makeMove(list->getMove(i));
    }
    board->setState(state, list->getMove(n));
    list->setCurrentItem(list->item(index));
  }
}

void KifuViewer::updateState()
{
  osl::NumEffectState state(initialState);
  for (int i = 0; i <= index; i++)
  {
    state.makeMove(list->getMove(i));
  }
  for (int i = 0; i <= manualIndex && i < (int)manualMoves.size(); i++)
  {
    state.makeMove(manualMoves[i]);
  }
  if (manualIndex >= 0 && manualIndex < (int)manualMoves.size())
    board->setState(state, manualMoves[manualIndex]);
  else
  {
    const osl::Move move = index >= 0 ? list->getMove(index) : osl::Move();
    board->setState(state, move);
  }
  if (index == -1)
  {
    list->setCurrentItem(list->item(0));
  }
  else
  {
    list->setCurrentItem(list->item(index));
  }
}

void KifuViewer::boardMoved(osl::Move move)
{
  if (ignoreMoveSignal)
    return;
  if (move == list->getMove(index + 1) && manualMoves.size() == 0)
  {
    index++;
  }
  else
  {
    manualMoves.push_back(move);
    manualIndex = manualMoves.size() - 1;
  }

  doAutoSaveMoves();
}

void KifuViewer::updateTime()
{
  timeView->clear();
  QStringList headers, vheaders;
  headers << QString::fromUtf8("消費時間");
  vheaders << QString::fromUtf8("先手  ") << QString::fromUtf8("後手  ");
  timeView->setHorizontalHeaderLabels(headers);
  timeView->setVerticalHeaderLabels(vheaders);
  QTableWidgetItem *white_time = new QTableWidgetItem(secondToTime(consumedTime(osl::WHITE)));
  white_time->setFlags(Qt::NoItemFlags);
  white_time->setTextAlignment(Qt::AlignRight);
  QTableWidgetItem *black_time = new QTableWidgetItem(secondToTime(consumedTime(osl::BLACK)));
  black_time->setFlags(Qt::NoItemFlags);
  black_time->setTextAlignment(Qt::AlignRight);
  timeView->setItem(0, 0, black_time);
  timeView->setItem(1, 0, white_time);

  timeView->resizeColumnsToContents();
  timeView->resizeRowsToContents();
#if QT_VERSION >= 0x050000
  timeView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
  timeView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif

  moveInfoView->clear();
  commentView->clear();
  if (0 <= index && manualIndex == -1)
  {
    if (index < (int) comments.size())
      commentView->setText(comments[index]);
    if (index < (int) move_info.size())
    {
      const osl::record::SearchInfo& info = move_info[index];
      new QListWidgetItem(QString::fromUtf8("評価値 ") +
                          QString("%1").arg(info.value), moveInfoView);
      for (size_t i = 0; i < info.moves.size(); ++i) {
        new QListWidgetItem(gpsshogi::gui::Util::moveToString(info.moves[i]),
                            moveInfoView);
      }
    }
  }
}

int KifuViewer::consumedTime(osl::Player player) const
{
  int time = 0;
  for (int i = (player == osl::BLACK) ? 0 : 1; i <= index; i += 2)
  {
    time += list->getTime(i);
  }
  return time;
}

QString KifuViewer::secondToTime(int time)
{
  int hour = 0;
  int min = 0;
  int second = time;

  if (second >= 60)
  {
    min = second / 60;
    second = second % 60;
    if (min >= 60)
    {
      hour = min / 60;
      min = min % 60;
    }
  }
  QString str;
  str.sprintf("%2d %s", second, "秒");
  if (min > 0 || hour > 0)
  {
    QString minStr;
    minStr.sprintf("%2d %s ", min, "分");
    str.prepend(minStr);
    if (hour > 0)
    {
      QString hourStr;
      hourStr.sprintf("%2d %s ", hour, "時間");
      str.prepend(hourStr);
    }
  }
  return str;
}

void KifuViewer::reloadFile()
{
  if (! watchFileEnabled)
    return;
  QMutexLocker lk(&kifuFileMutex);
  if (kifuFile && kifuFile->reloadIfChanged())
  {
    open(kifuFile.get());
    toLastState();
  }
}

class OnlineViewItem : public QTreeWidgetItem
{
  int depth;
  int relative_value;
  size_t node_count;
public:
  OnlineViewItem(QTreeWidget *pvList, int _depth, double elapsed, size_t _node_count,
		 int value, QString pv, osl::Player turn)
    : QTreeWidgetItem(pvList,
                      QStringList() << QString("%1").arg(_depth, 2) <<
		                       QString("%1").arg(elapsed, 0, 'f', 1) <<
		                       QString("%1").arg(QLocale(QLocale::English).toString(static_cast<qulonglong>(_node_count))) <<
		                       QString("%1").arg(value, 5) <<
		                       pv),
      depth(_depth), relative_value(value*osl::sign(turn)), node_count(_node_count)
  {
  }
  bool operator<(const QTreeWidgetItem& other_item) const
  {
    const OnlineViewItem& item = (const OnlineViewItem&)other_item;
    
    if (depth != item.depth)
    {
      return depth > item.depth;
    }
    else
    {
      return node_count > item.node_count;
    }
  }
};



KifuViewer::AnalysisOnlineDialog::AnalysisOnlineDialog(const osl::NumEffectState& state, 
			      QWidget *parent)
  : QDialog(parent), pvList(0),
    codec(QTextCodec::codecForName("EUC-JP")), 
    root(state), root_key(state), 
    node_count(0), elapsed(0), closed(false)
{
  QVBoxLayout *layout = new QVBoxLayout(this);

  pvList = new QTreeWidget(this);
  pvList->setColumnCount(5);
  pvList->setHeaderLabels(QStringList() << "Depth" <<
			  "Elapsed" <<
			  "Node" <<
			  "Value" <<
			  "Principal Variation");
  pvList->setAllColumnsShowFocus(true) ;
  layout->addWidget(pvList);
}

void KifuViewer::AnalysisOnlineDialog::updateStatus(const QString& status)
{
  if (! cur_move.isNormal() || closed)
    return;

  setWindowTitle(status);
}

void KifuViewer::AnalysisOnlineDialog::showPVInt(unsigned long key,
      int depth, size_t node_count, double elapsed, int value, const std::vector<int>& pv_int)
{
  std::vector<osl::Move> pv;
  pv.reserve(pv_int.size());
  for (const int m : pv_int)
  {
    pv.push_back(osl::Move::makeDirect(m));
  }
  showPV(key, depth, node_count, elapsed, value, pv, "", nullptr);
}

void KifuViewer::AnalysisOnlineDialog::
showPV(unsigned long key,
       int depth, size_t node_count, double elapsed, int value,
       const std::vector<osl::Move>& pv, const char *additional_message,
       const std::vector<char> *pv_threatmate)
{
  if (closed || key != root_key.signature())
    return;
  std::string euc_pv = pv_threatmate
    ? osl::ki2::show(&*pv.begin(), &*pv.end(),
		     &*pv_threatmate->begin(), &*pv_threatmate->end(),
		     root)
    : osl::ki2::show(&*pv.begin(), &*pv.end(), root);
  {
    pvList->setUpdatesEnabled(false);
    pvList->setSortingEnabled(false);

    QTreeWidgetItem *item =
      new OnlineViewItem(pvList, depth, elapsed, node_count, 
			 value,
			 codec->toUnicode(euc_pv.c_str(), euc_pv.length())+additional_message,
			 root.turn());
    for (int i=0; i<=3; ++i)
    {
      item->setTextAlignment(i, Qt::AlignRight);
      pvList->resizeColumnToContents(i);
    }

    pvList->setSortingEnabled(true);
    pvList->scrollToItem(item);
    pvList->setUpdatesEnabled(true);
  }
  this->node_count = node_count;
  this->elapsed = elapsed;
}

void KifuViewer::AnalysisOnlineDialog::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu(this);
  QAction *copyToClipBoard = new QAction(tr("Copy to Clip Board"), this);
  menu.addAction(copyToClipBoard);
  connect(copyToClipBoard, SIGNAL(triggered()),
	  this, SLOT(onCopyToClipBoard()));

  menu.exec(event->globalPos());
}

void KifuViewer::AnalysisOnlineDialog::onCopyToClipBoard()
{
  QList<QTreeWidgetItem *> selected = pvList->selectedItems();
  if (selected.empty())
    return;

  OnlineViewItem *item = (OnlineViewItem *)selected[0];
  if (item)
  {
    QClipboard *clipboard = QApplication::clipboard();

    clipboard->setText(QString("[%1] %2").arg(item->text(3))
                                         .arg(item->text(4)));
  }
}




QRect KifuViewer::AnalysisOnlineDialog::lastGeometry(100, 100, 700, 300);

struct KifuViewer::AnalysisOnlineThread : public QThread, public osl::misc::Align16New
{
  bool isUsiProxyAvailable;
  osl::NumEffectState input;
  std::unique_ptr<osl::game_playing::SearchPlayer> player;
  QMutex mutex;
  volatile bool stop;
  std::shared_ptr<ViewerSearchMonitor> monitor;
  osl::HashKey key;
  AnalysisOnlineThread(bool _isUsiProxyAvailable, const osl::NumEffectState& initial) 
    : isUsiProxyAvailable(_isUsiProxyAvailable), input(initial), stop(false), key(initial)
  {
  }
  void run()
  {
    while (! stop)
    {
      std::unique_ptr<osl::game_playing::GameState> state;
      osl::HashKey last_key;
      {
	QMutexLocker lk(&mutex);
	state.reset(new osl::game_playing::GameState(input));

        if (isUsiProxyAvailable)
        {
          player.reset(new osl::game_playing::UsiProxyPlayer);
        }
        else
        {
          player.reset(new osl::game_playing::AlphaBeta2OpenMidEndingEvalPlayer);
        }
	monitor.reset(new ViewerSearchMonitor(input));

        connect(monitor.get(), SIGNAL(updated(const QString&)),
                analysisOnlineDialog, SLOT(updateStatus(const QString&)));
        connect(monitor.get(), SIGNAL(showPVSignal(unsigned long, int, unsigned long, double, int, const std::vector<int>&)),
                analysisOnlineDialog, SLOT(showPVInt(unsigned long, int, unsigned long, double, int, const std::vector<int>&)));
        connect(monitor.get(), SIGNAL(rootMoveSignal(unsigned long, int)),
                analysisOnlineDialog, SLOT(rootMove(unsigned long, int)));
        connect(monitor.get(), SIGNAL(timeInfoSignal(unsigned long, unsigned long, double)),
                analysisOnlineDialog, SLOT(timeInfo(unsigned long, unsigned long, double)));
        connect(monitor.get(), SIGNAL(rootForcedMoveSignal(unsigned long, int)),
                analysisOnlineDialog, SLOT(rootForcedMove(unsigned long, int)));
        connect(monitor.get(), SIGNAL(rootLossByCheckmateSignal(unsigned long)),
                analysisOnlineDialog, SLOT(rootLossByCheckmate(unsigned long)));

	player->addMonitor(monitor);
	player->setDepthLimit(2000, 400, 200);
	player->setNodeLimit(std::numeric_limits<size_t>::max());
	player->setTableLimit(std::numeric_limits<size_t>::max(), 200);
	player->enableMultiPV(100);
	player->setVerbose(0);
	last_key = key;
      }
      osl::MoveWithComment ret
	= player->selectBestMove(*state, 0,0,3600);
      while (! stop && last_key == key)
	QThread::msleep(500);
    }
  }
  void setState(const osl::NumEffectState& state)
  {
    const osl::HashKey new_key(state);
    QMutexLocker lk(&mutex);
    if (key == new_key)
      return;
    input = state;
    key = new_key;
    if (player && player->canStopSearch())
      player->stopSearchNow();
  }
  void finish()
  {
    QMutexLocker lk(&mutex);
    stop = true;
    if (player && player->canStopSearch())
      player->stopSearchNow();
    wait();
  }
};

void KifuViewer::prepareUpdateForAnalysis(const osl::SimpleState &origin,
					 const std::vector<osl::Move> &moves,
					 int /*limit*/, osl::Move /*next_move*/)
{
  if (! analyzeOnlineEnabled)
    return;
  osl::NumEffectState state(origin);
  for (size_t i = 0; i < moves.size(); i++)
    state.makeMove(moves[i]);
  stateForAnalysis.reset(new osl::NumEffectState(state));
  stateForAnalysisChanged = true;
}

void KifuViewer::updateStatusForAnalysis()
{
  if (!analyzeOnlineEnabled || !stateForAnalysisChanged)
    return;
  static time_t prev = 0;
  time_t now = time(0);
  if (now - prev < 1)
    return;
  prev = now;
  if (analysisOnlineDialog) {
    analysisOnlineDialog->setState(*stateForAnalysis);
  } else {
    analysisOnlineDialog = new AnalysisOnlineDialog(*stateForAnalysis, this);
    connect(analysisOnlineDialog, SIGNAL(accepted()), this, SIGNAL(analyzeOnlineDisabled()));
    connect(analysisOnlineDialog, SIGNAL(rejected()), this, SIGNAL(analyzeOnlineDisabled()));
    analysisOnlineDialog->show();
  }

  if (! thread) {
    thread.reset(new AnalysisOnlineThread(getMainWindow()->isUsiProxyAvailable(), *stateForAnalysis));
    thread->start();
  }
  else
  {
    thread->setState(*stateForAnalysis);
  }
  stateForAnalysisChanged = false;
}

void KifuViewer::stopAnalysisOnline()
{
  if (analyzeOnlineEnabled)
    setAnalyzeOnline(false);
}


void KifuViewer::setAnalyzeOnline(bool enable)
{
  analyzeOnlineEnabled = enable;
  if (thread && !enable) {
    thread->finish();
    thread.reset();
  }
  if (analysisOnlineDialog && !enable) {
    AnalysisOnlineDialog *dialog = analysisOnlineDialog;
    dialog->closed = true;
    analysisOnlineDialog = 0;
    dialog->close();
  }
}

void KifuViewer::clearState()
{
  comments.clear();
  move_info.clear();
  move_eval.clear();
  move_progress.clear();
  players->clear();
  list->clear();
  manualMoves.clear();
  index = -1;
  manualIndex = -1;
}

void KifuViewer::resetState()
{
  clearState();
  emit stateReset();
}

void KifuViewer::openEvalGraph(bool force)
{
  int move_info_count=0;
  if (! force)
  {
    for (size_t i=0; i< move_info.size(); ++i)
      if (move_info[i].value)
	++move_info_count;
  }
  if (force || move_info_count > 0)
  {
    DualEvaluationDialog *eval = new DualEvaluationDialog(); // no parent
    eval->setAttribute(Qt::WA_DeleteOnClose); // delete the pointer on close
    eval->setInfo(move_info);
    eval->setRawValues(move_eval, move_progress);

    connect(this, SIGNAL(indexUpdate(int)), eval, SLOT(setIndex(int)));
    connect(eval, SIGNAL(selected(int)), this, SLOT(updateIndex(int)));
    connect(this, SIGNAL(stateReset()), eval, SLOT(accept()));
    eval->show();
  }
}

void KifuViewer::open(const osl::SimpleState& state,
		      const std::vector<osl::Move>& board_moves)
{
  std::vector<int> t(board_moves.size(), 1);
  open(state, board_moves, t);
}

void KifuViewer::open(KifuFile *file)
{
  QMutexLocker lk(&kifuFileMutex);
  clearState();
  initialState.copyFrom(osl::NumEffectState(file->getInitialState()));
  board->setState(initialState);
  list->setState(initialState, file->getMoves(), file->getTime(), file->getComments());
  filename = file->getFilename();
  new QListWidgetItem(QString::fromUtf8("先手: ") +
                      file->getPlayerName(osl::BLACK), players);
  new QListWidgetItem(QString::fromUtf8("後手: ") +
                      file->getPlayerName(osl::WHITE), players);
  if (file->getYear() > 0)
    new QListWidgetItem(QString::fromUtf8("対局日: ")
			+ QString("%1").arg(file->getYear())
			+ QString::fromUtf8("年"), players);
  for (size_t i = 0; i < file->getInitialComment().size(); ++i) {
    new QListWidgetItem(file->getInitialComment()[i], players);
  }
  comments = file->getComments();
  move_info = file->getSearchInfo();
  {
    osl::NumEffectState state(initialState);
    osl::eval::ml::OpenMidEndingEval eval(state);
    const double scale = 200.0
      / eval.captureValue(osl::newPtypeO(osl::WHITE,osl::PAWN));
    for (osl::Move move: file->getMoves()) {
      state.makeMove(move);
      eval.update(state, move);
      move_eval.push_back(eval.value()*scale);
      move_progress.push_back(1.0*eval.progressValue()/eval.progressMax());
    }
  }
  if (file != kifuFile.get())
  {
    kifuFile.reset(file);
  }
  emit stateReset();
  openEvalGraph(false);
}

void KifuViewer::open(const osl::SimpleState& state,
		      const std::vector<osl::Move>& board_moves,
		      const std::vector<int>& consumed_time)
{
  clearState();
  initialState.copyFrom(osl::NumEffectState(state));
  filename = "";
  board->setState(initialState);
  list->setState(initialState, board_moves, consumed_time, std::vector<QString>());
  emit stateReset();
}


void KifuViewer::watchFile(bool enable)
{
  watchFileEnabled = enable;
}

osl::NumEffectState KifuViewer::getStateAndMovesToCurrent(std::vector<osl::Move> &m)
{
  for (int i = 0; i <= index; i++)
  {
    m.push_back(list->getMove(i));
  }
  for (int i = 0; i <= manualIndex; i++)
  {
    m.push_back(manualMoves[i]);
  }
  return initialState;
}

void KifuViewer::toInitialState()
{
  manualMoves.clear();
  index = -1;
  manualIndex = -1;
  updateState();
  emit indexUpdate(index);
}

void KifuViewer::toLastState()
{
  manualMoves.clear();
  manualIndex = -1;
  index = list->numMoves() - 1;
  updateState();
  emit indexUpdate(index);
}

void KifuViewer::updateIndex(int i)
{
  manualMoves.clear();
  manualIndex = -1;
  index = i;
  updateStateToIndex(i);
  emit indexUpdate(i);
}

QWidget *KifuViewer::moveGenerateDialog()
{
  MoveGeneratorDialog *dialog
    = (MoveGeneratorDialog *)BoardTabChild::moveGenerateDialog();
  if (dialog)
    connect(this, SIGNAL(statusChanged(const osl::SimpleState &,
				       const std::vector<osl::Move> &,
				       int, osl::Move)),
	    dialog, SLOT(setStatus(const osl::SimpleState &,
				   const std::vector<osl::Move> &,
				   int, osl::Move)));
  return dialog;
}

osl::Move KifuViewer::getNextMove()
{
  return list->getMove(index + 1);
}

void KifuViewer::analyze()
{
  std::vector<osl::Move> moves;
  for (size_t i = 0; i < list->numMoves(); i++)
  {
    moves.push_back(list->getMove(i));
  }

  KifuAnalyzer *analyzer = new KifuAnalyzer(initialState, moves);
  analyzer->show();
}

const KifuFile *KifuViewer::getKifuFile()
{
  QMutexLocker lk(&kifuFileMutex);
  return kifuFile.get();
}

void KifuViewer::setAutoSaveFile(const QString& filename)
{
  if (filename.endsWith(".usi"))
  {
    autoSaveFile.reset(new QFile(filename));
  }
  else
  {
    autoSaveFile.reset(NULL);
  }
}

void KifuViewer::doAutoSaveMoves()
{
  if (!autoSaveFile)
    return;

  std::vector<osl::Move> moves;
  getStateAndMovesToCurrent(moves);

  if (autoSaveFile->fileName().endsWith(".usi"))
  {
    autoSaveFile->open(QIODevice::WriteOnly | QIODevice::Append);
    autoSaveFile->write("position startpos moves ");
    QStringList slist;
    for (size_t i=0; i<moves.size(); ++i)
    {
      slist << (osl::usi::show(moves[i]).c_str());
    }
    autoSaveFile->write(slist.join(" ").toStdString().c_str());
    autoSaveFile->write("\n");
    autoSaveFile->close();
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
