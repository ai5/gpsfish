#include "moveList.h"

#include "gpsshogi/gui/util.h"
#include <functional>
#include <numeric>

MoveList::MoveList(QWidget *parent)
  : QListWidget(parent), state(osl::SimpleState(osl::HIRATE))
{
}

size_t MoveList::numMoves() const
{
  return moves.size();
}

osl::Move MoveList::getMove(unsigned int n) const
{
  if (n < moves.size())
    return moves[n];
  else
    return osl::Move::INVALID();
}

int MoveList::getTime(unsigned int n) const
{
  if (n < timeList.size())
    return timeList[n];
  else
    return 0;
}

void MoveList::setState(const osl::SimpleState& initialState,
			const std::vector<osl::Move>& m,
			const std::vector<int>& t,
			const std::vector<QString>& c)
{
  clear();
  state = initialState;
  moves = m;
  timeList = t;
  const bool has_time = std::accumulate(t.begin(), t.end(), 0, std::plus<int>()) > 0;
  osl::NumEffectState s(state);
  for (unsigned int i = 0; i < m.size(); i++)
  {
    QString moveNumber;
    moveNumber.sprintf("%3d  ", i + 1);
    QString ms = (i == 0)
      ? gpsshogi::gui::Util::moveToString(m[i], s)
      : gpsshogi::gui::Util::moveToString(m[i], s, m[i-1]);
    QString text = moveNumber + ms;
    if (has_time)
      text += QString(" %1").arg(t[i], 6);
    if (i < c.size() && c[i].size() > 0)
      text += QString("*");
    new QListWidgetItem(text, this);
    s.makeMove(m[i]);
  }
}

void MoveList::pushMove(osl::Move move)
{
  moves.push_back(move);
  int time = 1;
  timeList.push_back(time);

  QString moveNumber;
  moveNumber.sprintf("%3d  ", (int)moves.size());
  new QListWidgetItem(moveNumber
                      + gpsshogi::gui::Util::moveToString(move)
                      + ",          " + QString("%1").arg(time),
                      this);
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
