GPSSHOGI_HOME = ../../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

OTHERFLAGS = -g -pipe $(CPUOPTION)
ifdef DEBUG
CXXOPTFLAGS = -O0
OTHERFLAGS += -DDEBUG
else
CXXOPTFLAGS = $(RELEASE_CXX_OPTFLAGS)
WARNING_FLAGS += $(WARN_INLINE)
ifdef RELEASE
OTHERFLAGS +=  -DNDEBUG
endif
ifdef PROFILE
OTHERFLAGS +=  -DNDEBUG
endif
endif

WARNING_FLAGS += -Wno-sign-compare

CXXFLAGS = $(PROF) $(OTHERFLAGS) $(CXXOPTFLAGS) $(WARNING_FLAGS) $(INCLUDES)
INCLUDES += -I.

all: liblegacycheckmate.a

CC = $(CXX)

OBJS = analyzer/checkTableAnalyzer.o analyzer/disproofTreeTraverser.o \
 analyzer/proofTreeDepth.o analyzer/proofTreeTraverser.o analyzer/recordSet.o \
 analyzer/showAllTree.o analyzer/treeTraverser.o analyzer/treeWriter.o \
 arrayCheckHashTable.o checkHashRecord.o checkmateRecorder.o \
 checkmateSearcher.o checkMove.o checkMoveGenerator.o checkMoveList.o \
 checkMoveListProvider.o checkStack.o disproofPieces.o dominanceTable.o \
 dualCheckmateSearcher.o oracleDisprover.o oraclePool.o oraclePoolLastMove.o \
 oracleProver.o oracleProverLight.o proofPieces.o sameBoardList.o \
 simpleCheckHashTable.o twinList.o twinTable.o visitedCounter.o
SRCS = $(OBJS:.o=.cc)

liblegacycheckmate.a : $(OBJS)
	-rm $@
	$(ARC) $@ $(OBJS)
	-$(RANLIB) $@

clean: 
	-rm -f core $(OBJS) *.o $(PROGRAMS) nohup.out *~
	-rm -rf .deps
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))
