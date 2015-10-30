/* network/mainWindow.h
 */
#ifndef _NETWORK_MAIN_WINDOW
#define _NETWORK_MAIN_WINDOW

#include <qpushbutton.h>
#include <qvbox.h>

class MainWindow : public QVBox 
{
  Q_OBJECT

 public:
  MainWindow();
  ~MainWindow();

 public slots:
  void showBoard(QString);
  void saveClicked(void);

 signals:
  void quit(void);

 private slots:
  void quitClicked(void);

 private:
  void setupMenu();

  QString board;
};

#endif // _NETWORK_MAIN_WINDOW
