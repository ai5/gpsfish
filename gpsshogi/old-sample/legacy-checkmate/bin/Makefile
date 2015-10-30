GPSSHOGI_HOME = ../../..
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

ifdef PROFILE
PROF = $(PROFILE_FLAGS)
LDFLAGS += $(PROF)
endif

INCLUDES += -I../lib
CXXFLAGS = $(PROF) $(OTHERFLAGS) $(CXXOPTFLAGS) $(WARNING_FLAGS) $(INCLUDES)
LOADLIBES += -lboost_program_options$(BOOST_POSTFIX) -lboost_thread$(BOOST_POSTFIX_MT) -lz

SRCS = checkstat.cc
PROGRAMS = $(SRCS:.cc=)

LIB_LOCAL = ../lib/liblegacycheckmate.a

all: 
	$(MAKE) update-lib 
	$(MAKE) $(PROGRAMS)
$(PROGRAMS): $(LIB_LOCAL) $(FILE_OSL_ALL)

CC = $(CXX)

update-lib:
	cd ../lib; $(MAKE) RELEASE=$(RELEASE)

clean: 
	-rm -f core *.o $(PROGRAMS) nohup.out *~
	-rm -rf .deps
	$(MAKE) -C ../lib clean
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

.PHONY: all update-lib clean distclean
