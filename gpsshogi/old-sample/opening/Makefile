GPSSHOGI_HOME = ../..
INCLUDES += -I/usr/include/qdbm
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

# type make RELEASE=yes to build optimized programs
# type make DEBUG=true to build programs for debugging

ifdef DEBUG
CXXOPTFLAGS = -O0
OTHERFLAGS = -pipe -Wall $(CXXOPTFLAGS) -g -DDEBUG
else
CXXOPTFLAGS = -O2 -frename-registers -funroll-loops 
OTHERFLAGS = -pipe -Wall $(RELEASE_CXX_OPTFLAGS)
ifdef RELEASE
OTHERFLAGS += -DNDEBUG
endif
endif
LDFLAGS +=  -L$(CXX_HOME)/lib -Wl,-rpath,$(CXX_HOME)/lib -lqdbm

CXXFLAGS = $(PROF) $(INCLUDES) $(OTHERFLAGS)

SRCS = convert.cc
PROGRAMS = $(SRCS:.cc=)

all: convert
convert: convert.o editor.o converter.o
$(PROGRAMS): $(OSL_LIB_DIR)/libosl_board.a

CC = $(CXX)

clean: 
	-rm -f core *.o $(PROGRAMS) nohup.out *~
	-rm -rf .deps
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

