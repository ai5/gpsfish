#include "evalDebug.h"

#include <cstdlib>
#include <QAbstractTableModel>
#include <QTableView>
#include <QTreeView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QTabBar>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include "osl/numEffectState.h"
#include "osl/eval/progressEval.h"
#include "osl/eval/endgame/attackDefense.h"
#include "osl/eval/endgame/kingPieceValues.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"

using namespace osl::eval::ml;

QStringList OpenMidEndingEvalUtil::featureName()
{
  QStringList list;
  list <<
    "PIECE" <<
    "BISHOP_EXCHANGE_SILVER_KING" <<
    "ENTER_KING_DEFENSE" <<
    "KING25_EFFECT_ATTACK" <<
    "PIECE_PAIR" <<
    "PIECE_PAIR_KING" <<
    "KING_PIECE_RELATIVE" <<
    "PIECE_STAND" <<
    "KING25_EFFECT_EACH" <<
    "PTYPEX" <<
    "PTYPEY" <<
    "ROOK_MOBILITY" <<
    "BISHOP_MOBILITY" <<
    "LANCE_MOBILITY" <<
    "ROOK_EFFECT" <<
    "BISHOP_EFFECT" <<
    "PIECE_STAND_COMBINATION" <<
    "PIECE_STAND_TURN" <<
    "ROOK_PAWN" <<
    "PAWN_DROP" <<
    "PIECE_STAND_Y" <<
    "KNIGHT_CHECK" <<
    "PAWN_ADVANCE" <<
    "PAWN_PTYPEO" <<
    "PROMOTED_MINOR_PIECE" <<
    "KING_PIECE_RELATIVE_NOSUPPORT" <<
    "NON_PAWN_ATTACKED" <<
    "NON_PAWN_ATTACKED_PTYPE" <<
    "PTYPE_YY" <<
    "KING3PIECES" <<
    "BISHOP_HEAD" <<
    "KNIGHT_HEAD" <<
    "ROOK_PROMOTE_DEFENSE" <<
    "PTYPE_COUNT" <<
    "LANCE_EFFECT_PIECE" <<
    "PTYPE_Y_PAWN_Y" <<
    "BISHOP_AND_KING" <<
    "PIECE_FORK_TURN" <<
    "ROOK_SILVER_KNIGHT" <<
    "BISHOP_SILVER_KNIGHT" <<
    "KING25_EFFECT_SUPPORTED" <<
    "KING_ROOK_BISHOP" <<
    "KING_X_BLOCKED3" <<
    "GOLD_RETREAT" <<
    "SILVER_RETREAT" <<
    "ALL_GOLD" <<
    "ALL_MAJOR" <<
    "KING25_EFFECT_DEFENSE" <<
    "ANAGUMA_EMPTY" <<
    "NO_PAWN_ON_STAND" <<
    "NON_PAWN_PIECE_STAND" <<
    "PIN_PTYPE_ALL" <<
    "KING_MOBILITY" <<
    "GOLD_AND_SILVER_NEAR_KING" <<
    "PTYPE_COMBINATION" <<
    "KING25_BOTH_SIDE" <<
    "KING25_MOBILITY" <<
    "BISHOP_STAND_FILE5" <<
    "MAJOR_CHECK_WITH_CAPTURE" <<
    "SILVER_ADVANCE26" <<
    "KING25_EFFECT3" <<
    "BISHOP_BISHOP_PIECE" <<
    "ROOK_ROOK" <<
    "ROOK_ROOK_PIECE" <<
    "KING25_EFFECT_COUNT_COMBINATION" <<
    "NON_PAWN_ATTACKED_PTYPE_PAIR" <<
    "ATTACK_MAJORS_IN_BASE";
  return list;
}

int OpenMidEndingEvalUtil::progressAdjustedValue(
  const osl::eval::ml::OpenMidEndingEvalDebugInfo &debug_info,
  int progress,
  int progress_stage,
  int index)
{
  const int progress_max = osl::progress::ml::NewProgress::maxProgress();
  const int c0 = progress_max/3, c1 = c0*2;

  if (index < osl::eval::ml::OpenMidEndingEvalDebugInfo::PROGRESS_INDEPENDENT_FEATURE_LIMIT)
  {
    if (progress_stage == 0)
      return debug_info.progress_independent_values[index] * c0;
    else
      return 0;
  }
  else
  {
    osl::MultiInt value = debug_info.stage_values[index - osl::eval::ml::OpenMidEndingEvalDebugInfo::PROGRESS_INDEPENDENT_FEATURE_LIMIT];
    if (progress < c0)
    {
      switch (progress_stage)
      {
      case 0:
        return value[0] * (c0 -progress);
      case 1:
        return value[1] * progress;
      default:
        return 0;
      }
    }
    else if (progress < c1)
    {
      switch (progress_stage)
      {
      case 1:
        return value[1] * (c1 - progress);
      case 2:
        return value[2] * (progress - c0);
      default:
        return 0;
      }
    }
    else
    {
      switch (progress_stage)
      {
      case 2:
        return value[2] * (progress_max - progress);
      case 3:
        return value[3] * (progress - c1);
      default:
        return 0;
      }
    }
  }
}

int OpenMidEndingEvalUtil::progressAdjustedTotal(
  const osl::eval::ml::OpenMidEndingEvalDebugInfo &debug_info,
  int progress,
  int index)
{
  int result = 0;
  for (int i = 0; i < 4; ++i)
  {
    result += progressAdjustedValue(debug_info, progress, i, index);
  }
  return result;
}

class OpenMidEndingEvalDebugModel : public QAbstractTableModel
{
public:
  OpenMidEndingEvalDebugModel(const osl::NumEffectState &s,
                              QObject *parent = 0);
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
  void setStatus(const osl::SimpleState &state);
  const osl::NumEffectState &getState() const { return state; }
private:
  QStringList headers;
  QStringList row_headers;
  int progress_value;
  osl::eval::ml::OpenMidEndingEvalDebugInfo debug_info;
  osl::NumEffectState state;
  double pawn_value;
};

OpenMidEndingEvalDebugModel::OpenMidEndingEvalDebugModel(
  const osl::NumEffectState &s,
  QObject *parent)
  : QAbstractTableModel(parent), state(s)
{
  osl::eval::ml::OpenMidEndingEval eval(state);
  debug_info = eval.debugInfo(s);
  osl::progress::ml::NewProgress new_progress(state);
  progress_value = new_progress.progress();
  headers = OpenMidEndingEvalUtil::featureName();

  row_headers << "opening" << "mid1" << "mid2" << "ending"
              << "opening (adjusted)" << "mid1 (adjusted)" << "mid2 (adjusted)"
              << "ending (adjusted)" << "Adjusted total";
  pawn_value = eval.captureValue(osl::newPtypeO(osl::WHITE, osl::PAWN)) / 2;
}

int OpenMidEndingEvalDebugModel::rowCount(const QModelIndex &) const
{
  return headers.size();
}

int OpenMidEndingEvalDebugModel::columnCount(const QModelIndex &) const
{
  return row_headers.size();
}

QVariant OpenMidEndingEvalDebugModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }
  const int column = index.column();
  const int row = index.row();
  if (row > rowCount() ||
      column >  columnCount())
  {
    return QVariant();
  }

  if (role == Qt::TextAlignmentRole)
  {
    return Qt::AlignRight;
  }
  else if (role == Qt::DisplayRole)
  {
    if (column == 8)
    {
      return OpenMidEndingEvalUtil::progressAdjustedTotal(debug_info, progress_value, row) / pawn_value * 100;
    }
    else if (column >= 4)
    {
      return OpenMidEndingEvalUtil::progressAdjustedValue(debug_info, progress_value, column - 4, row) / pawn_value * 100;
    }
    if (row < osl::eval::ml::OpenMidEndingEvalDebugInfo::PROGRESS_INDEPENDENT_FEATURE_LIMIT)
    {
      return debug_info.progress_independent_values[row] / 128.0 * 100;
    }
    else
    {
      return debug_info.stage_values[row - osl::eval::ml::OpenMidEndingEvalDebugInfo::PROGRESS_INDEPENDENT_FEATURE_LIMIT][column] / 128.0 * 100;
    }
  }
  else
  {
    return QVariant();
  }
}

QVariant OpenMidEndingEvalDebugModel::headerData(int section,
                                                 Qt::Orientation orientation,
                                                 int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant();
  }

  if (orientation == Qt::Vertical && section < headers.size())
  {
    return headers[section];
  }
  else if (orientation == Qt::Horizontal && section < row_headers.size())
  {
    return row_headers[section];
  }
  return QVariant();
}

void OpenMidEndingEvalDebugModel::setStatus(const osl::SimpleState &s_state)
{
  state = osl::NumEffectState(s_state);
  osl::eval::ml::OpenMidEndingEval eval(state);
  debug_info = eval.debugInfo(state);
  osl::progress::ml::NewProgress new_progress(state);
  progress_value = new_progress.progress();
  emit dataChanged(createIndex(0, 0), createIndex(0, headers.size()));
}

void OpenMidEndingEvalDebug::setStatus(const osl::SimpleState &state)
{
  model->setStatus(state);
}

OpenMidEndingEvalDebug::OpenMidEndingEvalDebug(const osl::NumEffectState &s,
			     QWidget *parent)
  : QWidget(parent)
{
  model = new OpenMidEndingEvalDebugModel(s, this);
  QTableView *view = new QTableView(this);
  view->setSortingEnabled(true);
  QSortFilterProxyModel *proxy_model = new QSortFilterProxyModel(this);
  proxy_model->setSourceModel(model);
  view->setModel(proxy_model);
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget(view);
  delegate.reset(new DoubleItemDelegate);
  view->setItemDelegate(delegate.get());
}

OpenMidEndingEvalDebugDialog::OpenMidEndingEvalDebugDialog(
  const osl::SimpleState &s,
  QWidget *parent)
  : QDialog(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  debug = new OpenMidEndingEvalDebug(osl::NumEffectState(s), this);
  layout->addWidget(debug);

  QPushButton *button = new QPushButton(this);
  button->setText("&OK");
  layout->addWidget(button);
  connect(button, SIGNAL(clicked()), this, SLOT(accept()));
}

void OpenMidEndingEvalDebugDialog::setStatus(
  const osl::SimpleState &state,
  const std::vector<osl::Move> &,
  int, osl::Move)
{
  debug->setStatus(state);
}


class OpenMidEndingEvalDiffModel : public QAbstractTableModel
{
public:
  OpenMidEndingEvalDiffModel(const osl::NumEffectState &s1,
                             const osl::NumEffectState &s2,
                             QObject *parent = 0);
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
private:
  QStringList headers;
  QStringList row_headers;
  int progress_value1, progress_value2;
  osl::eval::ml::OpenMidEndingEvalDebugInfo debug_info1, debug_info2;
  osl::NumEffectState state1, state2;
  double pawn_value;
};

OpenMidEndingEvalDiffModel::OpenMidEndingEvalDiffModel(
  const osl::NumEffectState &s1,
  const osl::NumEffectState &s2,
  QObject *parent)
  : QAbstractTableModel(parent), state1(s1), state2(s2)
{
  osl::eval::ml::OpenMidEndingEval eval1(state1);
  debug_info1 = eval1.debugInfo(s1);
  osl::progress::ml::NewProgress new_progress1(state1);
  progress_value1 = new_progress1.progress();

  osl::eval::ml::OpenMidEndingEval eval2(state2);
  debug_info2 = eval2.debugInfo(s2);
  osl::progress::ml::NewProgress new_progress2(state2);
  progress_value2 = new_progress1.progress();

  headers = OpenMidEndingEvalUtil::featureName();

  row_headers << "diff (second - first)" << "diff (abs)";

  pawn_value = eval1.captureValue(osl::newPtypeO(osl::WHITE, osl::PAWN)) / 2;
}

int OpenMidEndingEvalDiffModel::rowCount(const QModelIndex &) const
{
  return headers.size();
}

int OpenMidEndingEvalDiffModel::columnCount(const QModelIndex &) const
{
  return row_headers.size();
}

QVariant OpenMidEndingEvalDiffModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }
  const int column = index.column();
  const int row = index.row();
  if (row > rowCount() ||
      column >  columnCount())
  {
    return QVariant();
  }

  if (role == Qt::TextAlignmentRole)
  {
    return Qt::AlignRight;
  }
  else if (role == Qt::DisplayRole)
  {
    if (column == 0)
    {
      return (OpenMidEndingEvalUtil::progressAdjustedTotal(debug_info2, progress_value2, row) - OpenMidEndingEvalUtil::progressAdjustedTotal(debug_info1, progress_value1, row)) / pawn_value * 100;
    }
    else if (column == 1)
    {
      return abs(OpenMidEndingEvalUtil::progressAdjustedTotal(debug_info2, progress_value2, row) - OpenMidEndingEvalUtil::progressAdjustedTotal(debug_info1, progress_value1, row)) / pawn_value * 100;
    }
  }
  return QVariant();
}

QVariant OpenMidEndingEvalDiffModel::headerData(int section,
                                                 Qt::Orientation orientation,
                                                 int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant();
  }

  if (orientation == Qt::Vertical && section < headers.size())
  {
    return headers[section];
  }
  else if (orientation == Qt::Horizontal && section < row_headers.size())
  {
    return row_headers[section];
  }
  return QVariant();
}

OpenMidEndingEvalDiff::OpenMidEndingEvalDiff(const osl::NumEffectState &s1,
                                             const osl::NumEffectState &s2,
                                             QWidget *parent)
  : QWidget(parent) {
  model = new OpenMidEndingEvalDiffModel(s1, s2, this);
  QTableView *view = new QTableView(this);
  view->setSortingEnabled(true);
  QSortFilterProxyModel *proxy_model = new QSortFilterProxyModel(this);
  proxy_model->setSourceModel(model);
  view->setModel(proxy_model);
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget(view);
  delegate.reset(new DoubleItemDelegate);
  view->setItemDelegate(delegate.get());
}

OpenMidEndingEvalDiffDialog::OpenMidEndingEvalDiffDialog(
  const osl::NumEffectState &s1,
  const osl::NumEffectState &s2,
  QWidget *parent) : QDialog(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  QTabBar *tab = new QTabBar(this);
  layout->addWidget(tab);
  debug = new OpenMidEndingEvalDiff(s1, s2, this);
  layout->addWidget(debug);
  debug1 = new OpenMidEndingEvalDebug(osl::NumEffectState(s1), this);
  layout->addWidget(debug1);
  debug1->setVisible(false);
  debug2 = new OpenMidEndingEvalDebug(osl::NumEffectState(s2), this);
  layout->addWidget(debug2);
  debug2->setVisible(false);
  connect(tab, SIGNAL(currentChanged(int)), this, SLOT(selectTab(int)));
  tab->addTab("EvalDebug");
  tab->addTab("Eval 1");
  tab->addTab("Eval 2");

  QPushButton *button = new QPushButton(this);
  button->setText("&OK");
  layout->addWidget(button);
  connect(button, SIGNAL(clicked()), this, SLOT(accept()));
}

void OpenMidEndingEvalDiffDialog::selectTab(int index)
{
  if (index == 0)
  {
    debug1->setVisible(false);
    debug2->setVisible(false);
    debug->setVisible(true);
  }
  else if (index == 1)
  {
    debug->setVisible(false);
    debug2->setVisible(false);
    debug1->setVisible(true);
  }
  else if (index == 2)
  {
    debug->setVisible(false);
    debug1->setVisible(false);
    debug2->setVisible(true);
  }
}

QSize OpenMidEndingEvalDiffDialog::sizeHint() const
{
  return QSize(640, 480);
}
