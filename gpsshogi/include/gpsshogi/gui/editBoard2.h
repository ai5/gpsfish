#ifndef GPSSHOGI_GUI_EDITBOARD2_H
#define GPSSHOGI_GUI_EDITBOARD2_H

#include "gpsshogi/gui/abstractBoard.h"
#include "osl/numEffectState.h"

class QKeyEvent;

namespace gpsshogi
{
  namespace gui
  {
    class EditBoard2 : public AbstractBoard
    {
      Q_OBJECT
    public:
      EditBoard2(QWidget *parent = 0);
      // throws an exception
      osl::SimpleState getState();
      void setState(const osl::SimpleState &state);
      void moveReserve(osl::Player, osl::Ptype, osl::Square);
      void reserveToStand(osl::Player);
      void setTurn(osl::Player);
    public slots:
      void clearState();
    protected:
      virtual osl::Piece pieceOf(int i);
      virtual int countPiecesOnStand(osl::Player, osl::Ptype);
      virtual osl::Piece pieceAt(osl::Square);
      virtual osl::Piece getReservePiece(osl::Ptype);
      virtual osl::Piece getStandPiece(osl::Player, osl::Ptype);
      void mousePressEvent(QMouseEvent *);
      void mouseReleaseEvent(QMouseEvent *);
      void keyPressEvent(QKeyEvent *);
      void keyReleaseEvent(QKeyEvent *);
    private:
      template<osl::Ptype ptype> void initPieces();
      osl::CArray<osl::Piece, osl::Square::SIZE> board;
      osl::CArray<osl::Piece, osl::Piece::SIZE> pieces;
      int modifiers;
      osl::Player turn;
    };
  }
}

#endif // GPSSHOGI_GUI_EDITBOARD2_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
