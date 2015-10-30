#ifndef GPSSHOGI_GUI_ABSTRACT_BOARD_H
#define GPSSHOGI_GUI_ABSTRACT_BOARD_H
#include <QWidget>
#include <QPixmap>
#include <QMouseEvent>

#include "osl/basic_type.h"

class QResizeEvent;

namespace gpsshogi
{
  namespace gui
  {
    class AbstractBoard : public QWidget
    {
      Q_OBJECT
    public:
      AbstractBoard(QWidget *parent = 0);
      QSize sizeHint() const;
      QSize minimumSize() const;
    protected:
      virtual void paintEvent(QPaintEvent *);
      virtual void resizeEvent(QResizeEvent *);
    signals:
      void statusChanged();
    protected:
      virtual void paintPiece(QPainter *painter, int x, int y, osl::Piece piece);
      virtual osl::Piece pieceOf(int i) = 0;
      virtual int countPiecesOnStand(osl::Player, osl::Ptype) = 0;
      virtual osl::Piece pieceAt(osl::Square) = 0;
      virtual osl::Piece getReservePiece(osl::Ptype) = 0;
      virtual osl::Piece getStandPiece(osl::Player, osl::Ptype) = 0;
      virtual bool reversed() const {
	return false;
      }
      virtual void mouseMoveEvent(QMouseEvent *);
      const osl::Square getSquare(int x, int y) const;
      const osl::Piece getPiece(int x, int y);
      const QPoint positionToPoint(const osl::Square position) const;
      const QPoint standToPoint(osl::Player player, osl::Ptype ptype) const;
      const QPixmap pieceToImage(osl::Piece) const;
      const QString pieceString(osl::Piece) const;
      osl::Piece selectedPiece;
      QPoint mousePoint;
      qreal scale_mult;
      static const int STAND_SIZE;
      static const int BOX_SIZE;
      static const int BLACK_STAND_X;
      static const int MARGIN_HEAD;

      bool isOnBoard(int x, int f) const;
    };
  }
}
#endif // GPSSHOGI_GUI_ABSTRACT_BOARD_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
