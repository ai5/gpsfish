#ifndef _SHOW_STATES_DIALOG_H_
#define _SHOW_STATES_DIALOG_H_
#include <QDialog>
#include "osl/state/simpleState.h"
#include "osl/apply_move/doUndoMoveStack.h"
#include "gpsshogi/gui/board.h"
#include "openingEditor.h"
#include "movelist.h"

class QLabel;

struct StateMove
{
  StateMove(const osl::state::SimpleState &state,
	    const osl::stl::vector<osl::record::opening::WMove> &moves)
    : state(state), moves(moves) {
  }

  osl::state::SimpleState state;
  osl::stl::vector<osl::record::opening::WMove> moves;
};

class ShowStatesDialog : public QDialog
{
Q_OBJECT
public:
ShowStatesDialog(Editor *editor,
		 osl::state::SimpleState state, QWidget *parent);
private slots:
  void next();
  void previous();
private:
  void addMoves(osl::Player player,
		int count,
		osl::apply_move::DoUndoMoveStack<osl::state::SimpleState> &stack,
		osl::state::SimpleState &state,
		osl::stl::vector<osl::record::opening::WMove> &moves);
  void updateStatus();
  Editor *editor;
  osl::state::SimpleState state;
  gpsshogi::gui::Board *board;
  QLabel *countLabel;
  MoveListView *list;
  osl::stl::vector<StateMove> states;
  int index;
};

#endif // _SHOW_STATES_DIALOG_H_
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
