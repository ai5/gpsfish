GPSSHOGI_HOME = ../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

LOADLIBES = 

ifdef DEBUG
CXXOPTFLAGS += -O
OTHERFLAGS += -pipe -Wall $(CXXOPTFLAGS) -g -DDEBUG
else
CXXOPTFLAGS += -O2 -frename-registers -funroll-loops 
OTHERFLAGS += -pipe -Wall $(CXXOPTFLAGS) $(CPUOPTION) -fomit-frame-pointer -finline-limit=2000
ifdef RELEASE
OTHERFLAGS += -DNDEBUG
endif
endif

CXXFLAGS += $(PROF) $(INCLUDES) $(OTHERFLAGS)

SRCS = show_progress.cc learn.cc show_ptypeposition.cc show_weights.cc count.cc
OBJS = $(SRCS:.cc=.o)
PROGRAMS = $(SRCS:.cc=)
LIBS = $(GPSSHOGI_LIB_DIR)/libgpsshogi.a $(OSL_LIB_DIR)/libosl.a 

CC = $(CXX)

all: $(PROGRAMS)
$(PROGRAMS): $(LIBS)

clean: 
	rm -f core *.o $(PROGRAMS) nohup.out *~
distclean: clean 
	rm -f *~

# for experiments
weights200k-size.txt:
	./learn -s -o $@ -N 200000
weights200k.txt:
	./learn -o $@ -N 200000

weights-ptype-rank200k.txt:
	./learn -g -o $@ -N 200000
weights-ptype-rank200k-size.txt:
	./learn -s -g -o $@ -N 200000

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))
