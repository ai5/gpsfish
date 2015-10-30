TEMPLATE = app
CONFIG += qt thread warn_on

include(../../qt-common-local.pro)
include(../../qt-common.pro)

INCLUDEPATH += . $$OSL_HOME_QT/full $$OSL_HOME_QT/std $$OSL_HOME_QT/core ../../include
DEPENDPATH  += . $$OSL_HOME_QT/full $$OSL_HOME_QT/std $$OSL_HOME_QT/core ../../include
isEmpty(GPSSHOGIGUI) {
GPSSHOGIGUI = ../../lib/gui
}
macx {
} else {
  LIBS += $$RPATH`pwd`/$$GPSSHOGIGUI
}

QMAKE_LIBDIR += $$GPSSHOGIGUI 
LIBS = -lgpsshogigui $$LIBS -lboost_thread$$BOOST_POSTFIX_MT -lboost_system$$BOOST_POSTFIX_MT -lboost_date_time$$BOOST_POSTFIX_MT 
LIBS += $$ADDITIONAL_LIBS

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += xml network
RESOURCES = viewer.qrc
# FIXME
FORMS += searchdialog4.ui kisendialog4.ui networkdialog4.ui \
    progressdebugdialog.ui
HEADERS += httpwindow.h itemDelegate.h \
    progressDebug.h
SOURCES += httpwindow.cpp itemDelegate.cc \
    progressDebug.cc

# Input
HEADERS += mainwindow.h viewer.h moveList.h analysisViewer.h checkmateViewer.h \
	tabchild.h kifuViewer.h boardTabChild.h \
	moveTree.h boardAndListTabChild.h searchdialogimpl.h \
        moveGeneratorDialog.h quiescenceViewer.h \
	quiescenceTree.h searchMonitor.h copyLabel.h \
        kifuAnalyzer.h networkViewer.h dualEvaluationDialog.h \
        network/networkClient.h network/networkBoard.h\
        kifuFile.h boardEditor2.h evalDebug.h clickableLabel.h \
        pvModel.h kisenModel.h
SOURCES += main.cc mainwindow.cc viewer.cc moveList.cc analysisViewer.cc \
	checkmateViewer.cc kifuViewer.cc boardTabChild.cc \
	moveTree.cc boardAndListTabChild.cc searchdialogimpl.cc \
	moveGeneratorDialog.cc quiescenceViewer.cc \
	quiescenceTree.cc searchMonitor.cc copyLabel.cc \
        kifuAnalyzer.cc networkViewer.cc dualEvaluationDialog.cc \
        network/networkClient.cc network/networkBoard.cc\
        kifuFile.cc boardEditor2.cc evalDebug.cc clickableLabel.cc\
        pvModel.cc kisenModel.cc
TRANSLATIONS = viewer_ja.ts

