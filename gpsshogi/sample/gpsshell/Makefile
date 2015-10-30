GPSSHOGI_HOME = ../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

programs: gpsshell

SRCS = $(PROGRAM_SRCS) 
OBJS = $(patsubst %.cc,%.o,$(SRCS))

CC = $(CXX)
PROGRAMS = gpsshell

LOADLIBES += -lPocoNet -lPocoFoundation -lreadline -lgsl -lgslcblas -lpthread 
INCLUDES += -I. -I/usr/include/gsl

SRCS = gpsshell.cc board.cc book.cc commands.cc httpclient.cc ignorelist.cc
OBJS = $(patsubst %.cc,%.o,$(SRCS))

all: $(PROGRAMS)
gpsshell: $(OBJS)

CC = $(CXX)

gpsshell : $(FILE_OSL_ALL)

clean:
	-rm *.o $(PROGRAMS)
	-rm -rf .deps

light-clean:
	-rm -rf .deps .objs .gch 

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))
