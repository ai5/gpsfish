#ifndef _TABCHILD_H
#define _TABCHILD_H
#include <qwidget.h>
#include "osl/numEffectState.h"

class TabChild : public QWidget
{
public:
  TabChild(QWidget *parent = 0) :
    QWidget(parent) {
  }
  virtual void forward() = 0;
  virtual void backward() = 0;
  virtual void toInitialState() = 0;
  virtual void toLastState() = 0;
  virtual int moveCount() const = 0;
  virtual osl::Player turn() const = 0;
  virtual const osl::NumEffectState& getState() = 0;
  virtual void toggleOrientation() = 0;
  virtual osl::NumEffectState getStateAndMovesToCurrent(std::vector<osl::Move> &moves) = 0;
  virtual QString getFilename() {
    return QString();
  }
  virtual void copy() {
  }
  virtual void copyBoardAndMoves() {
  }
  virtual void copyUsi() {
  }
  virtual void copyBoardAndMovesUsi() {
  }
  virtual void enableEffect(bool) {
  }
  virtual bool effectEnabled() {
    return false;
  }
  virtual void highlightLastMove(bool) {
  }
  virtual void highlightBookMove(bool) {
  }
  virtual void showArrowMove(bool) {
  }
  virtual QWidget *moveGenerateDialog() = 0;
};

#endif // _TABCHILD_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
