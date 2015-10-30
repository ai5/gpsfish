TEMPLATE = app
CONFIG += qt warn_on thread debug
INCLUDEPATH += . ../../../../osl/include  ../../../include
DEPENDPATH += . ../../../../osl/include  ../../../include
isEmpty(GPSSHOGIGUI) {
GPSSHOGIGUI = ../../../lib/gui
}
include(../../../qt-common.pro)
include(../../../qt-common-local.pro)

macx {
  DEFINES += __WORDSIZE=32
  LIBS += -L../../../../osl/lib -losl -lboost_filesystem
} else {
  LIBS += -Wl,-rpath,`pwd`/$$GPSSHOGIGUI -L../../../../osl/lib -losl -lboost_filesystem
}

# FIXME
LIBS -= -L../../../osl/lib -losl
LIBS += -L$$GPSSHOGIGUI -losl_board -lgpsshogigui -lboost_thread -lboost_filesystem -lqdbm

qt_version = $$[QT_VERSION]
qt4 = $$find(qt_version, "^4")
count (qt4, 1) {
  QT += qt3support xml network
}
# Input
HEADERS += mainwindow.h movelist.h openingEditor.h ../editor.h moveHist.h showStatesDialog.h
SOURCES += main.cc mainwindow.cc movelist.cc openingEditor.cc ../editor.cc moveHist.cc showStatesDialog.cc
