# Programs in this directory requires Tokyo Cabinet to be installed.
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
LOADLIBES += -lboost_program_options$(BOOST_POSTFIX) -lboost_thread$(BOOST_POSTFIX) -ltbb -ltokyocabinet -lz -lrt -lpthread -lm -lc $(OSL_HOME)/lib/third_party/$(PROTOBUF)/src/.libs/libprotobuf.a
INCLUDES += -I$(OSL_HOME)/lib/third_party/$(PROTOBUF)/src

SRCS = add-record.cc query.cc filter.cc eval.cc
PROGRAMS = $(SRCS:.cc=)

all:
	$(MAKE) update-lib
	$(MAKE) $(PROGRAMS)
$(PROGRAMS): $(GPSSHOGI_HOME)/lib/recorddb/libgpsshogirecorddb.a $(GPSSHOGI_HOME)/lib/dbm/libgpsshogidbm.a $(FILE_OSL_ALL)

CC = $(CXX)

update-lib:
	cd ../../lib/dbm; $(MAKE) RELEASE=$(RELEASE)
clean: 
	-rm -f core *.o $(PROGRAMS) nohup.out *~ 
	-rm -rf .deps *.pb.cc *.pb.h
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

.PHONY: all update-lib clean distclean
