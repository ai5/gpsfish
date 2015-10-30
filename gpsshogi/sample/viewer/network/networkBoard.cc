#include "networkBoard.h"
#include <qstringlist.h>
#include <qregexp.h>

/* The format we will be expected:
 *
 * BEGIN Game_Summary
 * Protocol_Version:1.1
 * Protocol_Mode:Server
 * Format:Shogi 1.0
 * Declaration:Jishogi 1.1
 * Game_ID:wdoor+floodgate-900-0+gps_normal+simk+20120104210001
 * Name+:gps_normal
 * Name-:simk
 * Rematch_On_Draw:NO
 * To_Move:+
 * BEGIN Time
 * Time_Unit:1sec
 * Total_Time:900
 * Byoyomi:0
 * Least_Time_Per_Move:1
 * Remaining_Time+:397
 * Remaining_Time-:312
 * Last_Move:+6453UM,T20
 * Current_Turn:77
 * END Time
 * BEGIN Position
 * P1-KY *  *  *  *  *  *  * -KY
 * P2 *  *  *  *  *  * -KI-OU *
 * P3-FU *  *  * +UM-KI-KE-GI *
 * P4 *  *  *  *  * -GI * -FU *
 * P5 *  * +HI *  *  *  *  * -FU
 * P6 *  * +FU+FU+FU+FU * +KE *
 * P7+FU+FU *  *  *  * +FU+FU+FU
 * P8 *  * +KI *  * -FU+GI+OU *
 * P9+KY * +GI-UM * +KI * +KE+KY
 * P+00FU00FU00FU00FU00FU00HI
 * P-00KE
 * -
 * END Position
 * END Game_Summary
 */
NetworkBoard::
NetworkBoard(const QString& str)
{
  QStringList lines = str.split("\n");
  QRegExp line_re("(.+)\\:(.*)");

  QStringList::iterator line_it = lines.begin();

  //read headers
  for (;line_it != lines.end(); line_it++)
  {
    const QString& line = *line_it;
    if (line_re.indexIn(line) == -1)
    {
      if (line.indexOf("BEGIN Position") != -1) break;
    }
    values[line_re.cap(1)] = line_re.cap(2);
    header.append(line);
  }

  line_it++; //BEGIN Position

  //read board
  for (;line_it != lines.end(); line_it++)
  {
    const QString& line = *line_it;
    if (line.indexOf("END Position") != -1)
    {
      break;
    }
    board.append(line);
    board.append("\n");
  }
}

QString
NetworkBoard::
getBoard() const
{
  return board;
}

QString
NetworkBoard::
getValue(const QString& str) const
{
  return values[str];
}

QString
NetworkBoard::
getCsa() const
{
  QString csa;
  
  //append some information from header
  csa.append("'ID ");
  csa.append(values["Game_ID"]);
  csa.append("\nN+");
  csa.append(values["Name+"]);
  csa.append("\nN-");
  csa.append(values["Name-"]);
  csa.append("\n");
  csa.append(board);

  //a bug in shogi server 2005/11/4
  if (values["Last_Move"].indexOf("+") != -1)
  {
    csa.replace(csa.lastIndexOf('+'), 1, '-');
  }

  csa.append("'LAST ");
  csa.append(values["Last_Move"]);

  return csa;
}
