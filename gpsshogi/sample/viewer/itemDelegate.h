#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QStyledItemDelegate>

class DoubleItemDelegate : public QStyledItemDelegate {
public:
  QString displayText(const QVariant &value, const QLocale &locale) const;
};

#endif // ITEMDELEGATE_H
