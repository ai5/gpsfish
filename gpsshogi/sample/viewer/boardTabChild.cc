#include "boardTabChild.h"
#include "mainwindow.h"

#include "gpsshogi/gui/board.h"
#include "moveGeneratorDialog.h"
#include "osl/csa.h"
#include "osl/usi.h"

#include <qclipboard.h>
#include <qapplication.h>
#include <qinputdialog.h>
#include <sstream>

BoardTabChild::BoardTabChild(QWidget *parent)
 : TabChild(parent)
{
  board = new gpsshogi::gui::Board(initialState, this);
  connect(board, SIGNAL(statusChanged()),
	  this, SIGNAL(statusChanged()));
}


void BoardTabChild::toInitialState()
{
  board->setState(initialState);
}

osl::Player BoardTabChild::turn() const
{
  return board->getState().turn();
}

const osl::NumEffectState& BoardTabChild::getState()
{
  return board->getState();
}

void BoardTabChild::toggleOrientation()
{
  board->toggleOrientation();
}

void BoardTabChild::setOrientation(bool sente)
{
  board->setView(sente);
}

bool BoardTabChild::isSenteView() const
{
  return board->isSenteView();
}

void BoardTabChild::copy()
{
  std::ostringstream oss(std::ostringstream::out);
  oss << board->getState();
  const std::string &board_string = oss.str();
  QString boardString (board_string.c_str());

  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(boardString, QClipboard::Clipboard);
  if (clipboard->supportsSelection())
  {
    clipboard->setText(boardString, QClipboard::Selection);
  }
}

void BoardTabChild::copyBoardAndMoves()
{
  std::ostringstream oss(std::ostringstream::out);
  std::vector<osl::Move> moves;
  osl::SimpleState state = getStateAndMovesToCurrent(moves);
  oss << state;
  for (size_t i = 0; i < moves.size(); i++)
  {
    oss << osl::csa::show(moves[i]) << std::endl;
  }

  const std::string &board_string = oss.str();
  QString boardString (board_string.c_str());

  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(boardString, QClipboard::Clipboard);
  if (clipboard->supportsSelection())
  {
    clipboard->setText(boardString, QClipboard::Selection);
  }
}

void BoardTabChild::copyUsi()
{
  std::ostringstream oss(std::ostringstream::out);
  oss << "position " << osl::usi::show(board->getState());
  const std::string &board_string = oss.str();
  QString boardString (board_string.c_str());

  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(boardString, QClipboard::Clipboard);
  if (clipboard->supportsSelection())
  {
    clipboard->setText(boardString, QClipboard::Selection);
  }
}

void BoardTabChild::copyBoardAndMovesUsi()
{
  std::ostringstream oss(std::ostringstream::out);
  std::vector<osl::Move> moves;
  osl::SimpleState state = getStateAndMovesToCurrent(moves);
  oss << "position " << osl::usi::show(osl::NumEffectState(state)) << " moves";
  for (size_t i = 0; i < moves.size(); i++)
  {
    oss << " " << osl::usi::show(moves[i]);
  }

  const std::string &board_string = oss.str();
  QString boardString (board_string.c_str());

  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(boardString, QClipboard::Clipboard);
  if (clipboard->supportsSelection())
  {
    clipboard->setText(boardString, QClipboard::Selection);
  }
}

void BoardTabChild::enableEffect(bool on)
{
  board->setEffectType(on ? gpsshogi::gui::Board::BOTH :
		       gpsshogi::gui::Board::NONE);
}

bool BoardTabChild::effectEnabled()
{
  return board->getEffectType() == gpsshogi::gui::Board::BOTH;
}

void BoardTabChild::highlightLastMove(bool on)
{
  board->highlightLastMove(on);
}
void BoardTabChild::highlightBookMove(bool on)
{
  board->highlightBookMove(on);
}
void BoardTabChild::showArrowMove(bool on)
{
  board->showArrowMove(on);
}

QWidget *BoardTabChild::moveGenerateDialog()
{
  std::vector<osl::Move> moves;
  osl::SimpleState state = getStateAndMovesToCurrent(moves);
  MoveGeneratorDialog *dialog =
    new MoveGeneratorDialog(state, moves, getLimit(), 
			    getNextMove(), this);
  dialog->show();
  dialog->raise();
  return dialog;
}


void BoardTabChild::notifyState()
{
  osl::SimpleState s;
  std::vector<osl::Move> m;
  s = getStateAndMovesToCurrent(m);
  emit statusChanged(s, m, getLimit(), getNextMove());
}

MainWindow *BoardTabChild::getMainWindow() const
{
  MainWindow *ret = NULL;
  foreach(QWidget *widget, qApp->topLevelWidgets())
  {
    if(widget->inherits("QMainWindow"))
    {
      ret = qobject_cast<MainWindow*>(widget);
      break;
    }
  }
  Q_CHECK_PTR(ret);
  return ret;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
