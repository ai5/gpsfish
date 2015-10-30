/* dualEvaluationDialog.cc
 */
#include "dualEvaluationDialog.h"

#include <qpainter.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <QMouseEvent>
#include <QShortcut>

QRect DualEvaluationDialog::lastGeometry(100, 100, 640, 300);

DualEvaluationGraph::DualEvaluationGraph(DualEvaluationDialog *parent)
  : QWidget(parent), cursor(0), dialog(parent)
{
  QShortcut *forwardShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
  connect(forwardShortcut, SIGNAL(activated()), this, SLOT(forward()));
  QShortcut *backwardShortcut = new QShortcut(QKeySequence("Ctrl+B"), this);
  connect(backwardShortcut, SIGNAL(activated()), this, SLOT(backward()));
}

void DualEvaluationGraph::forward()
{
  const int new_index = cursor + 1;
  setCursor(new_index);
  dialog->emitSelected(new_index);
}

void DualEvaluationGraph::backward()
{
  const int new_index = cursor - 1;
  setCursor(new_index);
  dialog->emitSelected(new_index);
}

void DualEvaluationGraph::mouseReleaseEvent(QMouseEvent *e)
{
  const int new_index = x2index(e->x());
  setCursor(new_index);
  dialog->emitSelected(new_index);
}

void DualEvaluationGraph::paintEvent(QPaintEvent *)
{
  QImage image(this->size(), QImage::Format_ARGB32_Premultiplied); // empty image
  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QBrush brush(QColor("white"));
  painter.fillRect(0, 0, width(), height(), brush);
  painter.setPen(QColor("grey"));
  painter.drawLine(0, height()/4*1, width(), height()/4*1);
  painter.drawLine(0, height()/4*2, width(), height()/4*2);
  painter.drawLine(0, height()/4*3, width(), height()/4*3);

  painter.setPen(QColor("black"));
  painter.drawLine(x(cursor), 0, x(cursor), height());
  for (size_t i = 20; i < info.size(); i+=20)
  {
    painter.drawLine(x(i), height()/2-3, x(i), height()/2+3);
    painter.drawText(x(i), height()/2+20, QString("%1").arg(i));
  }
  painter.drawText(5, 20, QString::fromUtf8("先手優勢"));
  painter.drawText(5, height()-5, QString::fromUtf8("後手優勢"));
  painter.drawText(5, y(+2000), QString("+%1").arg(2000));
  painter.drawText(5, y(-2000), QString("-%1").arg(2000));

  painter.setPen(QPen(QColor("blue"), 2));
  for (size_t i = 2; i < info.size(); i+=2)
  {
    painter.drawLine(x(i-2), yvalue(i-2),
		     x(i),   yvalue(i));
  }
  painter.setPen(QPen(QColor("red"), 2));
  for (size_t i = 3; i < info.size(); i+=2)
  {
    painter.drawLine(x(i-2), yvalue(i-2, 1),
		     x(i),   yvalue(i, 1));
  }
  painter.setPen(QPen(QColor("gray"), 1));
  for (size_t i = 1; i < eval.size(); ++i)
  {
    painter.drawLine(x(i-1), yeval(i-1),
		     x(i),   yeval(i));
  }
  painter.setPen(QPen(QColor("green"), 1));
  painter.drawText(width()-50, height()-5, QString::fromUtf8("序盤"));
  painter.drawText(width()-50, 20, QString::fromUtf8("終盤"));
  for (size_t i = 1; i < progress.size(); ++i)
  {
    painter.drawLine(x(i-1), yprogress(i-1),
		     x(i),   yprogress(i));
  }

  // Draw the image onto the widget
  QPainter widgetPainter(this);
  widgetPainter.drawImage(0, 0, image);
}

DualEvaluationDialog::
DualEvaluationDialog(QWidget *parent)
  : QDialog(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  graph = new DualEvaluationGraph(this);
  layout->addWidget(graph);

  QPushButton *button = new QPushButton(this);
  button->setText("&OK");
  layout->addWidget(button);
  connect(button, SIGNAL(clicked()), this, SLOT(accept()));
}

void DualEvaluationDialog::showEvent(QShowEvent *event)
{
  setGeometry(lastGeometry);
  if (geometry().width() < 100 || geometry().height() < 100)
    resize(std::max(geometry().width(), 100), 
	   std::max(geometry().height(),100));
  QDialog::showEvent(event);
}

void DualEvaluationDialog::hideEvent(QHideEvent *event)
{
  lastGeometry = geometry();
  QDialog::hideEvent(event);
}

void DualEvaluationDialog::
setInfo(const std::vector<osl::record::SearchInfo>& info)
{
  graph->setInfo(info);
}

void DualEvaluationDialog::
setRawValues(const std::vector<int>& eval, const std::vector<double>& progress)
{
  graph->setRawValues(eval, progress);
}

void DualEvaluationDialog::setIndex(int index)
{
  graph->setCursor(index);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
