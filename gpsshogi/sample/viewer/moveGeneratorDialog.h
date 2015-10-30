#ifndef _MOVE_GENERATOR_DIALOG_H
#define _MOVE_GENERATOR_DIALOG_H
#include <qdialog.h>
#include "osl/simpleState.h"
#include <vector>

class QSpinBox;
class QTableView;
class MoveGeneratorDialog : public QDialog
{
Q_OBJECT
public:
  MoveGeneratorDialog(const osl::SimpleState &state,
		      const std::vector<osl::Move> &moves,
		      int limit, 
		      osl::Move next = osl::Move::INVALID(),
		      QWidget *parent = 0);
public slots:
  void setStatus(const osl::SimpleState &state,
		 const std::vector<osl::Move> &moves,
		 int limit, osl::Move next_move);
private slots:
  void setLimit(int limit);
private:
  void updateMoves();
  QTableView *view;
  QSpinBox *spinBox;
  int limit;
  osl::SimpleState state;
  std::vector<osl::Move> moves;
  osl::Move next_move;
};

#endif // _MOVE_GENERATOR_DIALOG_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
