GPSSHOGI_HOME = ../../..

-include makefile.local
include $(GPSSHOGI_HOME)/makefile.conf

INCLUDES += -I../lib
LOADLIBES += -lpthread

PROGRAM_SRCS = stdio-bridge.cc usiserver.cc udpreceiver.cc csagame.cc csaproblem.cc
SRCS = $(PROGRAM_SRCS) 
OBJS = $(patsubst %.cc,%.o,$(SRCS))

CC = $(CXX)
PROGRAMS = $(PROGRAM_SRCS:.cc=)
LIB_TCPCLUSTER = ../lib/libtcpcluster.a

all: 
	$(MAKE) update-lib 
	$(MAKE) $(PROGRAMS)

programs: $(PROGRAMS)
$(PROGRAMS): $(LIB_TCPCLUSTER) $(FILE_OSL_ALL)

update-lib:
	cd ../lib; $(MAKE) RELEASE=$(RELEASE)

clean:
	-rm *.o $(PROGRAMS)
	-rm -rf .deps
	cd ../lib; $(MAKE) clean

light-clean:
	-rm -rf .deps .objs .gch 

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

.PHONY: all clean light-clean run-profile run-profile-mp run-profile-usi update-gpsusione
