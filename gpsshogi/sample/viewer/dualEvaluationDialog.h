/* dualEvaluationDialog.h
 */
#ifndef _DUALEVALUATIONDIALOG_H
#define _DUALEVALUATIONDIALOG_H

#include "osl/record/searchInfo.h"
#include <qdialog.h>
#include <cmath>

class DualEvaluationGraph;
class DualEvaluationDialog : public QDialog
{
Q_OBJECT
public:
  DualEvaluationDialog(QWidget *parent = 0);
  void setInfo(const std::vector<osl::record::SearchInfo>& info);
  void setRawValues(const std::vector<int>& eval, const std::vector<double>& progress);
  void emitSelected(int index) { // XXX
    emit selected(index);
  }
  void showEvent(QShowEvent *);
  void hideEvent(QHideEvent *);
public slots:
  void setIndex(int index);
signals:
  void selected(int index);
private:
  DualEvaluationGraph *graph;
  static QRect lastGeometry;
};

class DualEvaluationGraph : public QWidget
{
Q_OBJECT
public:
  DualEvaluationGraph(DualEvaluationDialog *parent);
  void setInfo(const std::vector<osl::record::SearchInfo>& info) {
    this->info = info;
  }
  void setRawValues(const std::vector<int>& eval, const std::vector<double>& progress)
  {
    this->eval = eval;
    this->progress = progress;
  }
  void setCursor(int new_cursor) { 
    cursor = new_cursor; 
    update();
  }
protected:
  void paintEvent(QPaintEvent *);
  void mouseReleaseEvent(QMouseEvent *e);
private:
  int y(int value, int adjust=0) const
  {
    const int h = std::max(height(),1);
    return std::max(adjust, 
		    std::min(h-adjust, h/2 + adjust - value*h/8000));
  }
  int yvalue(size_t index, int adjust=0) const
  {
    return y(info.at(index).value, adjust);
  }
  int yeval(size_t index, int adjust=0) const
  {
    return y(eval.at(index), adjust);
  }
  int yprogress(size_t index) const
  {
    const int h = std::max(height(),1);
    const double v = progress.at(index);
    return h - std::min(h, static_cast<int>(round(v*h)));
  }
  int x(int index) const
  {
    return width()*index/(1+xsize());
  }
  int x2index(int x) const
  {
    return x*(1+xsize())/width();
  }
  int xsize() const
  {
    return std::max(std::max(info.size(), eval.size()), progress.size());
  }
  std::vector<osl::record::SearchInfo> info;
  std::vector<int> eval;
  std::vector<double> progress;
  int cursor;
  DualEvaluationDialog *dialog;
private slots:
  void forward();
  void backward();
};

#endif /* _DUALEVALUATIONDIALOG_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
