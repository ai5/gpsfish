#include "copyLabel.h"

#include <qclipboard.h>
#if QT_VERSION >= 0x050000
#  include <QtWidgets>
#else
#  include <QtGui>
#endif
#include <qapplication.h>
#include <cmath>

void CopyLabel::contextMenuEvent(QContextMenuEvent *e)
{
  if (text() == "")
    return;

  QMenu menu(this);
  QAction *copyAction = new QAction("&Copy", this);
  connect(copyAction, SIGNAL(triggered()),
          this, SLOT(copy()));
  menu.addAction(copyAction);
  menu.exec(e->globalPos());
}

void CopyLabel::copy()
{
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(text(), QClipboard::Clipboard);
  if (clipboard->supportsSelection())
  {
    clipboard->setText(text(), QClipboard::Selection);
  }
}

  
CopyRateLabel::CopyRateLabel(const QString &text, QWidget *parent)
  : CopyLabel(text, parent) 
{
  setColor(QColor(255, 255, 0), QColor(0, 128, 255));
  setRate(0.0);
}

void CopyRateLabel::setRate(double new_rate)
{
  rate = std::max(-1.0, std::min(1.0, new_rate));
  int r, g, b;
  if (rate > 0.0)
    positive.getRgb(&r, &g, &b);
  else
    negative.getRgb(&r, &g, &b);

  const double p = std::abs(rate);
  r = static_cast<int>(r*p + 255*(1-p));
  g = static_cast<int>(g*p + 255*(1-p));
  b = static_cast<int>(b*p + 255*(1-p));  
  QPalette palette; 
  palette.setColor(QPalette::Window, QColor(r, g, b)); 
  setPalette(palette); 
  setAutoFillBackground(true);
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
