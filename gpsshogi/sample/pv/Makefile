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
LOADLIBES += -lboost_program_options$(BOOST_POSTFIX) -lboost_thread$(BOOST_POSTFIX) -ltokyocabinet -lz -lrt -lpthread -lm -lc $(OSL_HOME)/lib/third_party/$(PROTOBUF)/src/.libs/libprotobuf.a
INCLUDES += -I$(OSL_HOME)/lib/third_party/$(PROTOBUF)/src

SRCS = pv.cc dump-pvs.cc pv-per-iteration.cc iter-hist.cc
PROGRAMS = $(SRCS:.cc=)

all:
	$(MAKE) update-lib
	$(MAKE) $(PROGRAMS)
$(PROGRAMS): $(FILE_OSL_ALL) ../../lib/dbm/libgpsshogidbm.a moves.pb.o

CC = $(CXX)

protoc_middleman: moves.proto
	$(OSL_HOME)/lib/third_party/$(PROTOBUF)/src/protoc --cpp_out=. moves.proto
	@touch protoc_middleman
moves.pb.cc: protoc_middleman
moves.pb.o: moves.pb.cc

update-lib:
	cd ../../lib/dbm; $(MAKE) RELEASE=$(RELEASE)
clean: 
	-rm -f core *.o $(PROGRAMS) nohup.out *~ protoc_middleman
	-rm -rf .deps moves.pb.cc moves.pb.h
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

.PHONY: all update-lib clean distclean
