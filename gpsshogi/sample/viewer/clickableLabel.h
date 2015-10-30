#ifndef _CLICKABLE_LABEL_H
#define _CLICKABLE_LABEL_H
#include "copyLabel.h"

class QMouseEvent;

class ClickableLabel : public CopyRateLabel
{
Q_OBJECT
public:
  ClickableLabel(const QString &text, QWidget *parent)
    : CopyRateLabel(text, parent) {
  }
protected:
  void mousePressEvent(QMouseEvent *event);
signals:
  void clicked();
};

#endif // _CLICKABLE_LABEL_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
