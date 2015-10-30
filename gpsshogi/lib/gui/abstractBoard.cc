#include "gpsshogi/gui/abstractBoard.h"
#include "osl/numEffectState.h"
#include <QPainter>
#include <QResizeEvent>

const int gpsshogi::gui::AbstractBoard::STAND_SIZE    = 80;
const int gpsshogi::gui::AbstractBoard::BOX_SIZE      = 40;
const int gpsshogi::gui::AbstractBoard::BLACK_STAND_X = gpsshogi::gui::AbstractBoard::STAND_SIZE +
                                                        gpsshogi::gui::AbstractBoard::BOX_SIZE * 9 + 20;
const int gpsshogi::gui::AbstractBoard::MARGIN_HEAD    = gpsshogi::gui::AbstractBoard::BOX_SIZE;

gpsshogi::gui::AbstractBoard::AbstractBoard(QWidget *parent)
  : QWidget(parent), selectedPiece(osl::Piece::EMPTY()), scale_mult(1.0)
{
  QFont new_font = font();
  new_font.setPixelSize(20);
  setFont(new_font);
}

QSize gpsshogi::gui::
AbstractBoard::sizeHint() const
{
  return QSize(520, 520);
}

QSize gpsshogi::gui::
AbstractBoard::minimumSize() const
{
  return QSize(260, 260);
}

const QPixmap gpsshogi::gui::
AbstractBoard::pieceToImage(osl::Piece p) const
{
  QString prefix;
  if ((p.owner() == osl::BLACK && !reversed())
      || (p.owner() == osl::WHITE && reversed()))
  {
    prefix = "";
  }
  else
  {
    prefix = "r";
  }
  QString imagename;
  switch (p.ptype())
  {
  case osl::PAWN:
    imagename = "fu.png";
    break;
  case osl::LANCE:
    imagename = "kyo.png";
    break;
  case osl::KNIGHT:
    imagename = "kei.png";
    break;
  case osl::SILVER:
    imagename = "gin.png";
    break;
  case osl::GOLD:
    imagename = "kin.png";
    break;
  case osl::BISHOP:
    imagename = "kaku.png";
    break;
  case osl::ROOK:
    imagename = "hi.png";
    break;
  case osl::KING:
    imagename = "gyoku.png";
    break;
  case osl::PPAWN:
    imagename = "to.png";
    break;
  case osl::PLANCE:
    imagename = "nkyo.png";
    break;
  case osl::PKNIGHT:
    imagename = "nkei.png";
    break;
  case osl::PSILVER:
    imagename = "ngin.png";
    break;
  case osl::PBISHOP:
    imagename = "uma.png";
    break;
  case osl::PROOK:
    imagename = "ryu.png";
    break;
  default:
    abort();
  }
#if QT_VERSION >= 0x040000
  return QPixmap(":/images/" + prefix + imagename);
#else  
  return QPixmap::fromMimeSource(prefix + imagename);
#endif
}

const QString gpsshogi::gui::
AbstractBoard::pieceString(osl::Piece p) const
{
  switch (p.ptype())
  {
  case osl::PAWN:
    return QString::fromUtf8("歩");
  case osl::LANCE:
    return QString::fromUtf8("香");
  case osl::KNIGHT:
    return QString::fromUtf8("桂");
  case osl::SILVER:
    return QString::fromUtf8("銀");
  case osl::GOLD:
    return QString::fromUtf8("金");
  case osl::BISHOP:
    return QString::fromUtf8("角");
  case osl::ROOK:
    return QString::fromUtf8("飛");
  case osl::KING:
    return QString::fromUtf8("王");
  case osl::PPAWN:
    return QString::fromUtf8("と");
  case osl::PLANCE:
    return QString::fromUtf8("杏");
  case osl::PKNIGHT:
    return QString::fromUtf8("圭");
  case osl::PSILVER:
    return QString::fromUtf8("全");
  case osl::PBISHOP:
    return QString::fromUtf8("馬");
  case osl::PROOK:
    return QString::fromUtf8("竜");
  default:
    abort();
  }
}

void gpsshogi::gui::
AbstractBoard::paintPiece(QPainter *painter, int x, int y, osl::Piece piece)
{
  QString str = pieceString(piece);
  if ((piece.owner() == osl::WHITE && !reversed()) ||
      (piece.owner() == osl::BLACK && reversed()))
  {
    painter->save();
    painter->translate(x + BOX_SIZE, y + BOX_SIZE);
    painter->rotate(180);
    painter->drawText(0, 0, BOX_SIZE, BOX_SIZE, Qt::AlignCenter, pieceString(piece));
    painter->restore();
  }
  else
  {
    painter->drawText(x, y, BOX_SIZE, BOX_SIZE, Qt::AlignCenter, pieceString(piece));
  }
}

void gpsshogi::gui::
AbstractBoard::resizeEvent(QResizeEvent *event)
{
  const QSize new_size = event->size();

  scale_mult = std::min(new_size.width(), new_size.height()) / 520.0;
}

void gpsshogi::gui::
AbstractBoard::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.scale(scale_mult, scale_mult);

  for (int i = 0; i < 10; i++)
  {
    painter.drawLine(STAND_SIZE, i * BOX_SIZE + MARGIN_HEAD,
		     STAND_SIZE + BOX_SIZE * 9, i * BOX_SIZE + MARGIN_HEAD);
    painter.drawLine(STAND_SIZE + i * BOX_SIZE, 0 + MARGIN_HEAD,
		     STAND_SIZE + i * BOX_SIZE, BOX_SIZE * 9 + MARGIN_HEAD);
  }

  osl::CArray<int, osl::PTYPE_MAX + 1> edit_pieces;
  edit_pieces.fill(0);
  for (int i = 0; i < 40; i++)
  {
    osl::Piece p = pieceOf(i);
    if (!p.square().isValid())
    {
      edit_pieces[osl::unpromote(p.ptype())]++;
    }
    else if (p.isOnBoard() && p != selectedPiece)
    {
      QPoint point = positionToPoint(p.square());

      int x = point.x();
      int y = point.y();
      paintPiece(&painter, x, y, p);
    }
  }
  // Black
  for (unsigned int i = 0; i < osl::PieceStand::order.size(); i++)
  {
    int y = BOX_SIZE + i * BOX_SIZE;
    osl::Ptype pt = osl::PieceStand::order[i];
    osl::Player player = !reversed() ? osl::BLACK : osl::WHITE;
    int count = countPiecesOnStand(player, pt);
    if (! selectedPiece.isEmpty() && selectedPiece.owner() == player
	&& selectedPiece.square().isValid()
	&& !selectedPiece.isOnBoard() && selectedPiece.ptype() == pt)
      count--;
    if (count > 0)
    {
      osl::Piece p(player, pt, 0, osl::Square::STAND());
      painter.drawText(BLACK_STAND_X + 10, y + 10, 20, 20, Qt::AlignCenter, pieceString(p));
      if (count != 1)
	painter.drawText(BLACK_STAND_X + 10, y + 30, 20, 20, Qt::AlignCenter, QString("%1").arg(count));
    }
  }
  // White
  for (unsigned int i = 0; i < osl::PieceStand::order.size(); i++)
  {
    int x = 0;
    int y = BOX_SIZE * 9 - i * BOX_SIZE;
    osl::Ptype pt = osl::PieceStand::order[i];
    osl::Player player = !reversed() ? osl::WHITE : osl::BLACK;
    int count = countPiecesOnStand(player, pt);
    if (! selectedPiece.isEmpty() && selectedPiece.owner() == player
	&& selectedPiece.square().isValid()
	&& !selectedPiece.isOnBoard() && selectedPiece.ptype() == pt)
      count--;
    if (count > 0)
    {
      osl::Piece p(player, pt, 0, osl::Square::STAND());
      painter.save();
      painter.translate(x + BOX_SIZE, y + BOX_SIZE);
      painter.rotate(180);
      painter.drawText(-10, 10, 20, 20, Qt::AlignCenter, pieceString(p));
      if (count != 1)
	painter.drawText(-10, 30, 20, 20, Qt::AlignHCenter, QString("%1").arg(count));
      painter.restore();
    }
  }
  // Edit pieces
  if (edit_pieces[osl::KING] > 0)
  {
    int count = edit_pieces[osl::KING];
    if (! selectedPiece.isEmpty() && selectedPiece.owner() == osl::BLACK
	&& !selectedPiece.square().isValid()
	&& selectedPiece.ptype() != osl::KING)
      count--;
    osl::Piece p(osl::BLACK, osl::KING, 0, osl::Square::STAND());
    painter.drawText(STAND_SIZE, 10 * BOX_SIZE,
                     BOX_SIZE, BOX_SIZE, Qt::AlignCenter,
                     pieceString(p));
    if (count != 1)
      painter.drawText(STAND_SIZE, 11 * BOX_SIZE,
                       BOX_SIZE, BOX_SIZE, Qt::AlignCenter,
		       QString("%1").arg(count));
  }
  for (unsigned int i = 0; i < osl::PieceStand::order.size(); i++)
  {
    int count = edit_pieces[osl::PieceStand::order[i]];
    if (! selectedPiece.isEmpty() && selectedPiece.owner() == osl::BLACK
	&& !selectedPiece.square().isValid()
	&& selectedPiece.ptype() != osl::KING)
      count--;
    osl::Piece p(osl::BLACK, osl::PieceStand::order[i], 0, osl::Square::STAND());
    if (count > 0)
      painter.drawText(STAND_SIZE + BOX_SIZE * (i + 1), 10 * BOX_SIZE,
                       BOX_SIZE, BOX_SIZE, Qt::AlignCenter,
                       pieceString(p));
    if (count > 1)
      painter.drawText(STAND_SIZE + BOX_SIZE * (i + 1), 11 * BOX_SIZE,
                       BOX_SIZE, BOX_SIZE, Qt::AlignCenter,
		       QString("%1").arg(count));
  }
  if (!selectedPiece.isEmpty())
  {
    if ((selectedPiece.owner() == osl::BLACK && !reversed()) ||
        (selectedPiece.owner() == osl::WHITE && reversed()))
    {
      painter.drawText((mousePoint.x() - 10) / scale_mult,
                       (mousePoint.y() + 10) / scale_mult,
                       pieceString(selectedPiece));
    }
    else
    {
      painter.save();
      painter.translate((mousePoint.x() + 10) / scale_mult,
                        (mousePoint.y() + 10) / scale_mult);
      painter.rotate(180);
      painter.drawText(0, 10 / scale_mult, pieceString(selectedPiece));
      painter.restore();
    }
  }
  painter.end();
}


bool gpsshogi::gui::
AbstractBoard::isOnBoard(int x, int y) const
{
  if ((STAND_SIZE <= x && x <= STAND_SIZE + 9 * BOX_SIZE)
      && (0 <= y && y <= BOX_SIZE * 9))
    return true;
  else
    return false;
}

const osl::Square gpsshogi::gui::
AbstractBoard::getSquare(int x, int y) const
{
  y -= MARGIN_HEAD;
  if (isOnBoard(x, y))
  {
    int board_x = 9 - (x - STAND_SIZE) / BOX_SIZE;
    int board_y = y / BOX_SIZE + 1;
    if (reversed())
    {
      board_x = 10 - board_x;
      board_y = 10 - board_y;
    }
    return osl::Square(board_x, board_y);
  }
  else if (STAND_SIZE <= x &&
	   x <= STAND_SIZE + BOX_SIZE * ((int)osl::PieceStand::order.size()  + 1) &&
	   y >= 10 * BOX_SIZE && y <= 11 * BOX_SIZE)
    return osl::Square(0, 0);
  else
    return osl::Square::STAND();
}

#include <iostream>
const osl::Piece gpsshogi::gui::
AbstractBoard::getPiece(int x, int y)
{
  y -= MARGIN_HEAD;
  if (isOnBoard(x, y))
  {
    int board_x = 9 - (x - STAND_SIZE) / BOX_SIZE;
    int board_y = y / BOX_SIZE + 1;
    if (reversed())
    {
      board_x = 10 - board_x;
      board_y = 10 - board_y;
    }
    return pieceAt(osl::Square(board_x, board_y));
  }
  else if (STAND_SIZE <= x &&
	   x <= STAND_SIZE + BOX_SIZE * ((int)osl::PieceStand::order.size()  + 1) &&
	   y >= 9 * BOX_SIZE && y <= 11 * BOX_SIZE)
  {
    if (x <= STAND_SIZE + BOX_SIZE)
    {
      return getReservePiece(osl::KING);
    }
    return getReservePiece(osl::PieceStand::order[(x - STAND_SIZE) / BOX_SIZE - 1]);
  }
  else if (BLACK_STAND_X <= x && x <= BLACK_STAND_X + BOX_SIZE &&
	   y >= 0 && y < BOX_SIZE * 7)
  {
    return getStandPiece(osl::BLACK,
			 osl::PieceStand::order[y / BOX_SIZE]);
  }
  else if (0 <= x && x <= BOX_SIZE &&
	   y >= BOX_SIZE * 2 && y < BOX_SIZE * 9)
  {
    return getStandPiece(osl::WHITE,
			 osl::PieceStand::order[osl::PieceStand::order.size() - 1 -
						(y - BOX_SIZE * 2) / BOX_SIZE]);
  }

  return osl::Piece::EMPTY();
}

const QPoint gpsshogi::gui::
AbstractBoard::positionToPoint(osl::Square position) const
{
  int x = position.x();
  int y = position.y();
  assert (1 <= x &&  x <= 9);
  assert (1 <= y &&  y <= 9);
  if (reversed())
  {
    x = 10 - x;
    y = 10 - y;
  }
  return QPoint(STAND_SIZE + BOX_SIZE * (9 - x),
		BOX_SIZE * (y - 1) + MARGIN_HEAD);
}

const QPoint gpsshogi::gui::
AbstractBoard::standToPoint(osl::Player player, osl::Ptype ptype) const
{
  int i = std::find(osl::PieceStand::order.begin(),
		    osl::PieceStand::order.end(), ptype)
    - osl::PieceStand::order.begin();
  if ((player == osl::BLACK) ^ reversed()) {
    int y = BOX_SIZE + i * BOX_SIZE;
    return QPoint(BLACK_STAND_X, y);
  }
  else {
    int y = BOX_SIZE*2 + (osl::PieceStand::order.size()-i) * BOX_SIZE;
    return QPoint(BOX_SIZE, y);
  }
}

void gpsshogi::gui::
AbstractBoard::mouseMoveEvent(QMouseEvent *e)
{
  mousePoint = e->pos();
  if (!selectedPiece.isEmpty())
  {
    update();
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
