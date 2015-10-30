#include "pvModel.h"
#include "osl/hashKey.h"
#include "osl/search/fixedEval.h"
#include "osl/search/simpleHashRecord.h"
#include "gpsshogi/gui/util.h"

PvModel::PvModel(const osl::NumEffectState &state, 
                 const osl::search::SimpleHashTable &table,
                 QObject *parent) : QAbstractTableModel(parent) {
  turn = state.turn();
  const osl::hash::HashKey key(state);
  osl::MoveVector moves;
  state.generateLegal(moves);
  moves.push_back(osl::Move::PASS(state.turn()));
  for (osl::MoveVector::const_iterator p = moves.begin();
       p != moves.end(); ++p) {
    if (table.find(key.newHashWithMove(*p))) {
      osl::MoveVector pv;
      pv.push_back(*p);
      const osl::hash::HashKey new_hash = key.newHashWithMove(*p);
      table.getPV(new_hash, pv);
      const int value = getRecordValue(table.find(new_hash), 
				       p->player());
      pvs.push_back(std::make_pair(value, pv));
    }
  }
}

// TODO: odd evaluation value
int PvModel::getRecordValue(const osl::search::SimpleHashRecord *r,
                            osl::Player turn)
{
  if (r && r->hasUpperBound(0))
      return r->upperBound();
  int value = osl::search::FixedEval::minusInfty(turn)*2;
  if (r && r->hasLowerBound(0))
      value += r->lowerBound();
  return value;
}

int PvModel::rowCount(const QModelIndex &) const {
  return pvs.size();
}

int PvModel::columnCount(const QModelIndex&) const {
  return 3;
}

QVariant PvModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  if (index.row() >= pvs.size() ||
      index.column() > 3) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    const std::pair<int, osl::MoveVector> &row = pvs.at(index.row());
    if (index.column() == 0) {
      return gpsshogi::gui::Util::moveToString(row.second[0]);
    } else if (index.column() == 1) {
      return row.first;
    } else if (index.column() == 2) {
      const osl::MoveVector &pv = row.second;
      QString best_moves;
      for (size_t i = 0; i < pv.size(); ++i) {
        if (best_moves != "") {
          best_moves.append(", ");
        }
        best_moves.append(gpsshogi::gui::Util::moveToString(pv[i]));
      }
      return best_moves;
    }
  } else if (role == Qt::TextAlignmentRole) {
    if (index.column() == 1) {
      return Qt::AlignRight;
    }
  } else if (role == FirstMoveRole) {
    const std::pair<int, osl::MoveVector> &row = pvs.at(index.row());
    return row.second[0].intValue();
  } else if (role == SortRole) {
    const std::pair<int, osl::MoveVector> &row = pvs.at(index.row());
    if (index.column() == 0) {
      return row.second[0].intValue();
    } else if (index.column() == 1) {
      return row.first;
    }
  }
  return QVariant();
}

QVariant PvModel::headerData(int section, Qt::Orientation orientation,
                             int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }
  if (orientation == Qt::Horizontal) {
    switch (section) {
    case 0:
      return tr("Move");
    case 1:
      return tr("Value");
    case 2:
      return tr("Principal Variation");
    }
  }
  return QVariant();
}

bool PvProxyModel::lessThan(const QModelIndex &left,
                            const QModelIndex &right) const {
  if (left.column() == 0) {
    osl::Move m1 =
      osl::Move::makeDirect(sourceModel()->data(left, PvModel::SortRole).toInt());
    osl::Move m2 =
      osl::Move::makeDirect(sourceModel()->data(right, PvModel::SortRole).toInt());
    return gpsshogi::gui::Util::compare(m1, m2) < 0;
  } else if (left.column() == 1) {
    if (static_cast<PvModel *>(sourceModel())->getTurn() == osl::BLACK) {
      return sourceModel()->data(left, PvModel::SortRole).toInt() >
        sourceModel()->data(right, PvModel::SortRole).toInt();
    } else {
      return sourceModel()->data(left, PvModel::SortRole).toInt() <
        sourceModel()->data(right, PvModel::SortRole).toInt();
    }
  }
  return true;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
