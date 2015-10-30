GPSSHOGI_HOME = ../..
-include makefile.local
include $(GPSSHOGI_HOME)/makefile.conf

#
LOADLIBES += -lboost_thread$(BOOST_POSTFIX_MT) -lboost_program_options$(BOOST_POSTFIX_MT)

## #gcc
OTHERFLAGS = -pipe $(CPUOPTION)
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

SRCS = analyze-kif.cc annotate.cc
OBJS = 
PROGRAMS = analyze-kif annotate
CC = $(CXX)

all: $(PROGRAMS)
analyze-kif: analyze-kif.o $(OBJS) $(FILE_OSL_ALL)
annotate: annotate.o $(OBJS) $(FILE_OSL_ALL)

clean:
	-rm *.o $(PROGRAMS)
	-rm -rf .deps

light-clean:
	-rm -rf .deps .objs .gch 

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))
