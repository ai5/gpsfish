#include "gpsshogi/gui/editBoard2.h"
#include "osl/bits/ptypeTraits.h"
#include "osl/csa.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include <sstream>

gpsshogi::gui::EditBoard2::EditBoard2(QWidget *parent)
  : AbstractBoard(parent), modifiers(Qt::NoModifier), turn(osl::BLACK)
{
  initPieces<osl::PAWN>();
  initPieces<osl::LANCE>();
  initPieces<osl::KNIGHT>();
  initPieces<osl::SILVER>();
  initPieces<osl::GOLD>();
  initPieces<osl::BISHOP>();
  initPieces<osl::ROOK>();
  initPieces<osl::KING>();
  board.fill(osl::Piece::EMPTY());
  setFocusPolicy(Qt::ClickFocus);
}

template<osl::Ptype ptype>
void gpsshogi::gui::EditBoard2::initPieces()
{
  for (int i = osl::PtypeTraits<ptype>::indexMin;
       i < osl::PtypeTraits<ptype>::indexLimit;
       ++i)
  {
    // Reserve にある駒の owner は関係ないけど、一応半分に分けておく
    // 駒の数は偶数なのでちゃんと分かれる。
    pieces[i] =
      osl::Piece(i < (osl::PtypeTraits<ptype>::indexLimit +
		      osl::PtypeTraits<ptype>::indexMin) ?
		 osl::BLACK : osl::WHITE,
		 ptype, i, osl::Square(0, 0));
  }
}

osl::Piece gpsshogi::gui::EditBoard2::pieceOf(int i)
{
  return pieces[i];
}

osl::Piece gpsshogi::gui::EditBoard2::pieceAt(osl::Square pos)
{
  return board[pos.index()];
}

osl::Piece gpsshogi::gui::EditBoard2::getReservePiece(osl::Ptype ptype)
{
  for (size_t i = 0; i < pieces.size(); ++i)
  {
    if (pieces[i].ptype() == osl::unpromote(ptype) &&
	!pieces[i].square().isValid())
    {
      return pieces[i];
    }
  }
  return osl::Piece::EMPTY();
}
osl::Piece gpsshogi::gui::EditBoard2::getStandPiece(osl::Player p, osl::Ptype ptype)
{
  for (size_t i = 0; i < pieces.size(); ++i)
  {
    if (pieces[i].ptype() == ptype &&
	pieces[i].square().isValid() && pieces[i].square().isPieceStand() &&
	pieces[i].owner() == p)
    {
      return pieces[i];
    }
  }
  return osl::Piece::EMPTY();
}

int gpsshogi::gui::EditBoard2::countPiecesOnStand(osl::Player p, osl::Ptype ptype)
{
  int count = 0;
  for (int i = 0; i < osl::Piece::SIZE; ++i)
  {
    if (pieces[i].square().isValid() &&
	pieces[i].square().isPieceStand() &&
	pieces[i].owner() == p &&
	pieces[i].ptype() == ptype)
      ++count;
  }
  return count;
}

void gpsshogi::gui::EditBoard2::mousePressEvent(QMouseEvent *e)
{
  if (e->button() != Qt::LeftButton || modifiers != Qt::NoModifier)
    return;
  const int x = e->x() / scale_mult;
  const int y = e->y() / scale_mult;
  osl::Piece piece = getPiece(x, y);
  selectedPiece = piece;
  if (!selectedPiece.isEmpty())
  {
    mousePoint = e->pos();
    update();
  }
}

void gpsshogi::gui::EditBoard2::mouseReleaseEvent(QMouseEvent *e)
{
  const int x = e->x() / scale_mult;
  const int y = e->y() / scale_mult;

  if (!selectedPiece.isEmpty())
  {
    osl::Square pos = getSquare(x, y);
    if (pos.isValid() && pos.isOnBoard() && pieceAt(pos).isEmpty())
    {
      // 盤上
      if (selectedPiece.square().isValid() && selectedPiece.isOnBoard())
      {
	board[selectedPiece.square().index()] = osl::Piece::EMPTY();
      }
      // number がリセットされるみたい
      // selectedPiece.setSquare(pos);
      osl::Piece piece(selectedPiece.owner(), selectedPiece.ptype(),
		       selectedPiece.number(), pos);
      pieces[selectedPiece.number()] = piece;
      board[pos.index()] = piece;
    }
    // 駒台
    else if ((x >= BLACK_STAND_X || x <= STAND_SIZE) &&
	     selectedPiece.ptype() != osl::KING)
    {
      if (selectedPiece.square().isValid() && selectedPiece.isOnBoard())
      {
	board[selectedPiece.square().index()] = osl::Piece::EMPTY();
      }
      osl::Piece piece(x >= BLACK_STAND_X ? osl::BLACK : osl::WHITE,
		       osl::unpromote(selectedPiece.ptype()),
		       selectedPiece.number(), osl::Square::STAND());
      pieces[selectedPiece.number()] = piece;
    }
  }
  // 成る
  else if (e->button() == Qt::MidButton ||
	   (e->button() == Qt::LeftButton && modifiers & Qt::SHIFT))
  {
    osl::Square pos = getSquare(x, y);
    if (pos.isValid() && pos.isOnBoard() && !pieceAt(pos).isEmpty())
    {
      const osl::Piece p = pieceAt(pos);
      if (p.ptype() != osl::GOLD && p.ptype() != osl::KING)
      {
	const osl::Piece piece =
	  p.ptype() == osl::unpromote(p.ptype()) ? p.promote() :
	  p.unpromote();
	pieces[piece.number()] = piece;
	board[pos.index()] = piece;
      }
    }
  }
  // 反転
  else if (e->button() == Qt::RightButton)
  {
    osl::Square pos = getSquare(x, y);
    if (pos.isValid() && pos.isOnBoard() && !pieceAt(pos).isEmpty())
    {
      const osl::Piece p = pieceAt(pos);
      const osl::Piece piece(osl::alt(p.owner()), p.ptype(),
			     p.number(), p.square());
      pieces[piece.number()] = piece;
      board[pos.index()] = piece;
    }
  }
  selectedPiece = osl::Piece::EMPTY();
  setCursor(QCursor());
  update();
}


void gpsshogi::gui::EditBoard2::keyPressEvent(QKeyEvent *e)
{
  modifiers = e->modifiers();
}

void gpsshogi::gui::EditBoard2::keyReleaseEvent(QKeyEvent *e)
{
  modifiers = e->modifiers();
}

osl::SimpleState gpsshogi::gui::EditBoard2::getState()
{
  std::ostringstream sstream;
  for (int y = 1; y <= 9; ++y)
  {
    sstream << "P" << y;
    for (int x = 1; x <= 9; ++x)
    {
      sstream << osl::csa::show(pieceAt(osl::Square(10 - x, y)));
    }
    sstream << std::endl;
  }
  std::ostringstream black_stand, white_stand;
  for (int i = pieces.size() - 1; i >= 0; --i)
  {
    if (pieces[i].square().isValid() && pieces[i].square().isPieceStand())
    {
      if (pieces[i].owner() == osl::BLACK)
	black_stand << "00" << osl::csa::show(pieces[i].ptype());
      else
	white_stand << "00" << osl::csa::show(pieces[i].ptype());
    }
  }
  if (!black_stand.str().empty())
  {
    sstream << "P+" << black_stand.str() << std::endl;
  }
  if (!white_stand.str().empty())
  {
    sstream << "P-" << white_stand.str() << std::endl;
  }
  sstream << (turn == osl::BLACK ? "+" : "-") << std::endl;
  const std::string &board = sstream.str();
  osl::CsaString csa(board.c_str());
  try
  {
    return csa.initialState();
  }
  catch (osl::CsaIOError &)
  {
    return osl::SimpleState(osl::HIRATE);
  }
}

void gpsshogi::gui::EditBoard2::setState(const osl::SimpleState &state)
{
  for (int i = 0; i < (int)board.size(); ++i)
  {
    // Edge は使わないので気にしない。
    board[i] = osl::Piece::EMPTY();
  }

  for (int i = 0; i < 40; i++)
  {
    const osl::Piece p = state.pieceOf(i);
    pieces[i] = p;
    if (p.isOnBoard())
    {
      board[p.square().index()] = p;
    }
  }
}

void gpsshogi::gui::EditBoard2::clearState()
{
  for (int i = 0; i < 40; ++i)
  {
    if (pieces[i].square().isValid() && pieces[i].square().isOnBoard())
    {
      board[pieces[i].square().index()] = osl::Piece::EMPTY();
    }
    pieces[i] = osl::Piece(pieces[i].owner(), osl::unpromote(pieces[i].ptype()),
			   pieces[i].number(), osl::Square(0, 0));
  }
  repaint();
}

void gpsshogi::gui::EditBoard2::moveReserve(osl::Player player,
					    osl::Ptype ptype,
					    osl::Square position)
{
  osl::Piece piece = getReservePiece(ptype);
  if (!piece.isEmpty() && pieceAt(position).isEmpty())
  {
    if (position.isPieceStand())
    {
      ptype = osl::unpromote(ptype);
    }
    const osl::Piece p = osl::Piece(player, ptype, piece.number(),
				    position);
    pieces[piece.number()] = p;
    if (position.isOnBoard())
    {
      board[position.index()] = p;
    }
    update();
  }
}

void gpsshogi::gui::EditBoard2::reserveToStand(osl::Player player)
{
  for (int i = 0; i < (int)pieces.size(); ++i)
  {
    if (!pieces[i].square().isValid() && pieces[i].ptype() != osl::KING)
    {
      pieces[i] = osl::Piece(player, pieces[i].ptype(), i, osl::Square::STAND());
    }
  }
  update();
}

void gpsshogi::gui::EditBoard2::setTurn(osl::Player player)
{
  turn = player;
}
