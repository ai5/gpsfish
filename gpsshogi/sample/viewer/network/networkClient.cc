#include "networkClient.h"
#include <qmessagebox.h>
#include <qcolor.h>
#include <qdatetime.h>
#include "networkBoard.h"
#include <qlayout.h>

#include <stdlib.h>

/* GUI Layout
 */
NetworkClient::
NetworkClient(QWidget *parent)
  : QWidget(parent),
    chat_re("^##\\[([A-Z]+)\\]\\[([A-z0-9\\@\\_\\-\\.]+)\\](.+)"),
    monitor_re("^##\\[([A-Z]+)\\]\\[([A-z0-9\\@\\_\\+\\-\\.]+)\\] (.+)"),
    command_re("^##\\[([A-Z]+)\\] (.+)"),
    who_re("(.+) x1 (.+)")
{
  /* list box */
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QHBoxLayout *usersLayout = new QHBoxLayout;
  QLabel *lbu = new QLabel(tr("USERS:"), this);
  who = new QComboBox(this);
  who->setObjectName("Users");
  who->insertItem(-1, "(Not logged in)");
  connect(this, SIGNAL(gotWho(QString)),
	  SLOT(update_who(QString)));
  usersLayout->addWidget(lbu);
  usersLayout->addWidget(who, 1);
  mainLayout->addLayout(usersLayout);

  QHBoxLayout *gamesLayout = new QHBoxLayout;
  QLabel *lbg = new QLabel(tr("GAMES:"), this);
  games = new QComboBox(this);
  games->setObjectName("Games");
  games->insertItem(-1, "(Not logged in)");
  connect(this, SIGNAL(gotGames(QString)),
	  SLOT(update_games(QString)));
  gamesLayout->addWidget(lbg);
  gamesLayout->addWidget(games, 1);
  mainLayout->addLayout(gamesLayout);

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  update = new QPushButton(tr("UPDATE"), this);
  update->setEnabled(false);

  connect(update, SIGNAL(clicked()),
	  SLOT(updateList()));
  shownow = new QPushButton(tr("SHOW"), this);
  connect(shownow, SIGNAL(clicked()),
	  SLOT(show_game()));
  shownow->setEnabled(false);

  monitor = new QCheckBox("MONITOR", this);
  connect(monitor, SIGNAL(clicked()),
	  SLOT(monitor_game()));
  monitor->setEnabled(false);
  buttonLayout->addWidget(update);
  buttonLayout->addWidget(shownow);
  buttonLayout->addWidget(monitor);
  mainLayout->addLayout(buttonLayout);

  info_text = new QTextEdit(this);
  info_text->setReadOnly(true);
  mainLayout->addWidget(info_text);

  /* host box */
  QHBoxLayout *hostLayout = new QHBoxLayout;
  QLabel *host = new QLabel(tr("HOST:"), this);
  hostname = new QLineEdit("wdoor.c.u-tokyo.ac.jp:4081", this);
  hostname->setMinimumWidth(180);
  const char *user = getenv("USER");
  username = new QLineEdit(user ? user : "", this);
  password = new QLineEdit("", this);
  password->setEchoMode(QLineEdit::Password);

  connect(hostname, SIGNAL(returnPressed()),
	  SLOT(toggleConnection()));
  connect(username, SIGNAL(returnPressed()),
	  SLOT(toggleConnection()));
  connect(password, SIGNAL(returnPressed()),
	  SLOT(toggleConnection()));

  toggle = new QPushButton(tr(" OPEN "), this);
  connect(toggle, SIGNAL(clicked()),
	  SLOT(toggleConnection()));

  hostLayout->addWidget(host);
  hostLayout->addWidget(hostname);
  hostLayout->addWidget(username);
  hostLayout->addWidget(password);
  hostLayout->addWidget(toggle);
  mainLayout->addLayout(hostLayout);

  /* message box */
  QHBoxLayout *chatLayout = new QHBoxLayout;
  QLabel *chat = new QLabel(tr("CHAT:"), this);
  input = new QLineEdit(this);
  QPushButton *send = new QPushButton(tr("SEND") , this);
  connect(input, SIGNAL(returnPressed()), SLOT(send_chat()));
  connect(send, SIGNAL(clicked()), SLOT(send_chat()));
  chatLayout->addWidget(chat);
  chatLayout->addWidget(input);
  chatLayout->addWidget(send);
  mainLayout->addLayout(chatLayout);

  /* socket */
  socket = new QTcpSocket();
  connected = false;
  monitoring = false;
  last_player = "*";

  connect(socket, SIGNAL(connected()),
	  SLOT(socketConnected()));
  connect(socket, SIGNAL(disconnected()),
	  SLOT(socketConnectionClosed()));
  connect(socket, SIGNAL(readyRead()),
	  SLOT(socketReadyRead()));
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
	  SLOT(socketError(QAbstractSocket::SocketError)));
}

void NetworkClient::
setHostname(QString str)
{
  if (connected) return;
  hostname->setText(str);
}

void NetworkClient::
setUsername(QString str)
{
  if (connected) return;
  username->setText(str);
}

void NetworkClient::
setPassword(QString str)
{
  if (connected) return;
  password->setText(str);
}

void NetworkClient::
openConnection()
{
  /* check if should connect
   */
  if (connected)
  {
    QMessageBox::warning(this, "QT Shogi Client", "Already connected!");
    return;
  }

  bool error = false;
  QString message;
  QString host_str;
  QString port_str = "4081";

  if (hostname->text().isEmpty())
  {
    error = true;
    message.append(tr("Server should be given in host:port\n"));
  }

  if (!error)
  {
    const QStringList host_and_port = hostname->text().trimmed().split(":");
    host_str = host_and_port[0];
    if (host_and_port.size() == 2)
    {
      port_str = host_and_port[1];
    }
    else if (host_and_port.size() > 2)
    {
      error = true;
      message.append(tr("Server should be given in host:port\n"));
    }
  }

  QRegExp invalid_str("\\s|^$");
  if (!error &&
      invalid_str.indexIn(username->text()) != -1)
  {
    error = true;
    message.append(tr("Invalid username %1\n").arg(username->text()));
  }

  if (!error &&
      invalid_str.indexIn(password->text()) != -1)
  {
    error = true;
    message.append(tr("Invalid password\n"));
  }

  if (error)
  {
    QMessageBox::warning(this, "QT Shogi Client",
			 message);
    return;
  }

  int port_num = port_str.toInt();

  info_text->append(tr("Trying %1 at port %2...")
		    .arg(host_str)
		    .arg(port_num));
  /* opening connection
   */
  hostname->setReadOnly(true);
  username->setReadOnly(true);
  password->setReadOnly(true);
  {
    QPalette palette;
    palette.setColor(hostname->backgroundRole(), Qt::darkGray);
    hostname->setPalette(palette);
  }
  {
    QPalette palette;
    palette.setColor(username->backgroundRole(), Qt::darkGray);
    username->setPalette(palette);
  }
  {
    QPalette palette;
    palette.setColor(password->backgroundRole(), Qt::darkGray);
    password->setPalette(palette);
  }

  connected = true;
  retr_who = false;
  retr_games = false;
  socket->connectToHost(host_str, port_num);
  toggle->setText("CLOSE");
  update->setEnabled(true);
  shownow->setEnabled(true);
  monitor->setEnabled(true);
}

void NetworkClient::
closeConnection()
{
  /* closing connection
   */
  
  QTextStream os(socket);
  os << "LOGOUT\n";
  socket->close();

  if (socket->state() == QAbstractSocket::ClosingState)
  {
    // We have a delayed close.
    connect(socket, SIGNAL(delayedCloseFinished()),
	    SLOT(socketClosed()));
  }
  else
  {
    // The socket is closed.
    socketClosed();
  }
}

void NetworkClient::
toggleConnection()
{
  if (connected)
  {
    closeConnection();
  }
  else
  {
    openConnection();
  }
}

void NetworkClient::
updateList()
{
  if (!connected)
  {
    info_text->append("NOT CONNECTED");
    return;
  }
  QTextStream os(socket);
  os << "%%LIST\n%%WHO\n";
}

void NetworkClient::
show(QString gamename)
{
  QTextStream os(socket);

  os << "%%SHOW " << gamename << "\n";
}

static bool
handle_message(bool& reading,
	       QString& mes,
	       const QString& arg)
{
  if (!reading)
  {
    reading = true;
    mes = "";
  }

  if (arg == "+OK")
  {
    reading = false;
    return true;
  }

  mes.append(arg + "\n");
  return false;
}

void NetworkClient::
socketReadyRead()
{
  // read from the server
  while (socket->canReadLine())
  {
    QString line = QString::fromUtf8(socket->readLine());
    // assuming we get LF as newline
    line.truncate(line.length() - 1);
    QTime now = QTime::currentTime();

    if (chat_re.indexIn(line) != -1)
    {
      info_text->append(QString("%1 %2 %3")
			.arg(now.toString("hh:mm:ss"))
			.arg(chat_re.cap(2))
			.arg(chat_re.cap(3)));
      emit chatReceived();
    }
    else if (monitor_re.indexIn(line) != -1)
    {
      if (handle_message(retr_board, board_str, monitor_re.cap(3)))
      {
	NetworkBoard board(board_str);

	if (!monitoring)
	{
	  gotBoard(board.getCsa());
	  monitoring = true;
	}
	else
	{
	  QString last_move = board.getValue("Last_Move");

	  if (last_move[0] == last_player[0])
	  {
	    monitoring = false;
	    monitor->setChecked(false);
	    last_player = "*";
	  }
	  else
	  {
	    last_player = last_move;
	    gotLastMove(last_move);
	  }
	}
	  
      }
    }
    else if (command_re.indexIn(line) != -1)
    {
      handle_command(command_re.cap(1),
		     command_re.cap(2));
    }
    else
    {
      info_text->append(tr("SYSTEM:%1").arg(line));
    }
  }
}

void NetworkClient::
socketConnected()
{
  info_text->append(tr("Connected\n"));
  QTextStream os(socket);
  os << QString("LOGIN %1 %2 x1\n")
    .arg(username->text())
    .arg(password->text());
}

void NetworkClient::
socketConnectionClosed()
{
  info_text->append(tr("Connection closed by the server\n"));
  socketClosed();
}

void NetworkClient::
socketClosed()
{
  connected = false;
  toggle->setText(" OPEN ");

  hostname->setReadOnly(false);
  username->setReadOnly(false);
  password->setReadOnly(false);
  update->setEnabled(false);
  shownow->setEnabled(false);
  monitor->setEnabled(false);
  {
    QPalette palette;
    palette.setColor(hostname->backgroundRole(), Qt::white);
    hostname->setPalette(palette);
  }
  {
    QPalette palette;
    palette.setColor(username->backgroundRole(), Qt::white);
    username->setPalette(palette);
  }
  {
    QPalette palette;
    palette.setColor(password->backgroundRole(), Qt::white);
    password->setPalette(palette);
  }

  info_text->append(tr("Connection closed\n"));
}

void NetworkClient::
socketError(QAbstractSocket::SocketError e)
{
  info_text->append(tr("Socket error %1\n").arg(e));
}

void NetworkClient::
send_chat()
{
  if (!connected)
  {
    info_text->append("NOT CONNECTED");
    return;
  }

  // write to the server
  QTextStream os(socket);
  os.setCodec("UTF-8");
  QString line = input->text();

  if (line.startsWith("%"))
  {
    os << line << "\n";
  }
  else
  {
    os << "%%CHAT "<< line << "\n";
  }
  input->setText("");
}

void NetworkClient::
show_game()
{
  if (!connected)
  {
    info_text->append("NOT CONNECTED");
    return;
  }
  if (games->currentText().isEmpty())
  {
    info_text->append("GAME NOT SELECTED");
    return;
  }
  show(games->currentText());
}

void NetworkClient::
monitor_game()
{
  QTextStream os(socket);

  if (!connected)
  {
    info_text->append("NOT CONNECTED");
    return;
  }

  if (!monitor->isChecked())
  {
    os << "%%MONITOROFF " << monitoring_game << "\n";
    monitoring = false;
  }
  else
  {
    monitoring_game = games->currentText();
    os << "%%MONITORON " << monitoring_game << "\n";
    monitoring = false;
  }
}

void NetworkClient::
show_message(QString str)
{
  info_text->append(str);
}

void NetworkClient::
update_who(QString str)
{
  who->clear();
  who->insertItems(-1, str.split("\n"));
}

void NetworkClient::
update_games(QString str)
{
  games->clear();
  games->insertItems(-1, str.split("\n"));
}

void NetworkClient::
handle_command(QString com,
	       QString arg)
{
  if (com == "LOGIN")
  {
    if (arg != "+OK x1")
    {
      QMessageBox::warning(this, "Login failed",
			   tr("Login failed: got'%1'").arg(arg),
			   QMessageBox::Ok,
			   QMessageBox::NoButton,
			   QMessageBox::NoButton);
      socketClosed();
    }
    info_text->append(tr("Login as %1\n").arg(username->text()));
    updateList();
  }
  else if (com == "ERROR")
  {
    info_text->append(tr("ERROR %1").arg(arg));
  }
  else if (com == "WHO")
  {
    if (handle_message(retr_who, who_str, arg))
    {
      gotWho(who_str);
    }
  }
  else if (com == "LIST")
  {
    if (handle_message(retr_games, games_str, arg))
    {
      gotGames(games_str);
    }
  }
  else if (com == "SHOW")
  {
    if (handle_message(retr_board, board_str, arg))
    {
      NetworkBoard board(board_str);
      gotBoard(board.getCsa());
    }
  }
  else
  {
    info_text->append(tr("UNKNOWN COMMAND '%1' with args '%2'").arg(com).arg(arg));
  }
}
