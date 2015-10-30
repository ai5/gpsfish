GPSSHOGI_HOME = ../../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

OTHERFLAGS = -g -pipe $(CPUOPTION)
ifdef DEBUG
CXXOPTFLAGS = -O0
OTHERFLAGS += -DDEBUG
else
CXXOPTFLAGS = $(RELEASE_CXX_OPTFLAGS)
WARNING_FLAGS += $(WARN_INLINE)
ifdef RELEASE
OTHERFLAGS +=  -DNDEBUG
endif
ifdef PROFILE
OTHERFLAGS +=  -DNDEBUG
endif
endif

WARNING_FLAGS += -Wno-sign-compare

CXXFLAGS = $(PROF) $(OTHERFLAGS) $(CXXOPTFLAGS) $(WARNING_FLAGS) $(INCLUDES)
INCLUDES += -I.

all: libmoveprob.a

CC = $(CXX)

OBJS = feature.o featureSet.o stateInfo.o moveInfo.o
SRCS = $(OBJS:.o=.cc)

libmoveprob.a : $(OBJS)
	-rm $@
	$(ARC) $@ $(OBJS)
	-$(RANLIB) $@

clean: 
	-rm -f core $(OBJS) *.o $(PROGRAMS) nohup.out *~
	-rm -rf .deps
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))
