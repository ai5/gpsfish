GPSSHOGI_HOME = ../../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

LDFLAGS += -L../osl -L../../std/osl -L../../core/osl
LOADLIBES += -lboost_unit_test_framework $(LDLIBS)
INCLUDES += -I../lib

PROGRAMS = testAll
LIB_MOVEPROB = ../lib/libmoveprob.a

all : 
	$(MAKE) update-lib
	$(MAKE) test-target
test-target: $(PROGRAMS)
update-lib:
	cd ../lib; $(MAKE)

TEST_OBJS = feature.t.o stateInfo.t.o featureSet.t.o moveInfo.t.o
OBJS = testAll.o $(TEST_OBJS)
SRCS = $(patsubst %.o,%.cc,$(OBJS))

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

testAll: $(OBJS) $(LIB_MOVEPROB) $(FILE_OSL_ALL)
	mkdir -p `dirname $(OUTPUTPREFIX)$@` > /dev/null 2>&1
	$(CXX) $(LDFLAGS) -o $(OUTPUTPREFIX)$@ $^ $(LOADLIBES) $(GTEST_LIBES)

light-clean:
	-rm -rf .deps 
clean: light-clean
	-rm -f core *.o $(OBJS) $(PROGRAMS) nohup.out checkmate.log
distclean: clean 
	rm -f *~
