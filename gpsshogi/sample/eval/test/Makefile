GPSSHOGI_HOME = ../../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

LOADLIBES += -lboost_unit_test_framework $(LDLIBS) -lz
INCLUDES += -I../lib

PROGRAMS = testAll
LIB_EVAL = ../lib/libeval.a

all : 
	$(MAKE) update-lib
	$(MAKE) test-target
test-target: $(PROGRAMS)
update-lib:
	cd ../lib; $(MAKE)

TEST_OBJS = eval.t.o minorPiece.t.o majorPiece.t.o \
	quiesce.t.o pvFile.t.o l1Ball.t.o loss.t.o
OBJS = testAll.o $(TEST_OBJS)
SRCS = $(patsubst %.o,%.cc,$(OBJS))

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

testAll: $(OBJS) $(LIB_EVAL) $(FILE_OSL_ALL)
	mkdir -p `dirname $(OUTPUTPREFIX)$@` > /dev/null 2>&1
	$(CXX) $(LDFLAGS) -o $@ $^ $(LOADLIBES) 

light-clean:
	-rm -rf .deps 
clean: light-clean
	-rm -f core *.o $(OBJS) $(PROGRAMS) nohup.out checkmate.log
distclean: clean 
	rm -f *~
