#ifndef _BOARD_TABCHILD_H
#define _BOARD_TABCHILD_H
#include "tabchild.h"

class MainWindow;

namespace gpsshogi
{
  namespace gui
  {
    class Board;
  }
}

class BoardTabChild : public TabChild
{
  Q_OBJECT
public:
  BoardTabChild(QWidget *parent = 0);
  void toInitialState();
  osl::Player turn() const;
  const osl::NumEffectState& getState();
  void toggleOrientation();
  void setOrientation(bool sente);
  bool isSenteView() const;
  void copy();
  void copyBoardAndMoves();
  void copyUsi();
  void copyBoardAndMovesUsi();
  void enableEffect(bool on);
  bool effectEnabled();
  void highlightLastMove(bool on);
  void highlightBookMove(bool on);
  void showArrowMove(bool on);
  MainWindow *getMainWindow() const;
  virtual osl::NumEffectState getStateAndMovesToCurrent(std::vector<osl::Move> &moves) = 0;
  virtual int getLimit() { return 1000; };
public slots:
  QWidget *moveGenerateDialog();
signals:
  void statusChanged();
  void statusChanged(const osl::SimpleState &state,
		     const std::vector<osl::Move> &moves,
		     int limit, osl::Move next);
protected:
  virtual osl::Move getNextMove() { return osl::Move::INVALID(); }
  gpsshogi::gui::Board *board;
  osl::NumEffectState initialState;
protected slots:
  void notifyState();
};

#endif // _BOARD_TABCHILD_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
