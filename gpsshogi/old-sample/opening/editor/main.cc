#include <qapplication.h>
#include "mainwindow.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
#if QT_VERSION > 0x040000
  Q_INIT_RESOURCE(gui);
#endif
  MainWindow mainWin;
  app.setMainWidget(&mainWin);
  mainWin.show();
  return app.exec();
}
