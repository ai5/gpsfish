#include <qapplication.h>
#include <qhbox.h>
#include "mainWindow.h"
#include "networkClient.h"

int main (int argc, char **argv)
{
  QApplication app(argc, argv);

  MainWindow main;
  QObject::connect(&main, SIGNAL(quit()), &app, SLOT(quit()));

  NetworkClient network(&main);
  QObject::connect(&network, SIGNAL(gotBoard(QString)),
		   &main, SLOT(showBoard(QString)));
  QObject::connect(&network, SIGNAL(gotLastMove(QString)),
		   &network, SLOT(show_message(QString)));

  QHBox hbox(&main);
  QPushButton save("Save", &hbox);
  QObject::connect(&save, SIGNAL(clicked()), &main, SLOT(saveClicked()));
  QPushButton quit("Quit", &hbox);
  QObject::connect(&quit, SIGNAL(clicked()), &app, SLOT(quit()));

  app.setMainWidget(&main);
  main.show();
  return app.exec();
}
