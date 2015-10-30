#ifndef _NETWORK_VIEWER_H
#define _NETWORK_VIEWER_H
#include "boardTabChild.h"
#include "network/networkClient.h"

class MoveList;

class NetworkViewer : public BoardTabChild
{
  Q_OBJECT
  NetworkClient* networkClient;

public:
  NetworkViewer(QWidget *parent = 0);
  void forward();
  void backward();
  void toLastState();
  int moveCount() const;
  osl::NumEffectState getStateAndMovesToCurrent(std::vector<osl::Move>&);
  MoveList *moveList;

signals:
  void chatReceived();
  void painted();

public slots:
  void setHostname(QString);
  void setUsername(QString);
  void setPassword(QString);
  void connect();

protected:
  virtual void paintEvent(QPaintEvent *event);

private slots:
  void receive_board(QString);
  void receive_last_move(QString);

private:
  int index;

  void updateState();
};
#endif // _NETWORK_VIEWER_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
