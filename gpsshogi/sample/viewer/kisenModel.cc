#include "kisenModel.h"

#include <QTextCodec>
#include "osl/record/kisen.h"

KisenModel::KisenModel(osl::record::KisenIpxFile *ipx,
                       QObject *parent)
  : QAbstractTableModel(parent), ipxFile(ipx) {
}

int KisenModel::rowCount(const QModelIndex &) const {
  return ipxFile->size();
}

int KisenModel::columnCount(const QModelIndex&) const {
  return 5;
}

QVariant KisenModel::headerData(int section, Qt::Orientation orientation,
                                int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }
  if (orientation == Qt::Horizontal) {
    switch (section) {
    case 0:
      return QString::fromUtf8("局番号");
    case 1:
    case 3:
      return QString::fromUtf8("段位");
    case 2:
      return QString::fromUtf8("先手");
    case 4:
      return QString::fromUtf8("後手");
    }
  }
  return QVariant();
}

QVariant KisenModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  if (static_cast<size_t>(index.row()) >= ipxFile->size() ||
      index.column() > 5) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    if (index.column() == 0) {
      return QString("%1").arg(index.row());
    }
    QTextCodec *codec = QTextCodec::codecForName("euc-jp");
    if (index.column() == 1) {
      std::string black_title = ipxFile->title(index.row(), osl::BLACK);
      return codec->toUnicode(black_title.c_str(), black_title.length());
    } else if (index.column() == 2) {
      std::string black = ipxFile->player(index.row(), osl::BLACK);
      return codec->toUnicode(black.c_str(), black.length());
    } else if (index.column() == 3) {
      std::string white_title = ipxFile->title(index.row(), osl::WHITE);
      return codec->toUnicode(white_title.c_str(), white_title.length());
    } else if (index.column() == 4) {
      std::string white = ipxFile->player(index.row(), osl::WHITE);
      return codec->toUnicode(white.c_str(), white.length());
    }
  }
  return QVariant();
}
