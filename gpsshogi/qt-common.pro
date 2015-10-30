CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11 -DOSL_SMP -fno-strict-aliasing -march=native

macx {
  MAKE_MACOSX_DEPLOYMENT_TARGET=10.9
  # Workaround: See http://qt-project.org/forums/viewthread/22263
  QMAKE_CXXFLAGS -= -mmacosx-version-min=10.5
  QMAKE_CXXFLAGS += -mmacosx-version-min=10.9 -Wno-gnu-folding-constant
  QMAKE_LFLAGS += -mmacosx-version-min=10.9 -Wno-gnu-folding-constant
  BOOST_POSTFIX =
  BOOST_POSTFIX_MT = -mt
}

isEmpty(OSL_HOME_QT) {
  OSL_HOME_QT = ../../../osl
}

contains(TEMPLATE, app) {
 OSL_LIB *= lib
 equals(OSL_LIB, lib) {
 QMAKE_LIBDIR += $$OSL_HOME_QT/full/osl $$OSL_HOME_QT/std/osl $$OSL_HOME_QT/core/osl
 LIBS += -losl_full -losl_std -losl_core -lboost_filesystem$$BOOST_POSTFIX -lboost_serialization$$BOOST_POSTFIX -lboost_iostreams$$BOOST_POSTFIX
 } else {
 QMAKE_LIBDIR += $$OSL_HOME_QT/full/osl $$OSL_HOME_QT/std/osl $$OSL_HOME_QT/core/osl
 LIBS += -losl_full -losl_std -losl_core -lboost_filesystem$$BOOST_POSTFIX -lboost_serialization$$BOOST_POSTFIX -lboost_iostreams$$BOOST_POSTFIX
 }

 macx {
   LIBS += -liconv
 }
}

// OSL_MALLOC *= pool
isEmpty(TCMALLOC) {
  TCMALLOC = google-perftools-1.7
}
isEmpty(TCMALLOC_NAME) {
  equals(TCMALLOC, google-perftools-1.7) {
    TCMALLOC_NAME = tcmalloc_minimal
  } else {
    TCMALLOC_NAME = tcmalloc
  }
}
equals(TCMALLOC, google-perftools-1.7) {
  TCMALLOC = $$TCMALLOC/.libs
}
contains(OSL_MALLOC,tbb) {
  DEFINES += USE_TBB_SCALABLE_ALLOCATOR
  contains(TEMPLATE, app) {
    LIBS += -ltbbmalloc
  }
}
contains(OSL_MALLOC,pool) {
  DEFINES += USE_BOOST_POOL_ALLOCATOR1
}
contains(OSL_MALLOC,tcmalloc) {
  contains(TEMPLATE,app) {
       QMAKE_LIBDIR += $$OSL_HOME_QT/lib/third_party/$$TCMALLOC
       LIBS += -l$$TCMALLOC_NAME
  }
}

isEmpty(RPATH) {
  RPATH *= -Wl,-rpath,
}

equals(USE_TOKYO_CABINET,1) {
  DEFINES += USE_TOKYO_CABINET=$$USE_TOKYO_CABINET
}
