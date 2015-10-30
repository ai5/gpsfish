#include "checkmateViewer.h"
#include "gpsshogi/gui/util.h"
#include "gpsshogi/gui/board.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/checkmate/dfpnRecord.h"
#include "osl/checkmate/proofTreeDepthDfpn.h"
#include "osl/csa.h"
#include "osl/record/ki2.h"

#if QT_VERSION >= 0x050000
#  include <QtWidgets>
#else
#  include <QtGui>
#endif
#include <qtextcodec.h>
#include <qmessagebox.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <sstream>

static const bool show_side_nodes = false;

using gpsshogi::gui::Util;

class CheckMoveItem : public MoveTreeItem
{
public:
  CheckMoveItem(CheckMoveViewer *parent, osl::Move move, int moves, 
		const osl::checkmate::DfpnRecord& pr,
		const osl::checkmate::DfpnRecord& mr, bool attack) :
    MoveTreeItem(parent, move, move.isPass() ? "PASS" : Util::moveToString(move)),
    numMoves(moves), parent_record(pr), record(mr), is_attack(attack), tree(parent), parent_item(0) {
    setRecord();
    setCellColor();
  }
  CheckMoveItem(CheckMoveItem *parent, osl::Move move, int moves,
		const osl::checkmate::DfpnRecord& pr, 
		const osl::checkmate::DfpnRecord& mr, bool attack) :
    MoveTreeItem(parent, move, move.isPass() ? "PASS" : Util::moveToString(move)),
    numMoves(moves), parent_record(pr), record(mr), is_attack(attack), tree(parent->tree), parent_item(parent) {
    setRecord();
    setCellColor();
  }
  bool isBestMove() const {
    return numMoves >= 0;
  }
  int getNumMoves() const {
    return numMoves;
  }
  bool operator<(const QTreeWidgetItem& other) const
  {
    const CheckMoveItem& item = (const CheckMoveItem&)other;
    return numMoves > item.getNumMoves();
  }
  const osl::checkmate::DfpnRecord getRecord() const { return record; }
  bool isAttack() const { return is_attack; }

  static const QString proofString(const osl::checkmate::DfpnRecord record)
  {
    const int proof = record.proof_disproof.proof();
    if (proof > osl::ProofDisproof::PROOF_LIMIT)
      return QString("INF");
    return QString::number(proof);
  }
  static const QString disproofString(const osl::checkmate::DfpnRecord record)
  {
    const int disproof = record.proof_disproof.disproof();
    if (disproof > osl::ProofDisproof::DISPROOF_LIMIT)
      return QString("INF");
    return QString::number(disproof);
  }
private:
  void setRecord()
  {
    setExpanded(true);
    if (show_side_nodes) {
      setText(1, proofString(record));
      setText(2, disproofString(record));
    }
    for (int i=1; i<=2; ++i) {
      setTextAlignment(i, Qt::AlignRight);
    }
  }
public:
  void getMovesToCurrent(osl::MoveVector& moves) 
  {
    CheckMoveItem *i = this;
    while (i) {
      moves.push_back(i->getMove());
      i = i->parent_item;
    }
    std::reverse(moves.begin(), moves.end());
  }
  void setNextMove();
protected:
  void setCellColor();
private:
  int numMoves;
  const osl::checkmate::DfpnRecord parent_record, record;
  bool is_attack;
  CheckMoveViewer *tree;
  CheckMoveItem *parent_item;
};

namespace
{
  template <class Parent>
  void addItemAttack(Parent *parent,
		     const osl::checkmate::DfpnRecord& record,
		     const osl::NumEffectState& state,
		     const osl::checkmate::DfpnTable& table)
  {
    assert(parent);
    if (! record.proof_disproof.isFinal())
      return;
    const osl::Move best_move = record.best_move;
    if (best_move.isNormal())
    {
      osl::checkmate::ProofTreeDepthDfpn depth_analyzer(table);
      osl::HashKey key(state);
      int solution_depth = -1;
      if (record.proof_disproof.isCheckmateSuccess())
	solution_depth = depth_analyzer.depth(key, state, true);
      osl::checkmate::DfpnRecord child
	= table.probe(key.newHashWithMove(best_move),
		      osl::PieceStand(osl::WHITE,state).nextStand(osl::WHITE,best_move));
      CheckMoveItem *item = new CheckMoveItem(parent, best_move, solution_depth, record, child, true);
      item->setNextMove();
    }
#if 0
    if (! show_side_nodes)
      return;
    for (osl::checkmate::CheckMoveList::const_iterator i = record->moves.begin();
	 i != record->moves.end(); i++)
    {
      if (best_move && i->move == best_move->move)
	continue;
      osl::checkmate::ProofTreeDepthDfpn depth_analyzer(table);
      int solution_depth = -1;
      if (i->record && i->record->proofDisproof().isCheckmateSuccess())
	solution_depth = depth_analyzer.depth(i->record, false) + 1;
      new CheckMoveItem(parent, i->move, solution_depth, record, true);
    }
#endif
  }

  template <class Parent>
  void addItemDefense(Parent *parent,
		      const osl::checkmate::DfpnRecord& record,
		      const osl::NumEffectState& state,
		      const osl::checkmate::DfpnTable& table)
  {
    assert(parent);
    if (! record.proof_disproof.isFinal())
      return;
    osl::MoveVector moves;
    state.generateLegal(moves);
    const osl::HashKey key(state);
    for (osl::MoveVector::const_iterator p=moves.begin();
	 p!=moves.end(); ++p) {
      int solution_depth = -1;
      osl::checkmate::DfpnRecord child
	= table.probe(osl::HashKey(state).newHashWithMove(*p),
		      osl::PieceStand(osl::WHITE,state).nextStand(osl::WHITE,*p));
      if (child.proof_disproof.isCheckmateSuccess())
      {
	osl::checkmate::ProofTreeDepthDfpn depth_analyzer(table);
	osl::NumEffectState new_state(state);
	new_state.makeMove(*p);
	solution_depth = depth_analyzer.depth(osl::HashKey(new_state), new_state, true)+1;
      }
      CheckMoveItem *item = new CheckMoveItem(parent, *p, solution_depth, record, child, false);
      item->setNextMove();
    }
  }
}

void CheckMoveItem::setNextMove()
{
  if (!childCount())
  {
    treeWidget()->setUpdatesEnabled(false);

    osl::MoveVector history;
    getMovesToCurrent(history);
    osl::NumEffectState state(tree->initialState());
    for (size_t i=0; i<history.size(); ++i)
      state.makeMove(history[i]);

    if (getDepth() >= 256)
    {
      setExpanded(false);
      return;
    }

    if (is_attack)
      addItemDefense(this, record, state, tree->getTable());
    else
      addItemAttack(this, record, state, tree->getTable());

    treeWidget()->setUpdatesEnabled(true);
  }
}

void CheckMoveItem::setCellColor()
{
  static int column = 0;
  if (numMoves < 0)
  {
    QBrush brush(foreground(column));
    brush.setColor(QColor("gray"));
    setForeground(column, brush);
  }
}

CheckMoveViewer::CheckMoveViewer(QWidget *parent)
  : MoveTree(parent)
{
  setRootIsDecorated(true);

  QStringList headers;
  headers << "Move";
  if (show_side_nodes) {
    headers << "proof" << "disproof";
  }
  setColumnCount(headers.size());
  setHeaderLabels(headers);
}

CheckMoveViewer::~CheckMoveViewer()
{
}

bool CheckMoveViewer::analyze(const osl::NumEffectState &sstate,
			      int limit, bool change_turn)
{
  int table_size = 20000000;
  check_state result;
  bool search_again = true;

  while (search_again)
  {
    result = analyze(sstate, table_size, limit, change_turn);
    search_again = false;

    if (result == UNKNOWN)
    {
      QString nodeCount;
      nodeCount.sprintf("%s %ld\n", "ノード数",
			(long int)searcher->totalNodeCount());
      limit *= 2;
      if (table_size < limit)
	table_size *= 2;
      int ret = QMessageBox::question(this, "Checkmate Search",
				      nodeCount +
				      QString("Unknown, Search again with limit %1 size %2 ?").arg(limit).arg(table_size),
				      QMessageBox::Yes,
				      QMessageBox::No | QMessageBox::Default);
      if (ret == QMessageBox::Yes)
      {
	search_again = true;
      }
    }
    else
    {
      QString message;
      if (result == CHECKMATE)
	message = "Checkmate";
      else if (result == NO_CHECKMATE)
	message = "No Checkmate";
      else if (result == NO_CHECKMATE_LOOP)
	message = "No Checkmate (loop)";

      QString nodeCount;
      nodeCount.sprintf("\n%s %ld", "ノード数",
			(long int)searcher->totalNodeCount());
      message.append(nodeCount);
      QMessageBox::information(this, "Checkmate State", message);
    }
  }
  return result == CHECKMATE;
}

CheckMoveViewer::check_state CheckMoveViewer::analyze(const osl::NumEffectState &sstate,
						      int table_size,
						      int limit, bool change_turn)
{
  clear();
  searcher.reset(new searcher_t(table_size));

  osl::Move checkmateMove;
  osl::NumEffectState state(sstate);
  const bool king_in_check = state.inCheck();
  if (change_turn && (!king_in_check))
    state.changeTurn();
  const osl::PathEncoding path(state.turn());
  const bool is_defense = king_in_check && change_turn;
  const bool win 
    = (is_defense
       ? searcher->isLosingState(limit, state, osl::HashKey(state), path)
       : searcher->isWinningState(limit, state, osl::HashKey(state), path, checkmateMove));

  this->table
    = &(searcher->table(is_defense ? alt(state.turn()) : state.turn()));
  initial_state = sstate;
  const osl::checkmate::DfpnRecord record = table->probe(osl::HashKey(state), osl::PieceStand(osl::WHITE,state));
  if (win || show_side_nodes)
  {
    if (!change_turn && !king_in_check)
      addItemAttack(this, record, state, *table);
    else if (king_in_check)
    {
      addItemDefense(this, record, state, *table);
    }
    else
    {
      assert(change_turn && !king_in_check);
      CheckMoveItem *item = new CheckMoveItem(this,
					      osl::Move::PASS(osl::alt(path.turn())), 
					      0, osl::checkmate::DfpnRecord(), record, false);
      addItemAttack(item, record, state, *table);
      item->setNextMove();
    }
  }

  check_state result = UNKNOWN;
  if (record.proof_disproof.isCheckmateSuccess())
  {
    result = CHECKMATE;
  }
  else if (record.proof_disproof.isCheckmateFail())
  {
    result = NO_CHECKMATE;
  }
  
  return result;
}

void CheckMoveViewer::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu(this);
  QAction *showRecordAction = new QAction(tr("Dump Record Data"), this);
  menu.addAction(showRecordAction);
  connect(showRecordAction, SIGNAL(triggered()),
	  this, SLOT(showRecord()));

  QAction *showMovesAction = new QAction(tr("Show moves to this node"), this);
  menu.addAction(showMovesAction);
  connect(showMovesAction, SIGNAL(triggered()),
	  this, SLOT(showMoves()));

  QAction *showMovesInKanjiAction =
    new QAction(tr("Show moves to this node in Kanji"), this);
  menu.addAction(showMovesInKanjiAction);
  connect(showMovesInKanjiAction, SIGNAL(triggered()),
	  this, SLOT(showMovesInKanji()));
  menu.exec(event->globalPos());
}

void CheckMoveViewer::showRecord(const osl::checkmate::DfpnRecord& /*record*/)
{
#if 0 
  if (! record)
    return;
  // not yet implemented
  std::ostringstream oss(std::ostringstream::out);
  record->dump(oss, 1);
  const std::string &record_string = oss.str();
  QMessageBox::information(this, "QuiescenceRecord Dump",
			   QString(record_string.c_str()), QMessageBox::Ok);
#endif
}

void CheckMoveViewer::showRecord()
{
  QList<QTreeWidgetItem *> selected = selectedItems();
  if (selected.empty())
    return;

  CheckMoveItem *item = (CheckMoveItem *)selected[0];
  showRecord(item->getRecord());
}

void CheckMoveViewer::showMoves()
{
  QList<QTreeWidgetItem *> selected = selectedItems();
  if (selected.empty())
    return;

  CheckMoveItem *item = (CheckMoveItem *)selected[0];

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

void CheckMoveViewer::showMovesInKanji()
{
  QList<QTreeWidgetItem *> selected = selectedItems();
  if (selected.empty())
    return;

  CheckMoveItem *item = (CheckMoveItem *)selected[0];

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



CheckmateViewer::CheckmateViewer(QWidget *parent)
  : BoardAndListTabChild(parent)
{
  moveTree = new CheckMoveViewer(this);

  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(board);
  mainLayout->addWidget(moveTree);


  connect(moveTree, SIGNAL(itemExpanded(QTreeWidgetItem*)),
          this, SLOT(expandMove(QTreeWidgetItem*)));

  init();
}

bool CheckmateViewer::analyze(const osl::NumEffectState &state,
			      int node_limit, bool change_turn)
{
  initialState = state;
  board->setState(state);
  return ((CheckMoveViewer *)moveTree)->analyze(state, node_limit,
						change_turn);
}

void CheckmateViewer::expandMove(QTreeWidgetItem *item)
{
  moveTree->setSortingEnabled(false);

  for (int i=0; i<item->childCount(); ++i)
  {
    CheckMoveItem *c = (CheckMoveItem *)item->child(i);
    c->setNextMove();
  }

  moveTree->setSortingEnabled(true);
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
