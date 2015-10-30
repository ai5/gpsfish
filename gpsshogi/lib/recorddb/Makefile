GPSSHOGI_HOME = ../..
include $(GPSSHOGI_HOME)/makefile.conf
-include $(GPSSHOGI_HOME)/makefile.local
-include makefile.local

CXXOPTFLAGS ?= -O
OTHERFLAGS = -pipe -Wall -g $(CPUOPTION)
INCLUDES += -I$(OSL_HOME)/lib/third_party/$(PROTOBUF)/src

CXXFLAGS = -DOSL_HOME=\"`pwd`/../..\" $(CXXOPTFLAGS) $(OTHERFLAGS) $(PROF) $(INCLUDES)
all:
	$(MAKE) -C$(GPSSHOGI_HOME)/lib/dbm
	$(MAKE) -C$(GPSSHOGI_HOME)/include/gpsshogi/recorddb
	$(MAKE) libgpsshogirecorddb.a

OBJS = recordDb.o facade.o evalDb.o
SRCS = $(OBJS:.o=.cc)

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

libgpsshogirecorddb.a : $(OBJS)
	$(ARC) $@ $(OBJS)
	-$(RANLIB) $@

clean: 
	$(MAKE) -C$(GPSSHOGI_HOME)/lib/dbm clean
	$(MAKE) -C$(GPSSHOGI_HOME)/include/gpsshogi/recorddb clean
	-rm -f core *.o $(OBJS) $(PROGNAME) nohup.out
	-rm -rf .deps
distclean: clean 
	rm -f *~
