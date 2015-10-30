#include "moveGeneratorDialog.h"
#include "osl/search/analyzer/categoryMoveVector.h"
#include "osl/search/moveGenerator.h"
#include "osl/search/searchState2.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/eval/progressEval.h"
#include "osl/container/moveStack.h"
#include "osl/rating/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "gpsshogi/gui/util.h"

#include <QAbstractTableModel>
#include <QTableView>
#include <QVector>
#include <QSortFilterProxyModel>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>

struct MoveData
{
  MoveData() {}
  MoveData(const std::string &c,
	   osl::MoveLogProb m,
	   const std::string &a) : category(c.c_str()), move(m),
				   annotate(a.c_str()) {}
  QString category;
  osl::MoveLogProb move;
  QString annotate;
};

class MoveOrderModel : public QAbstractTableModel
{
public:
  MoveOrderModel(const osl::search::analyzer::CategoryMoveVector &moves,
		 const osl::NumEffectState &s,
		 const osl::Move m,
		 QObject *parent = 0);
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
private:
  QStringList headers;
  QVector<MoveData> move_data;
  osl::NumEffectState state;
  osl::Move correct_move;
};

MoveOrderModel::MoveOrderModel(
  const osl::search::analyzer::CategoryMoveVector &moves,
  const osl::NumEffectState &s,
  const osl::Move m,
  QObject *parent) : QAbstractTableModel(parent), state(s),
		     correct_move(m)
{
  int size = 0;
  for (osl::search::analyzer::CategoryMoveVector::const_iterator p =
	 moves.begin();
       p != moves.end(); p++)
  {
    size += p->moves.size();
  }
  move_data.reserve(size);
  for (osl::search::analyzer::CategoryMoveVector::const_iterator p =
	 moves.begin();
       p != moves.end(); p++)
  {
    for (size_t i = 0; i < p->moves.size(); ++i)
    {
      std::string annotate = "";
      {
	static osl::rating::StandardFeatureSet fs;
	osl::RatingEnv env;
	env.make(state);
	annotate =
	  fs.annotate(state, env, p->moves[i].move());
      }
      move_data.push_back(MoveData(p->category, p->moves[i], annotate));
    }
  }
  headers << "Category" << "Move" << "Prob.";
  headers << "Annotate";
}

int MoveOrderModel::rowCount(const QModelIndex &) const
{
  return move_data.size();
}

int MoveOrderModel::columnCount(const QModelIndex &) const
{
  return headers.size();
}

QVariant MoveOrderModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || index.row() > rowCount() ||
      index.column() >  columnCount())
    return QVariant();

  if (role == Qt::DisplayRole)
  {
    switch (index.column())
    {
    case 0:
      return move_data[index.row()].category;
    case 1:
      return gpsshogi::gui::Util::moveToString(
	move_data[index.row()].move.move());
    case 2:
      return QString("%1").arg(move_data[index.row()].move.logProb());
    case 3:
    {
      return move_data[index.row()].annotate;
    }
    default:
      return QVariant();
    }
  }
  else if (role == Qt::ForegroundRole && index.column() == 1 &&
	   move_data[index.row()].move.move() == correct_move)
  {
    return QBrush(QColor("blue"));
  }
  else if (role == Qt::UserRole)
  {
    switch (index.column())
    {
    case 0:
      return move_data[index.row()].category;
    case 1:
      return move_data[index.row()].move.move().intValue();
    case 2:
      return static_cast<uint>(move_data[index.row()].move.logProb());
    case 3:
      return move_data[index.row()].annotate;
    default:
      return QVariant();
    }
  }
  else
  {
    return QVariant();
  }
}

QVariant MoveOrderModel::headerData(int section, Qt::Orientation orientation,
				    int role) const
{
  if (role != Qt::DisplayRole || section > headers.size())
    return QVariant();

  if (orientation == Qt::Horizontal)
    return headers[section];
  else
    return QString("%1").arg(section);
}

class SortFilterProxyModel : public QSortFilterProxyModel
{
public:
  SortFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent) { }
  bool lessThan(const QModelIndex &left, const QModelIndex &right) const
  {
    QVariant left_data = sourceModel()->data(left, sortRole());
    QVariant right_data = sourceModel()->data(right, sortRole());
    if (left_data.type() == QVariant::Int)
    {
      osl::Move m1 = osl::Move::makeDirect(left_data.toInt());
      osl::Move m2 = osl::Move::makeDirect(right_data.toInt());
      return gpsshogi::gui::Util::compare(m1, m2) < 0;
    }
    return QSortFilterProxyModel::lessThan(left, right);
  }
};

MoveGeneratorDialog::MoveGeneratorDialog(const osl::SimpleState &state,
					 const std::vector<osl::Move> &moves,
					 int limit, osl::Move next,
					 QWidget *parent)
  : QDialog(parent), limit(limit), state(state), moves(moves),
    next_move(next)
{
  osl::search::analyzer::CategoryMoveVector category_moves;
  QAbstractItemModel *model =
    new MoveOrderModel(category_moves, osl::NumEffectState(state),
		       osl::Move());
  view = new QTableView(this);
  view->setSortingEnabled(true);
  view->setModel(model);

  QVBoxLayout *layout = new QVBoxLayout(this);
  spinBox = new QSpinBox(this);
  spinBox->setRange(0, 2000);
  spinBox->setSingleStep(100);
  layout->addWidget(spinBox);
  layout->addWidget(view);
  QPushButton *button = new QPushButton(this);
  button->setText("&OK");
  layout->addWidget(button);
  connect(spinBox, SIGNAL(valueChanged(int)), SLOT(setLimit(int)));
  connect(button, SIGNAL(clicked()), this, SLOT(accept()));

  updateMoves();

  resize(layout->sizeHint());
}

void MoveGeneratorDialog::updateMoves()
{
  spinBox->setValue(limit);
  osl::MoveStack history;
  osl::NumEffectState new_state(state);
  osl::eval::ProgressEval eval(new_state);
  osl::search::SimpleHashRecord record;

  for (size_t i = 0; i < moves.size(); i++)
  {
    new_state.makeMove(moves[i]);
    history.push(moves[i]);
  }
  osl::search::analyzer::CategoryMoveVector category_moves;
  {
    osl::search::MoveGenerator generator;
    generator.initOnce();
    
    record.setInCheck(new_state.inCheck());
    osl::eval::ProgressEval eval(new_state);
    generator.init(limit, &record, eval, new_state, true, osl::Move(), limit == 0);
    osl::search::SearchState2::checkmate_t checkmate;
    osl::search::SearchState2 sstate(new_state, checkmate);
    sstate.setHistory(history);
    generator.generateAll(new_state.turn(), sstate, category_moves);
  }
  
  QAbstractItemModel *model =
    new MoveOrderModel(category_moves, new_state, next_move);
  SortFilterProxyModel *proxy_model = new SortFilterProxyModel(this);
  proxy_model->setSourceModel(model);
  proxy_model->setSortRole(Qt::UserRole);
  view->setModel(proxy_model);
  view->resizeColumnsToContents();
}

void MoveGeneratorDialog::setLimit(int new_limit)
{
  limit = new_limit;
  updateMoves();
}

void MoveGeneratorDialog::setStatus(const osl::SimpleState &s,
				    const std::vector<osl::Move> &m,
				    int l, osl::Move next)
{
  state = s;
  moves = m;
  limit = l;
  next_move = next;
  updateMoves();
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
