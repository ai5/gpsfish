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

INCLUDES += -I../eval/lib
LIB_EVAL = ../eval/lib/libeval.a
CXXFLAGS = $(PROF) $(OTHERFLAGS) $(CXXOPTFLAGS) $(WARNING_FLAGS) $(INCLUDES)
LOADLIBES += -lboost_thread$(BOOST_POSTFIX) -lboost_program_options$(BOOST_POSTFIX)

SRCS = win-ratio.cc kisen-result.cc values.cc show-states.cc values2.cc
PROGRAMS = $(SRCS:.cc=)

all:
	cd ../eval/lib; $(MAKE)  RELEASE=$(RELEASE)
	$(MAKE) $(PROGRAMS)
$(PROGRAMS): $(LIB_EVAL) $(FILE_OSL_ALL)

CC = $(CXX)

clean: 
	-rm -f core *.o $(PROGRAMS) nohup.out *~
	-rm -rf .deps
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

