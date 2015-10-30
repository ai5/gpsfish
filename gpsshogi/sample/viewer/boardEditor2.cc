#include "boardEditor2.h"
#include "gpsshogi/gui/editBoard2.h"
#include "osl/csa.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QRegExp>
#include <QRadioButton>

BoardEditor2::BoardEditor2(QWidget *parent)
  : TabChild(parent), state(), player(osl::BLACK)
{
  QHBoxLayout *hlayout = new QHBoxLayout;
  board = new gpsshogi::gui::EditBoard2(this);
  hlayout->addWidget(board);
  QVBoxLayout *vlayout = new QVBoxLayout;
  line_edit = new QLineEdit(this);
  vlayout->addWidget(line_edit);
  QPushButton *clear_button = new QPushButton("&Clear Board", this);
  vlayout->addWidget(clear_button);
  QPushButton *to_black_stand_button = new QPushButton("To &Black Stand", this);
  QPushButton *to_white_stand_button = new QPushButton("To &White Stand", this);
  vlayout->addWidget(to_black_stand_button);
  vlayout->addWidget(to_white_stand_button);
  QRadioButton *black_turn = new QRadioButton(tr("Black Turn"), this);
  QRadioButton *white_turn = new QRadioButton(tr("White Turn"), this);
  vlayout->addWidget(black_turn);
  vlayout->addWidget(white_turn);
  hlayout->addLayout(vlayout);
  setLayout(hlayout);

  connect(clear_button, SIGNAL(clicked()),
	  board, SLOT(clearState()));
  connect(line_edit, SIGNAL(returnPressed()),
	  this, SLOT(processText()));
  connect(to_black_stand_button, SIGNAL(clicked()),
	  this, SLOT(reserveToBlack()));
  connect(to_white_stand_button, SIGNAL(clicked()),
	  this, SLOT(reserveToWhite()));
  connect(black_turn, SIGNAL(toggled(bool)),
          this, SLOT(blackTurn(bool)));
  connect(white_turn, SIGNAL(toggled(bool)),
          this, SLOT(whiteTurn(bool)));
}

const osl::NumEffectState& BoardEditor2::getState()
{
  state.copyFrom(osl::NumEffectState(board->getState()));
  return state;
}

void BoardEditor2::setState(const osl::SimpleState &state)
{
  board->setState(state);
}

void BoardEditor2::processText()
{
  QRegExp regex("^(\\+|-)?([1-9][1-9])?(..)$");
  if (regex.indexIn(line_edit->text()) != -1)
  {
    osl::Ptype ptype;
    try
    {
      ptype = osl::csa::strToPtype(regex.cap(3).toUpper().toStdString());
    }
    catch (osl::CsaIOError &)
    {
      return;
    }
    int pos = regex.cap(2).toInt();
    if (regex.cap(1) == "-")
      player = osl::WHITE;
    else if (regex.cap(1) == "+")
      player = osl::BLACK;
    board->moveReserve(player,
		       ptype,
		       pos == 0 ? osl::Square::STAND() :
		       osl::Square(pos / 10, pos % 10));
    line_edit->setText("");
  }
}

void BoardEditor2::reserveToBlack()
{
  board->reserveToStand(osl::BLACK);
}

void BoardEditor2::reserveToWhite()
{
  board->reserveToStand(osl::WHITE);
}

void BoardEditor2::blackTurn(bool checked)
{
  if (checked)
  {
    board->setTurn(osl::BLACK);
  }
}

void BoardEditor2::whiteTurn(bool checked)
{
  if (checked)
  {
    board->setTurn(osl::WHITE);
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
