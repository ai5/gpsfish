GPSSHOGI_HOME = ../../..

-include makefile.local
include $(GPSSHOGI_HOME)/makefile.conf

INCLUDES += -I.

all: libtcpcluster.a

CC = $(CXX)

OBJS = slaveManager.o coordinator.o usiSlave.o usiServer.o usiStatus.o \
  searchTree.o searchNode.o logging.o upStream.o csaServer.o \
  inaniwa.o
SRCS = $(OBJS:.o=.cc)

libtcpcluster.a : $(OBJS)
	-rm $@
	$(ARC) $@ $(OBJS)
	-$(RANLIB) $@

clean: 
	-rm -f core $(OBJS) *.o *.a nohup.out *~
	-rm -rf .deps
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))
