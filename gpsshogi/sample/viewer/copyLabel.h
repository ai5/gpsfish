#ifndef _COPY_LABEL_H
#define _COPY_LABEL_H
#include <qlabel.h>

class QContextMenuEvent;

class CopyLabel : public QLabel
{
Q_OBJECT
public:
  CopyLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent) {
    setMinimumSize(sizeHint());
  }
  void contextMenuEvent(QContextMenuEvent *);
private slots:
  void copy();
};

class CopyRateLabel : public CopyLabel
{
Q_OBJECT
public:
  CopyRateLabel(const QString &text, QWidget *parent);
  void setColor(QColor p, QColor n ) { positive = p; negative = n; }
  void setRate(double r);
private:
  QColor positive, negative;
  double rate; // [-1,1]
};

#endif // _COPY_LABEL_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
