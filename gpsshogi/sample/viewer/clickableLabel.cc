#include "clickableLabel.h"
#include <QMouseEvent>

void ClickableLabel::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    emit clicked();
  else
    QLabel::mousePressEvent(event);
}
