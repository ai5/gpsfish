#ifndef PV_MODEL_H
#define PV_MODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QSortFilterProxyModel>

#include "osl/search/simpleHashTable.h"
#include "osl/numEffectState.h"

class PvModel : public QAbstractTableModel {
  Q_OBJECT
public:
  PvModel(const osl::NumEffectState&, 
          const osl::search::SimpleHashTable&,
          QObject *parent);
  int rowCount(const QModelIndex&) const;
  int columnCount(const QModelIndex&) const;
  QVariant data(const QModelIndex &, int) const;
  QVariant headerData(int, Qt::Orientation, int) const;
  enum { FirstMoveRole = Qt::UserRole, SortRole };
  osl::Player getTurn() const { return turn; };
private:
  static int getRecordValue(const osl::search::SimpleHashRecord *r,
                            osl::Player turn);
  osl::Player turn;
  QList<std::pair<int, osl::MoveVector> > pvs;
};

class PvProxyModel : public QSortFilterProxyModel {
public:
  bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // PV_MODEL_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
