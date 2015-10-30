GPSSHOGI_HOME = ../..
include $(GPSSHOGI_HOME)/makefile.conf
-include $(GPSSHOGI_HOME)/makefile.local
-include makefile.local

CXXOPTFLAGS ?= -O
OTHERFLAGS = -pipe -Wall -g $(CPUOPTION)

CXXFLAGS = -DOSL_HOME=\"`pwd`/../..\" $(CXXOPTFLAGS) $(OTHERFLAGS) $(PROF) $(INCLUDES)
all : libgpsshogidbm.a

OBJS = doNothing.o
ifeq ($(USE_TOKYO_CABINET),1)
OBJS += tokyoCabinet.o
endif
SRCS = $(OBJS:.o=.cc)

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

libgpsshogidbm.a : $(OBJS)
	$(ARC) $@ $(OBJS)
	-$(RANLIB) $@

clean: 
	-rm -f core *.o $(OBJS) $(PROGNAME) nohup.out
	-rm -rf .deps
distclean: clean 
	rm -f *~
