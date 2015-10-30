#ifndef GPSSHOGI_GUI_BOARD_H
#define GPSSHOGI_GUI_BOARD_H
#include "gpsshogi/gui/abstractBoard.h"
#include "osl/numEffectState.h"
#include "osl/hashKey.h"

namespace osl
{
  namespace search
  {
    class SimpleHashTable;
  }
  namespace state
  {
    class HistoryState;
  }
}

namespace gpsshogi
{
  namespace gui
  {
    class Board : public AbstractBoard
    {
      Q_OBJECT
    public:
      Board(const osl::NumEffectState& state, QWidget *parent = 0);
      void toggleOrientation();
      void setView(bool sente);
      const osl::NumEffectState& getState() const {
	return state;
      }
      void setManualMovement(bool enable) {
	allowManualMovement = enable;
      }
      bool isSenteView() const {
	return senteView;
      }
      enum EffectType { NONE, BLACK, WHITE, BOTH };
      void setEffectType(EffectType type) {
	effect = type;
	update();
      }
      EffectType getEffectType() const {
	return effect;
      }
      void highlightLastMove(bool on) { 
	highlight_enabled = on; 
	update();
      }
      void highlightBookMove(bool on) { 
	highlight_book = on; 
	updateBook();
	update();
      }
      void showArrowMove(bool on) { 
	arrow_enabled = on; 
	update();
      }
      void setTable(const osl::search::SimpleHashTable *);
    protected:
      void paintEvent(QPaintEvent *);
      void mousePressEvent(QMouseEvent *);
      void mouseReleaseEvent(QMouseEvent *);
      void updateBook();
      void drawArrow(QPainter&, osl::Move m, bool random=true);
      struct MoveSet;
      struct HashSet;
      void showRecursive(QPainter&, osl::state::HistoryState&, const osl::HashKey&,
			 MoveSet& pv, HashSet& visited, int level);
    public slots:
      void move(osl::Move m);
      void setState(const osl::NumEffectState& state, osl::Move last_move=osl::Move());
    signals:
      void moved(osl::Move);
      void statusChanged();
    protected:
      virtual osl::Piece pieceOf(int i);
      virtual int countPiecesOnStand(osl::Player, osl::Ptype);
      virtual osl::Piece pieceAt(osl::Square);
      virtual osl::Piece getReservePiece(osl::Ptype);
      virtual osl::Piece getStandPiece(osl::Player, osl::Ptype);
      virtual bool reversed() const {
	return !senteView;
      }
      bool senteView;
      osl::NumEffectState state;
    private:
      bool allowManualMovement;
      osl::Move last_move;
      EffectType effect;
      osl::MoveVector book_moves;
      const osl::search::SimpleHashTable *table;
      static bool highlight_enabled, highlight_book, arrow_enabled;
    };
  }
}
#endif // GPSSHOGI_GUI_BOARD_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
