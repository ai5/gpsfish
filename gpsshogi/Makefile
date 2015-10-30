GPSSHOGI_HOME = .
-include makefile.local
include $(GPSSHOGI_HOME)/makefile.conf

all:

viewer: update-revision gui-lib
	-rm sample/viewer/viewer
	cd sample/viewer; env $(QMAKEENV) $(QMAKEPATH)qmake $(QMAKEARG)
	cd sample/viewer; env $(QMAKEENV) $(MAKE) 

viewer-release: update-revision gui-lib
	-rm sample/viewer/viewer
	cd sample/viewer; env $(QMAKEENV) $(QMAKEPATH)qmake $(QMAKEARG) OSL_LIB=release
	cd sample/viewer; env $(QMAKEENV) $(MAKE)

gui-lib:
	-rm lib/gui/libgpsshogigui*
	cd lib/gui; env $(QMAKEENV) $(QMAKEPATH)qmake $(QMAKEARG)
	cd lib/gui; env $(QMAKEENV) $(MAKE)

clean:
	cd lib/gui; $(MAKE) clean
	cd sample/viewer; $(MAKE) clean
	cd bin; $(MAKE) clean
	cd lib; $(MAKE) clean

update-revision:
	cat $(GPSSHOGI_REVISION_H) | sed -e 's/r[0-9][0-9A-Z:]*/r'`svnversion`'/g' > $(GPSSHOGI_REVISION_H).new
	if cmp $(GPSSHOGI_REVISION_H) $(GPSSHOGI_REVISION_H).new > /dev/null; then \
	  rm $(GPSSHOGI_REVISION_H).new; \
	else \
	  mv $(GPSSHOGI_REVISION_H).new $(GPSSHOGI_REVISION_H); \
	fi
