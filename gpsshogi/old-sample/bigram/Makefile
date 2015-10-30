GPSSHOGI_HOME = ../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

OTHERFLAGS = -pipe $(CPUOPTION)
ifdef DEBUG
CXXOPTFLAGS ?= -O0
OTHERFLAGS += -g -DDEBUG
else
CXXOPTFLAGS = $(RELEASE_CXX_OPTFLAGS)
WARNING_FLAGS += $(WARN_INLINE)
ifdef RELEASE
OTHERFLAGS +=  -DNDEBUG
endif
endif

CXXFLAGS = $(PROF) $(OTHERFLAGS) $(CXXOPTFLAGS) $(WARNING_FLAGS) $(INCLUDES)
LOADLIBES += -lboost_program_options$(BOOST_POSTFIX) -lboost_thread$(BOOST_POSTFIX)

SRCS = show.cc
PROGRAMS = $(SRCS:.cc=)

all: $(PROGRAMS)
$(PROGRAMS): $(FILE_OSL_ALL)

CC = $(CXX)

clean: 
	-rm -f core *.o $(PROGRAMS) nohup.out *~
	-rm -rf .deps
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

