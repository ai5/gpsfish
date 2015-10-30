#include "mainwindow.h"
#include "gpsshogi/revision.h"
#include "osl/oslConfig.h"
#include <qapplication.h>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QMetaType>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  Q_INIT_RESOURCE(gui);

  QStringList opts = app.arguments();
  if (opts.contains("--version")) {
    std::cout << "viewer "  << gpsshogi::gpsshogi_revision << "\n\n"
       << gpsshogi::gpsshogi_copyright << "\n";
    osl::OslConfig::healthCheck();
    return 0;
  } else if (opts.contains("--help")) {
    std::cout << 
      "USAGE: viewer [options]\n" <<
      " --version  show version\n" <<
      " --help     show this help message\n";
    return 0;
  }

  qRegisterMetaType<std::vector<int> >("std::vector<int>"); // to use vector in Signal-Slot

  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);

  QTranslator translator;
  translator.load("viewer_" + QLocale::system().name());
  app.installTranslator(&translator);
  MainWindow mainWin;
  mainWin.show();
  osl::OslConfig::setUp();
  return app.exec();
}
