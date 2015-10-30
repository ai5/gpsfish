#include "itemDelegate.h"
#include <QPainter>

QString DoubleItemDelegate::displayText(const QVariant &value,
                                        const QLocale &locale) const {
  if (value.type() == QVariant::Double)
  {
    return locale.toString(value.toDouble(), 'f', 2);
  }
  else
  {
    return QStyledItemDelegate::displayText(value, locale);
  }
}
