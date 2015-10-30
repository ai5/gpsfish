#ifndef _MOVELIST_H
#define _MOVELIST_H
#include <qglobal.h>
#include <QListWidget>
#include "osl/simpleState.h"
#include <vector>

class MoveList : public QListWidget
{
Q_OBJECT
public:
  MoveList(QWidget *parent = 0);
  void setState(const osl::SimpleState& initialState,
		const std::vector<osl::Move>& m,
		const std::vector<int>& t,
		const std::vector<QString>&);
  osl::Move getMove(unsigned int n) const;
  void pushMove(osl::Move move);
  int getTime(unsigned int n) const;
  size_t numMoves() const;
private:
  osl::SimpleState state;
  std::vector<osl::Move> moves;
  std::vector<int> timeList;
};

#endif // _MOVELIST_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
