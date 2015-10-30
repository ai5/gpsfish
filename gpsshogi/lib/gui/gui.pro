TEMPLATE = lib
CONFIG -= moc
CONFIG += thread
CONFIG += staticlib
TARGET = gpsshogigui
include(../../qt-common-local.pro)
include(../../qt-common.pro)
INCLUDEPATH += . $$OSL_HOME_QT/full $$OSL_HOME_QT/std $$OSL_HOME_QT/core ../../include
DEPENDPATH  += . $$OSL_HOME_QT/full $$OSL_HOME_QT/std $$OSL_HOME_QT/core ../../include
QMAKE_LIBDIR += $$OSL_HOME_QT/lib 
LIBS += -losl_std
RESOURCES = gui.qrc

# Input
HEADERS += ../../include/gpsshogi/gui/board.h ../../include/gpsshogi/gui/util.h \
 ../../include/gpsshogi/gui/abstractBoard.h \
 ../../include/gpsshogi/gui/editBoard2.h
SOURCES += board.cc util.cc abstractBoard.cc editBoard2.cc
