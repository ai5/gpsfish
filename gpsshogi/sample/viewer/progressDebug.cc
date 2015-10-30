#include "progressDebug.h"
#include "ui_progressdebugdialog.h"
#include "osl/progress.h"

#include <QAbstractTableModel>

class NewProgressDebugModel : public QAbstractTableModel
{
public:
  NewProgressDebugModel(const osl::NumEffectState &state,
                        QObject *parent = 0);
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
private:
  QStringList column_headers, row_headers;
  osl::progress::ml::NewProgressDebugInfo debug_info;
};

NewProgressDebugModel::NewProgressDebugModel(
  const osl::NumEffectState &state,
  QObject *parent)
  : QAbstractTableModel(parent)
{
  osl::progress::ml::NewProgress progress(state);
  debug_info = progress.debugInfo();
  column_headers << "Black" << "White" << "Sum";
  row_headers << "ATTACK_5X3"
              << "DEFENSE_5X3"
              << "ATTACK5X5"
              << "STAND"
              << "EFFECT5X5"
              << "KING_RELATIVE_ATTACK"
              << "KING_RELATIVE_DEFENSE"
              << "NON_PAWN_ATTACKED_PAIR";
}

int NewProgressDebugModel::rowCount(const QModelIndex &) const
{
  return row_headers.size();
}

int NewProgressDebugModel::columnCount(const QModelIndex &) const
{
  return column_headers.size();
}

QVariant NewProgressDebugModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant();
  }

  if (orientation == Qt::Vertical && section < row_headers.size())
  {
    return row_headers[section];
  }
  else if (orientation == Qt::Horizontal && section < column_headers.size())
  {
    return column_headers[section];
  }
  return QVariant();
}

QVariant NewProgressDebugModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }
  if (role != Qt::DisplayRole)
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

  if (column == 0)
  {
    return debug_info.black_values[row];
  }
  else if (column == 1)
  {
    return debug_info.white_values[row];
  }
  else if (column == 2)
  {
    return debug_info.black_values[row] + debug_info.white_values[row];
  }
  return QVariant();
}

NewProgressDebugDialog::NewProgressDebugDialog(const osl::NumEffectState &state,
                                               QWidget *parent)
  : QDialog(parent), ui(new Ui::ProgressDebugDialog)
{
  ui->setupUi(this);
  model = new NewProgressDebugModel(state, this);
  ui->tableView->setModel(model);
}

NewProgressDebugDialog::~NewProgressDebugDialog()
{
  delete ui;
}
