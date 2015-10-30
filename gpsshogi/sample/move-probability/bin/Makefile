GPSSHOGI_HOME = ../../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

INCLUDES += -I../lib
LOADLIBES += 

SRCS = sgd.cc validate.cc
PROGRAMS = $(SRCS:.cc=)

LIB_MOVEPROB = ../lib/libmoveprob.a

all: 
	$(MAKE) update-lib 
	$(MAKE) $(PROGRAMS)
$(PROGRAMS): $(LIB_MOVEPROB) $(FILE_OSL_ALL)

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
