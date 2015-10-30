#include "quiescenceTree.h"

#include "osl/search/simpleHashRecord.h"
#include "osl/search/simpleHashTable.h"

#include "gpsshogi/gui/util.h"

#if QT_VERSION >= 0x050000
#  include <QtWidgets>
#else
#  include <QtGui>
#endif
#include <qmessagebox.h>
#include <sstream>

static bool hasChildAfterMove(const osl::SimpleState& state,
			      const osl::search::SimpleHashTable& table,
			      const osl::HashKey& child_key,
			      const osl::Move move)
{
  osl::NumEffectState child_state(state);
  child_state.makeMove(move);
  osl::MoveVector moves;
  child_state.generateLegal(moves);
  for (size_t i=0; i<moves.size(); ++i)
    if (table.find(child_key.newHashWithMove(moves[i])))
      return true;
  return false;
}

class QuiescenceItem : public MoveTreeItem
{
public:
  QuiescenceItem(QuiescenceTree *parent, osl::Move m,
                 bool _bestMove,
                 const osl::search::SimpleHashRecord *_record,
                 const osl::hash::HashKey& _key,
                 const osl::search::SimpleHashTable *_table)
    : MoveTreeItem(parent, m),
      bestMove(_bestMove), record(_record), key(_key), table(_table),
      parent_item(0), tree(parent)
  {
    setUpText(m, record);
    setCellColor();
  }
  QuiescenceItem(QuiescenceItem *parent, osl::Move m,
		 bool _bestMove,
		 const osl::search::SimpleHashRecord *_record,
		 const osl::hash::HashKey& _key,
		 const osl::search::SimpleHashTable *_table)
    : MoveTreeItem(parent, m),
      bestMove(_bestMove), record(_record), key(_key), table(_table),
      parent_item(parent), tree(parent->tree)
  {
    setUpText(m, record);
    setCellColor();
  }
  void setCellColor();
  void setNextMove();
  bool isBestMove() const {
    return bestMove;
  }
  const osl::search::SimpleHashRecord *getRecord() const {
    return record;
  }

  void getMovesToCurrent(osl::MoveVector& moves) 
  {
    QuiescenceItem *i = this;
    while (i) {
      moves.push_back(i->getMove());
      i = i->parent_item;
    }
    std::reverse(moves.begin(), moves.end());
  }
private:
  void setUpText(osl::Move, const osl::search::SimpleHashRecord *record);
  const char *staticValueType();
  QString staticValue();

  bool bestMove;
  const osl::search::SimpleHashRecord *record;
  const osl::hash::HashKey key;
  const osl::search::SimpleHashTable *table;

  QuiescenceItem *parent_item;
  QuiescenceTree *tree;
};

void QuiescenceItem::setCellColor()
{
  static int column = 0;

  QBrush brush(foreground(column));
  if (bestMove)
  {
    brush.setColor(QColor("blue"));
  }
  setForeground(column, brush);
}

void QuiescenceItem::setUpText(osl::Move move,
			       const osl::search::SimpleHashRecord *record)
{
  int column = 0;
  setText(column++, (move.isInvalid() ? "ROOT"
		     : gpsshogi::gui::Util::moveToString(move)));
  if (!record)
    return;
  setText(column++, record->qrecord.lowerDepth() >= 0 ?
	  QString("%1").arg(record->qrecord.lowerBound()) : "*");
  setText(column++, record->qrecord.upperDepth() >= 0 ?
	  QString("%1").arg(record->qrecord.upperBound()) : "*");
  setText(column++, QString("%1").arg(record->qrecord.lowerDepth()));
  setText(column++, QString("%1").arg(record->qrecord.upperDepth()));
  setText(column++, staticValueType());
  setText(column++, staticValue());

  for (int i=1; i<columnCount(); ++i) {
    setTextAlignment(i, Qt::AlignRight);
  }
}

const char *QuiescenceItem::staticValueType()
{
  if (! record)
    return "*";
  return osl::search::QuiescenceRecord::
    toString(record->qrecord.staticValueType());
}

QString QuiescenceItem::staticValue()
{
  if (! record)
    return "*";
  return (record->qrecord.hasStaticValue()
	  ? QString("%1").arg(record->qrecord.staticValue()) 
	  : "*");
}

void QuiescenceItem::setNextMove()
{
  if (!childCount() && record)
  {
    treeWidget()->setUpdatesEnabled(false);

    const osl::search::QuiescenceRecord& qrecord = record->qrecord;
    osl::MoveVector history, moves;
    getMovesToCurrent(history);
    osl::NumEffectState state(tree->initialState());
    for (size_t i=0; i<history.size(); ++i)
      state.makeMove(history[i]);
    state.generateLegal(moves);
  
    const osl::HashKey key(state);

    for (size_t i = 0; i < moves.size(); i++)
    {
      const osl::Move move = moves[i];
      const osl::hash::HashKey newKey = key.newHashWithMove(move);
      const osl::search::SimpleHashRecord *newRecord = table->find(newKey);

      if (! newRecord) 
	continue;
      
      new QuiescenceItem(this,
                         move,
                         qrecord.bestMove() == move,
                         newRecord,
                         newKey,
                         table);
    }
    treeWidget()->setUpdatesEnabled(true);
  }
}



QuiescenceTree::QuiescenceTree(QWidget *parent)
  : MoveTree(parent)
{
  setColumnCount(7);
  setHeaderLabels(QStringList() << "Move" <<
                                   "Lower Bound" <<
                                   "Upper Bound" <<
                                   "Lower Limit" <<
                                   "Upper Limit" <<
                                   "Static Value Type" <<
                                   "Static Value");
  for (int i = 1; i < columnCount(); i++)
  {
    setColumnWidth(i, columnWidth(i) / 2);
  }

  connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)),
          this, SLOT(expandMove(QTreeWidgetItem*)));
}

void QuiescenceTree::showRecord(const osl::hash::HashKey& key, 
				const osl::SimpleState& state,
				const osl::search::SimpleHashTable *table)
{
  clear();
  initial_state = state;
  const osl::search::SimpleHashRecord *record = table->find(key);

  addItem(0, 0, key, initial_state, table, record);
  // Root
  if (record)
  {
    new QuiescenceItem(this,
		       osl::Move::INVALID(),
		       false,
		       record,
		       key,
		       table); // dummy key
  }
}

void QuiescenceTree::addItem(int depth, QuiescenceItem *parent,
			     const osl::hash::HashKey& key,
			     const osl::SimpleState& state,
			     const osl::search::SimpleHashTable *table,
			     const osl::search::SimpleHashRecord *record)
{
  if (depth > osl::search::QSearchTraits::MaxDepth)
    return;
  if (!record)
    return;

  const osl::search::QuiescenceRecord& qrecord = record->qrecord;
  osl::MoveVector moves;
  osl::NumEffectState initial(initial_state);
  initial.generateLegal(moves);

  for (size_t i = 0; i < moves.size(); i++)
  {
    QuiescenceItem *item;
    const osl::Move move = moves[i];
    const osl::hash::HashKey newKey = key.newHashWithMove(move);
    const osl::search::SimpleHashRecord *newRecord = table->find(newKey);
    if (! newRecord)
      continue;
    if (parent)
      item = new QuiescenceItem(parent, move,
				qrecord.bestMove() == move,
				newRecord,
				newKey,
				table);
    else
      item = new QuiescenceItem(this, move,
				qrecord.bestMove() == move,
				newRecord,
				newKey,
				table);
    item->setNextMove();
    item->setExpanded(hasChildAfterMove(state, *table, newKey, move));
  }
}

void QuiescenceTree::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu(this);
  QAction *showRecordAction = new QAction(tr("Dump Record Data"), this);
  menu.addAction(showRecordAction);
  connect(showRecordAction, SIGNAL(triggered()),
	  this, SLOT(showRecord()));
  menu.exec(event->globalPos());
}

void QuiescenceTree::showRecord()
{
  QList<QTreeWidgetItem *> selected = selectedItems();
  if (selected.empty())
    return;
  QuiescenceItem *item = (QuiescenceItem *)selected[0];
  if (!item || !item->getRecord())
    return;

  std::ostringstream oss(std::ostringstream::out);
  const osl::search::QuiescenceRecord& record = item->getRecord()->qrecord;
  record.dump(oss);
  const std::string &record_string = oss.str();
  QMessageBox::information(this, "QuiescenceRecord Dump",
			   QString(record_string.c_str()), QMessageBox::Ok);
}

void QuiescenceTree::expandMove(QTreeWidgetItem *item)
{
  setSortingEnabled(false);

  for (int i=0; i<item->childCount(); ++i)
  {
    QuiescenceItem *c = (QuiescenceItem *)item->child(i);
    c->setNextMove();
  }

  setSortingEnabled(true);
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
