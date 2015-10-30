GPSSHOGI_HOME = ../..
include $(GPSSHOGI_HOME)/makefile.conf
-include $(GPSSHOGI_HOME)/makefile.local
-include makefile.local

ifdef PROFILE
RELEASE = true
endif

all : libgpsshogiredis.a

ifdef PROFILE
PROF = $(PROFILE_FLAGS)
LDFLAGS += $(PROF)
endif

LOADLIBES += -lhiredis -lglog

OSL_HOME_FLAGS = -DOSL_HOME=\"$(shell dirname `dirname \`pwd\``)/osl\"
## #gcc
OTHERFLAGS = -pipe -Wall $(CPUOPTION)
ifdef DEBUG
CXXOPTFLAGS = -O
OTHERFLAGS += -g -DDEBUG 
else
CXXOPTFLAGS = $(RELEASE_CXX_OPTFLAGS)
WARNING_FLAGS += $(WARN_INLINE)
ifdef RELEASE
OTHERFLAGS +=  -DNDEBUG
endif
ifndef PROF
CXXOPTFLAGS += -fomit-frame-pointer
endif
endif

CXXFLAGS = $(PROF) $(OTHERFLAGS) $(CXXOPTFLAGS) $(WARNING_FLAGS) $(INCLUDES)
OBJS = redis.o searchResult.o
SRCS = $(OBJS:.o=.cc)

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

libgpsshogiredis.a : $(OBJS)
	$(ARC) $@ $(OBJS)
	-$(RANLIB) $@

clean: 
	-rm -f core *.o $(OBJS) *.a
	-rm -rf .deps
distclean: clean 
	rm -f *~

.PHONY: all clean distclean
