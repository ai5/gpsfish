GPSSHOGI_HOME = ../../gpsshogi
ifdef PROFILE
RELEASE = true
endif
-include makefile.local
include $(GPSSHOGI_HOME)/makefile.conf


CXX_CPP_FLAGS += -I../../osl/std -I../../osl/core
.deps/%.cpp.d: %.cpp
	if [ ! -d `dirname $@` ]; then mkdir -p `dirname $@`; fi; 
	$(GXX) -MM $(CXX_CPP_FLAGS) $< \
		| sed 's/\(.*\.o\)[ :]*/$(subst /,\/,$(patsubst .deps/%.cpp.d,%.o,$@)) $(subst /,\/,$@) : /g' > $@
	grep $< $@ > /dev/null 2>&1 || (rm -f $@; false)

LOADLIBES += -lpthread -lpsapi

GPSFISH_HOME_FLAGS = -DGPSFISH_HOME=\"./\"
OSL_HOME_FLAGS = -DOSL_HOME=\"$(shell dirname `dirname \`pwd\``)/osl\"
CXXFLAGS += -msse -DGPSFISH -DPROMOTE_AS_CAPTURE -DOLD_LOCKS $(GPSFISH_HOME_FLAGS) $(OSL_HOME_FLAGS)

SRCS = benchmark.cpp misc.cpp timeman.cpp evaluate.cpp move.cpp position.cpp tt.cpp main.cpp movegen.cpp search.cpp uci.cpp book.cpp movepick.cpp thread.cpp ucioption.cpp
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
CC = $(CXX)

all: 
	$(MAKE) programs RELEASE=t

programs : gpsfish

gpsfish: $(OBJS) $(FILE_OSL_ALL)
	$(CXX) -o $@ $(CXXFLAGS) $(PROFILE_USE) $(LDFLAGS) $(OBJS) $(FILE_OSL_ALL) $(LDLIBS) $(LOADLIBES) -lbz2

gpsfishoneprof: mainone.cc
	$(CXX) -o $@ $(CXXFLAGS) $(PROFILE_GENERATE) $(LDFLAGS) $< $(LDLIBS) $(filter-out -losl_%,$(LOADLIBES)) -lbz2

gpsfishone: mainone.cc mainone.gcda gpsfishoneprof
	$(CXX) -o $@ $(CXXFLAGS) $(PROFILE_USE) $(LDFLAGS) $< $(LDLIBS) $(filter-out -losl_%,$(LOADLIBES)) -lbz2

update-gpsfishone:
	-rm gpsfishone gpsfishoneprof mainone.gcda
	$(MAKE) gpsfishoneprof RELEASE=t
	$(MAKE) run-profile
	$(MAKE) gpsfishone RELEASE=t

run-profile: gpsfishoneprof
	./gpsfishoneprof bench 32 1 12 default depth

clean:
	-rm *.o $(PROGRAM)
	-rm -rf .deps

light-clean:
	-rm -rf .deps .objs .gch 

-include $(patsubst %.cpp,.deps/%.cpp.d,$(SRCS))

.PHONY: all clean light-clean run-profile update-gpsusione
