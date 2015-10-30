GPSSHOGI_HOME = ../../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

INCLUDES += -I../lib
LOADLIBES += -lz

SRCS = pv.cc pvall.cc show-eval.cc cross-validate.cc \
  learn.cc move-rating.cc select-record.cc find-pv.cc \
  one-sided-features.cc find-overfit.cc compare-pv.cc \
  bonanza-learn.cc progressevalstat.cc analyze-eval.cc
PROGRAMS = $(SRCS:.cc=)

LIB_EVAL = ../lib/libeval.a

all: 
	$(MAKE) update-lib 
	$(MAKE) $(PROGRAMS)
$(PROGRAMS): $(LIB_EVAL) $(FILE_OSL_ALL)

CC = $(CXX)

update-lib:
	cd ../lib; $(MAKE) RELEASE=$(RELEASE)

clean: 
	-rm -f core *.o $(PROGRAMS) nohup.out *~
	-rm -rf .deps
	$(MAKE) -C ../lib clean
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

.PHONY: all update-lib clean distclean
