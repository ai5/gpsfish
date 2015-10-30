/* networkBoard.h
 */
#ifndef _NETWORK_NETWORK_BOARD
#define _NETWORK_NETWORK_BOARD
#include <qstring.h>
#include <qmap.h>

class NetworkBoard
{
  QString header;
  QString board;

  QMap<QString, QString> values;

 public:
  NetworkBoard(const QString& str);

  QString getBoard() const;
  QString getValue(const QString& key) const;
  QString getCsa() const;
};
#endif //_NETWORK_NETWORK_BOARD
