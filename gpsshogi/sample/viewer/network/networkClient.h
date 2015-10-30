#ifndef _NETWORK_CLIENT_H
#define _NETWORK_CLIENT_H
#include <qglobal.h>
#include <QTcpSocket>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qregexp.h>

class NetworkClient : public QWidget
{
  Q_OBJECT

 public:
  NetworkClient(QWidget *parent = 0);

 public slots:
  void setHostname(QString);
  void setUsername(QString);
  void setPassword(QString);
  void openConnection();
  void closeConnection();
  void toggleConnection();
  void updateList();
  void show(QString);

 signals:
  void gotWho(QString);
  void gotGames(QString);
  void gotBoard(QString);
  void gotLastMove(QString);
  void chatReceived();

 private slots:
  void socketReadyRead();
  void socketConnected();
  void socketConnectionClosed();
  void socketClosed();
  void socketError(QAbstractSocket::SocketError e);

  void send_chat();
  void show_game();
  void monitor_game();
  void show_message(QString);
  void update_who(QString);
  void update_games(QString);

private:
  bool connected, monitoring, retr_who, retr_games, retr_board;

  void handle_command(QString com, QString arg);

  QTcpSocket *socket;
  QString who_str, games_str, board_str;
  QString monitoring_game, last_player;
  QComboBox *who, *games;
  QTextEdit *info_text;
  QLineEdit *input;
  QLineEdit *hostname, *username, *password;
  QPushButton *toggle, *update, *shownow;
  QCheckBox *monitor;
  QRegExp chat_re, monitor_re, command_re, who_re;
};

#endif // _NETWORK_CLIENT_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
