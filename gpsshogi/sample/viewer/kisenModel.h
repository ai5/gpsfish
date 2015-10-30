#ifndef KISENMODEL_H
#define KISENMODEL_H

#include <QAbstractTableModel>

namespace osl {
  namespace record {
    class KisenIpxFile;
  }
}

class KisenModel : public QAbstractTableModel {
  Q_OBJECT
public:
  KisenModel(osl::record::KisenIpxFile *ipx, QObject *parent);
  int rowCount(const QModelIndex&) const;
  int columnCount(const QModelIndex&) const;
  QVariant data(const QModelIndex &, int) const;
  QVariant headerData(int, Qt::Orientation, int) const;
private:
  osl::record::KisenIpxFile *ipxFile;
};

#endif // KISENMODEL_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
