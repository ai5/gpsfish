#ifndef _BOARDEDITOR2_H
#define _BOARDEDITOR2_H
#include "tabchild.h"

namespace gpsshogi
{
  namespace gui
  {
    class EditBoard2;
  }
}

class QLineEdit;

class BoardEditor2 : public TabChild
{
  Q_OBJECT
public:
  BoardEditor2(QWidget *parent = 0);
  virtual void forward() {
  }
  virtual void backward() {
  }
  virtual void toInitialState() {
  }
  virtual void toLastState() {
  }
  virtual int moveCount() const {
    return 0;
  }
  virtual osl::Player turn() const {
    return osl::BLACK;
  }
  virtual const osl::NumEffectState& getState();
  virtual void toggleOrientation() {
  }
  virtual osl::NumEffectState getStateAndMovesToCurrent(std::vector<osl::Move> &/*moves*/) {
    return getState();
  }
  virtual QWidget *moveGenerateDialog() {
    return NULL;
  }
  void setState(const osl::SimpleState &state);
private slots:
  void processText();
  void reserveToBlack();
  void reserveToWhite();
  void blackTurn(bool);
  void whiteTurn(bool);
private:
  osl::NumEffectState state;
  osl::Player player;
  gpsshogi::gui::EditBoard2 *board;
  QLineEdit *line_edit;
};
#endif // _BOARDEDITOR2_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
