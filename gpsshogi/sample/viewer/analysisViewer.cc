#include "analysisViewer.h"
#include "moveTree.h"
#include "searchdialogimpl.h"
#include "quiescenceTree.h"
#include "moveGeneratorDialog.h"
#include "searchMonitor.h"

#include "osl/search/simpleHashTable.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/search/moveGenerator.h"
#include "osl/search/fixedEval.h"
#include "osl/search/searchState2.h"
#include "osl/search/analyzer/recordSet_.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/sennichite.h"
#include "osl/csa.h"
#include "osl/record/ki2.h"

#include "gpsshogi/gui/util.h"
#include "gpsshogi/gui/board.h"

#if QT_VERSION >= 0x050000
#  include <QtWidgets>
#else
#  include <QtGui>
#endif
#include <QVBoxLayout>
#include <QComboBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QThread>
#include <QApplication>
#include <QString>
#include <QTextCodec>
#include <QTableView>

#include <sstream>
#include <unistd.h>
#include <time.h>

using gpsshogi::gui::Util;

const QString AnalysisViewer::PROGRESS_ALPHA_BETA = QString("Progress Alpha Beta2");
const QString AnalysisViewer::TEST_ALPHA_BETA = QString("OpenMidEndingEval Alpha Beta2");
const QString AnalysisViewer::USI_PROXY = QString("USI Engine (gpsfish)");


static void generateMoves(const osl::SimpleHashRecord *record,
			  const osl::SimpleState& sstate,
			  const osl::MoveStack& history,
			  osl::MoveLogProbVector& moves)
{
  osl::NumEffectState state(sstate);
  osl::search::MoveGenerator generator;
  moves.clear();

  osl::eval::ml::OpenMidEndingEval eval(state);
  generator.init(2000, record, eval, state, true, osl::Move());

  static osl::search::SearchState2::checkmate_t c;
  osl::search::SearchState2 search_state(state, c);
  search_state.setHistory(history);
  generator.generateAll(state.turn(), search_state, moves);
  if (! record->inCheck())
    moves.push_back(osl::MoveLogProb(osl::Move::PASS(state.turn()), 200));
}

static bool hasChildAfterMove(const osl::SimpleState& state,
			      const osl::search::SimpleHashTable& table,
			      const osl::MoveStack& history,
			      const osl::HashKey& child_key,
			      const osl::SimpleHashRecord *child_record,
			      const osl::Move move)
{
  osl::NumEffectState child_state(state);
  child_state.makeMove(move);
  osl::MoveStack new_history = history;
  new_history.push(move);
  osl::MoveLogProbVector moves;
  generateMoves(child_record, child_state, new_history, moves);
  for (size_t i=0; i<moves.size(); ++i)
  {
    if (table.find(child_key.newHashWithMove(moves[i].move())))
      return true;
  }
  return false;
}



class QuiescenceDialog : public QDialog
{
public:
  QuiescenceDialog(const osl::hash::HashKey& key, 
		   const osl::SimpleState& state,
		   const osl::search::SimpleHashTable *table,
		   QWidget *parent = 0)
    : QDialog(parent) {
    QuiescenceTree *tree = new QuiescenceTree(this);
    tree->showRecord(key, state, table);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tree);
    QPushButton *button = new QPushButton(this);
    button->setText("&OK");
    layout->addWidget(button);
    connect(button, SIGNAL(clicked()), this, SLOT(accept()));
    resize(layout->sizeHint());
  }
};



class SearchMoveTreeItem : public MoveTreeItem
{
public:
  SearchMoveTreeItem(SearchMoveTree *parent, osl::Move m, int v,
		     bool _bestMove, int _winState,
		     const osl::search::SimpleHashRecord *_record)
    : MoveTreeItem(parent, m),
      value(v), bestMove(_bestMove), winState(_winState), record(_record),
      parent_item(0), tree(parent)
  {
    setupText(m, v, record);
    setCellColor();
  }
  SearchMoveTreeItem(SearchMoveTreeItem *parent, osl::Move m, int v,
                     bool _bestMove, int _winState,
                     const osl::search::SimpleHashRecord *_record)
    : MoveTreeItem(parent, m),
      value(v), bestMove(_bestMove), winState(_winState), record(_record),
      parent_item(parent), tree(parent->tree)
  {
    setupText(m, v, record);
    setCellColor();
  }
  void setNextMove();
  bool isBestMove() const {
    return bestMove;
  }
  bool operator<(const QTreeWidgetItem& other) const;
  MoveTreeItem *bestChild() const;
  const osl::search::SimpleHashRecord *getRecord() const {
    return record;
  }
  void getMovesToCurrent(osl::MoveVector& moves) 
  {
    SearchMoveTreeItem *i = this;
    while (i) {
      moves.push_back(i->getMove());
      i = i->parent_item;
    }
    std::reverse(moves.begin(), moves.end());
  }
private:
  void setCellColor();
  void setupText(osl::Move m, int v,
		 const osl::search::SimpleHashRecord *record) {
    setText(0, Util::moveToString(m));
    setText(1, QString("%1").arg(v));

    if (m.player() == osl::WHITE) // here, turn is BLACK
    {
      setText(2, (record && record->lowerLimit() >= 0) ?
	      QString("%1").arg(record->lowerBound()) :
	      "*");
      setText(3, (record && record->upperLimit() >= 0) ?
	      QString("%1").arg(record->upperBound()) : "*");
      setText(4, record ? QString("%1").arg(record->lowerLimit()) : "*");
      setText(5, record ? QString("%1").arg(record->upperLimit()) : "*"); 
      setText(6, record ? QString("%1").arg(record->nodeCount()) : "");
    }
    else
    {
      setText(2, (record && record->upperLimit() >= 0) ?
	      QString("%1").arg(record->upperBound()) : "*");
      setText(3, (record && record->lowerLimit() >= 0) ?
	      QString("%1").arg(record->lowerBound()) :
	      "*");
      setText(4, record ? QString("%1").arg(record->upperLimit()) : "*");
      setText(5, record ? QString("%1").arg(record->lowerLimit()) : "*");
      setText(6, record ? QString("%1").arg(record->nodeCount()) : "");
    }
    for (int i=1; i<columnCount(); ++i) {
      setTextAlignment(i, Qt::AlignRight);
    }
  }
  int value;
  bool bestMove;
  int winState;
  const osl::search::SimpleHashRecord *record;
  SearchMoveTreeItem *parent_item;
  SearchMoveTree *tree;
public:
  static const int CHECKMATE_WIN = 0;
  static const int CHECKMATE_LOSE = 1;
  static const int NO_CHECKMATE = 2;
};

void SearchMoveTreeItem::setNextMove()
{
  if (!childCount() && record)
  {
    treeWidget()->setUpdatesEnabled(false);

    osl::MoveVector history;
    getMovesToCurrent(history);
    osl::NumEffectState state(tree->initialState());
    for (size_t i=0; i<history.size(); ++i)
      state.makeMove(history[i]);
  
    osl::MoveLogProbVector moves;
    osl::MoveStack move_history;
    if (history.size() > 1) 
      move_history.push(history[history.size()-2]);
    if (! history.empty()) 
      move_history.push(history[history.size()-1]);
    generateMoves(record, state, move_history, moves);
    const osl::HashKey key(state);

    for (size_t i=0; i<moves.size(); ++i)
    {
      const osl::Move move = moves[i].move();
      if (!move.isNormal())
      {
        continue;
      }
      const osl::HashKey child_key = key.newHashWithMove(move);
      const osl::SimpleHashRecord *child_record = tree->getTable().find(child_key);
      if (record->threatmate().maybeThreatmate(move.player()) &&
	  child_record &&
	  child_record->lowerLimit() == osl::search::SearchTable::CheckmateSpecialDepth &&
	  child_record->bestMove().isNormal())
	continue;

      int winState = NO_CHECKMATE;
      if (record->lowerLimit() == osl::search::SearchTable::CheckmateSpecialDepth &&
	  record->bestMove().isNormal() &&
	  move == record->bestMove().move())
	winState = CHECKMATE_WIN;
      else if (child_record &&
	       child_record->lowerLimit() == osl::search::SearchTable::CheckmateSpecialDepth &&
	       child_record->bestMove().isNormal())
	winState = CHECKMATE_LOSE;

      new SearchMoveTreeItem(this,
                             move,
                             moves[i].logProb(),
                             move == record->bestMove().move(),
                             winState,
                             child_record);
    }

    treeWidget()->setUpdatesEnabled(true);
  }
}


void SearchMoveTreeItem::setCellColor()
{
  static int column = 0;
  QBrush brush(foreground(column));
  if (winState == CHECKMATE_WIN)
  {
    brush.setColor(QColor("red"));
  }
  else if (winState == CHECKMATE_LOSE)
  {
    brush.setColor(QColor("gray"));
  }
  else if (record 
           && (record->threatmate().maybeThreatmate(osl::BLACK)
               || record->threatmate().maybeThreatmate(osl::WHITE)))
  {
    brush.setColor(QColor("purple"));
  }
  else if (bestMove)
  {
    brush.setColor(QColor("blue"));
  }
  setForeground(column, brush);
}

bool SearchMoveTreeItem::operator<(const QTreeWidgetItem& other) const
{
  const int col = treeWidget()->sortColumn();
  const SearchMoveTreeItem& item = (const SearchMoveTreeItem&)other;

  if (col == 0)
  {
    if (isBestMove())
    {
      return true;
    }
    else if (item.isBestMove())
    {
      return false;
    }
    else
    {
      return MoveTreeItem::operator<(other);
    }
  }
  else
  {
    bool b1 = true;
    const int i1 = text(col).toInt(&b1); // true for integer
    bool b2 = true;
    const int i2 = item.text(col).toInt(&b2);
    if (b1 && b2)
    {
      return i1 < i2;
    }
    else if (!b1 && !b2)
    {
      return text(col) < item.text(col);

    }
    else if (b1)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

SearchMoveTree::SearchMoveTree(QWidget *parent)
  : MoveTree(parent)
{
  setColumnCount(7);
  setHeaderLabels(QStringList() << "Move" <<
                                   "Probability" <<
                                   "Lower Bound" <<
                                   "Upper Bound" <<
                                   "Lower Limit" <<
                                   "Upper Limit" <<
                                   "Node Count");
  for (int i = 1; i < columnCount(); i++) {
    setColumnWidth(i, columnWidth(i) / 2);
  }
}

void SearchMoveTree::addRecord(const osl::SimpleState& state,
			       const osl::search::SimpleHashTable *t,
			       const osl::search::SimpleHashRecord *record,
			       const osl::game_playing::SearchPlayer *p,
			       const osl::MoveStack& history)
{
  initial_state = state;
  table = t;
  player = p;
  clear();
  addRootItem(record, history);
}

void SearchMoveTree::addRootItem(const osl::search::SimpleHashRecord *root_record, 
				 const osl::MoveStack& history)
{
  setSortingEnabled(false);

  osl::MoveLogProbVector moves;
  generateMoves(root_record, initial_state, history, moves);
  const osl::HashKey key(initial_state);
  for (size_t i=0; i<moves.size(); ++i)
  {
    const osl::HashKey child_key = key.newHashWithMove(moves[i].move());
    const osl::SimpleHashRecord *record = table->find(child_key);
    if (record == 0)
      continue;
    int winState = SearchMoveTreeItem::NO_CHECKMATE;
    if (root_record->lowerLimit() == osl::search::SearchTable::CheckmateSpecialDepth &&
	root_record->bestMove().isNormal() &&
	moves[i].move() == root_record->bestMove().move())
      winState = SearchMoveTreeItem::CHECKMATE_WIN;
    else if (record &&
	     record->lowerLimit() == osl::search::SearchTable::CheckmateSpecialDepth &&
	     record->bestMove().isPass())
      winState = SearchMoveTreeItem::CHECKMATE_LOSE;

    SearchMoveTreeItem *item = new SearchMoveTreeItem(this,
                                  moves[i].move(),
                                  moves[i].logProb(),
                                  moves[i].move() == root_record->bestMove().move(),
                                  winState,
                                  record);
    if (record && hasChildAfterMove(initial_state, *table, history, 
				    child_key, record, moves[i].move()))
    {
      item->setNextMove();
    }
  }

  setSortingEnabled(true);
}

void SearchMoveTree::showQuiescenceRecord()
{
  QList<QTreeWidgetItem *> selected = selectedItems();
  if (selected.empty())
    return;

  SearchMoveTreeItem *item = (SearchMoveTreeItem *)selected[0];

  std::vector<osl::Move> moves;
  for (; item; item = (SearchMoveTreeItem *)item->parent())
  {
    moves.push_back(item->getMove());
  }
  osl::NumEffectState newState(initial_state);
  for (int i = moves.size() - 1; i >= 0; i--)
  {
    if (moves[i].isInvalid())
      break;
    newState.makeMove(moves[i]);
  }
  QuiescenceDialog *dialog = new QuiescenceDialog(osl::HashKey(newState),
						  newState,
						  table,
						  this);
  dialog->exec();
}

void SearchMoveTree::showRecord()
{
  QList<QTreeWidgetItem *> selected = selectedItems();
  if (selected.empty())
    return;

  SearchMoveTreeItem *item = (SearchMoveTreeItem *)selected[0];
  if (!item || !item->getRecord())
    return;

  std::ostringstream oss(std::ostringstream::out);
  const osl::search::SimpleHashRecord& record = *item->getRecord();
  record.dump(oss);
  const std::string& srecord = oss.str();
  QMessageBox::information(this, "SimpleHashRecord Dump",
			   QString(srecord.c_str()), QMessageBox::Ok);
}

void SearchMoveTree::showMoves()
{
  QList<QTreeWidgetItem *> selected = selectedItems();
  if (selected.empty())
    return;

  SearchMoveTreeItem *item = (SearchMoveTreeItem *)selected[0];

  osl::MoveVector moves;
  item->getMovesToCurrent(moves);
  QString moves_text;
  for (size_t i = 0; i < moves.size(); ++i)
  {
    moves_text.append(osl::csa::show(moves[i]).c_str()).append("\n");
  }
  
  QMessageBox::information(this, "Moves",
			   moves_text, QMessageBox::Ok);
}

void SearchMoveTree::showMovesInKanji()
{
  QList<QTreeWidgetItem *> selected = selectedItems();
  if (selected.empty())
    return;

  SearchMoveTreeItem *item = (SearchMoveTreeItem *)selected[0];

  osl::MoveVector moves;
  item->getMovesToCurrent(moves);
  osl::NumEffectState state(initial_state);
  QString moves_text;
  QTextCodec *codec = QTextCodec::codecForName("EUC-JP");
  for (size_t i = 0; i < moves.size(); ++i)
  {
    std::string euc_move = i 
      ? osl::ki2::show(moves[i], state, moves[i-1])
      : osl::ki2::show(moves[i], state);
    moves_text.append(codec->toUnicode(euc_move.c_str(), euc_move.length()))
      .append("\n");
    state.makeMove(moves[i]);
  }
  
  QMessageBox::information(this, "Moves",
			   moves_text, QMessageBox::Ok);
}

void SearchMoveTree::showSubtree()
{
  QList<QTreeWidgetItem *> selected = selectedItems();
  if (selected.empty())
    return;

  SearchMoveTreeItem *item = (SearchMoveTreeItem *)selected[0];

  std::vector<osl::Move> moves;
  for (; item; item = (SearchMoveTreeItem *)item->parent())
  {
    moves.push_back(item->getMove());
  }

  osl::game_playing::GameState gameState(initial_state);
  for (int i = (int)moves.size() - 1; i >= 0; i--)
  {
    if (moves[i].isInvalid())
      break;
    gameState.pushMove(moves[i]);
  }

  const osl::HashKey key(gameState.state());
  const osl::search::SimpleHashRecord *record = table->find(key);

  if (!record)
    return;

  QDialog *dialog = new QDialog(this);
  SearchMoveTree *tree = new SearchMoveTree(dialog);
  tree->addRecord(gameState.state(), table, record, player, gameState.moveHistory());
  QVBoxLayout *layout = new QVBoxLayout(dialog);
  layout->addWidget(tree);
  QPushButton *button = new QPushButton(dialog);
  button->setText("&OK");
  layout->addWidget(button);
  connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
  dialog->exec();
}

void SearchMoveTree::buildContextMenu(QMenu *menu)
{
  MoveTree::buildContextMenu(menu);
  menu->addSeparator();
  QAction *showQRecordAction = new QAction(tr("Show Quiescence Record"), this);
  menu->addAction(showQRecordAction);
  connect(showQRecordAction, SIGNAL(triggered()),
	  this, SLOT(showQuiescenceRecord()));

  QAction *showTreeAction = new QAction(tr("Show This Subtree"), this);
  menu->addAction(showTreeAction);
  connect(showTreeAction, SIGNAL(triggered()),
	  this, SLOT(showSubtree()));

  menu->addSeparator();
  QAction *showRecordAction = new QAction(tr("Dump Record Data"), this);
  menu->addAction(showRecordAction);
  connect(showRecordAction, SIGNAL(triggered()),
	  this, SLOT(showRecord()));

  QAction *showMovesAction = new QAction(tr("Show moves to this node"), this);
  menu->addAction(showMovesAction);
  connect(showMovesAction, SIGNAL(triggered()),
	  this, SLOT(showMoves()));

  QAction *showMovesInKanjiAction =
    new QAction(tr("Show moves to this node in Kanji"), this);
  menu->addAction(showMovesInKanjiAction);
  connect(showMovesInKanjiAction, SIGNAL(triggered()),
	  this, SLOT(showMovesInKanji()));
}



class SearchThread : public QThread
{
public:
  SearchThread(osl::game_playing::SearchPlayer& player,
	       const osl::game_playing::GameState& gameState, const int time)
    : player(player), gameState(gameState), time(time) {
  }
  virtual void run() {
    player.selectBestMove(gameState, 0, 0, time);
  }
private:
  osl::game_playing::SearchPlayer& player;
  const osl::game_playing::GameState& gameState;
  const int time;
};

AnalysisViewer::AnalysisViewer(QWidget *parent)
  : BoardAndListTabChild(parent),
    record(0),
    algorithm(TEST_ALPHA_BETA),
    algorithmIndex(0),
    depthLimit(SearchDialogImpl::DEFAULT_NODE_DEPTH_LIMIT),
    initialDepthLimit(SearchDialogImpl::DEFAULT_NODE_INITIAL_DEPTH_LIMIT),
    deepningStep(SearchDialogImpl::DEFAULT_NODE_DEEPNING_STEP),
    nodeLimit(SearchDialogImpl::DEFAULT_NODE_LIMIT),
    tableSizeLimit(SearchDialogImpl::DEFAULT_TABLE_SIZE_LIMIT),
    tableRecordLimit(SearchDialogImpl::DEFAULT_TABLE_RECORD_LIMIT),
    multiPVWidth(SearchDialogImpl::DEFAULT_MULTI_PV_WIDTH),
    searchTime(SearchDialogImpl::DEFAULT_TIME)
{
  moveTree = new SearchMoveTree(this);

  pvView = new QTableView(this);
  pvView->verticalHeader()->hide();
  pvView->setShowGrid(false);
  pvView->setSortingEnabled(true);
  sortedPvModel.reset(new PvProxyModel);
  
  QVBoxLayout *moveLayout = new QVBoxLayout;
  moveLayout->addWidget(moveTree, 2);
  moveLayout->addWidget(pvView, 1);

  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(board);
  mainLayout->addLayout(moveLayout);

  connect(pvView, SIGNAL(doubleClicked(const QModelIndex &)),
          this, SLOT(openMoves(const QModelIndex &)));
  connect(moveTree, SIGNAL(itemExpanded(QTreeWidgetItem*)),
          this, SLOT(expandMove(QTreeWidgetItem*)));

  init();
}

QSize AnalysisViewer::sizeHint() const
{
  return moveTree->sizeHint();
}

bool AnalysisViewer::analyze(const osl::SimpleState& s,
			     const std::vector<osl::Move> &moves)
{
  QStringList strList;
  strList
    << TEST_ALPHA_BETA
    << USI_PROXY
    << PROGRESS_ALPHA_BETA
    ;
  std::unique_ptr<SearchDialogImpl>
    dialog(new SearchDialogImpl(&strList,
				algorithmIndex,
				depthLimit,
				initialDepthLimit,
				deepningStep,
				nodeLimit,
				tableSizeLimit,
				tableRecordLimit,
				multiPVWidth,
				searchTime));
  int result = dialog->exec();
  if (result != QDialog::Accepted)
  {
    return false;
  }
  saveSearchValue(*dialog);
  return analyzeWithSavedValue(s, moves);
}

bool AnalysisViewer::analyzeWithSavedValue(const osl::SimpleState& s,
					   const std::vector<osl::Move> &moves)
{
  moveTree->clear();
  initialState.copyFrom(s);
  osl::game_playing::GameState gameState(s);  
  for (int i = 0; i < (int)moves.size(); i++)
  {
    if (moves[i].isInvalid())
      break;
    gameState.pushMove(moves[i]);
    initialState.makeMove(moves[i]);
  }
  board->setState(initialState);

  if (algorithm == PROGRESS_ALPHA_BETA)
  {
    player.reset(new osl::game_playing::AlphaBeta2ProgressEvalPlayer());
  }
  else if (algorithm == TEST_ALPHA_BETA)
  {
    player.reset(new osl::game_playing::AlphaBeta2OpenMidEndingEvalPlayer());
    dynamic_cast<osl::game_playing::AlphaBeta2OpenMidEndingEvalPlayer&>(*player).enableMultiPV(multiPVWidth);
  }
  else if (algorithm == USI_PROXY)
  {
    player.reset(new osl::game_playing::UsiProxyPlayer());
    dynamic_cast<osl::game_playing::UsiProxyPlayer&>(*player).enableMultiPV(multiPVWidth);
  }
 else
  {
    abort(); // shouldn't happen;
  }
  player->setVerbose(true);
  player->setDepthLimit(depthLimit,
			initialDepthLimit,
			deepningStep);
  if (osl::OslConfig::isMemoryLimitEffective()) 
  {
    player->setTableLimit(std::numeric_limits<size_t>::max(), 
			  tableRecordLimit);
    player->setNodeLimit(std::numeric_limits<size_t>::max());
  }
  else 
  {
    player->setNodeLimit(nodeLimit);
    player->setTableLimit(tableSizeLimit,
			  tableRecordLimit);
  }
  player->setNextIterationCoefficient(1);
  const int time = searchTime;
#ifdef OSL_NOTHREADS
  player->selectBestMove(gameState, 0, 0, time);
#else
  QProgressDialog progress(tr("Searching..........................................."),
                           tr("Abort Search"), 0, time*8,
                           this);
  progress.setLabelText(tr("Searching"));
  std::shared_ptr<ViewerSearchMonitor> monitor(new ViewerSearchMonitor(gameState.state()));
  player->addMonitor(monitor);

  connect(monitor.get(), SIGNAL(updated(const QString&)),
          &progress, SLOT(setLabelText(const QString&)));

  SearchThread thread(*player, gameState, time);
  thread.start();
  for (int i = 0; i < time*8; i++)
  {
    timespec rq = { 0, 125000000 }, rm;
    while (nanosleep(&rq, &rm) == -1) 
      rq = rm;
    progress.setValue(i);
    qApp->processEvents();

    if (progress.wasCanceled())
    {
      player->stopSearchNow();
      thread.wait();
      progress.setValue(time);
      break;
    }
    if (thread.isFinished())
      break;
  }

  thread.wait();
  progress.setValue(time*4);
#endif
  const osl::search::SimpleHashTable& table = *player->table();
  const osl::HashKey key(gameState.state());
  record = table.find(key);

  if (record)
  {
    ((SearchMoveTree *)moveTree)->addRecord(initialState, &table, record,
					    player.get(),
					    gameState.moveHistory());
  }
  bestMoves(gameState.state(), table);
  board->setTable(&table);
  return true;
}

void AnalysisViewer::saveSearchValue(const SearchDialogImpl& dialog)
{
  algorithm = dialog.searchPlayerComboBox->currentText();
  algorithmIndex = dialog.searchPlayerComboBox->currentIndex();
  depthLimit = dialog.depthLimitBox->value();
  initialDepthLimit = dialog.initialDepthLimitBox->value();
  deepningStep = dialog.deepningStepBox->value();
  nodeLimit = dialog.nodeLimitBox->value();
  tableSizeLimit = dialog.tableSizeLimitBox->value();
  tableRecordLimit = dialog.tableRecordLimitBox->value();
  multiPVWidth = dialog.multiPVWidthBox->value();
  searchTime = dialog.searchTimeBox->value();
}

void AnalysisViewer::bestMoves(const osl::NumEffectState& state, 
			       const osl::search::SimpleHashTable& table)
{
  pvModel.reset(new PvModel(state, table, this));
  sortedPvModel->setSourceModel(pvModel.get());
  pvView->setModel(sortedPvModel.get());
  pvView->resizeColumnsToContents();
  pvView->sortByColumn(0, Qt::AscendingOrder);
}

int AnalysisViewer::getRecordValue(const osl::search::SimpleHashRecord *r,
				   osl::Player turn)
{
  if (r && r->hasUpperBound(0))
      return r->upperBound();
  int value = osl::search::FixedEval::minusInfty(turn)*2;
  if (r && r->hasLowerBound(0))
      value += r->lowerBound();
  return value;
}

const osl::search::SimpleHashRecord *AnalysisViewer::selectedRecord()
{
  QList<QTreeWidgetItem *> selected = moveTree->selectedItems();
  if (selected.empty())
    return 0;

  SearchMoveTreeItem *item = (SearchMoveTreeItem *)selected[0];
  if (item)
  {
    return item->getRecord();
  }

  return 0;
}

void AnalysisViewer::openMoves(const QModelIndex & index)
{
  QVariant v = sortedPvModel->data(index, PvModel::FirstMoveRole);
  osl::Move move = osl::Move::makeDirect(v.toInt());

  for (int i=0; i < moveTree->topLevelItemCount(); ++i) {
    SearchMoveTreeItem *child = (SearchMoveTreeItem *)moveTree->topLevelItem(i);
    if (child->getMove() == move)
    {
      child->setSelected(true);
      moveTree->expandBest();
      moveTree->scrollToItem(child);
      return;
    }
  }
}

QWidget *AnalysisViewer::moveGenerateDialog()
{
  MoveGeneratorDialog *dialog
    = (MoveGeneratorDialog *)BoardAndListTabChild::moveGenerateDialog();

  if (dialog)
    connect(this, SIGNAL(statusChanged(const osl::SimpleState &,
				       const std::vector<osl::Move> &,
				       int, osl::Move)),
	    dialog, SLOT(setStatus(const osl::SimpleState &,
				   const std::vector<osl::Move> &,
				   int, osl::Move)));
  return dialog;
}

int AnalysisViewer::getLimit()
{
  const osl::search::SimpleHashRecord *record = selectedRecord();
  if (record)
  {
    return ((record->lowerLimit() > record->upperLimit()) 
	    ? record->lowerLimit() : record->upperLimit());
  }
  else
    return 1000;
}

void AnalysisViewer::expandMove(QTreeWidgetItem *item)
{
  moveTree->setSortingEnabled(false);

  for (int i=0; i<item->childCount(); ++i)
  {
    SearchMoveTreeItem *c = (SearchMoveTreeItem *)item->child(i);
    c->setNextMove();
  }

  moveTree->setSortingEnabled(true);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
