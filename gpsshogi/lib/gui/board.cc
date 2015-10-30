#include <qsizepolicy.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qcursor.h>

#include "gpsshogi/gui/board.h"
#include "osl/book/bookInMemory.h"
#include "osl/state/historyState.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/simpleHashRecord.h"
#include <unordered_set>

#if QT_VERSION >= 0x040000
#include <QMouseEvent>
#endif
#include <complex>

bool gpsshogi::gui::Board::highlight_enabled = false;
bool gpsshogi::gui::Board::highlight_book = false;
bool gpsshogi::gui::Board::arrow_enabled = false;

gpsshogi::gui::Board::Board(const osl::NumEffectState& state,
			    QWidget *parent)
  : AbstractBoard(parent),
    senteView(true), state(state), allowManualMovement(true),
    effect(NONE), table(0)
{
  QColor color(255, 255, 223); 
  QPalette palette; 
  palette.setColor(QPalette::Window, color); 
  setPalette(palette); 
  setAutoFillBackground(true);
}

void gpsshogi::gui::
Board::mousePressEvent(QMouseEvent *e)
{
  const int x = e->x() / scale_mult;
  const int y = e->y() / scale_mult;
  if (!allowManualMovement)
    return;
  osl::Square pos = getSquare(x, y);
  if (!pos.isValid())
    return;
  if (! pos.isPieceStand())
  {
    osl::Piece piece = pieceAt(pos);
    if (piece.isPiece() && piece.owner() == state.turn())
      selectedPiece = state.pieceAt(pos);
  }
  else
  {
    int pieceNum = -1;
    if (((state.turn() == osl::BLACK && senteView)
	 || (state.turn() == osl::WHITE && !senteView))
	&& BLACK_STAND_X <= x && x <= BLACK_STAND_X + BOX_SIZE)
    {
      pieceNum = (y - BOX_SIZE) / BOX_SIZE;
    }
    if (((state.turn() == osl::WHITE && senteView)
	 || (state.turn() == osl::BLACK && !senteView))
	&& 0 <= x && x <= 0 + BOX_SIZE)
    {
      pieceNum = 9 - (y / BOX_SIZE);
    }
    if (0 <= pieceNum && pieceNum < (int)osl::PieceStand::order.size())
    {
      osl::Ptype pt = osl::PieceStand::order[pieceNum];
      for (int i = 0; i < 40; i++)
      {
	const osl::Piece p = pieceOf(i);
	if (p.ptype() == pt && p.square() == osl::Square::STAND()
	    && p.owner() == state.turn())
	{
	  selectedPiece = p;
	  break;
	}
      }
    }
  }
  if (!selectedPiece.isEmpty())
  {
    mousePoint = e->pos();
    update();
  }
}

void gpsshogi::gui::
Board::mouseReleaseEvent(QMouseEvent *e)
{
  const int x = e->x() / scale_mult;
  const int y = e->y() / scale_mult;
  if (!selectedPiece.isEmpty())
  {
    osl::Square pos = getSquare(x, y);
    if (pos.isValid() && pos.isOnBoard() && pieceAt(pos).ptype() != osl::KING)
    {
      osl::Move mv = osl::Move::INVALID();
      osl::Move m(selectedPiece.square(), pos,
		  selectedPiece.ptype(),
		  pieceAt(pos).ptype(),
		  false, state.turn());
      if (state.isValidMove(m, false))
	mv = m;

      osl::Ptype piece = selectedPiece.ptype();
      if (osl::isBasic(piece) && piece != osl::KING && piece != osl::GOLD
	  && selectedPiece.square() != osl::Square::STAND()
	  && (((state.turn() == osl::BLACK && 1 <= pos.y() && pos.y() <= 3)
	      || (state.turn() == osl::WHITE
		  && 7 <= pos.y() && pos.y() <= 9))
	      || ((state.turn() == osl::BLACK
		   && 1 <= selectedPiece.square().y() && selectedPiece.square().y() <= 3)
		  || (state.turn() == osl::WHITE
		      && 7 <= selectedPiece.square().y() && selectedPiece.square().y() <= 9))))
      {
	osl::Move promote_move(selectedPiece.square(), pos,
			       osl::promote(selectedPiece.ptype()),
			       state.pieceAt(pos).ptype(),
			       true, state.turn());
	if (state.isValidMove(promote_move, false))
	{
	  // non-promote move is invalid
	  if (! mv.isNormal())
	    mv = promote_move;
	  else
	  {
	    int ret = QMessageBox::question(this, "Promote", "Promote?",
					    QMessageBox::Yes | QMessageBox::Default,
					    QMessageBox::No);
	    if (ret == QMessageBox::Yes)
	      mv = promote_move;
	  }
	}
      }
      if (mv.isNormal())
	move(mv);
    }
    selectedPiece = osl::Piece::EMPTY();
  }
  setCursor(QCursor());
  update();
}

void gpsshogi::gui::
Board::updateBook()
{
  static const osl::BookInMemory& book = osl::BookInMemory::instance();
  book_moves.clear();
  book.find(osl::HashKey(state), book_moves);
}

void gpsshogi::gui::
Board::move(osl::Move m)
{
  state.makeMove(m);  
  last_move = m;
  updateBook();
  
  emit moved(m);
  emit statusChanged();
  update();
}

void gpsshogi::gui::
Board::setState(const osl::NumEffectState& sstate, osl::Move l)
{
  if (!(state == sstate))
  {
    state = sstate;
    last_move = l;
    updateBook();

    update();
    emit statusChanged();
  }
}

void gpsshogi::gui::
Board::setView(bool sente)
{
  senteView = sente;
  update();
}

void gpsshogi::gui::
Board::toggleOrientation()
{
  senteView = !senteView;
  update();
}

osl::Piece gpsshogi::gui::Board::pieceOf(int i)
{
  return state.pieceOf(i);
}

osl::Piece gpsshogi::gui::Board::pieceAt(osl::Square pos)
{
  return state.pieceAt(pos);
}

int gpsshogi::gui::Board::countPiecesOnStand(osl::Player p, osl::Ptype ptype)
{
  return state.countPiecesOnStand(p, ptype);
}

osl::Piece gpsshogi::gui::Board::getReservePiece(osl::Ptype)
{
  return osl::Piece::EMPTY();
}

osl::Piece gpsshogi::gui::Board::getStandPiece(osl::Player p, osl::Ptype ptype)
{
  for (size_t i = 0; i < 40; ++i)
  {
    const osl::Piece piece = pieceOf(i);
    if (piece.ptype() == ptype &&
	!piece.square().isValid() && piece.square().isPieceStand() &&
	piece.owner() == p)
    {
      return piece;
    }
  }
  return osl::Piece::EMPTY();
}

struct gpsshogi::gui::Board::MoveSet
  : public std::unordered_set<osl::Move,std::hash<osl::Move>>
{
};
struct gpsshogi::gui::Board::HashSet
  : public std::unordered_set<osl::HashKey,std::hash<osl::HashKey>>
{
};

void gpsshogi::gui::Board::paintEvent(QPaintEvent *pe)
{
  QPainter painter(this);
  painter.scale(scale_mult, scale_mult);
  if (effect != NONE)
  {
    osl::NumEffectState nState(state);
    for (int x = 1; x <= 9; x++)
    {
      for (int y = 1; y <= 9; y++)
      {
	int r = 255;
	int g = 255;
	int b = 255;
	const osl::Square pos(x, y);
	int black_count = nState.countEffect(osl::BLACK, pos);
	int white_count = nState.countEffect(osl::WHITE, pos);

	if (effect != WHITE)
	  r = std::max(255 - black_count * 16, 0);
	if (effect != BLACK)
	  b = std::max(255 - white_count * 16, 0);
	g = std::max(255 - (black_count + white_count) * 16, 0);
	
	{
	  QColor color(r, g, b);
	  QPoint point = positionToPoint(pos);
	  QBrush brush(color);
	  painter.fillRect(point.x(), point.y(), BOX_SIZE, BOX_SIZE, brush);
	}
      }
    }
  }
  if (highlight_enabled && last_move.isNormal()) 
  {
    QPoint point = positionToPoint(last_move.to());
    QBrush brush(QColor(255, 230, 200));
    painter.fillRect(point.x(), point.y(), BOX_SIZE, BOX_SIZE, brush);
    painter.setPen(QPen(QColor("red"), 2));
    drawArrow(painter, last_move, false);
  }
  if (highlight_book)
  {
    for (osl::Move move: book_moves) {
      if (last_move.isNormal() && move.to() == last_move.to())
	continue;
      QPoint point = positionToPoint(move.to());
      QBrush brush(QColor(230, 255, 200));
      painter.fillRect(point.x(), point.y(), BOX_SIZE, BOX_SIZE, brush);
    }
    
    painter.setPen(QPen(QColor("green"), 2));
    for (osl::Move move: book_moves) {
      drawArrow(painter, move, false);
    }
  }
  if (table && arrow_enabled) 
  {
    using namespace osl::search;
    osl::HistoryState state(this->state);
    osl::HashKey key(this->state);
    MoveSet pv;
    HashSet visited;
    showRecursive(painter, state, key, pv, visited, 0);
  }
  painter.end();
  AbstractBoard::paintEvent(pe);
}

void gpsshogi::gui::
Board::showRecursive(QPainter& painter, osl::HistoryState& state,
		     const osl::HashKey& key,
		     MoveSet& pv, HashSet& visited, int level)
{
  const osl::search::SimpleHashRecord *record = table->find(key);
  if (! record || record->nodeCount() < 10 || (level>0 && ! visited.insert(key).second))
    return;
  osl::Move best_move = record->bestMove().move();
  if (! best_move.isNormal()) 
    return;
  if (level > 0) {
    drawArrow(painter, best_move);
    state.makeMove(best_move);
    showRecursive(painter, state, key.newMakeMove(best_move), pv, visited, level);
    state.unmakeMove();
  }

  if (++level >= 2)
    return;
  if (level == 1) {
    QPen wpen(QColor(230, 230, 255), 1);
    wpen.setStyle(Qt::DotLine);
    painter.setPen(wpen);
  }
  osl::MoveVector moves;
  static_cast<const osl::NumEffectState&>(state).generateLegal(moves);
  for (osl::Move move: moves) {
    if (move == best_move)
      continue;
    state.makeMove(move);
    showRecursive(painter, state, key.newMakeMove(move), pv, visited, level);
    state.unmakeMove();
  }

  if (--level == 0) {
    QPen bpen(QColor("blue"), 2);
    bpen.setStyle(Qt::SolidLine);
    painter.setPen(bpen);
    drawArrow(painter, best_move);
    state.makeMove(best_move);
    showRecursive(painter, state, key.newMakeMove(best_move), pv, visited, level);
    state.unmakeMove();
  }
}

#include <iostream>
void gpsshogi::gui::Board::drawArrow(QPainter& painter, osl::Move move,
				     bool randomness)
{
  if (! arrow_enabled)
    return;
  QPoint point = positionToPoint(move.to());
  const QPoint from = move.isDrop()
    ? standToPoint(move.player(), move.ptype())
    : positionToPoint(move.from());
  const int w = BOX_SIZE/8;
  int to_x = point.x() + BOX_SIZE/2 + (randomness ? random()%w : 0);
  int to_y = point.y() + BOX_SIZE/2 + (randomness ? random()%w : 0);
  std::complex<double> v(point.x() - from.x(), point.y() - from.y());
  const double distance = abs(v);
  v *= 1.0*BOX_SIZE/distance;
  const int dx = v.real(), dy = v.imag();
  to_x -= dx/8;
  to_y -= dy/8;
  int from_x = from.x()+BOX_SIZE/2+dx/8+(randomness ? random()%w : 0);
  int from_y = from.y()+BOX_SIZE/2+dy/8+(randomness ? random()%w : 0);
  if (distance > BOX_SIZE*2) {
    QRectF rect(std::min(to_x, from_x), std::min(to_y, from_y),
		std::max(BOX_SIZE,abs(to_x-from_x)),
		std::max(BOX_SIZE,abs(to_y - from_y)));
    QPainterPath path;
    path.moveTo(from_x, from_y);
    const int cx = (from_x+to_x)/2, cy = (from_y+to_y)/2;
    path.quadTo(cx+dy/2, cy-dx/2, to_x, to_y);
    painter.drawPath(path);
  }
  else {
    painter.drawLine(to_x, to_y, from_x, from_y);
  }
  painter.drawLine(to_x, to_y, to_x + dy/8 - dx/4, to_y - dx/8 - dy/4);
  painter.drawLine(to_x, to_y, to_x - dy/8 - dx/4, to_y + dx/8 - dy/4);
}

void gpsshogi::gui::Board::setTable(const osl::search::SimpleHashTable *t)
{
  table = t;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
