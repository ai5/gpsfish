OSL_HOME = ../..
-include makefile.local
-include $(OSL_HOME)/makefile.local
-include $(OSL_HOME)/makefile.conf

LDFLAGS += -L../osl -L../../core/osl
LOADLIBES += -losl_std -losl_core $(BOOST_LIBS) -lboost_unit_test_framework $(LDLIBS)
INCLUDES += -I.. -I../../core

PROGRAMS = testAll

CHECKMATE_SRC = \
 checkmateIfCapture.t.cc  fixedDepthSearcher.t.cc  proofDisproof.t.cc \
 dfpn.t.cc                oracleAdjust.t.cc        proofNumberTable.t.cc \
 dualDfpn.t.cc            oracleProver.t.cc        proofPieces.t.cc \
 pawnCheckmateMoves.t.cc pawnDropCheckmate.t.cc
MP_SRC = feature.t.cc  featureSet.t.cc  moveInfo.t.cc  stateInfo.t.cc
RATING_SRC = feature/patternLong.t.cc  ratedMoveVector.t.cc \
 feature/threatmate.t.cc   standardFeatureSet.t.cc
RECORD_SRC = kanjiMove.t.cc  kanjiPrint.t.cc  ki2.t.cc  kisen.t.cc  record.t.cc \
 csaRecord.t.cc
SRCS = $(patsubst %.cc,checkmate/%.cc,$(CHECKMATE_SRC)) \
 $(patsubst %.cc,move_probability/%.cc,$(MP_SRC)) \
 $(patsubst %.cc,rating/%.cc,$(RATING_SRC)) \
 $(patsubst %.cc,record/%.cc,$(RECORD_SRC)) \
 centering5x3.t.cc centering3x3.t.cc pathEncoding.t.cc \
 book/miniBoard.t.cc        misc/base64.t.cc         progress/effect5x3d.t.cc \
 book/miniBoardChar50.t.cc  progress/effect5x3.t.cc  progress/ptypeProgress.t.cc \
 moveStack.t.cc moveLogProbVector.t.cc

TEST_OBJS = $(patsubst %.cc,%.o,$(SRCS))
OBJS = testAll.o $(TEST_OBJS)

all : 
	$(MAKE) update-lib
	$(MAKE) test-target
test-target:
	$(MAKE) $(PROGRAMS)
update-lib:
	cd ../../core/osl; $(MAKE)
	cd ../../std/osl; $(MAKE)
	cd ../osl; $(MAKE)

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

testAll: $(OBJS) $(FILE_OSL_STD) $(FILE_OSL_CORE)
	mkdir -p `dirname $@` > /dev/null 2>&1
	$(CXX) $(LDFLAGS) -o $@ $^ $(LOADLIBES) 

%.t: %.t.o testOne.o $(FILE_OSL_STD) $(FILE_OSL_CORE)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LOADLIBES) 

light-clean:
	-rm -rf .deps 
clean: light-clean
	-rm -f core *.o $(OBJS) $(PROGRAMS) 
