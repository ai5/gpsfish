#include "networkViewer.h"
#include "moveList.h"

#include "gpsshogi/gui/util.h"
#include "gpsshogi/gui/board.h"

#include "osl/record/csaRecord.h"

#include <qmessagebox.h>
#include <qlayout.h>
#include <qstring.h>
#include <qlayout.h>
#include <iostream>

using gpsshogi::gui::Util;
/* ======================================================================
 * NetworkViewer
 *  the viewer, with the board on the left and chat screen on the right
 */
NetworkViewer::
NetworkViewer(QWidget *parent)
  : BoardTabChild(parent), index(0)
{
  board->setManualMovement(false);

  QHBoxLayout *mainLayout = new QHBoxLayout();
  moveList = new MoveList();
  mainLayout->addWidget(moveList);
  mainLayout->addWidget(board);

  QVBoxLayout *sideLayout = new QVBoxLayout();
  networkClient = new NetworkClient();
  QObject::connect(networkClient, SIGNAL(gotBoard(QString)),
		   SLOT(receive_board(QString)));
  QObject::connect(networkClient, SIGNAL(gotLastMove(QString)),
		   SLOT(receive_last_move(QString)));
  QObject::connect(networkClient, SIGNAL(chatReceived()),
		   this, SIGNAL(chatReceived()));
  sideLayout->addWidget(networkClient);

  mainLayout->addLayout(sideLayout);

  setLayout(mainLayout);
}

void
NetworkViewer::
setHostname(QString str)
{
  networkClient->setHostname(str);
}

void
NetworkViewer::
setUsername(QString str)
{
  networkClient->setUsername(str);
}

void
NetworkViewer::
setPassword(QString str)
{
  networkClient->setPassword(str);
}

void
NetworkViewer::
connect()
{
  networkClient->openConnection();
}

void
NetworkViewer::
receive_board(QString content)
{
  try {
    std::istringstream is(content.toStdString());
    osl::CsaFile csa(is);
    std::vector<osl::Move> moves;
    std::vector<int> consumed_time;
    csa.load().load(moves, consumed_time);

    initialState = csa.initialState();
    board->setState(csa.initialState());
    moveList->setState(initialState, moves, consumed_time, std::vector<QString>());
  } catch (osl::CsaIOError& e) {
    std::cerr << "receive_board " << e.what() << "\n" << content.toStdString() << "\n";
  }
}

void
NetworkViewer::
receive_last_move(QString content)
{
  osl::Move move = osl::csa::strToMove(content.toStdString(), board->getState());
  moveList->pushMove(move);
  toLastState();
}

void NetworkViewer::forward()
{
  if (index > (int)moveList->numMoves())
    index++;

  updateState();
}

void NetworkViewer::backward()
{
  if (index > 0)
    index--;

  updateState();
}

void NetworkViewer::updateState()
{
  osl::NumEffectState state(initialState);
  for (int i = 0; i < index; i++)
  {
    state.makeMove(moveList->getMove(i));
  }
  board->setState(state);
}

void NetworkViewer::toLastState()
{
  index = moveList->numMoves();
  updateState();
}

int NetworkViewer::moveCount() const
{
  return index;
}

osl::NumEffectState NetworkViewer::
getStateAndMovesToCurrent(std::vector<osl::Move> &moves)
{
  moves.reserve(index);
  for (int i = 0; i < index; i++)
    moves.push_back(moveList->getMove(i));

  return initialState;
}

void NetworkViewer::paintEvent(QPaintEvent *event)
{
  emit painted();
  BoardTabChild::paintEvent(event);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
